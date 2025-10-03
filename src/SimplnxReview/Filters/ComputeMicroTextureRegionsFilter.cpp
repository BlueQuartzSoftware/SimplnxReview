#include "ComputeMicroTextureRegionsFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/ComputeMicroTextureRegions.hpp"

#include "simplnx/DataStructure/AttributeMatrix.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/DataGroupCreationParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/GeometrySelectionParameter.hpp"
#include "simplnx/Utilities/SIMPLConversion.hpp"

using namespace nx::core;

namespace nx::core
{
//------------------------------------------------------------------------------
std::string ComputeMicroTextureRegionsFilter::name() const
{
  return FilterTraits<ComputeMicroTextureRegionsFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string ComputeMicroTextureRegionsFilter::className() const
{
  return FilterTraits<ComputeMicroTextureRegionsFilter>::className;
}

//------------------------------------------------------------------------------
Uuid ComputeMicroTextureRegionsFilter::uuid() const
{
  return FilterTraits<ComputeMicroTextureRegionsFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string ComputeMicroTextureRegionsFilter::humanName() const
{
  return "Compute MicroTexture Regions";
}

//------------------------------------------------------------------------------
std::vector<std::string> ComputeMicroTextureRegionsFilter::defaultTags() const
{
  return {className(), "Statistics", "Morphological"};
}

//------------------------------------------------------------------------------
Parameters ComputeMicroTextureRegionsFilter::parameters() const
{
  Parameters params;

  // Create the parameter descriptors that are needed for this filter
  params.insertSeparator(Parameters::Separator{"Input Data"});
  params.insert(
      std::make_unique<GeometrySelectionParameter>(k_ImageGeomPath_Key, "Image Geometry", "The selected image geometry", DataPath{}, GeometrySelectionParameter::AllowedTypes{IGeometry::Type::Image}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_FeatureIdsArrayPath_Key, "Cell Feature Ids", "Data Array that specifies to which Feature each Element belongs", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::int32}, ArraySelectionParameter::AllowedComponentShapes{{1}}));
  params.insert(std::make_unique<DataGroupSelectionParameter>(k_CellFeatureAttributeMatrixPath_Key, "Parent Feature Attribute Matrix", "Input Feature Attribute Matrix for microtexture regions",
                                                              DataPath{}, DataGroupSelectionParameter::AllowedTypes{BaseGroup::GroupType::AttributeMatrix}));

  params.insertSeparator(Parameters::Separator{"Output Feature Data"});
  params.insert(std::make_unique<DataObjectNameParameter>(k_MicroTextureRegionNumCellsArrayName_Key, "Micro Texture Region Number of Cells Array Name",
                                                          "Output Number of cells per microtexture region", "MT Region Number of Cells"));
  params.insert(std::make_unique<DataObjectNameParameter>(k_MicroTextureRegionFractionOccupiedArrayName_Key, "Micro Texture Region Fraction Occupied Array Name",
                                                          "Output Region Fraction occupied data array", "MT Region Fraction Occupied"));

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer ComputeMicroTextureRegionsFilter::clone() const
{
  return std::make_unique<ComputeMicroTextureRegionsFilter>();
}

//------------------------------------------------------------------------------
IFilter::VersionType ComputeMicroTextureRegionsFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::PreflightResult ComputeMicroTextureRegionsFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                         const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  auto pFeatureIdsArrayPathValue = filterArgs.value<DataPath>(k_FeatureIdsArrayPath_Key);
  auto pCellFeatureAttributeMatrixNameValue = filterArgs.value<DataPath>(k_CellFeatureAttributeMatrixPath_Key);
  auto pMicroTextureRegionNumCellsArrayNameValue = filterArgs.value<std::string>(k_MicroTextureRegionNumCellsArrayName_Key);
  auto pMicroTextureRegionFractionOccupiedArrayNameValue = filterArgs.value<std::string>(k_MicroTextureRegionFractionOccupiedArrayName_Key);

  nx::core::Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  auto* cellFeatureAM = dataStructure.getDataAs<AttributeMatrix>(pCellFeatureAttributeMatrixNameValue);
  {
    DataPath createArrayDataPath = pCellFeatureAttributeMatrixNameValue.createChildPath(pMicroTextureRegionFractionOccupiedArrayNameValue);
    // Create the face areas DataArray Action and store it into the resultOutputActions
    auto createArrayAction = std::make_unique<CreateArrayAction>(nx::core::DataType::float32, cellFeatureAM->getShape(), std::vector<usize>{1}, createArrayDataPath);
    resultOutputActions.value().appendAction(std::move(createArrayAction));
  }

  {
    DataPath createArrayDataPath = pCellFeatureAttributeMatrixNameValue.createChildPath(pMicroTextureRegionNumCellsArrayNameValue);
    // Create the face areas DataArray Action and store it into the resultOutputActions
    auto createArrayAction = std::make_unique<CreateArrayAction>(nx::core::DataType::int32, cellFeatureAM->getShape(), std::vector<usize>{1}, createArrayDataPath);
    resultOutputActions.value().appendAction(std::move(createArrayAction));
  }

  preflightUpdatedValues.push_back({"WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});
  resultOutputActions.warnings().push_back({-65432, "WARNING: This filter is experimental in nature and has not had any testing, validation or verification. Use at your own risk"});

  // Return both the resultOutputActions and the preflightUpdatedValues via std::move()
  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> ComputeMicroTextureRegionsFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                                       const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  ComputeMicroTextureRegionsInputValues inputValues;

  inputValues.ImageGeomPath = filterArgs.value<DataPath>(k_ImageGeomPath_Key);
  inputValues.FeatureIdsArrayPath = filterArgs.value<DataPath>(k_FeatureIdsArrayPath_Key);
  inputValues.CellFeatureAttributeMatrixPath = filterArgs.value<DataPath>(k_CellFeatureAttributeMatrixPath_Key);
  inputValues.MicroTextureRegionNumCellsArrayPath = inputValues.CellFeatureAttributeMatrixPath.createChildPath(filterArgs.value<std::string>(k_MicroTextureRegionNumCellsArrayName_Key));
  inputValues.MicroTextureRegionFractionOccupiedArrayPath =
      inputValues.CellFeatureAttributeMatrixPath.createChildPath(filterArgs.value<std::string>(k_MicroTextureRegionFractionOccupiedArrayName_Key));

  return ComputeMicroTextureRegions(dataStructure, messageHandler, shouldCancel, &inputValues)();
}

namespace
{
namespace SIMPL
{
constexpr StringLiteral k_CellFeatureAttributeMatrixNameKey = "CellFeatureAttributeMatrixName";
constexpr StringLiteral k_MicroTextureRegionFractionOccupiedArrayNameKey = "MicroTextureRegionFractionOccupiedArrayName";
constexpr StringLiteral k_MicroTextureRegionNumCellsArrayNameKey = "MicroTextureRegionNumCellsArrayName";
constexpr StringLiteral k_FeatureIdsArrayPathKey = "FeatureIdsArrayPath";
} // namespace SIMPL
} // namespace

Result<Arguments> ComputeMicroTextureRegionsFilter::FromSIMPLJson(const nlohmann::json& json)
{
  Arguments args = ComputeMicroTextureRegionsFilter().getDefaultArguments();

  std::vector<Result<>> results;

  results.push_back(
      SIMPLConversion::ConvertParameter<SIMPLConversion::DataContainerSelectionFilterParameterConverter>(args, json, SIMPL::k_CellFeatureAttributeMatrixNameKey, k_CellFeatureAttributeMatrixPath_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedPathCreationFilterParameterConverter>(args, json, SIMPL::k_MicroTextureRegionFractionOccupiedArrayNameKey,
                                                                                                                   k_MicroTextureRegionFractionOccupiedArrayName_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::LinkedPathCreationFilterParameterConverter>(args, json, SIMPL::k_MicroTextureRegionNumCellsArrayNameKey,
                                                                                                                   k_MicroTextureRegionNumCellsArrayName_Key));
  results.push_back(SIMPLConversion::ConvertParameter<SIMPLConversion::DataArraySelectionFilterParameterConverter>(args, json, SIMPL::k_FeatureIdsArrayPathKey, k_FeatureIdsArrayPath_Key));

  Result<> conversionResult = MergeResults(std::move(results));

  return ConvertResultTo<Arguments>(std::move(conversionResult), std::move(args));
}

} // namespace nx::core
