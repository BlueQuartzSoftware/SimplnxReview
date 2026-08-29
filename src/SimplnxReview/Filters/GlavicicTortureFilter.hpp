#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
/**
 * @class GlavicicTortureFilter
 * @brief This filter will...
 */
class SIMPLNXREVIEW_EXPORT GlavicicTortureFilter : public IFilter
{
public:
  GlavicicTortureFilter() = default;
  ~GlavicicTortureFilter() noexcept override = default;

  GlavicicTortureFilter(const GlavicicTortureFilter&) = delete;
  GlavicicTortureFilter(GlavicicTortureFilter&&) noexcept = delete;

  GlavicicTortureFilter& operator=(const GlavicicTortureFilter&) = delete;
  GlavicicTortureFilter& operator=(GlavicicTortureFilter&&) noexcept = delete;

  // Parameter Keys
  static constexpr StringLiteral k_UseSeed_Key = "use_seed";
  static constexpr StringLiteral k_SeedValue_Key = "seed_value";
  static constexpr StringLiteral k_ImageGeometryPath_Key = "input_image_geometry_path";
  static constexpr StringLiteral k_ExpectedMTRIdsArrayPath_Key = "expected_mtr_ids_array_path";
  static constexpr StringLiteral k_Group0ArrayPath_Key = "group_0_array_path";
  static constexpr StringLiteral k_Group1ArrayPath_Key = "group_1_array_path";
  static constexpr StringLiteral k_Group2ArrayPath_Key = "group_2_array_path";
  static constexpr StringLiteral k_Group3ArrayPath_Key = "group_3_array_path";
  static constexpr StringLiteral k_Group4ArrayPath_Key = "group_4_array_path";
  static constexpr StringLiteral k_Group6ArrayPath_Key = "group_6_array_path";
  static constexpr StringLiteral k_EulerAnglesArrayName_Key = "euler_angles_array_name";

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
   * @brief Returns the human-readable name of the filter.
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
   * @param dataStructure The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param messageHandler The MessageHandler object
   * @param shouldCancel Atomic boolean value that can be checked to cancel the filter
   * @param executionContext The ExecutionContext that can be used to determine the correct absolute path from a relative path
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  PreflightResult preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel,
                                const ExecutionContext& executionContext) const override;

  /**
   * @brief Applies the filter's algorithm to the DataStructure with the given arguments. Returns any warnings/errors.
   * On failure, there is no guarantee that the DataStructure is in a correct state.
   * @param dataStructure The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param pipelineNode The node in the pipeline that is being executed
   * @param messageHandler The MessageHandler object
   * @param shouldCancel Atomic boolean value that can be checked to cancel the filter
   * @param executionContext The ExecutionContext that can be used to determine the correct absolute path from a relative path
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  Result<> executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel,
                       const ExecutionContext& executionContext) const override;
};
} // namespace nx::core

SIMPLNX_DEF_FILTER_TRAITS(nx::core, GlavicicTortureFilter, "4c280213-add8-4332-8767-ed0faa30ecb4");
