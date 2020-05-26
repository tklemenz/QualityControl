// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file   Clusters.cxx
/// \author Jens Wiechula
///

#include "Framework/PartRef.h"
#include "Framework/WorkflowSpec.h" // o2::framework::mergeInputs
#include "Framework/DataRefUtils.h"
#include "Framework/DataSpecUtils.h"
#include "Framework/ControlService.h"
#include "Framework/ConfigParamRegistry.h"
#include "DataFormatsTPC/TPCSectorHeader.h"

#include <bitset>

using namespace o2::framework;
using namespace o2::header;
using namespace o2::tpc;




// root includes
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>

// O2 includes
#include "Framework/ProcessingContext.h"
#include "DataFormatsTPC/Defs.h"
#include "TPCQC/Helpers.h"
#include "TPCBase/Painter.h"
#include "DataFormatsTPC/ClusterNative.h"
#include "DataFormatsTPC/ClusterNativeHelper.h"
#include "SimulationDataFormat/MCTruthContainer.h"
#include "DataFormatsTPC/Constants.h"

// QC includes
#include "QualityControl/QcInfoLogger.h"
#include "TPC/Clusters.h"







namespace o2::quality_control_modules::tpc
{

Clusters::Clusters() : TaskInterface() {}

Clusters::~Clusters()
{
}

void Clusters::initialize(o2::framework::InitContext& /*ctx*/)
{
  QcInfoLogger::GetInstance() << "initialize TPC Clusters QC task" << AliceO2::InfoLogger::InfoLogger::endm;

  o2::tpc::Side ASide{o2::tpc::Side::A};
  o2::tpc::Side CSide{o2::tpc::Side::C};

  auto clusAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getNClusters(), ASide));
  auto clusCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getNClusters(), CSide));
  auto qMaxAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getQMax(), ASide));
  auto qMaxCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getQMax(), CSide));
  auto qTotAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getQTot(), ASide));
  auto qTotCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getQTot(), CSide));
  auto sigmaTimeAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaTime(), ASide));
  auto sigmaTimeCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaTime(), CSide));
  auto sigmaPadAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaPad(), ASide));
  auto sigmaPadCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaPad(), CSide));
  auto timeBinAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getTimeBin(), ASide));
  auto timeBinCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getTimeBin(), CSide));

  mHistoVector.emplace_back(*clusAHisto);
  mHistoVector.emplace_back(*clusCHisto);
  mHistoVector.emplace_back(*qMaxAHisto);
  mHistoVector.emplace_back(*qMaxCHisto);
  mHistoVector.emplace_back(*qTotAHisto);
  mHistoVector.emplace_back(*qTotCHisto);
  mHistoVector.emplace_back(*sigmaTimeAHisto);
  mHistoVector.emplace_back(*sigmaTimeCHisto);
  mHistoVector.emplace_back(*sigmaPadAHisto);
  mHistoVector.emplace_back(*sigmaPadCHisto);
  mHistoVector.emplace_back(*timeBinAHisto);
  mHistoVector.emplace_back(*timeBinCHisto);

  delete(clusAHisto);
  delete(clusCHisto);
  delete(qMaxAHisto);
  delete(qMaxCHisto);
  delete(qTotAHisto);
  delete(qTotCHisto);
  delete(sigmaTimeAHisto);
  delete(sigmaTimeCHisto);
  delete(sigmaPadAHisto);
  delete(sigmaPadCHisto);
  delete(timeBinAHisto);
  delete(timeBinCHisto);

  for (auto& hist : mHistoVector) {
    getObjectsManager()->startPublishing(&hist);
    getObjectsManager()->addMetadata(hist.GetName(), "custom", "34");
  }

}

void Clusters::startOfActivity(Activity& /*activity*/)
{
  QcInfoLogger::GetInstance() << "startOfActivity" << AliceO2::InfoLogger::InfoLogger::endm;
  //mQCClusters.resetHistograms();
}

void Clusters::startOfCycle()
{
  QcInfoLogger::GetInstance() << "startOfCycle" << AliceO2::InfoLogger::InfoLogger::endm;
}

void Clusters::monitorData(o2::framework::ProcessingContext& ctx)
{

  o2::tpc::Side ASide{o2::tpc::Side::A};
  o2::tpc::Side CSide{o2::tpc::Side::C};

  constexpr static size_t NSectors = o2::tpc::Sector::MAXSECTOR;
  std::array<std::vector<MCLabelContainer>, NSectors> mcInputs;
  std::bitset<NSectors> validInputs = 0;
  int operation = 0;
  std::vector<int> inputIds(36); 
  std::iota(inputIds.begin(),inputIds.end(),0);
  std::map<int, DataRef> datarefs;
  for (auto const& inputId : inputIds) {
    std::string inputLabel = "input" + std::to_string(inputId);
    auto ref = ctx.inputs().get(inputLabel);
    auto const* sectorHeader = DataRefUtils::getHeader<o2::tpc::TPCSectorHeader*>(ref);
    if (sectorHeader == nullptr) {
      // FIXME: think about error policy
      LOG(ERROR) << "sector header missing on header stack";
      return;
    }
    const int& sector = sectorHeader->sector;
    if (sector < 0) {
      if (operation < 0 && operation != sector) {
        // we expect the same operation on all inputs
        LOG(ERROR) << "inconsistent lane operation, got " << sector << ", expecting " << operation;
      } else if (operation == 0) {
        // store the operation
        operation = sector;
      }
      continue;
    }
    if (validInputs.test(sector)) {
      // have already data for this sector, this should not happen in the current
      // sequential implementation, for parallel path merged at the tracker stage
      // multiple buffers need to be handled
      throw std::runtime_error("can only have one cluster data set per sector");
    }
    //activeSectors |= sectorHeader->activeSectors;
    validInputs.set(sector);
    datarefs[sector] = ref;
  }



  std::array<gsl::span<const char>, o2::tpc::Sector::MAXSECTOR> inputs;
  auto inputStatus = validInputs;
  for (auto const& refentry : datarefs) {
    auto& sector = refentry.first;
    auto& ref = refentry.second;
    inputs[sector] = gsl::span(ref.payload, DataRefUtils::getPayloadSize(ref));
    inputStatus.reset(sector);
    //printInputLog(ref, "received", sector);
  }

  ClusterNativeAccess clusterIndex;
  std::unique_ptr<ClusterNative[]> clusterBuffer;
  MCLabelContainer clustersMCBuffer;
  memset(&clusterIndex, 0, sizeof(clusterIndex));
  ClusterNativeHelper::Reader::fillIndex(clusterIndex, clusterBuffer, clustersMCBuffer, 
                                         inputs, mcInputs, [&validInputs](auto& index) { return validInputs.test(index); });
        
  for (int isector = 0; isector < o2::tpc::Constants::MAXSECTOR; ++isector) {
    for (int irow = 0; irow < o2::tpc::Constants::MAXGLOBALPADROW; ++irow) {
      const int nClusters = clusterIndex.nClusters[isector][irow];
      for (int icl = 0; icl < nClusters; ++icl) {
        const auto& cl = *(clusterIndex.clusters[isector][irow] + icl);
        mQCClusters.processCluster(cl, o2::tpc::Sector(isector), irow);
      }
    }
  }

  mQCClusters.analyse();

  auto clusAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getNClusters(), ASide));
  auto clusCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getNClusters(), CSide));
  auto qMaxAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getQMax(), ASide));
  auto qMaxCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getQMax(), CSide));
  auto qTotAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getQTot(), ASide));
  auto qTotCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getQTot(), CSide));
  auto sigmaTimeAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaTime(), ASide));
  auto sigmaTimeCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaTime(), CSide));
  auto sigmaPadAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaPad(), ASide));
  auto sigmaPadCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaPad(), CSide));
  auto timeBinAHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getTimeBin(), ASide));
  auto timeBinCHisto = static_cast<TH2F*>(o2::tpc::painter::getHistogram2D(mQCClusters.getTimeBin(), CSide));

  mHistoVector[0] = *clusAHisto;
  mHistoVector[1] = *clusCHisto;
  mHistoVector[2] = *qMaxAHisto;
  mHistoVector[3] = *qMaxCHisto;
  mHistoVector[4] = *qTotAHisto;
  mHistoVector[5] = *qTotCHisto;
  mHistoVector[6] = *sigmaTimeAHisto;
  mHistoVector[7] = *sigmaTimeCHisto;
  mHistoVector[8] = *sigmaPadAHisto;
  mHistoVector[9] = *sigmaPadCHisto;
  mHistoVector[10] = *timeBinAHisto;
  mHistoVector[11] = *timeBinCHisto;

  delete(clusAHisto);
  delete(clusCHisto);
  delete(qMaxAHisto);
  delete(qMaxCHisto);
  delete(qTotAHisto);
  delete(qTotCHisto);
  delete(sigmaTimeAHisto);
  delete(sigmaTimeCHisto);
  delete(sigmaPadAHisto);
  delete(sigmaPadCHisto);
  delete(timeBinAHisto);
  delete(timeBinCHisto);
}

void Clusters::endOfCycle()
{
  QcInfoLogger::GetInstance() << "endOfCycle" << AliceO2::InfoLogger::InfoLogger::endm;
}

void Clusters::endOfActivity(Activity& /*activity*/)
{
  QcInfoLogger::GetInstance() << "endOfActivity" << AliceO2::InfoLogger::InfoLogger::endm;
}

void Clusters::reset()
{
  // clean all the monitor objects here

  QcInfoLogger::GetInstance() << "Resetting the histogram" << AliceO2::InfoLogger::InfoLogger::endm;
  //mQCClusters.resetHistograms();
}

} // namespace o2::quality_control_modules::tpc
