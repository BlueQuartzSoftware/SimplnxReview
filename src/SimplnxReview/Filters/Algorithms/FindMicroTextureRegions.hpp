#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/DataGroupSelectionParameter.hpp"
#include "simplnx/Parameters/StringParameter.hpp"

namespace nx::core
{

struct SIMPLNXREVIEW_EXPORT FindMicroTextureRegionsInputValues
{
  DataPath ImageGeomPath;
  DataPath FeatureIdsArrayPath;
  DataPath CellFeatureAttributeMatrixPath;
  DataPath MicroTextureRegionNumCellsArrayPath;
  DataPath MicroTextureRegionFractionOccupiedArrayPath;
};

/**
 * @class ConditionalSetValue
 * @brief This filter replaces values in the target array with a user specified value
 * where a bool mask array specifies.
 */

class SIMPLNXREVIEW_EXPORT FindMicroTextureRegions
{
public:
  FindMicroTextureRegions(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, FindMicroTextureRegionsInputValues* inputValues);
  ~FindMicroTextureRegions() noexcept;

  FindMicroTextureRegions(const FindMicroTextureRegions&) = delete;
  FindMicroTextureRegions(FindMicroTextureRegions&&) noexcept = delete;
  FindMicroTextureRegions& operator=(const FindMicroTextureRegions&) = delete;
  FindMicroTextureRegions& operator=(FindMicroTextureRegions&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const FindMicroTextureRegionsInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};
} // namespace nx::core
