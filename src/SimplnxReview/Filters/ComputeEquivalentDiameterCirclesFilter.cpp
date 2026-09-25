#include "ComputeEquivalentDiameterCirclesFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/ComputeEquivalentDiameterCircles.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Filter/Actions/CreateGeometry1DAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/DataGroupCreationParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"

using namespace nx::core;

namespace nx::core
{
//------------------------------------------------------------------------------
std::string ComputeEquivalentDiameterCirclesFilter::name() const
{
  return FilterTraits<ComputeEquivalentDiameterCirclesFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string ComputeEquivalentDiameterCirclesFilter::className() const
{
  return FilterTraits<ComputeEquivalentDiameterCirclesFilter>::className;
}

//------------------------------------------------------------------------------
Uuid ComputeEquivalentDiameterCirclesFilter::uuid() const
{
  return FilterTraits<ComputeEquivalentDiameterCirclesFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string ComputeEquivalentDiameterCirclesFilter::humanName() const
{
  return "Compute Equivalent Diameter Circles";
}

//------------------------------------------------------------------------------
std::vector<std::string> ComputeEquivalentDiameterCirclesFilter::defaultTags() const
{
  return {className()};
}

//------------------------------------------------------------------------------
Parameters ComputeEquivalentDiameterCirclesFilter::parameters() const
{
  Parameters params;
  // Create the parameter descriptors that are needed for this filter
  params.insertSeparator(Parameters::Separator{"Input Parameter(s)"});
  params.insert(std::make_unique<ArraySelectionParameter>(k_CentroidsArrayPath_Key, "Feature Centroids", "X, Y, Z coordinates of Feature center of mass", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::float32}, ArraySelectionParameter::AllowedComponentShapes{{3}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_EquivalentDiametersArrayPath_Key, "Equivalent Diameters", "Input feature based Equivalent Diameters", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::float32}, ArraySelectionParameter::AllowedComponentShapes{{1}}));
  params.insert(std::make_unique<NumberParameter<uint64>>(k_CircleResolution_Key, "Circle Resolution", "The number of edges that each circle will have", 100));
  params.insert(std::make_unique<NumberParameter<int64>>(k_ZPlane_Key, "Z Plane", "The Z plane that the circles will be calculated on.", 0));

  params.insertSeparator(Parameters::Separator{"Output Edge Geometry"});
  params.insert(std::make_unique<DataGroupCreationParameter>(k_OutputEdgeGeometryPath_Key, "Created Edge Geometry", "The name of the created Edge Geometry", DataPath({"Circles"})));
  params.insert(std::make_unique<DataObjectNameParameter>(k_EdgeAttributeMatrixName_Key, "Edge Attribute Matrix", "Attribute Matrix to store information about the created edges", "Edge Data"));
  params.insert(std::make_unique<DataObjectNameParameter>(k_CreatedFeatureIdsArrayName_Key, "Edge Feature Ids", "Identifies the Feature Id to which each edge belongs", "Feature Ids"));

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer ComputeEquivalentDiameterCirclesFilter::clone() const
{
  return std::make_unique<ComputeEquivalentDiameterCirclesFilter>();
}

//------------------------------------------------------------------------------
IFilter::VersionType ComputeEquivalentDiameterCirclesFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::PreflightResult ComputeEquivalentDiameterCirclesFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                               const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  auto pCentroidsArrayPath = filterArgs.value<DataPath>(k_CentroidsArrayPath_Key);
  auto pEquivalentDiametersArrayPath = filterArgs.value<DataPath>(k_EquivalentDiametersArrayPath_Key);
  auto pCircleResolution = filterArgs.value<uint64>(k_CircleResolution_Key);
  auto pOutputEdgeGeometryPath = filterArgs.value<DataGroupCreationParameter::ValueType>(k_OutputEdgeGeometryPath_Key);
  auto pEdgeAttributeMatrixName = filterArgs.value<DataObjectNameParameter::ValueType>(k_EdgeAttributeMatrixName_Key);
  auto pFeatureIdsArrayName = filterArgs.value<DataObjectNameParameter::ValueType>(k_CreatedFeatureIdsArrayName_Key);

  Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  auto& centroidsArray = dataStructure.getDataRefAs<Float32Array>(pCentroidsArrayPath);
  auto& equivalentDiametersArray = dataStructure.getDataRefAs<Float32Array>(pEquivalentDiametersArrayPath);
  usize numberOfCentroids = centroidsArray.getNumberOfTuples();
  if(numberOfCentroids != equivalentDiametersArray.getNumberOfTuples())
  {
    return MakePreflightErrorResult(-13860, fmt::format("Centroids array has {} centroids, and the equivalent diameters array has {} diameters.  These must be equal.", numberOfCentroids,
                                                        equivalentDiametersArray.getNumberOfTuples()));
  }

  auto createGeometryAction =
      std::make_unique<CreateEdgeGeometryAction>(pOutputEdgeGeometryPath, pCircleResolution * (numberOfCentroids - 1), (pCircleResolution + 1) * (numberOfCentroids - 1),
                                                 INodeGeometry0D::k_VertexAttributeMatrixName, pEdgeAttributeMatrixName, EdgeGeom::k_SharedVertexListName, EdgeGeom::k_SharedEdgeListName);
  resultOutputActions.value().appendAction(std::move(createGeometryAction));

  DataPath path = pOutputEdgeGeometryPath.createChildPath(pEdgeAttributeMatrixName).createChildPath(pFeatureIdsArrayName);
  auto createArray = std::make_unique<CreateArrayAction>(DataType::int32, std::vector<usize>{pCircleResolution * (numberOfCentroids - 1)}, std::vector<usize>{1}, path);
  resultOutputActions.value().appendAction(std::move(createArray));

  preflightUpdatedValues.push_back({"WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});
  resultOutputActions.warnings().push_back({-65432, "WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});

  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> ComputeEquivalentDiameterCirclesFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                                             const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  ComputeEquivalentDiameterCirclesInputValues inputValues;

  inputValues.CentroidsArrayPath = filterArgs.value<DataPath>(k_CentroidsArrayPath_Key);
  inputValues.EquivalentDiametersArrayPath = filterArgs.value<DataPath>(k_EquivalentDiametersArrayPath_Key);
  inputValues.CircleResolution = filterArgs.value<uint64>(k_CircleResolution_Key);
  inputValues.ZPlane = filterArgs.value<int64>(k_ZPlane_Key);
  inputValues.OutputEdgeGeometryPath = filterArgs.value<DataGroupCreationParameter::ValueType>(k_OutputEdgeGeometryPath_Key);
  inputValues.EdgeAttributeMatrixName = filterArgs.value<DataObjectNameParameter::ValueType>(k_EdgeAttributeMatrixName_Key);
  inputValues.FeatureIdsArrayName = filterArgs.value<DataObjectNameParameter::ValueType>(k_CreatedFeatureIdsArrayName_Key);

  return ComputeEquivalentDiameterCircles(dataStructure, messageHandler, shouldCancel, &inputValues)();
}
} // namespace nx::core
