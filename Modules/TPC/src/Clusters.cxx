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

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getNClusters(), ASide));
  getObjectsManager()->addMetadata(mQCClusters.getNClusters().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getNClusters(), CSide));
  getObjectsManager()->addMetadata(mQCClusters.getNClusters().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getQMax(), ASide));
  getObjectsManager()->addMetadata(mQCClusters.getQMax().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getQMax(), CSide));
  getObjectsManager()->addMetadata(mQCClusters.getQMax().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getQTot(), ASide));
  getObjectsManager()->addMetadata(mQCClusters.getQTot().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getQTot(), CSide));
  getObjectsManager()->addMetadata(mQCClusters.getQTot().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaTime(), ASide));
  getObjectsManager()->addMetadata(mQCClusters.getSigmaTime().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaTime(), CSide));
  getObjectsManager()->addMetadata(mQCClusters.getSigmaTime().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaPad(), ASide));
  getObjectsManager()->addMetadata(mQCClusters.getSigmaPad().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getSigmaPad(), CSide));
  getObjectsManager()->addMetadata(mQCClusters.getSigmaPad().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getTimeBin(), ASide));
  getObjectsManager()->addMetadata(mQCClusters.getTimeBin().getName(), "custom", "43");

  getObjectsManager()->startPublishing(o2::tpc::painter::getHistogram2D(mQCClusters.getTimeBin(), CSide));
  getObjectsManager()->addMetadata(mQCClusters.getTimeBin().getName(), "custom", "43");
  
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
  constexpr static size_t NSectors = o2::tpc::Sector::MAXSECTOR;
  std::array<std::vector<MCLabelContainer>, NSectors> mcInputs;
  std::bitset<NSectors> validInputs = 0;
  int operation = 0;
  std::vector<int> inputIds(36); 
  std::iota(inputIds.begin(),inputIds.end(),0);                                               // inputIds is input of getCATrackerSpec, std::vector<int> const& inputIds
                                                                            // which is laneConfiguration in RecoWorkflow.cxx
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






  /*using ClusterType = std::vector<o2::tpc::ClusterNative>;
  auto clusters = ctx.inputs().get<ClusterType>("inputClusters");
  QcInfoLogger::GetInstance() << "monitorData " << AliceO2::InfoLogger::InfoLogger::endm;

  o2::tpc::ClusterNativeAccess clusterIndex;
  std::unique_ptr<o2::tpc::ClusterNative[]> clusterBuffer;
  o2::tpc::MCLabelContainer clustersMCBuffer;
  memset(&clusterIndex, 0, sizeof(clusterIndex));
  o2::tpc::ClusterNativeHelper::Reader::fillIndex(clusterIndex, clusterBuffer, clustersMCBuffer);

  for (int isector = 0; isector < o2::tpc::Constants::MAXSECTOR; ++isector) {
    for (int irow = 0; irow < o2::tpc::Constants::MAXGLOBALPADROW; ++irow) {
      const int nClusters = clusterIndex.nClusters[isector][irow];
      for (int icl = 0; icl < nClusters; ++icl) {
        const auto& cl = *(clusterIndex.clusters[isector][irow] + icl);
        mQCClusters.processCluster(cl, o2::tpc::Sector(isector), irow);
      }
    }
  }

  mQCClusters.analyse();*/

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
