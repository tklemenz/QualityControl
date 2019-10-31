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
/// \file   TPCTrackQA.cxx
/// \author Thomas Klemenz
///

// root includes
#include <TCanvas.h>
#include <TH1.h>

// O2 includes
#include "Framework/ProcessingContext.h"

// QC includes
#include "QualityControl/QcInfoLogger.h"
#include "TPC/TPCTrackQA.h"

namespace o2::quality_control_modules::tpc
{

TPCTrackQA::TPCTrackQA() : TaskInterface() {}

TPCTrackQA::~TPCTrackQA()
{
}

void TPCTrackQA::initialize(o2::framework::InitContext& /*ctx*/)
{
  QcInfoLogger::GetInstance() << "initialize TPC TPCTrackQA QC task" << AliceO2::InfoLogger::InfoLogger::endm;

  mQCTPCTrackQA.initializeHistograms();
  mQCTPCTrackQA.setNiceStyle();

  for (auto& hist : mQCTPCTrackQA.getHistograms1D()) {
    getObjectsManager()->startPublishing(&hist);
    getObjectsManager()->addMetadata(hist.GetName(), "custom", "41");
  }
  for (auto& hist : mQCTPCTrackQA.getHistograms2D()) {
    getObjectsManager()->startPublishing(&hist);
    getObjectsManager()->addMetadata(hist.GetName(), "custom", "43");
  }
}

void TPCTrackQA::startOfActivity(Activity& /*activity*/)
{
  QcInfoLogger::GetInstance() << "startOfActivity" << AliceO2::InfoLogger::InfoLogger::endm;
  mQCTPCTrackQA.resetHistograms();
}

void TPCTrackQA::startOfCycle()
{
  QcInfoLogger::GetInstance() << "startOfCycle" << AliceO2::InfoLogger::InfoLogger::endm;
}

void TPCTrackQA::monitorData(o2::framework::ProcessingContext& ctx)
{
  using TrackType = std::vector<o2::tpc::TrackTPC>;
  auto tracks = ctx.inputs().get<TrackType>("tpc-sampled-tracks");

  QcInfoLogger::GetInstance() << "monitorData: " << tracks.size() << AliceO2::InfoLogger::InfoLogger::endm;

  for (auto const& track : tracks) {
    mQCTPCTrackQA.processTrack(track);
  }
}

void TPCTrackQA::endOfCycle()
{
  QcInfoLogger::GetInstance() << "endOfCycle" << AliceO2::InfoLogger::InfoLogger::endm;
}

void TPCTrackQA::endOfActivity(Activity& /*activity*/)
{
  QcInfoLogger::GetInstance() << "endOfActivity" << AliceO2::InfoLogger::InfoLogger::endm;
}

void TPCTrackQA::reset()
{
  // clean all the monitor objects here

  QcInfoLogger::GetInstance() << "Resetting the histogram" << AliceO2::InfoLogger::InfoLogger::endm;
  mQCTPCTrackQA.resetHistograms();
}

} // namespace o2::quality_control_modules::tpc
