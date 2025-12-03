#include "ComputeLocalAverageCAxisMisalignmentsFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/ComputeLocalAverageCAxisMisalignments.hpp"

#include "simplnx/DataStructure/AttributeMatrix.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Filter/Actions/DeleteDataAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataGroupSelectionParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/NeighborListSelectionParameter.hpp"
#include "simplnx/Utilities/SIMPLConversion.hpp"

using namespace nx::core;

namespace
{
const DataPath k_AvgCAxisMisalignmentsPath = DataPath({"AvgCAxisMisalignments-Temp"});
const DataPath k_LocalCAxisMisalignmentsPath = DataPath({"LocalCAxisMisalignments-Temp"});
const DataPath k_NeighborListPath = DataPath({"NeighborList-Temp"});
const DataPath k_CAxisMisalignmentListPath = DataPath({"CAxisMisalignmentList-Temp"});
const DataPath k_UnbiasedLocalCAxisMisalignmentsPath = DataPath({"UnbiasedLocalCAxisMisalignments-Temp"});
} // namespace

namespace nx::core
{
//------------------------------------------------------------------------------
std::string ComputeLocalAverageCAxisMisalignmentsFilter::name() const
{
  return FilterTraits<ComputeLocalAverageCAxisMisalignmentsFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string ComputeLocalAverageCAxisMisalignmentsFilter::className() const
{
  return FilterTraits<ComputeLocalAverageCAxisMisalignmentsFilter>::className;
}

//------------------------------------------------------------------------------
Uuid ComputeLocalAverageCAxisMisalignmentsFilter::uuid() const
{
  return FilterTraits<ComputeLocalAverageCAxisMisalignmentsFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string ComputeLocalAverageCAxisMisalignmentsFilter::humanName() const
{
  return "Compute Local Average C-Axis Misalignments";
}

//------------------------------------------------------------------------------
std::vector<std::string> ComputeLocalAverageCAxisMisalignmentsFilter::defaultTags() const
{
  return {className(), "Statistics", "Crystallography"};
}

//------------------------------------------------------------------------------
Parameters ComputeLocalAverageCAxisMisalignmentsFilter::parameters() const
{
  Parameters params;

  // Create the parameter descriptors that are needed for this filter
  params.insertSeparator(Parameters::Separator{"Input Parameter(s)"});
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_CalcUnbiasedAvg_Key, "Calculate Unbiased Local C-Axis Mis-alignments", "Calculate Unbiased Local C-Axis Mis-alignments", false));
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_CalcBiasedAvg_Key, "Calculate Local C-Axis Mis-alignments", "Calculate Local C-Axis Mis-alignments", false));

  params.insertSeparator(Parameters::Separator{"Input Feature Data"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_FeatureParentIdsPath_Key, "Feature Parent Ids", "Input feature based ParentIds data array", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::int32}, ArraySelectionParameter::AllowedComponentShapes{{1}}));
  params.insert(
      std::make_unique<NeighborListSelectionParameter>(k_NeighborListPath_Key, "Neighbor List", "Feature based Neighbors", DataPath{}, NeighborListSelectionParameter::AllowedTypes{DataType::int32}));
  params.insert(std::make_unique<NeighborListSelectionParameter>(k_CAxisMisalignmentListPath_Key, "C-Axis Mis-alignment NeighborList", "Input feature based C-Axis Mis-alignment NeighborList",
                                                                 DataPath{}, NeighborListSelectionParameter::AllowedTypes{DataType::float32}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_AvgCAxisMisalignmentsPath_Key, "Average C-Axis Mis-alignments", "Input feature based Average C-Axis Mis-alignments", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::int32}, ArraySelectionParameter::AllowedComponentShapes{{1}}));

  params.insertSeparator(Parameters::Separator{"Output Feature Data"});
  params.insert(std::make_unique<DataGroupSelectionParameter>(k_NewCellFeatureAttributeMatrixPath_Key, "New Feature Attribute Matrix Name", "Output Feature Attribute Matrix to hold results",
                                                              DataPath{}, DataGroupSelectionParameter::AllowedTypes{BaseGroup::GroupType::AttributeMatrix}));
  params.insert(std::make_unique<DataObjectNameParameter>(k_NumFeaturesPerParentName_Key, "Number of Features Per Parent Array Name", "Output feature data array to hold number of features per parent",
                                                          "NumFeaturesPerParent"));
  params.insert(std::make_unique<DataObjectNameParameter>(k_LocalCAxisMisalignmentsName_Key, "Local C-Axis Mis-alignments Array Name",
                                                          "Output feature data array to hold the local c-axis mis-alignments", "LocalCAxisMisalignments"));
  params.insert(std::make_unique<DataObjectNameParameter>(k_UnbiasedLocalCAxisMisalignmentsName_Key, "Unbiased Local CAxis Mis-alignments Array Name",
                                                          "Output feature data array to hold the unbiased local c-axis mis-alignments", "UnbiasedLocalCAxisMisalignments"));

  // Associate the Linkable Parameter(s) to the children parameters that they control
  params.linkParameters(k_CalcBiasedAvg_Key, k_LocalCAxisMisalignmentsName_Key, true);
  params.linkParameters(k_CalcBiasedAvg_Key, k_AvgCAxisMisalignmentsPath_Key, true);

  params.linkParameters(k_CalcUnbiasedAvg_Key, k_UnbiasedLocalCAxisMisalignmentsName_Key, true);
  params.linkParameters(k_CalcUnbiasedAvg_Key, k_NeighborListPath_Key, true);
  params.linkParameters(k_CalcUnbiasedAvg_Key, k_CAxisMisalignmentListPath_Key, true);

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer ComputeLocalAverageCAxisMisalignmentsFilter::clone() const
{
  return std::make_unique<ComputeLocalAverageCAxisMisalignmentsFilter>();
}

//------------------------------------------------------------------------------
IFilter::VersionType ComputeLocalAverageCAxisMisalignmentsFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::PreflightResult ComputeLocalAverageCAxisMisalignmentsFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                                    const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  auto pCalcBiasedAvgValue = filterArgs.value<bool>(k_CalcBiasedAvg_Key);
  auto pCalcUnbiasedAvgValue = filterArgs.value<bool>(k_CalcUnbiasedAvg_Key);
  auto pNeighborListArrayPathValue = filterArgs.value<DataPath>(k_NeighborListPath_Key);
  auto pCAxisMisalignmentListArrayPathValue = filterArgs.value<DataPath>(k_CAxisMisalignmentListPath_Key);
  auto pAvgCAxisMisalignmentsArrayPathValue = filterArgs.value<DataPath>(k_AvgCAxisMisalignmentsPath_Key);
  auto pParentAMPathValue = filterArgs.value<DataPath>(k_NewCellFeatureAttributeMatrixPath_Key);
  auto pNumFeaturesPerParentNameValue = filterArgs.value<DataObjectNameParameter::ValueType>(k_NumFeaturesPerParentName_Key);
  auto pLocalMisalignmentsNameValue = filterArgs.value<DataObjectNameParameter::ValueType>(k_LocalCAxisMisalignmentsName_Key);
  auto pUnbiasedArrayNameValue = filterArgs.value<DataObjectNameParameter::ValueType>(k_UnbiasedLocalCAxisMisalignmentsName_Key);

  PreflightResult preflightResult;
  nx::core::Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  if(!pCalcBiasedAvgValue && !pCalcUnbiasedAvgValue)
  {
    return MakePreflightErrorResult(-43160, "Since both Calculate Local C-Axis Mis-alignments and Calculate Unbiased Local C-Axis Misalignments are false, nothing will be done in this filter, "
                                            "consider making one or both true or remove filter from pipeline.");
  }

  const auto* parentAM = dataStructure.getDataAs<AttributeMatrix>(pParentAMPathValue);

  {
    auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::int32, parentAM->getShape(), std::vector<usize>{1}, pParentAMPathValue.createChildPath(pNumFeaturesPerParentNameValue));
    resultOutputActions.value().appendAction(std::move(createArrayAction));
  }

  if(pCalcBiasedAvgValue)
  {
    {
      auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::float32, parentAM->getShape(), std::vector<usize>{1}, pParentAMPathValue.createChildPath(pLocalMisalignmentsNameValue));
      resultOutputActions.value().appendAction(std::move(createArrayAction));
    }
  }
  else
  {
    {
      auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::float32, std::vector<usize>{1}, std::vector<usize>{1}, ::k_AvgCAxisMisalignmentsPath);
      resultOutputActions.value().appendAction(std::move(createArrayAction));
    }
    {
      auto deleteDataAction = std::make_unique<DeleteDataAction>(::k_AvgCAxisMisalignmentsPath);
      resultOutputActions.value().appendDeferredAction(std::move(deleteDataAction));
    }

    {
      auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::float32, std::vector<usize>{1}, std::vector<usize>{1}, ::k_LocalCAxisMisalignmentsPath);
      resultOutputActions.value().appendAction(std::move(createArrayAction));
    }
    {
      auto deleteDataAction = std::make_unique<DeleteDataAction>(::k_LocalCAxisMisalignmentsPath);
      resultOutputActions.value().appendDeferredAction(std::move(deleteDataAction));
    }
  }

  if(pCalcUnbiasedAvgValue)
  {
    {
      auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::float32, parentAM->getShape(), std::vector<usize>{1}, pParentAMPathValue.createChildPath(pUnbiasedArrayNameValue));
      resultOutputActions.value().appendAction(std::move(createArrayAction));
    }
  }
  else
  {
    {
      auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::float32, std::vector<usize>{1}, std::vector<usize>{1}, ::k_NeighborListPath);
      resultOutputActions.value().appendAction(std::move(createArrayAction));
    }
    {
      auto deleteDataAction = std::make_unique<DeleteDataAction>(::k_NeighborListPath);
      resultOutputActions.value().appendDeferredAction(std::move(deleteDataAction));
    }

    {
      auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::float32, std::vector<usize>{1}, std::vector<usize>{1}, ::k_CAxisMisalignmentListPath);
      resultOutputActions.value().appendAction(std::move(createArrayAction));
    }
    {
      auto deleteDataAction = std::make_unique<DeleteDataAction>(::k_CAxisMisalignmentListPath);
      resultOutputActions.value().appendDeferredAction(std::move(deleteDataAction));
    }

    {
      auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::float32, std::vector<usize>{1}, std::vector<usize>{1}, ::k_UnbiasedLocalCAxisMisalignmentsPath);
      resultOutputActions.value().appendAction(std::move(createArrayAction));
    }
    {
      auto deleteDataAction = std::make_unique<DeleteDataAction>(::k_UnbiasedLocalCAxisMisalignmentsPath);
      resultOutputActions.value().appendDeferredAction(std::move(deleteDataAction));
    }
  }

  preflightUpdatedValues.push_back({"WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});
  resultOutputActions.warnings().push_back({-65432, "WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});

  // Return both the resultOutputActions and the preflightUpdatedValues via std::move()
  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> ComputeLocalAverageCAxisMisalignmentsFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                                                  const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  ComputeLocalAverageCAxisMisalignmentsInputValues inputValues;

  auto parentAM = filterArgs.value<DataPath>(k_NewCellFeatureAttributeMatrixPath_Key);
  inputValues.NumFeaturesPerParentPath = parentAM.createChildPath(filterArgs.value<StringParameter::ValueType>(k_NumFeaturesPerParentName_Key));
  inputValues.FeatureParentIdsPath = filterArgs.value<DataPath>(k_FeatureParentIdsPath_Key);

  inputValues.CalcBiasedAvg = filterArgs.value<bool>(k_CalcBiasedAvg_Key);
  if(inputValues.CalcBiasedAvg)
  {
    inputValues.AvgCAxisMisalignmentsPath = filterArgs.value<DataPath>(k_AvgCAxisMisalignmentsPath_Key);
    inputValues.LocalCAxisMisalignmentsPath = parentAM.createChildPath(filterArgs.value<StringParameter::ValueType>(k_LocalCAxisMisalignmentsName_Key));
  }
  else
  {
    inputValues.AvgCAxisMisalignmentsPath = k_AvgCAxisMisalignmentsPath;
    inputValues.LocalCAxisMisalignmentsPath = k_LocalCAxisMisalignmentsPath;
  }

  inputValues.CalcUnbiasedAvg = filterArgs.value<bool>(k_CalcUnbiasedAvg_Key);
  if(inputValues.CalcUnbiasedAvg)
  {
    inputValues.NeighborListPath = filterArgs.value<DataPath>(k_NeighborListPath_Key);
    inputValues.CAxisMisalignmentListPath = filterArgs.value<DataPath>(k_CAxisMisalignmentListPath_Key);
    inputValues.UnbiasedLocalCAxisMisalignmentsPath = parentAM.createChildPath(filterArgs.value<StringParameter::ValueType>(k_UnbiasedLocalCAxisMisalignmentsName_Key));
  }
  else
  {
    inputValues.NeighborListPath = k_NeighborListPath;
    inputValues.CAxisMisalignmentListPath = k_CAxisMisalignmentListPath;
    inputValues.UnbiasedLocalCAxisMisalignmentsPath = k_UnbiasedLocalCAxisMisalignmentsPath;
  }

  return ComputeLocalAverageCAxisMisalignments(dataStructure, messageHandler, shouldCancel, &inputValues)();
}

namespace
{
namespace SIMPL
{
constexpr StringLiteral k_AvgCAxisMisalignmentsArrayPathKey = "AvgCAxisMisalignmentsArrayPath";
constexpr StringLiteral k_CAxisMisalignmentListArrayPathKey = "CAxisMisalignmentListArrayPath";
constexpr StringLiteral k_CalcBiasedAvgKey = "CalcBiasedAvg";
constexpr StringLiteral k_CalcUnbiasedAvgKey = "CalcUnbiasedAvg";
constexpr StringLiteral k_FeatureParentIdsArrayPathKey = "FeatureParentIdsArrayPath";
constexpr StringLiteral k_LocalCAxisMisalignmentsArrayNameKey = "LocalCAxisMisalignmentsArrayName";
constexpr StringLiteral k_NeighborListArrayPathKey = "NeighborListArrayPath";
constexpr StringLiteral k_NumFeaturesPerParentArrayNameKey = "NumFeaturesPerParentArrayName";
constexpr StringLiteral k_NewCellFeatureAttributeMatrixNameKey = "NewCellFeatureAttributeMatrixName";
constexpr StringLiteral k_UnbiasedLocalCAxisMisalignmentsArrayNameKey = "UnbiasedLocalCAxisMisalignmentsArrayName";
} // namespace SIMPL
} // namespace

Result<Arguments> ComputeLocalAverageCAxisMisalignmentsFilter::FromSIMPLJson(const nlohmann::json& json)
{
  Arguments args = ComputeLocalAverageCAxisMisalignmentsFilter().getDefaultArguments();

  std::vector<Result<>> results;

  results.push_back(
      SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_AvgCAxisMisalignmentsArrayPathKey, k_AvgCAxisMisalignmentsPath_Key));
  results.push_back(
      SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_CAxisMisalignmentListArrayPathKey, k_CAxisMisalignmentListPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedBooleanFilterParameterConverter>(args, json, SIMPL::k_CalcBiasedAvgKey, k_CalcBiasedAvg_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedBooleanFilterParameterConverter>(args, json, SIMPL::k_CalcUnbiasedAvgKey, k_CalcUnbiasedAvg_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_FeatureParentIdsArrayPathKey, k_FeatureParentIdsPath_Key));
  results.push_back(
      SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedPathCreationFilterParameterConverter>(args, json, SIMPL::k_LocalCAxisMisalignmentsArrayNameKey, k_LocalCAxisMisalignmentsName_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_NeighborListArrayPathKey, k_NeighborListPath_Key));
  results.push_back(
      SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedPathCreationFilterParameterConverter>(args, json, SIMPL::k_NumFeaturesPerParentArrayNameKey, k_NumFeaturesPerParentName_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataContainerSelectionFilterParameterConverter>(args, json, SIMPL::k_NewCellFeatureAttributeMatrixNameKey,
                                                                                                                       k_NewCellFeatureAttributeMatrixPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedPathCreationFilterParameterConverter>(args, json, SIMPL::k_UnbiasedLocalCAxisMisalignmentsArrayNameKey,
                                                                                                                   k_UnbiasedLocalCAxisMisalignmentsName_Key));

  Result<> conversionResult = MergeResults(std::move(results));

  return ConvertResultTo<Arguments>(std::move(conversionResult), std::move(args));
}
} // namespace nx::core
