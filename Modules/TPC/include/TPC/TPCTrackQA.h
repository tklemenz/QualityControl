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
/// \file   TPCTrackQA.h
/// \author Thomas Klemenz
///

#ifndef QC_MODULE_TPC_TPCTrackQA_H
#define QC_MODULE_TPC_TPCTrackQA_H

// O2 includes
#include "TPCQC/TPCTrackQA.h"

// QC includes
#include "QualityControl/TaskInterface.h"

class TH1F;

using namespace o2::quality_control::core;

namespace o2::quality_control_modules::tpc
{

/// \brief Example Quality Control DPL Task
/// It is final because there is no reason to derive from it. Just remove it if needed.
/// \author Barthelemy von Haller
/// \author Piotr Konopka
class TPCTrackQA /*final*/ : public TaskInterface // todo add back the "final" when doxygen is fixed
{
 public:
  /// \brief Constructor
  TPCTrackQA();
  /// Destructor
  ~TPCTrackQA() override;

  // Definition of the methods for the template method pattern
  void initialize(o2::framework::InitContext& ctx) override;
  void startOfActivity(Activity& activity) override;
  void startOfCycle() override;
  void monitorData(o2::framework::ProcessingContext& ctx) override;
  void endOfCycle() override;
  void endOfActivity(Activity& activity) override;
  void reset() override;

 private:
  o2::tpc::qc::TPCTrackQA mQCTPCTrackQA{};
};

} // namespace o2::quality_control_modules::tpc

#endif // QC_MODULE_TPC_PID_H
