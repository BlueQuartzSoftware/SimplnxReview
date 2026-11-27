#include "MergeColoniesFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/MergeColonies.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Filter/Actions/CreateAttributeMatrixAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataGroupCreationParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/NeighborListSelectionParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"
#include "simplnx/Utilities/SIMPLConversion.hpp"

#include <random>

using namespace nx::core;

namespace nx::core
{
//------------------------------------------------------------------------------
std::string MergeColoniesFilter::name() const
{
  return FilterTraits<MergeColoniesFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string MergeColoniesFilter::className() const
{
  return FilterTraits<MergeColoniesFilter>::className;
}

//------------------------------------------------------------------------------
Uuid MergeColoniesFilter::uuid() const
{
  return FilterTraits<MergeColoniesFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string MergeColoniesFilter::humanName() const
{
  return "Merge Colonies";
}

//------------------------------------------------------------------------------
std::vector<std::string> MergeColoniesFilter::defaultTags() const
{
  return {"Reconstruction", "Grouping"};
}

//------------------------------------------------------------------------------
Parameters MergeColoniesFilter::parameters() const
{
  Parameters params;

  // Create the parameter descriptors that are needed for this filter
  params.insertSeparator(Parameters::Separator{"Input Parameter(s)"});
  params.insert(std::make_unique<Float32ParameterType>(k_AxisTolerance_Key, "Axis Tolerance (Degrees)", "The Axis tolerance to use", 0.0f));
  params.insert(std::make_unique<Float32ParameterType>(k_AngleTolerance_Key, "Angle Tolerance (Degrees)", "The Angle tolerance in Degrees", 0.0f));

  params.insertSeparator(Parameters::Separator{"Input Cell Data"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_FeatureIdsArrayPath_Key, "Cell Feature Ids", "Data Array that specifies to which Feature each Element belongs", DataPath{},
                                                          ArraySelectionParameter::AllowedTypeSetType{DataType::int32}, ArraySelectionParameter::AllowedComponentShapesType{{1}}));

  params.insert(std::make_unique<ArraySelectionParameter>(k_CellPhasesArrayPath_Key, "Cell Phases", "Specifies to which Ensemble each cell belongs", DataPath({"Phases"}),
                                                          ArraySelectionParameter::AllowedTypeSetType{DataType::int32}, ArraySelectionParameter::AllowedComponentShapesType{{1}}));

  params.insertSeparator(Parameters::Separator{"Input Feature Data"});
  params.insert(std::make_unique<NeighborListSelectionParameter>(k_ContiguousNeighborListArrayPath_Key, "Contiguous Neighbor List", "List of contiguous neighbors for each Feature.", DataPath{},
                                                                 NeighborListSelectionParameter::AllowedTypeSetType{DataType::int32}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_FeaturePhasesArrayPath_Key, "Feature Phases", "Feature Phases Data Array", DataPath{},
                                                          ArraySelectionParameter::AllowedTypeSetType{DataType::int32}, ArraySelectionParameter::AllowedComponentShapesType{{1}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_AvgQuatsArrayPath_Key, "Average Quaternions", "Average Quaternions Data Array", DataPath{},
                                                          ArraySelectionParameter::AllowedTypeSetType{DataType::float32}, ArraySelectionParameter::AllowedComponentShapesType{{4}}));

  params.insertSeparator(Parameters::Separator{"Non-Contiguous Neighborhood Option"});
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_UseNonContiguousNeighbors_Key, "Use Non-Contiguous Neighbors", "Use non-contiguous neighborhoods for computations", false));
  params.insert(std::make_unique<NeighborListSelectionParameter>(k_NonContiguousNeighborListArrayPath_Key, "Non-Contiguous Neighborhoods", "Non-Contiguous neighborhoods", DataPath{},
                                                                 NeighborListSelectionParameter::AllowedTypeSetType{DataType::int32}));

  params.insertSeparator(Parameters::Separator{"Input Ensemble Data"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_CrystalStructuresArrayPath_Key, "Crystal Structures", "Enumeration representing the crystal structure for each Ensemble", DataPath{},
                                                          ArraySelectionParameter::AllowedTypeSetType{nx::core::DataType::uint32}, ArraySelectionParameter::AllowedComponentShapesType{{1}}));

  params.insertSeparator(Parameters::Separator{"Random Number Generation"});
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_UseSeed_Key, "Use Seed for Random Generation", "When true the user will be able to put in a seed for random generation", false));
  params.insert(std::make_unique<NumberParameter<uint64>>(k_SeedValue_Key, "Seed", "The seed fed into the random generator", std::mt19937::default_seed));

  params.insertSeparator(Parameters::Separator{"Output Cell Data"});
  params.insert(std::make_unique<DataObjectNameParameter>(k_CellParentIdsArrayName_Key, "Cell Parent Ids", "Output Cell Parent Ids", "Cell Parent Ids"));

  params.insertSeparator(Parameters::Separator{"Output Feature Data"});
  params.insert(
      std::make_unique<DataGroupCreationParameter>(k_NewCellFeatureAttributeMatrixName_Key, "Feature Attribute Matrix", "Output Feature Attribute Matrix for microtexture regions", DataPath{}));
  params.insert(std::make_unique<DataObjectNameParameter>(k_FeatureParentIdsArrayName_Key, "Feature Parent Ids", "Output Cell level ParentFeatureIds data array", "Feature Parent Ids"));
  params.insert(std::make_unique<DataObjectNameParameter>(k_ActiveArrayName_Key, "Active", "Output Feature data array", "Active Features"));

  // Associate the Linkable Parameter(s) to the children parameters that they control
  params.linkParameters(k_UseNonContiguousNeighbors_Key, k_NonContiguousNeighborListArrayPath_Key, true);
  params.linkParameters(k_UseSeed_Key, k_SeedValue_Key, true);

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointerType MergeColoniesFilter::clone() const
{
  return std::make_unique<MergeColoniesFilter>();
}

//------------------------------------------------------------------------------
IFilter::VersionType MergeColoniesFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::PreflightResult MergeColoniesFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& /*messageHandler*/,
                                                            const std::atomic_bool& /*shouldCancel*/, const ExecutionContext& /*executionContext*/) const
{
  auto pFeaturePhasesPathValue = filterArgs.value<DataPath>(k_FeaturePhasesArrayPath_Key);
  auto pAvgQuatsPathValue = filterArgs.value<DataPath>(k_AvgQuatsArrayPath_Key);
  auto pFeatureIdsPathValue = filterArgs.value<DataPath>(k_FeatureIdsArrayPath_Key);
  auto pCellPhasesPathValue = filterArgs.value<DataPath>(k_CellPhasesArrayPath_Key);
  auto pCrystalStructuresPathValue = filterArgs.value<DataPath>(k_CrystalStructuresArrayPath_Key);
  auto pCellParentIdsNameValue = filterArgs.value<std::string>(k_CellParentIdsArrayName_Key);
  auto pCellFeatureAMPathValue = filterArgs.value<DataPath>(k_NewCellFeatureAttributeMatrixName_Key);
  auto pFeatureParentIdsNameValue = filterArgs.value<std::string>(k_FeatureParentIdsArrayName_Key);
  auto pActiveNameValue = filterArgs.value<DataObjectNameParameter::ValueType>(k_ActiveArrayName_Key);

  PreflightResult preflightResult;
  nx::core::Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  // Create the CreateArray action and add it to the resultOutputActions object
  {
    auto action = std::make_unique<CreateAttributeMatrixAction>(pCellFeatureAMPathValue, std::vector<usize>{0});
    resultOutputActions.value().appendAction(std::move(action));
  }

  // Create the CreateArray action and add it to the resultOutputActions object
  {
    auto action = std::make_unique<CreateArrayAction>(DataType::boolean, std::vector<usize>{0}, std::vector<usize>{1}, pCellFeatureAMPathValue.createChildPath(pActiveNameValue));
    resultOutputActions.value().appendAction(std::move(action));
  }

  // Create the CreateArray action and add it to the resultOutputActions object
  {
    std::vector<usize> tupDims = dataStructure.getDataAs<IArray>(pFeaturePhasesPathValue)->getTupleShape();
    DataPath featureParentIdsPath = pFeaturePhasesPathValue.replaceName(pFeatureParentIdsNameValue);
    auto action = std::make_unique<CreateArrayAction>(DataType::int32, tupDims, std::vector<usize>{1}, featureParentIdsPath);
    resultOutputActions.value().appendAction(std::move(action));
  }

  // Create the CreateArray action and add it to the resultOutputActions object
  {
    std::vector<usize> tupDims = dataStructure.getDataAs<IArray>(pFeatureIdsPathValue)->getTupleShape();
    DataPath cellParentIdsPath = pFeatureIdsPathValue.replaceName(pCellParentIdsNameValue);
    auto action = std::make_unique<CreateArrayAction>(DataType::int32, tupDims, std::vector<usize>{1}, cellParentIdsPath);
    resultOutputActions.value().appendAction(std::move(action));
  }

  preflightUpdatedValues.push_back({"WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});
  resultOutputActions.warnings().push_back({-65432, "WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});

  // Return both the resultOutputActions and the preflightUpdatedValues via std::move()
  return {.output_actions = std::move(resultOutputActions), .output_values = std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> MergeColoniesFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* /*pipelineNode*/, const MessageHandler& messageHandler,
                                          const std::atomic_bool& shouldCancel, const ExecutionContext& /*executionContext*/) const
{
  MergeColoniesInputValues inputValues;

  auto seed = filterArgs.value<uint64>(k_SeedValue_Key);
  if(!filterArgs.value<bool>(k_UseSeed_Key))
  {
    seed = static_cast<std::mt19937_64::result_type>(std::chrono::steady_clock::now().time_since_epoch().count());
  }

  inputValues.seed_value = seed;
  inputValues.axis_tolerance = filterArgs.value<float32>(k_AxisTolerance_Key);
  inputValues.angle_tolerance = filterArgs.value<float32>(k_AngleTolerance_Key);
  inputValues.feature_phases_path = filterArgs.value<DataPath>(k_FeaturePhasesArrayPath_Key);
  inputValues.avg_quats_path = filterArgs.value<DataPath>(k_AvgQuatsArrayPath_Key);
  inputValues.feature_ids_path = filterArgs.value<DataPath>(k_FeatureIdsArrayPath_Key);
  inputValues.cell_phases_path = filterArgs.value<DataPath>(k_CellPhasesArrayPath_Key);
  inputValues.crystal_structures_path = filterArgs.value<DataPath>(k_CrystalStructuresArrayPath_Key);
  inputValues.cell_parent_ids_path = inputValues.feature_ids_path.replaceName(filterArgs.value<std::string>(k_CellParentIdsArrayName_Key));
  inputValues.cell_feature_am_path = filterArgs.value<DataPath>(k_NewCellFeatureAttributeMatrixName_Key);
  inputValues.feature_parent_ids_path = inputValues.feature_phases_path.replaceName(filterArgs.value<std::string>(k_FeatureParentIdsArrayName_Key));
  inputValues.active_path = inputValues.cell_feature_am_path.createChildPath(filterArgs.value<DataObjectNameParameter::ValueType>(k_ActiveArrayName_Key));

  inputValues.use_non_contiguous_neighbors = filterArgs.value<bool>(k_UseNonContiguousNeighbors_Key);
  inputValues.non_contiguous_neighbor_list_array_path = filterArgs.value<DataPath>(k_NonContiguousNeighborListArrayPath_Key);
  inputValues.contiguous_neighbor_list_array_path = filterArgs.value<DataPath>(k_ContiguousNeighborListArrayPath_Key);

  return MergeColonies(dataStructure, messageHandler, shouldCancel, &inputValues)();
}

namespace
{
namespace simpl
{
constexpr StringLiteralType k_ActiveArrayNameKey = "ActiveArrayName";
constexpr StringLiteralType k_AngleToleranceKey = "AngleTolerance";
constexpr StringLiteralType k_AvgQuatsArrayPathKey = "AvgQuatsArrayPath";
constexpr StringLiteralType k_AxisToleranceKey = "AxisTolerance";
constexpr StringLiteralType k_CellParentIdsArrayNameKey = "CellParentIdsArrayName";
constexpr StringLiteralType k_CellPhasesArrayPathKey = "CellPhasesArrayPath";
constexpr StringLiteralType k_ContiguousNeighborListArrayPathKey = "ContiguousNeighborListArrayPath";
constexpr StringLiteralType k_CrystalStructuresArrayPathKey = "CrystalStructuresArrayPath";
constexpr StringLiteralType k_FeatureIdsArrayPathKey = "FeatureIdsArrayPath";
constexpr StringLiteralType k_FeatureParentIdsArrayNameKey = "FeatureParentIdsArrayName";
constexpr StringLiteralType k_FeaturePhasesArrayPathKey = "FeaturePhasesArrayPath";
constexpr StringLiteralType k_NewCellFeatureAttributeMatrixNameKey = "NewCellFeatureAttributeMatrixName";
constexpr StringLiteralType k_NonContiguousNeighborListArrayPathKey = "NonContiguousNeighborListArrayPath";
constexpr StringLiteralType k_UseNonContiguousNeighborsKey = "UseNonContiguousNeighbors";
} // namespace simpl
} // namespace

Result<Arguments> MergeColoniesFilter::FromSIMPLJson(const nlohmann::json& json)
{
  Arguments args = MergeColoniesFilter().getDefaultArguments();

  std::vector<Result<>> results;

  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::LinkedPathCreationFilterParameterConverter>(args, json, simpl::k_ActiveArrayNameKey, k_ActiveArrayName_Key));
  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::FloatFilterParameterConverter<float32>>(args, json, simpl::k_AngleToleranceKey, k_AngleTolerance_Key));
  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::DataArraySelectionFilterParameterConverter>(args, json, simpl::k_AvgQuatsArrayPathKey, k_AvgQuatsArrayPath_Key));
  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::FloatFilterParameterConverter<float32>>(args, json, simpl::k_AxisToleranceKey, k_AxisTolerance_Key));
  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::LinkedPathCreationFilterParameterConverter>(args, json, simpl::k_CellParentIdsArrayNameKey, k_CellParentIdsArrayName_Key));
  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::DataArraySelectionFilterParameterConverter>(args, json, simpl::k_CellPhasesArrayPathKey, k_CellPhasesArrayPath_Key));
  results.push_back(
      simpl_conversion::ConvertParameter<simpl_conversion::DataArraySelectionFilterParameterConverter>(args, json, simpl::k_ContiguousNeighborListArrayPathKey, k_ContiguousNeighborListArrayPath_Key));
  results.push_back(
      simpl_conversion::ConvertParameter<simpl_conversion::DataArraySelectionFilterParameterConverter>(args, json, simpl::k_CrystalStructuresArrayPathKey, k_CrystalStructuresArrayPath_Key));
  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::DataArraySelectionFilterParameterConverter>(args, json, simpl::k_FeatureIdsArrayPathKey, k_FeatureIdsArrayPath_Key));
  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::LinkedPathCreationFilterParameterConverter>(args, json, simpl::k_FeatureParentIdsArrayNameKey, k_FeatureParentIdsArrayName_Key));
  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::DataArraySelectionFilterParameterConverter>(args, json, simpl::k_FeaturePhasesArrayPathKey, k_FeaturePhasesArrayPath_Key));
  results.push_back(simpl_conversion::Convert2Parameters<simpl_conversion::AMPathBuilderFilterParameterConverter>(
      args, json, simpl::k_FeatureIdsArrayPathKey, simpl::k_NewCellFeatureAttributeMatrixNameKey, k_NewCellFeatureAttributeMatrixName_Key));
  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::DataArraySelectionFilterParameterConverter>(args, json, simpl::k_NonContiguousNeighborListArrayPathKey,
                                                                                                                   k_NonContiguousNeighborListArrayPath_Key));
  results.push_back(simpl_conversion::ConvertParameter<simpl_conversion::LinkedBooleanFilterParameterConverter>(args, json, simpl::k_UseNonContiguousNeighborsKey, k_UseNonContiguousNeighbors_Key));

  Result<> conversionResult = MergeResults(std::move(results));

  return ConvertResultTo<Arguments>(std::move(conversionResult), std::move(args));
}
} // namespace nx::core
