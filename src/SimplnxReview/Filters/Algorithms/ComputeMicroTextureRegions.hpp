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

struct SIMPLNXREVIEW_EXPORT ComputeMicroTextureRegionsInputValues
{
  DataPath ImageGeomPath;
  DataPath FeatureIdsArrayPath;
  DataPath CellFeatureAttributeMatrixPath;
  DataPath MicroTextureRegionNumCellsArrayPath;
  DataPath MicroTextureRegionFractionOccupiedArrayPath;
};

/**
 * @class ComputeMicroTextureRegions

 */

class SIMPLNXREVIEW_EXPORT ComputeMicroTextureRegions
{
public:
  ComputeMicroTextureRegions(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ComputeMicroTextureRegionsInputValues* inputValues);
  ~ComputeMicroTextureRegions() noexcept;

  ComputeMicroTextureRegions(const ComputeMicroTextureRegions&) = delete;
  ComputeMicroTextureRegions(ComputeMicroTextureRegions&&) noexcept = delete;
  ComputeMicroTextureRegions& operator=(const ComputeMicroTextureRegions&) = delete;
  ComputeMicroTextureRegions& operator=(ComputeMicroTextureRegions&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const ComputeMicroTextureRegionsInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};
} // namespace nx::core
