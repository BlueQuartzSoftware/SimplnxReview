#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
/**
 * @class FindLocalAverageCAxisMisalignmentsFilter
 * @brief This filter will ....
 */
class SIMPLNXREVIEW_EXPORT FindLocalAverageCAxisMisalignmentsFilter : public IFilter
{
public:
  FindLocalAverageCAxisMisalignmentsFilter() = default;
  ~FindLocalAverageCAxisMisalignmentsFilter() noexcept override = default;

  FindLocalAverageCAxisMisalignmentsFilter(const FindLocalAverageCAxisMisalignmentsFilter&) = delete;
  FindLocalAverageCAxisMisalignmentsFilter(FindLocalAverageCAxisMisalignmentsFilter&&) noexcept = delete;

  FindLocalAverageCAxisMisalignmentsFilter& operator=(const FindLocalAverageCAxisMisalignmentsFilter&) = delete;
  FindLocalAverageCAxisMisalignmentsFilter& operator=(FindLocalAverageCAxisMisalignmentsFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_CalcBiasedAvg_Key = "calc_biased_avg";
  static inline constexpr StringLiteral k_CalcUnbiasedAvg_Key = "calc_unbiased_avg";
  static inline constexpr StringLiteral k_NeighborListPath_Key = "neighbor_list_path";
  static inline constexpr StringLiteral k_CAxisMisalignmentListPath_Key = "c_axis_misalignment_list_path";
  static inline constexpr StringLiteral k_AvgCAxisMisalignmentsPath_Key = "avg_c_axis_misalignments_path";
  static inline constexpr StringLiteral k_FeatureParentIdsPath_Key = "feature_parent_ids_path";
  static inline constexpr StringLiteral k_NewCellFeatureAttributeMatrixPath_Key = "new_cell_feature_attribute_matrix_path";
  static inline constexpr StringLiteral k_NumFeaturesPerParentName_Key = "num_features_per_parent_name";
  static inline constexpr StringLiteral k_LocalCAxisMisalignmentsName_Key = "local_c_axis_misalignments_name";
  static inline constexpr StringLiteral k_UnbiasedLocalCAxisMisalignmentsName_Key = "unbiased_local_c_axis_misalignments_name";

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

SIMPLNX_DEF_FILTER_TRAITS(nx::core, FindLocalAverageCAxisMisalignmentsFilter, "6002e998-04b2-4a11-87bd-43c54bce2c20");
/* LEGACY UUID FOR THIS FILTER 49b2dd47-bb29-50d4-a051-5bad9b6b9f80 */
