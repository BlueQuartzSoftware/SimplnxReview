#include "FindLocalAverageCAxisMisalignmentsFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/FindLocalAverageCAxisMisalignments.hpp"

#include "simplnx/DataStructure/AttributeMatrix.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Filter/Actions/DeleteDataAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataGroupSelectionParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/NeighborListSelectionParameter.hpp"

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
std::string FindLocalAverageCAxisMisalignmentsFilter::name() const
{
  return FilterTraits<FindLocalAverageCAxisMisalignmentsFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string FindLocalAverageCAxisMisalignmentsFilter::className() const
{
  return FilterTraits<FindLocalAverageCAxisMisalignmentsFilter>::className;
}

//------------------------------------------------------------------------------
Uuid FindLocalAverageCAxisMisalignmentsFilter::uuid() const
{
  return FilterTraits<FindLocalAverageCAxisMisalignmentsFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string FindLocalAverageCAxisMisalignmentsFilter::humanName() const
{
  return "Find Local Average C-Axis Misalignments";
}

//------------------------------------------------------------------------------
std::vector<std::string> FindLocalAverageCAxisMisalignmentsFilter::defaultTags() const
{
  return {className(), "Statistics", "Crystallography"};
}

//------------------------------------------------------------------------------
Parameters FindLocalAverageCAxisMisalignmentsFilter::parameters() const
{
  Parameters params;

  // Create the parameter descriptors that are needed for this filter
  params.insertSeparator(Parameters::Separator{"Required Selections"});
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_CalcUnbiasedAvg_Key, "Calculate Unbiased Local C-Axis Misalignments", "", false));
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_CalcBiasedAvg_Key, "Calculate Local C-Axis Misalignments", "", false));

  params.insertSeparator(Parameters::Separator{"Required Input Feature Data"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_FeatureParentIdsPath_Key, "Feature Parent Ids", "", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::int32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{1}}));
  params.insert(std::make_unique<NeighborListSelectionParameter>(k_NeighborListPath_Key, "Neighbor List", "", DataPath{}, NeighborListSelectionParameter::AllowedTypes{DataType::int32}));
  params.insert(
      std::make_unique<NeighborListSelectionParameter>(k_CAxisMisalignmentListPath_Key, "C-Axis Misalignment List", "", DataPath{}, NeighborListSelectionParameter::AllowedTypes{DataType::float32}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_AvgCAxisMisalignmentsPath_Key, "Average C-Axis Misalignments", "", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::int32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{1}}));

  params.insertSeparator(Parameters::Separator{"Created Output Data"});
  params.insert(std::make_unique<DataGroupSelectionParameter>(k_NewCellFeatureAttributeMatrixPath_Key, "New Cell Feature Attribute Matrix Name", "", DataPath{},
                                                              DataGroupSelectionParameter::AllowedTypes{BaseGroup::GroupType::AttributeMatrix}));
  params.insert(std::make_unique<DataObjectNameParameter>(k_NumFeaturesPerParentName_Key, "Number of Features Per Parent Array Name", "", "NumFeaturesPerParent"));
  params.insert(std::make_unique<DataObjectNameParameter>(k_LocalCAxisMisalignmentsName_Key, "Local C-Axis Misalignments Array Name", "", "LocalCAxisMisalignments"));
  params.insert(std::make_unique<DataObjectNameParameter>(k_UnbiasedLocalCAxisMisalignmentsName_Key, "Unbiased Local CAxis Misalignments Array Name", "", "UnbiasedLocalCAxisMisalignments"));

  // Associate the Linkable Parameter(s) to the children parameters that they control
  params.linkParameters(k_CalcBiasedAvg_Key, k_LocalCAxisMisalignmentsName_Key, true);
  params.linkParameters(k_CalcBiasedAvg_Key, k_AvgCAxisMisalignmentsPath_Key, true);

  params.linkParameters(k_CalcUnbiasedAvg_Key, k_UnbiasedLocalCAxisMisalignmentsName_Key, true);
  params.linkParameters(k_CalcUnbiasedAvg_Key, k_NeighborListPath_Key, true);
  params.linkParameters(k_CalcUnbiasedAvg_Key, k_CAxisMisalignmentListPath_Key, true);

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer FindLocalAverageCAxisMisalignmentsFilter::clone() const
{
  return std::make_unique<FindLocalAverageCAxisMisalignmentsFilter>();
}

//------------------------------------------------------------------------------
IFilter::PreflightResult FindLocalAverageCAxisMisalignmentsFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                                 const std::atomic_bool& shouldCancel) const
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
    return MakePreflightErrorResult(-43160, "Since both Calculate Local C-Axis Misalignments and Calculate Unbiased Local C-Axis Misalignments are false, nothing will be done in this filter, "
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
Result<> FindLocalAverageCAxisMisalignmentsFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                                               const std::atomic_bool& shouldCancel) const
{
  FindLocalAverageCAxisMisalignmentsInputValues inputValues;

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

  return FindLocalAverageCAxisMisalignments(dataStructure, messageHandler, shouldCancel, &inputValues)();
}
} // namespace nx::core
