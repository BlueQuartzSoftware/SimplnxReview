#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
/**
 * @class ComputeMicroTextureRegionsFilter
 * @brief This filter will ....
 */
class SIMPLNXREVIEW_EXPORT ComputeMicroTextureRegionsFilter : public IFilter
{
public:
  ComputeMicroTextureRegionsFilter() = default;
  ~ComputeMicroTextureRegionsFilter() noexcept override = default;

  ComputeMicroTextureRegionsFilter(const ComputeMicroTextureRegionsFilter&) = delete;
  ComputeMicroTextureRegionsFilter(ComputeMicroTextureRegionsFilter&&) noexcept = delete;

  ComputeMicroTextureRegionsFilter& operator=(const ComputeMicroTextureRegionsFilter&) = delete;
  ComputeMicroTextureRegionsFilter& operator=(ComputeMicroTextureRegionsFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_ImageGeomPath_Key = "image_geom_path";
  static inline constexpr StringLiteral k_FeatureIdsArrayPath_Key = "feature_ids_array_path";
  static inline constexpr StringLiteral k_CellFeatureAttributeMatrixPath_Key = "cell_feature_attribute_matrix_path";
  static inline constexpr StringLiteral k_MicroTextureRegionNumCellsArrayName_Key = "micro_texture_region_num_cells_array_name";
  static inline constexpr StringLiteral k_MicroTextureRegionFractionOccupiedArrayName_Key = "micro_texture_region_fraction_occupied_array_name";

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
   * @brief Returns parameters version integer.
   * Initial version should always be 1.
   * Should be incremented everytime the parameters change.
   * @return VersionType
   */
  VersionType parametersVersion() const override;

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
  PreflightResult preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;

  /**
   * @brief Applies the filter's algorithm to the DataStructure with the given arguments. Returns any warnings/errors.
   * On failure, there is no guarantee that the DataStructure is in a correct state.
   * @param ds The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param messageHandler The MessageHandler object
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  Result<> executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                       const std::atomic_bool& shouldCancel) const override;
};
} // namespace nx::core

SIMPLNX_DEF_FILTER_TRAITS(nx::core, ComputeMicroTextureRegionsFilter, "be3477f0-f3f0-4790-864a-0658ac7568ba");
/* LEGACY UUID FOR THIS FILTER 90f8e3b1-2460-5862-95a1-a9e06f5ee75e */
