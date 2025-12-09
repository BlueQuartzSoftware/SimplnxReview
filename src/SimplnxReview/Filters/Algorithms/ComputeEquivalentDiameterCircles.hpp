#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{

struct SIMPLNXREVIEW_EXPORT ComputeEquivalentDiameterCirclesInputValues
{
  DataPath CentroidsArrayPath;
  DataPath EquivalentDiametersArrayPath;
  uint64 CircleResolution;
  int64 ZPlane;
  DataPath OutputEdgeGeometryPath;
  std::string EdgeAttributeMatrixName;
  std::string FeatureIdsArrayName;
};

/**
 * @class ComputeEquivalentDiameterCircles
 * @brief This filter determines the average C-axis location of each Feature.
 */

class SIMPLNXREVIEW_EXPORT ComputeEquivalentDiameterCircles
{
public:
  ComputeEquivalentDiameterCircles(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                   ComputeEquivalentDiameterCirclesInputValues* inputValues);
  ~ComputeEquivalentDiameterCircles() noexcept = default;

  ComputeEquivalentDiameterCircles(const ComputeEquivalentDiameterCircles&) = delete;
  ComputeEquivalentDiameterCircles(ComputeEquivalentDiameterCircles&&) noexcept = delete;
  ComputeEquivalentDiameterCircles& operator=(const ComputeEquivalentDiameterCircles&) = delete;
  ComputeEquivalentDiameterCircles& operator=(ComputeEquivalentDiameterCircles&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const ComputeEquivalentDiameterCirclesInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};

} // namespace nx::core
