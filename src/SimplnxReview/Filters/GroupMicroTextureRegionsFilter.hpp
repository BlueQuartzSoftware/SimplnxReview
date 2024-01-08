#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
/**
 * @class GroupMicroTextureRegionsFilter
 * @brief This filter will ....
 */
class SIMPLNXREVIEW_EXPORT GroupMicroTextureRegionsFilter : public IFilter
{
public:
  GroupMicroTextureRegionsFilter() = default;
  ~GroupMicroTextureRegionsFilter() noexcept override = default;

  GroupMicroTextureRegionsFilter(const GroupMicroTextureRegionsFilter&) = delete;
  GroupMicroTextureRegionsFilter(GroupMicroTextureRegionsFilter&&) noexcept = delete;

  GroupMicroTextureRegionsFilter& operator=(const GroupMicroTextureRegionsFilter&) = delete;
  GroupMicroTextureRegionsFilter& operator=(GroupMicroTextureRegionsFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_UseNonContiguousNeighbors_Key = "use_non_contiguous_neighbors";
  static inline constexpr StringLiteral k_NonContiguousNeighborListArrayPath_Key = "non_contiguous_neighbor_list_array_path";
  static inline constexpr StringLiteral k_ContiguousNeighborListArrayPath_Key = "contiguous_neighbor_list_array_path";
  static inline constexpr StringLiteral k_UseRunningAverage_Key = "use_running_average";
  static inline constexpr StringLiteral k_CAxisTolerance_Key = "c_axis_tolerance";
  static inline constexpr StringLiteral k_FeatureIdsArrayPath_Key = "feature_ids_array_path";
  static inline constexpr StringLiteral k_FeaturePhasesArrayPath_Key = "feature_phases_array_path";
  static inline constexpr StringLiteral k_VolumesArrayPath_Key = "volumes_array_path";
  static inline constexpr StringLiteral k_AvgQuatsArrayPath_Key = "avg_quats_array_path";
  static inline constexpr StringLiteral k_CrystalStructuresArrayPath_Key = "crystal_structures_array_path";
  static inline constexpr StringLiteral k_NewCellFeatureAttributeMatrixName_Key = "new_cell_feature_attribute_matrix_name";
  static inline constexpr StringLiteral k_CellParentIdsArrayName_Key = "cell_parent_ids_array_name";
  static inline constexpr StringLiteral k_FeatureParentIdsArrayName_Key = "feature_parent_ids_array_name";
  static inline constexpr StringLiteral k_ActiveArrayName_Key = "active_array_name";
  static inline constexpr StringLiteral k_SeedArrayName_Key = "seed_array_name";
  static inline constexpr StringLiteral k_UseSeed_Key = "use_seed";
  static inline constexpr StringLiteral k_SeedValue_Key = "seed_value";

  /**
   * @brief Returns the name of the filter.
   * @return
   */
  std::string name() const override;

  /**
   * @brief Returns the C++ classname of this filter.
   * @return
   */
  std::string className() const override;

  /**
   * @brief Returns the uuid of the filter.
   * @return
   */
  Uuid uuid() const override;

  /**
   * @brief Returns the human readable name of the filter.
   * @return
   */
  std::string humanName() const override;

  /**
   * @brief Returns the default tags for this filter.
   * @return
   */
  std::vector<std::string> defaultTags() const override;

  /**
   * @brief Returns the parameters of the filter (i.e. its inputs)
   * @return
   */
  Parameters parameters() const override;

  /**
   * @brief Returns a copy of the filter.
   * @return
   */
  UniquePointer clone() const override;

protected:
  /**
   * @brief Takes in a DataStructure and checks that the filter can be run on it with the given arguments.
   * Returns any warnings/errors. Also returns the changes that would be applied to the DataStructure.
   * Some parts of the actions may not be completely filled out if all the required information is not available at preflight time.
   * @param ds The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param messageHandler The MessageHandler object
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  PreflightResult preflightImpl(const DataStructure& ds, const Arguments& filterArgs, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;

  /**
   * @brief Applies the filter's algorithm to the DataStructure with the given arguments. Returns any warnings/errors.
   * On failure, there is no guarantee that the DataStructure is in a correct state.
   * @param ds The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param messageHandler The MessageHandler object
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  Result<> executeImpl(DataStructure& data, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;
};
} // namespace nx::core

SIMPLNX_DEF_FILTER_TRAITS(nx::core, GroupMicroTextureRegionsFilter, "3f695987-81b1-47c3-8cff-b49cfa219be0");
/* LEGACY UUID FOR THIS FILTER 5e18a9e2-e342-56ac-a54e-3bd0ca8b9c53 */
