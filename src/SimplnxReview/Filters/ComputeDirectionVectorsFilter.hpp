#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/Common/StringLiteral.hpp"
#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
/**
 * @class ComputeDirectionVectorsFilter
 * @brief
 */
class SIMPLNXREVIEW_EXPORT ComputeDirectionVectorsFilter : public IFilter
{
public:
  ComputeDirectionVectorsFilter() = default;
  ~ComputeDirectionVectorsFilter() noexcept override = default;

  ComputeDirectionVectorsFilter(const ComputeDirectionVectorsFilter&) = delete;
  ComputeDirectionVectorsFilter(ComputeDirectionVectorsFilter&&) noexcept = delete;

  ComputeDirectionVectorsFilter& operator=(const ComputeDirectionVectorsFilter&) = delete;
  ComputeDirectionVectorsFilter& operator=(ComputeDirectionVectorsFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_InputType_Key = "input_representation_index";
  static inline constexpr StringLiteral k_InputOrientationArrayPath_Key = "input_orientation_array_path";
  static inline constexpr StringLiteral k_LatticeConstantsInputType_Key = "lattice_constants_input_type_index";
  static inline constexpr StringLiteral k_LatticeConstantsArrayPath_Key = "lattice_constants_array_path";
  static inline constexpr StringLiteral k_LatticeConstantsLength_Key = "lattice_constants_length";
  static inline constexpr StringLiteral k_LatticeConstantsAngles_Key = "lattice_constants_angles";
  static inline constexpr StringLiteral k_OutputDirectionVectorsArrayName_Key = "output_direction_vectors_array_name";

  /**
   * @brief Returns the name of the filter.
   * @return std::string
   */
  std::string name() const override;

  /**
   * @brief Returns the C++ classname of this filter.
   * @return std::string
   */
  std::string className() const override;

  /**
   * @brief Returns the filter's UUID.
   * @return Uuid
   */
  Uuid uuid() const override;

  /**
   * @brief Returns the filter name name presented to the user.
   * @return std::string
   */
  std::string humanName() const override;

  /**
   * @brief Returns the default tags for this filter.
   * @return
   */
  std::vector<std::string> defaultTags() const override;

  /**
   * @brief Returns the parameters required to run the filter.
   * @return Parameters
   */
  Parameters parameters() const override;

  /**
   * @brief Returns parameters version integer.
   * The Initial version should always be 1.
   * Should be incremented everytime the parameters change.
   * @return VersionType
   */
  VersionType parametersVersion() const override;

  /**
   * @brief Creates a copy of the filter.
   * @return IFilter::UniquePointer
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
   * @param shouldCancel The atomic boolean that holds if the filter should be canceled
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

SIMPLNX_DEF_FILTER_TRAITS(nx::core, ComputeDirectionVectorsFilter, "1e2386db-c1a3-4e6a-b30b-34837d0a8555");
