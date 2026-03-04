#include "ComputeArrayNormFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/ComputeArrayNorm.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"
#include "simplnx/Utilities/SIMPLConversion.hpp"

using namespace nx::core;

namespace nx::core
{
//------------------------------------------------------------------------------
std::string ComputeArrayNormFilter::name() const
{
  return FilterTraits<ComputeArrayNormFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string ComputeArrayNormFilter::className() const
{
  return FilterTraits<ComputeArrayNormFilter>::className;
}

//------------------------------------------------------------------------------
Uuid ComputeArrayNormFilter::uuid() const
{
  return FilterTraits<ComputeArrayNormFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string ComputeArrayNormFilter::humanName() const
{
  return "Compute Array Norm";
}

//------------------------------------------------------------------------------
std::vector<std::string> ComputeArrayNormFilter::defaultTags() const
{
  return {className(), "Statistics", "DREAM3DReview"};
}

//------------------------------------------------------------------------------
Parameters ComputeArrayNormFilter::parameters() const
{
  Parameters params;

  params.insertSeparator(Parameters::Separator{"Input Parameter"});
  params.insert(std::make_unique<Float32Parameter>(k_PSpace_Key, "p-Space Value", "p-Value used for computing the norm (2 = Euclidean, 1 = Manhattan)", 2.0f));

  params.insertSeparator(Parameters::Separator{"Input Data"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_SelectedArrayPath_Key, "Input Attribute Array", "The input array for computing the norm", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::int8, DataType::uint8, DataType::int16, DataType::uint16, DataType::int32, DataType::uint32,
                                                                                                DataType::int64, DataType::uint64, DataType::float32, DataType::float64},
                                                          ArraySelectionParameter::AllowedComponentShapes{}));

  params.insertSeparator(Parameters::Separator{"Output Data"});
  params.insert(std::make_unique<DataObjectNameParameter>(k_NormArrayName_Key, "Norm Array Name", "The name of the output norm array", "Norm"));

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer ComputeArrayNormFilter::clone() const
{
  return std::make_unique<ComputeArrayNormFilter>();
}

//------------------------------------------------------------------------------
IFilter::VersionType ComputeArrayNormFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::PreflightResult ComputeArrayNormFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                               const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  auto pSpaceValue = filterArgs.value<float32>(k_PSpace_Key);
  auto pSelectedArrayPathValue = filterArgs.value<DataPath>(k_SelectedArrayPath_Key);
  auto pNormArrayNameValue = filterArgs.value<std::string>(k_NormArrayName_Key);

  PreflightResult preflightResult;
  nx::core::Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  if(pSpaceValue < 0.0f)
  {
    return {MakeErrorResult<OutputActions>(-11002, "p-space value must be greater than or equal to 0")};
  }

  const auto* inputArray = dataStructure.getDataAs<IDataArray>(pSelectedArrayPathValue);
  if(inputArray == nullptr)
  {
    return {MakeErrorResult<OutputActions>(-11003, fmt::format("Cannot find the selected input array at path '{}'", pSelectedArrayPathValue.toString()))};
  }

  DataPath normArrayPath = pSelectedArrayPathValue.replaceName(pNormArrayNameValue);

  {
    auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::float32, inputArray->getTupleShape(), std::vector<usize>{1}, normArrayPath);
    resultOutputActions.value().appendAction(std::move(createArrayAction));
  }

  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> ComputeArrayNormFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                             const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  ComputeArrayNormInputValues inputValues;

  inputValues.PSpace = filterArgs.value<float32>(k_PSpace_Key);
  inputValues.SelectedArrayPath = filterArgs.value<DataPath>(k_SelectedArrayPath_Key);
  inputValues.NormArrayPath = inputValues.SelectedArrayPath.replaceName(filterArgs.value<std::string>(k_NormArrayName_Key));

  return ComputeArrayNorm(dataStructure, messageHandler, shouldCancel, &inputValues)();
}

namespace
{
namespace SIMPL
{
constexpr StringLiteral k_SelectedArrayPathKey = "SelectedArrayPath";
constexpr StringLiteral k_NormArrayPathKey = "NormArrayPath";
constexpr StringLiteral k_PSpaceKey = "PSpace";
} // namespace SIMPL
} // namespace

Result<Arguments> ComputeArrayNormFilter::FromSIMPLJson(const nlohmann::json& json)
{
  Arguments args = ComputeArrayNormFilter().getDefaultArguments();

  std::vector<Result<>> results;

  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::FloatFilterParameterConverter<float32>>(args, json, SIMPL::k_PSpaceKey, k_PSpace_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_SelectedArrayPathKey, k_SelectedArrayPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArrayCreationToDataObjectNameFilterParameterConverter>(args, json, SIMPL::k_NormArrayPathKey, k_NormArrayName_Key));

  Result<> conversionResult = MergeResults(std::move(results));

  return ConvertResultTo<Arguments>(std::move(conversionResult), std::move(args));
}
} // namespace nx::core
