#include "FindSaltykovSizesFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/FindSaltykovSizes.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataGroupSelectionParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"

#include <random>

using namespace nx::core;

namespace nx::core
{
//------------------------------------------------------------------------------
std::string FindSaltykovSizesFilter::name() const
{
  return FilterTraits<FindSaltykovSizesFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string FindSaltykovSizesFilter::className() const
{
  return FilterTraits<FindSaltykovSizesFilter>::className;
}

//------------------------------------------------------------------------------
Uuid FindSaltykovSizesFilter::uuid() const
{
  return FilterTraits<FindSaltykovSizesFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string FindSaltykovSizesFilter::humanName() const
{
  return "Find Feature Saltykov Sizes";
}

//------------------------------------------------------------------------------
std::vector<std::string> FindSaltykovSizesFilter::defaultTags() const
{
  return {className(), "Statistics", "Morphological"};
}

//------------------------------------------------------------------------------
Parameters FindSaltykovSizesFilter::parameters() const
{
  Parameters params;

  params.insertSeparator(Parameters::Separator{"Optional Variables"});
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_UseSeed_Key, "Use Seed for Random Generation", "When true the user will be able to put in a seed for random generation", false));
  params.insert(std::make_unique<NumberParameter<uint64>>(k_SeedValue_Key, "Seed", "The seed fed into the random generator", std::mt19937::default_seed));

  params.insertSeparator(Parameters::Separator{"Required Objects"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_EquivalentDiametersArrayPath_Key, "Equivalent Diameters", "", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::float32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{1}}));

  params.insertSeparator(Parameters::Separator{"Created Objects"});
  params.insert(std::make_unique<DataObjectNameParameter>(k_SaltykovEquivalentDiametersName_Key, "Saltykov Equivalent Diameters Name", "", "Saltykov Equivalent Diameters"));

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer FindSaltykovSizesFilter::clone() const
{
  return std::make_unique<FindSaltykovSizesFilter>();
}

//------------------------------------------------------------------------------
IFilter::PreflightResult FindSaltykovSizesFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                const std::atomic_bool& shouldCancel) const
{
  auto pEquivalentDiametersArrayPathValue = filterArgs.value<DataPath>(k_EquivalentDiametersArrayPath_Key);
  auto pSaltykovEquivalentDiametersNameValue = filterArgs.value<std::string>(k_SaltykovEquivalentDiametersName_Key);

  PreflightResult preflightResult;
  nx::core::Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  DataPath saltykovPath = pEquivalentDiametersArrayPathValue.getParent().createChildPath(pSaltykovEquivalentDiametersNameValue);
  const auto* equivDiams = dataStructure.getDataAs<IDataArray>(pEquivalentDiametersArrayPathValue);
  {
    auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::float32, equivDiams->getTupleShape(), std::vector<usize>{1}, saltykovPath);
    resultOutputActions.value().appendAction(std::move(createArrayAction));
  }

  preflightUpdatedValues.push_back({"WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});
  resultOutputActions.warnings().push_back({-65432, "WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});

  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> FindSaltykovSizesFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                              const std::atomic_bool& shouldCancel) const
{
  auto seed = filterArgs.value<std::mt19937_64::result_type>(k_SeedValue_Key);
  if(!filterArgs.value<bool>(k_UseSeed_Key))
  {
    seed = static_cast<std::mt19937_64::result_type>(std::chrono::steady_clock::now().time_since_epoch().count());
  }

  FindSaltykovSizesInputValues inputValues;

  inputValues.EquivalentDiametersPath = filterArgs.value<DataPath>(k_EquivalentDiametersArrayPath_Key);
  inputValues.SaltykovEquivalentDiametersPath = inputValues.EquivalentDiametersPath.getParent().createChildPath(filterArgs.value<std::string>(k_SaltykovEquivalentDiametersName_Key));
  inputValues.Seed = seed;

  return FindSaltykovSizes(dataStructure, messageHandler, shouldCancel, &inputValues)();
}
} // namespace nx::core
