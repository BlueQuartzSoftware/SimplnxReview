#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/Common/StringLiteral.hpp"
#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
class SIMPLNXREVIEW_EXPORT InterpolateValuesToUnstructuredGridFilter : public IFilter
{
public:
  InterpolateValuesToUnstructuredGridFilter() = default;
  ~InterpolateValuesToUnstructuredGridFilter() noexcept override = default;

  InterpolateValuesToUnstructuredGridFilter(const InterpolateValuesToUnstructuredGridFilter&) = delete;
  InterpolateValuesToUnstructuredGridFilter(InterpolateValuesToUnstructuredGridFilter&&) noexcept = delete;

  InterpolateValuesToUnstructuredGridFilter& operator=(const InterpolateValuesToUnstructuredGridFilter&) = delete;
  InterpolateValuesToUnstructuredGridFilter& operator=(InterpolateValuesToUnstructuredGridFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_SourceGeometryPath_Key = "source_geometry_path";
  static inline constexpr StringLiteral k_InterpolatedArrayPaths_Key = "interpolated_array_paths";
  static inline constexpr StringLiteral k_UseExistingAttrMatrix_Key = "use_existing_attr_matrix";
  static inline constexpr StringLiteral k_ExistingAttrMatrixPath_Key = "existing_attr_matrix_path";
  static inline constexpr StringLiteral k_CreatedAttrMatrixName_Key = "created_attr_matrix_name";
  static inline constexpr StringLiteral k_DestinationGeometryPath_Key = "destination_geometry_path";

  /**
   * @brief Reads SIMPL json and converts it simplnx Arguments.
   * @param json
   * @return Result<Arguments>
   */
  static Result<Arguments> FromSIMPLJson(const nlohmann::json& json);

  /**
   * @brief
   * @return std::string
   */
  std::string name() const override;

  /**
   * @brief Returns the C++ classname of this filter.
   * @return std::string
   */
  std::string className() const override;

  /**
   * @brief
   * @return Uuid
   */
  Uuid uuid() const override;

  /**
   * @brief
   * @return std::string
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
   * @brief
   * @param data
   * @param filterArgs
   * @param messageHandler
   * @return Result<OutputActions>
   */
  PreflightResult preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel,
                                const ExecutionContext& executionContext) const override;

  /**
   * @brief
   * @param dataStructure
   * @param args
   * @param pipelineNode
   * @param messageHandler
   * @return Result<>
   */
  Result<> executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel,
                       const ExecutionContext& executionContext) const override;
};
} // namespace nx::core

SIMPLNX_DEF_FILTER_TRAITS(nx::core, InterpolateValuesToUnstructuredGridFilter, "d8477024-7f4a-44eb-ad3f-aed6d07e972b");
