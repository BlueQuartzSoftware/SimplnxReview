#include "ComputeGroupingDensityFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/ComputeGroupingDensity.hpp"

#include "simplnx/DataStructure/AttributeMatrix.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Filter/Actions/CreateNeighborListAction.hpp"
#include "simplnx/Filter/Actions/DeleteDataAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/NeighborListSelectionParameter.hpp"

using namespace nx::core;

namespace
{
const DataPath k_ThrowawayCheckedFeatures = DataPath({"HiddenTempCheckedFeatures"});
const DataPath k_ThrowawayNonContiguous = DataPath({"HiddenContiguousNL"});
} // namespace

namespace nx::core
{
//------------------------------------------------------------------------------
std::string ComputeGroupingDensityFilter::name() const
{
  return FilterTraits<ComputeGroupingDensityFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string ComputeGroupingDensityFilter::className() const
{
  return FilterTraits<ComputeGroupingDensityFilter>::className;
}

//------------------------------------------------------------------------------
Uuid ComputeGroupingDensityFilter::uuid() const
{
  return FilterTraits<ComputeGroupingDensityFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string ComputeGroupingDensityFilter::humanName() const
{
  return "Compute Grouping Densities";
}

//------------------------------------------------------------------------------
std::vector<std::string> ComputeGroupingDensityFilter::defaultTags() const
{
  return {className(), "Statistics", "Reconstruction"};
}

//------------------------------------------------------------------------------
Parameters ComputeGroupingDensityFilter::parameters() const
{
  Parameters params;
  // Create the parameter descriptors that are needed for this filter
  params.insertSeparator(Parameters::Separator{"Input Feature Data"});

  params.insert(std::make_unique<ArraySelectionParameter>(k_VolumesPath_Key, "Volumes", "", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::float32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{1}}));
  params.insert(std::make_unique<NeighborListSelectionParameter>(k_ContiguousNLPath_Key, "Contiguous Neighbor List", "", DataPath{}, NeighborListSelectionParameter::AllowedTypes{DataType::int32}));
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_UseNonContiguousNeighbors_Key, "Use Non-Contiguous Neighbors", "", false));
  params.insert(
      std::make_unique<NeighborListSelectionParameter>(k_NonContiguousNLPath_Key, "Non-Contiguous Neighbor List", "", DataPath{}, NeighborListSelectionParameter::AllowedTypes{DataType::int32}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_ParentIdsPath_Key, "Parent Ids", "", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::int32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{1}}));

  params.insertSeparator(Parameters::Separator{"Input Parent Data"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_ParentVolumesPath_Key, "Parent Volumes", "", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::float32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{1}}));

  params.insertSeparator(Parameters::Separator{"Output Feature Data"});
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_FindCheckedFeatures_Key, "Find Checked Features", "", false));
  params.insert(std::make_unique<DataObjectNameParameter>(k_CheckedFeaturesName_Key, "Checked Features Name", "", "Checked Features"));
  params.insert(std::make_unique<DataObjectNameParameter>(k_GroupingDensitiesName_Key, "Grouping Densities Name", "", "Grouping Densities"));

  // Link params
  params.linkParameters(k_UseNonContiguousNeighbors_Key, k_NonContiguousNLPath_Key, true);
  params.linkParameters(k_FindCheckedFeatures_Key, k_CheckedFeaturesName_Key, true);

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer ComputeGroupingDensityFilter::clone() const
{
  return std::make_unique<ComputeGroupingDensityFilter>();
}

//------------------------------------------------------------------------------
IFilter::VersionType ComputeGroupingDensityFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::PreflightResult ComputeGroupingDensityFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                     const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  auto pParentIdsPath = filterArgs.value<DataPath>(k_ParentIdsPath_Key);
  auto pParentVolumesPath = filterArgs.value<DataPath>(k_ParentVolumesPath_Key);
  auto pContiguousNLPath = filterArgs.value<DataPath>(k_ContiguousNLPath_Key);
  auto pVolumesPath = filterArgs.value<DataPath>(k_VolumesPath_Key);
  auto pGroupingDensitiesName = filterArgs.value<std::string>(k_GroupingDensitiesName_Key);

  auto pUseNonContiguousNeighbors = filterArgs.value<bool>(k_UseNonContiguousNeighbors_Key);
  auto pNonContiguousNLPath = filterArgs.value<DataPath>(k_NonContiguousNLPath_Key);
  auto pFindCheckedFeatures = filterArgs.value<bool>(k_FindCheckedFeatures_Key);
  auto pCheckedFeaturesName = filterArgs.value<std::string>(k_CheckedFeaturesName_Key);

  Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  auto* pParentAM = dataStructure.getDataAs<AttributeMatrix>(pParentVolumesPath.getParent());
  if(pParentAM == nullptr)
  {
    return MakePreflightErrorResult(-15670, fmt::format("Parent Volumes [{}] must be stored in an Attribute Matrix.", pParentVolumesPath.toString()));
  }
  {
    DataPath groupingDataPath = pParentVolumesPath.replaceName(pGroupingDensitiesName);
    auto createArrayAction = std::make_unique<CreateArrayAction>(nx::core::DataType::float32, pParentAM->getShape(), std::vector<usize>{1}, groupingDataPath);
    resultOutputActions.value().appendAction(std::move(createArrayAction));
  }

  auto* pFeatureAM = dataStructure.getDataAs<AttributeMatrix>(pVolumesPath.getParent());
  if(pFeatureAM == nullptr)
  {
    return MakePreflightErrorResult(-15671, fmt::format("Feature Volumes [{}] must be stored in an Attribute Matrix.", pVolumesPath.toString()));
  }

  if(pFindCheckedFeatures)
  {
    DataPath checkedFeaturesPath = pVolumesPath.replaceName(pCheckedFeaturesName);
    {
      auto createArrayAction = std::make_unique<CreateArrayAction>(nx::core::DataType::int32, pFeatureAM->getShape(), std::vector<usize>{1}, checkedFeaturesPath);
      resultOutputActions.value().appendAction(std::move(createArrayAction));
    }
  }
  else
  {
    {
      auto createArrayAction = std::make_unique<CreateArrayAction>(nx::core::DataType::int32, std::vector<usize>{1}, std::vector<usize>{1}, k_ThrowawayCheckedFeatures);
      resultOutputActions.value().appendAction(std::move(createArrayAction));
    }
    {
      auto removeAction = std::make_unique<DeleteDataAction>(k_ThrowawayCheckedFeatures);
      resultOutputActions.value().appendDeferredAction(std::move(removeAction));
    }
  }

  if(!pUseNonContiguousNeighbors)
  {
    {
      auto createArrayAction = std::make_unique<CreateNeighborListAction>(nx::core::DataType::int32, 1, k_ThrowawayNonContiguous);
      resultOutputActions.value().appendAction(std::move(createArrayAction));
    }
    {
      auto removeAction = std::make_unique<DeleteDataAction>(k_ThrowawayNonContiguous);
      resultOutputActions.value().appendDeferredAction(std::move(removeAction));
    }
  }

  preflightUpdatedValues.push_back({"WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});
  resultOutputActions.warnings().push_back({-65432, "WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});

  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> ComputeGroupingDensityFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                                   const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  ComputeGroupingDensityInputValues inputValues;

  inputValues.ParentIdsPath = filterArgs.value<DataPath>(k_ParentIdsPath_Key);
  inputValues.ParentVolumesPath = filterArgs.value<DataPath>(k_ParentVolumesPath_Key);
  inputValues.ContiguousNLPath = filterArgs.value<DataPath>(k_ContiguousNLPath_Key);
  inputValues.VolumesPath = filterArgs.value<DataPath>(k_VolumesPath_Key);
  inputValues.GroupingDensitiesPath = inputValues.ParentVolumesPath.replaceName(filterArgs.value<std::string>(k_GroupingDensitiesName_Key));

  inputValues.UseNonContiguousNeighbors = filterArgs.value<bool>(k_UseNonContiguousNeighbors_Key);
  if(inputValues.UseNonContiguousNeighbors)
  {
    inputValues.NonContiguousNLPath = filterArgs.value<DataPath>(k_NonContiguousNLPath_Key);
  }
  else
  {
    inputValues.NonContiguousNLPath = k_ThrowawayNonContiguous;
  }

  inputValues.FindCheckedFeatures = filterArgs.value<bool>(k_FindCheckedFeatures_Key);
  if(inputValues.FindCheckedFeatures)
  {
    inputValues.CheckedFeaturesPath = inputValues.VolumesPath.replaceName(filterArgs.value<std::string>(k_CheckedFeaturesName_Key));
  }
  else
  {
    inputValues.CheckedFeaturesPath = k_ThrowawayCheckedFeatures;
  }

  return ComputeGroupingDensity(dataStructure, messageHandler, shouldCancel, &inputValues)();
}
} // namespace nx::core
