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
  params.insert(std::make_unique<Float32Parameter>(k_AxisTolerance_Key, "Axis Tolerance (Degrees)", "The Axis tolerance to use", 0.0f));
  params.insert(std::make_unique<Float32Parameter>(k_AngleTolerance_Key, "Angle Tolerance (Degrees)", "The Angle tolerance in Degrees", 0.0f));

  params.insertSeparator(Parameters::Separator{"Input Cell Data"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_FeatureIdsArrayPath_Key, "Cell Feature Ids", "Data Array that specifies to which Feature each Element belongs", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::int32}, ArraySelectionParameter::AllowedComponentShapes{{1}}));

  params.insert(std::make_unique<ArraySelectionParameter>(k_CellPhasesArrayPath_Key, "Cell Phases", "Specifies to which Ensemble each cell belongs", DataPath({"Phases"}),
                                                          ArraySelectionParameter::AllowedTypes{DataType::int32}, ArraySelectionParameter::AllowedComponentShapes{{1}}));

  params.insertSeparator(Parameters::Separator{"Input Feature Data"});
  params.insert(std::make_unique<NeighborListSelectionParameter>(k_ContiguousNeighborListArrayPath_Key, "Contiguous Neighbor List", "List of contiguous neighbors for each Feature.", DataPath{},
                                                                 NeighborListSelectionParameter::AllowedTypes{DataType::int32}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_FeaturePhasesArrayPath_Key, "Feature Phases", "Feature Phases Data Array", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::int32}, ArraySelectionParameter::AllowedComponentShapes{{1}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_AvgQuatsArrayPath_Key, "Average Quaternions", "Average Quaternions Data Array", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::float32}, ArraySelectionParameter::AllowedComponentShapes{{4}}));

  params.insertSeparator(Parameters::Separator{"Non-Contiguous Neighborhood Option"});
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_UseNonContiguousNeighbors_Key, "Use Non-Contiguous Neighbors", "Use non-contiguous neighborhoods for computations", false));
  params.insert(std::make_unique<NeighborListSelectionParameter>(k_NonContiguousNeighborListArrayPath_Key, "Non-Contiguous Neighborhoods", "Non-Contiguous neighborhoods", DataPath{},
                                                                 NeighborListSelectionParameter::AllowedTypes{DataType::int32}));

  params.insertSeparator(Parameters::Separator{"Input Ensemble Data"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_CrystalStructuresArrayPath_Key, "Crystal Structures", "Enumeration representing the crystal structure for each Ensemble", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{nx::core::DataType::uint32}, ArraySelectionParameter::AllowedComponentShapes{{1}}));

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
IFilter::UniquePointer MergeColoniesFilter::clone() const
{
  return std::make_unique<MergeColoniesFilter>();
}

//------------------------------------------------------------------------------
IFilter::VersionType MergeColoniesFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::PreflightResult MergeColoniesFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler, const std::atomic_bool& shouldCancel,
                                                            const ExecutionContext& executionContext) const
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
  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> MergeColoniesFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                          const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  MergeColoniesInputValues inputValues;

  auto seed = filterArgs.value<uint64>(k_SeedValue_Key);
  if(!filterArgs.value<bool>(k_UseSeed_Key))
  {
    seed = static_cast<std::mt19937_64::result_type>(std::chrono::steady_clock::now().time_since_epoch().count());
  }

  inputValues.SeedValue = seed;
  inputValues.AxisTolerance = filterArgs.value<float32>(k_AxisTolerance_Key);
  inputValues.AngleTolerance = filterArgs.value<float32>(k_AngleTolerance_Key);
  inputValues.FeaturePhasesPath = filterArgs.value<DataPath>(k_FeaturePhasesArrayPath_Key);
  inputValues.AvgQuatsPath = filterArgs.value<DataPath>(k_AvgQuatsArrayPath_Key);
  inputValues.FeatureIdsPath = filterArgs.value<DataPath>(k_FeatureIdsArrayPath_Key);
  inputValues.CellPhasesPath = filterArgs.value<DataPath>(k_CellPhasesArrayPath_Key);
  inputValues.CrystalStructuresPath = filterArgs.value<DataPath>(k_CrystalStructuresArrayPath_Key);
  inputValues.CellParentIdsPath = inputValues.FeatureIdsPath.replaceName(filterArgs.value<std::string>(k_CellParentIdsArrayName_Key));
  inputValues.CellFeatureAMPath = filterArgs.value<DataPath>(k_NewCellFeatureAttributeMatrixName_Key);
  inputValues.FeatureParentIdsPath = inputValues.FeaturePhasesPath.replaceName(filterArgs.value<std::string>(k_FeatureParentIdsArrayName_Key));
  inputValues.ActivePath = inputValues.CellFeatureAMPath.createChildPath(filterArgs.value<DataObjectNameParameter::ValueType>(k_ActiveArrayName_Key));

  inputValues.UseNonContiguousNeighbors = filterArgs.value<bool>(k_UseNonContiguousNeighbors_Key);
  inputValues.NonContiguousNeighborListArrayPath = filterArgs.value<DataPath>(k_NonContiguousNeighborListArrayPath_Key);
  inputValues.ContiguousNeighborListArrayPath = filterArgs.value<DataPath>(k_ContiguousNeighborListArrayPath_Key);

  return MergeColonies(dataStructure, messageHandler, shouldCancel, &inputValues)();
}

namespace
{
namespace SIMPL
{
constexpr StringLiteral k_ActiveArrayNameKey = "ActiveArrayName";
constexpr StringLiteral k_AngleToleranceKey = "AngleTolerance";
constexpr StringLiteral k_AvgQuatsArrayPathKey = "AvgQuatsArrayPath";
constexpr StringLiteral k_AxisToleranceKey = "AxisTolerance";
constexpr StringLiteral k_CellParentIdsArrayNameKey = "CellParentIdsArrayName";
constexpr StringLiteral k_CellPhasesArrayPathKey = "CellPhasesArrayPath";
constexpr StringLiteral k_ContiguousNeighborListArrayPathKey = "ContiguousNeighborListArrayPath";
constexpr StringLiteral k_CrystalStructuresArrayPathKey = "CrystalStructuresArrayPath";
constexpr StringLiteral k_FeatureIdsArrayPathKey = "FeatureIdsArrayPath";
constexpr StringLiteral k_FeatureParentIdsArrayNameKey = "FeatureParentIdsArrayName";
constexpr StringLiteral k_FeaturePhasesArrayPathKey = "FeaturePhasesArrayPath";
constexpr StringLiteral k_NewCellFeatureAttributeMatrixNameKey = "NewCellFeatureAttributeMatrixName";
constexpr StringLiteral k_NonContiguousNeighborListArrayPathKey = "NonContiguousNeighborListArrayPath";
constexpr StringLiteral k_UseNonContiguousNeighborsKey = "UseNonContiguousNeighbors";
} // namespace SIMPL
} // namespace

Result<Arguments> MergeColoniesFilter::FromSIMPLJson(const nlohmann::json& json)
{
  Arguments args = MergeColoniesFilter().getDefaultArguments();

  std::vector<Result<>> results;

  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedPathCreationFilterParameterConverter>(args, json, SIMPL::k_ActiveArrayNameKey, k_ActiveArrayName_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::FloatFilterParameterConverter<float32>>(args, json, SIMPL::k_AngleToleranceKey, k_AngleTolerance_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_AvgQuatsArrayPathKey, k_AvgQuatsArrayPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::FloatFilterParameterConverter<float32>>(args, json, SIMPL::k_AxisToleranceKey, k_AxisTolerance_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedPathCreationFilterParameterConverter>(args, json, SIMPL::k_CellParentIdsArrayNameKey, k_CellParentIdsArrayName_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_CellPhasesArrayPathKey, k_CellPhasesArrayPath_Key));
  results.push_back(
      SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_ContiguousNeighborListArrayPathKey, k_ContiguousNeighborListArrayPath_Key));
  results.push_back(
      SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_CrystalStructuresArrayPathKey, k_CrystalStructuresArrayPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_FeatureIdsArrayPathKey, k_FeatureIdsArrayPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedPathCreationFilterParameterConverter>(args, json, SIMPL::k_FeatureParentIdsArrayNameKey, k_FeatureParentIdsArrayName_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_FeaturePhasesArrayPathKey, k_FeaturePhasesArrayPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataContainerCreationFilterParameterConverter>(args, json, SIMPL::k_NewCellFeatureAttributeMatrixNameKey,
                                                                                                                      k_NewCellFeatureAttributeMatrixName_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_NonContiguousNeighborListArrayPathKey,
                                                                                                                   k_NonContiguousNeighborListArrayPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedBooleanFilterParameterConverter>(args, json, SIMPL::k_UseNonContiguousNeighborsKey, k_UseNonContiguousNeighbors_Key));

  Result<> conversionResult = MergeResults(std::move(results));

  return ConvertResultTo<Arguments>(std::move(conversionResult), std::move(args));
}
} // namespace nx::core
