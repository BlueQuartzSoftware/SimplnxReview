#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
/**
 * @class FindGroupingDensityFilter
 * @brief This filter determines the average C-axis location of each Feature
 */
class SIMPLNXREVIEW_EXPORT FindGroupingDensityFilter : public IFilter
{
public:
  FindGroupingDensityFilter() = default;
  ~FindGroupingDensityFilter() noexcept override = default;

  FindGroupingDensityFilter(const FindGroupingDensityFilter&) = delete;
  FindGroupingDensityFilter(FindGroupingDensityFilter&&) noexcept = delete;

  FindGroupingDensityFilter& operator=(const FindGroupingDensityFilter&) = delete;
  FindGroupingDensityFilter& operator=(FindGroupingDensityFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_VolumesPath_Key = "volumes_path";
  static inline constexpr StringLiteral k_ContiguousNLPath_Key = "contiguous_neighbor_list_path";
  static inline constexpr StringLiteral k_UseNonContiguousNeighbors_Key = "use_non_contiguous_neighbors";
  static inline constexpr StringLiteral k_NonContiguousNLPath_Key = "non_contiguous_neighbor_list_path";
  static inline constexpr StringLiteral k_ParentIdsPath_Key = "parent_ids_path";
  static inline constexpr StringLiteral k_ParentVolumesPath_Key = "parent_volumes_path";
  static inline constexpr StringLiteral k_FindCheckedFeatures_Key = "find_checked_features";
  static inline constexpr StringLiteral k_CheckedFeaturesName_Key = "checked_features_name";
  static inline constexpr StringLiteral k_GroupingDensitiesName_Key = "grouping_densities_name";

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

SIMPLNX_DEF_FILTER_TRAITS(nx::core, FindGroupingDensityFilter, "ff46afcf-de32-4f37-98bc-8f0fd4b3c122");
/* LEGACY UUID FOR THIS FILTER 708be082-8b08-4db2-94be-52781ed4d53d */
