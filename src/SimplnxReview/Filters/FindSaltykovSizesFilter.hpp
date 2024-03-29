#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/Filter/FilterTraits.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{
/**
 * @class FindSaltykovSizesFilter
 * @brief This filter will...
 */
class SIMPLNXREVIEW_EXPORT FindSaltykovSizesFilter : public IFilter
{
public:
  FindSaltykovSizesFilter() = default;
  ~FindSaltykovSizesFilter() noexcept override = default;

  FindSaltykovSizesFilter(const FindSaltykovSizesFilter&) = delete;
  FindSaltykovSizesFilter(FindSaltykovSizesFilter&&) noexcept = delete;

  FindSaltykovSizesFilter& operator=(const FindSaltykovSizesFilter&) = delete;
  FindSaltykovSizesFilter& operator=(FindSaltykovSizesFilter&&) noexcept = delete;

  // Parameter Keys
  static inline constexpr StringLiteral k_UseSeed_Key = "use_seed";
  static inline constexpr StringLiteral k_SeedValue_Key = "seed_value";
  static inline constexpr StringLiteral k_EquivalentDiametersArrayPath_Key = "equivalent_diameters_array_path";
  static inline constexpr StringLiteral k_SaltykovEquivalentDiametersName_Key = "saltykov_equivalent_diameters_name";

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
   * @param dataStructure The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param messageHandler The MessageHandler object
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  PreflightResult preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;

  /**
   * @brief Applies the filter's algorithm to the DataStructure with the given arguments. Returns any warnings/errors.
   * On failure, there is no guarantee that the DataStructure is in a correct state.
   * @param dataStructure The input DataStructure instance
   * @param filterArgs These are the input values for each parameter that is required for the filter
   * @param messageHandler The MessageHandler object
   * @return Returns a Result object with error or warning values if any of those occurred during execution of this function
   */
  Result<> executeImpl(DataStructure& data, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel) const override;
};
} // namespace nx::core

SIMPLNX_DEF_FILTER_TRAITS(nx::core, FindSaltykovSizesFilter, "fde6ad53-efa2-4870-96e3-ef795b3568d0");
/* LEGACY UUID FOR THIS FILTER cc76cffe-81ad-5ece-be2a-ce127c5fa6d7 */
