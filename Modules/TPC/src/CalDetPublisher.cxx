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
/// \file   CalDetPublisher.cxx
/// \author Thomas Klemenz
///

// O2 includes
#include "TPCBase/Painter.h"
#include "TPCBase/CDBInterface.h"

// QC includes
#include "QualityControl/QcInfoLogger.h"
#include "TPC/CalDetPublisher.h"
#include "TPC/Utility.h"

//root includes
#include "TCanvas.h"

#include <fmt/format.h>

using namespace o2::quality_control::postprocessing;

namespace o2::quality_control_modules::tpc
{

const std::unordered_map<std::string, outputType> OutputMap{
  { "Pedestal", outputType::Pedestal },
  { "Noise", outputType::Noise }
};

void CalDetPublisher::configure(std::string name, const boost::property_tree::ptree& config)
{
  for (const auto& output : config.get_child("qc.postprocessing." + name + ".outputList")) {
    try {
      OutputMap.at(output.second.data());
      mOutputList.emplace_back(output.second.data());
    } catch (std::out_of_range&) {
      throw std::invalid_argument(std::string("Invalid output CalDet object specified in config: ") + output.second.data());
    }
  }

  for (const auto& timestamp : config.get_child("qc.postprocessing." + name + ".timestamps")) {
    mTimestamps.emplace_back(std::stol(timestamp.second.data()));
  }

  std::vector<std::string> keyVec{};
  std::vector<std::string> valueVec{};
  for (const auto& data : config.get_child("qc.postprocessing." + name + ".lookupMetaData")) {
    mLookupMaps.emplace_back(std::map<std::string, std::string>());
    if (const auto& keys = data.second.get_child_optional("keys"); keys.has_value()) {
      for (const auto& key : keys.value()) {
        keyVec.emplace_back(key.second.data());
      }
    }
    if (const auto& values = data.second.get_child_optional("values"); values.has_value()) {
      for (const auto& value : values.value()) {
        valueVec.emplace_back(value.second.data());
      }
    }
    auto vecIter = 0;
    if ((keyVec.size() > 0) && (keyVec.size() == valueVec.size())) {
      for (auto& key : keyVec) {
        mLookupMaps.back().insert(std::pair<std::string, std::string>(key, valueVec.at(vecIter)));
        vecIter++;
      }
    }
    if (keyVec.size() != valueVec.size()) {
      throw std::runtime_error("Number of keys and values for lookupMetaData are not matching");
    }
    keyVec.clear();
    valueVec.clear();
  }

  for (const auto& data : config.get_child("qc.postprocessing." + name + ".storeMetaData")) {
    mStoreMaps.emplace_back(std::map<std::string, std::string>());
    if (const auto& keys = data.second.get_child_optional("keys"); keys.has_value()) {
      for (const auto& key : keys.value()) {
        keyVec.emplace_back(key.second.data());
      }
    }
    if (const auto& values = data.second.get_child_optional("values"); values.has_value()) {
      for (const auto& value : values.value()) {
        valueVec.emplace_back(value.second.data());
      }
    }
    auto vecIter = 0;
    if ((keyVec.size() > 0) && (keyVec.size() == valueVec.size())) {
      for (auto& key : keyVec) {
        mStoreMaps.back().insert(std::pair<std::string, std::string>(key, valueVec.at(vecIter)));
        vecIter++;
      }
    }
    if (keyVec.size() != valueVec.size()) {
      throw std::runtime_error("Number of keys and values for storeMetaData are not matching");
    }
    keyVec.clear();
    valueVec.clear();
  }

  if ((mTimestamps.size() != mOutputList.size()) && (mTimestamps.size() > 0)) {
    throw std::runtime_error("You need to set a timestamp for every CalPad object or none at all");
  }
}

void CalDetPublisher::initialize(Trigger, framework::ServiceRegistry&)
{
  auto calDetIter = 0;
  for (auto& type : mOutputList) {
    mCalDetCanvasVec.emplace_back(std::vector<std::unique_ptr<TCanvas>>());
    addAndPublish(getObjectsManager(),
                  mCalDetCanvasVec.back(),
                  { fmt::format("c_Sides_{}", type).data(), fmt::format("c_ROCs_{}_1D", type).data(), fmt::format("c_ROCs_{}_2D", type).data() },
                  mStoreMaps.size() > 1 ? mStoreMaps.at(calDetIter) : mStoreMaps.at(0));
    calDetIter++;
  }
}

void CalDetPublisher::update(Trigger t, framework::ServiceRegistry&)
{
  ILOG(Info, Support) << "Trigger type is: " << t.triggerType << ", the timestamp is " << t.timestamp << ENDM;

  auto calDetIter = 0;
  for (auto& type : mOutputList) {
    auto& calDet = o2::tpc::CDBInterface::instance().getCalPad(fmt::format("TPC/Calib/{}", type).data(),
                                                               mTimestamps.size() > 0 ? mTimestamps.at(calDetIter) : -1,
                                                               mLookupMaps.size() > 1 ? mLookupMaps.at(calDetIter) : mLookupMaps.at(0));
    auto vecPtr = toVector(mCalDetCanvasVec.at(calDetIter));
    o2::tpc::painter::makeSummaryCanvases(calDet, 300, 0, 0, true, &vecPtr);
    calDetIter++;
  }
}

void CalDetPublisher::finalize(Trigger, framework::ServiceRegistry&)
{
  for (auto& calDetCanvasVec : mCalDetCanvasVec) {
    for (auto& canvas : calDetCanvasVec) {
      getObjectsManager()->stopPublishing(canvas.get());
    }
  }
}

} // namespace o2::quality_control_modules::tpc
