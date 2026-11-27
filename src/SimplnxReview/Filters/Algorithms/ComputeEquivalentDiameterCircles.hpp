#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{

struct SIMPLNXREVIEW_EXPORT ComputeEquivalentDiameterCirclesInputValues
{
  DataPath centroids_array_path;
  DataPath equivalent_diameters_array_path;
  uint64 circle_resolution{};
  int64 z_plane{};
  DataPath output_edge_geometry_path;
  std::string edge_attribute_matrix_name;
  std::string feature_ids_array_name;
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
