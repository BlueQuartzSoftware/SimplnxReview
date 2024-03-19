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
   * @brief
   * @return Parameters
   */
  Parameters parameters() const override;

  /**
   * @brief
   * @return UniquePointer
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
  PreflightResult preflightImpl(const DataStructure& data, const Arguments& args, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;

  /**
   * @brief
   * @param dataStructure
   * @param args
   * @param pipelineNode
   * @param messageHandler
   * @return Result<>
   */
  Result<> executeImpl(DataStructure& data, const Arguments& args, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;
};
} // namespace nx::core

SIMPLNX_DEF_FILTER_TRAITS(nx::core, InterpolateValuesToUnstructuredGridFilter, "d8477024-7f4a-44eb-ad3f-aed6d07e972b");
