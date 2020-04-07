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
/// \file   PID.cxx
/// \author Jens Wiechula
///

// root includes
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>

// O2 includes
#include "Framework/DataRefUtils.h"
#include "Framework/AlgorithmSpec.h"
#include "Framework/ServiceRegistry.h"
#include "Framework/runDataProcessing.h"
#include <Monitoring/Monitoring.h>
#include "Framework/ControlService.h"
#include "Framework/CallbackService.h"
#include "Framework/Logger.h"

#include "Framework/ProcessingContext.h"
#include "Framework/ControlService.h"
#include "DataFormatsTPC/TrackTPC.h"
#include "TPCQC/Helpers.h"

// QC includes
#include "QualityControl/QcInfoLogger.h"
#include "TPC/PID.h"

namespace o2::quality_control_modules::tpc
{

PID::PID() : TaskInterface() {}

PID::~PID()
{
}

void PID::initialize(o2::framework::InitContext& /*ctx*/)
{
  QcInfoLogger::GetInstance() << "initialize TPC PID QC task" << AliceO2::InfoLogger::InfoLogger::endm;

  mQCPID.initializeHistograms();
  //o2::tpc::qc::helpers::setStyleHistogram1D(mQCPID.getHistograms1D());
  o2::tpc::qc::helpers::setStyleHistogram2D(mQCPID.getHistograms2D());

  for (auto& hist : mQCPID.getHistograms1D()) {
    getObjectsManager()->startPublishing(&hist);
    getObjectsManager()->addMetadata(hist.GetName(), "custom", "34");
  }

  for (auto& hist : mQCPID.getHistograms2D()) {
    getObjectsManager()->startPublishing(&hist);
    getObjectsManager()->addMetadata(hist.GetName(), "custom", "43");
  }
}

void PID::startOfActivity(Activity& /*activity*/)
{
  QcInfoLogger::GetInstance() << "startOfActivity" << AliceO2::InfoLogger::InfoLogger::endm;
  mQCPID.resetHistograms();
}

void PID::startOfCycle()
{
  QcInfoLogger::GetInstance() << "startOfCycle" << AliceO2::InfoLogger::InfoLogger::endm;
}

void PID::monitorData(o2::framework::ProcessingContext& ctx)
{
  using TrackType = std::vector<o2::tpc::TrackTPC>;
  auto tracks = ctx.inputs().get<TrackType>("inputTracks");
  QcInfoLogger::GetInstance() << "monitorData: " << tracks.size() << AliceO2::InfoLogger::InfoLogger::endm;

  for (auto const& track : tracks) {
    mQCPID.processTrack(track);
    //const auto p = track.getP();
    //const auto dEdx = track.getdEdx().dEdxTotTPC;
    //printf("p: dEdx = %.2f: %.2f\n", p, dEdx);
  }
  //ctx.services().get<o2::framework::ControlService>().readyToQuit(o2::framework::QuitRequest::All);
  //ctx.services().get<o2::framework::ControlService>().endOfStream();
  //ctx.services().get<o2::framework::ControlService>().readyToQuit(o2::framework::QuitRequest::Me);
}

void PID::endOfCycle()
{
  QcInfoLogger::GetInstance() << "endOfCycle" << AliceO2::InfoLogger::InfoLogger::endm;
}

void PID::endOfActivity(Activity& /*activity*/)
{
  QcInfoLogger::GetInstance() << "endOfActivity" << AliceO2::InfoLogger::InfoLogger::endm;
}

void PID::reset()
{
  // clean all the monitor objects here

  QcInfoLogger::GetInstance() << "Resetting the histogram" << AliceO2::InfoLogger::InfoLogger::endm;
  mQCPID.resetHistograms();
}

} // namespace o2::quality_control_modules::tpc
