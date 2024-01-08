#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{

struct SIMPLNXREVIEW_EXPORT FindGroupingDensityInputValues
{
  DataPath VolumesPath;
  DataPath ContiguousNLPath;
  bool UseNonContiguousNeighbors;
  DataPath NonContiguousNLPath;
  DataPath ParentIdsPath;
  DataPath ParentVolumesPath;
  bool FindCheckedFeatures;
  DataPath CheckedFeaturesPath;
  DataPath GroupingDensitiesPath;
};

/**
 * @class FindGroupingDensity
 * @brief This filter determines the average C-axis location of each Feature.
 */

class SIMPLNXREVIEW_EXPORT FindGroupingDensity
{
public:
  FindGroupingDensity(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, FindGroupingDensityInputValues* inputValues);
  ~FindGroupingDensity() noexcept = default;

  FindGroupingDensity(const FindGroupingDensity&) = delete;
  FindGroupingDensity(FindGroupingDensity&&) noexcept = delete;
  FindGroupingDensity& operator=(const FindGroupingDensity&) = delete;
  FindGroupingDensity& operator=(FindGroupingDensity&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const FindGroupingDensityInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};

} // namespace nx::core
