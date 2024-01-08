#include "FindMicroTextureRegionsFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/FindMicroTextureRegions.hpp"

#include "simplnx/DataStructure/AttributeMatrix.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/DataGroupSelectionParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/GeometrySelectionParameter.hpp"

using namespace nx::core;

namespace nx::core
{
//------------------------------------------------------------------------------
std::string FindMicroTextureRegionsFilter::name() const
{
  return FilterTraits<FindMicroTextureRegionsFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string FindMicroTextureRegionsFilter::className() const
{
  return FilterTraits<FindMicroTextureRegionsFilter>::className;
}

//------------------------------------------------------------------------------
Uuid FindMicroTextureRegionsFilter::uuid() const
{
  return FilterTraits<FindMicroTextureRegionsFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string FindMicroTextureRegionsFilter::humanName() const
{
  return "Find MicroTexture Regions";
}

//------------------------------------------------------------------------------
std::vector<std::string> FindMicroTextureRegionsFilter::defaultTags() const
{
  return {className(), "Statistics", "Morphological"};
}

//------------------------------------------------------------------------------
Parameters FindMicroTextureRegionsFilter::parameters() const
{
  Parameters params;

  // Create the parameter descriptors that are needed for this filter
  params.insertSeparator(Parameters::Separator{"Required Input Data"});
  params.insert(std::make_unique<GeometrySelectionParameter>(k_ImageGeomPath_Key, "Selected Image Geometry", "", DataPath{}, GeometrySelectionParameter::AllowedTypes{IGeometry::Type::Image}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_FeatureIdsArrayPath_Key, "FeatureIds", "", DataPath{}, nx::core::GetAllDataTypes()));

  params.insertSeparator(Parameters::Separator{"Created Data Objects"});
  params.insert(std::make_unique<DataGroupSelectionParameter>(k_CellFeatureAttributeMatrixName_Key, "Parent Cell Feature Attribute Matrix", "", DataPath{},
                                                              DataGroupSelectionParameter::AllowedTypes{BaseGroup::GroupType::AttributeMatrix}));
  params.insert(std::make_unique<DataObjectNameParameter>(k_MicroTextureRegionNumCellsArrayName_Key, "Micro Texture Region Number of Cells Array Name", "", "MT Region Number of Cells"));
  params.insert(std::make_unique<DataObjectNameParameter>(k_MicroTextureRegionFractionOccupiedArrayName_Key, "Micro Texture Region Fraction Occupied Array Name", "", "MT Region Fraction Occupied"));

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer FindMicroTextureRegionsFilter::clone() const
{
  return std::make_unique<FindMicroTextureRegionsFilter>();
}

//------------------------------------------------------------------------------
IFilter::PreflightResult FindMicroTextureRegionsFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                      const std::atomic_bool& shouldCancel) const
{
  auto pFeatureIdsArrayPathValue = filterArgs.value<DataPath>(k_FeatureIdsArrayPath_Key);
  auto pCellFeatureAttributeMatrixNameValue = filterArgs.value<DataPath>(k_CellFeatureAttributeMatrixName_Key);
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
Result<> FindMicroTextureRegionsFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                                    const std::atomic_bool& shouldCancel) const
{
  FindMicroTextureRegionsInputValues inputValues;

  inputValues.ImageGeomPath = filterArgs.value<DataPath>(k_ImageGeomPath_Key);
  inputValues.FeatureIdsArrayPath = filterArgs.value<DataPath>(k_FeatureIdsArrayPath_Key);
  inputValues.CellFeatureAttributeMatrixPath = filterArgs.value<DataPath>(k_CellFeatureAttributeMatrixName_Key);
  inputValues.MicroTextureRegionNumCellsArrayPath = inputValues.CellFeatureAttributeMatrixPath.createChildPath(filterArgs.value<std::string>(k_MicroTextureRegionNumCellsArrayName_Key));
  inputValues.MicroTextureRegionFractionOccupiedArrayPath =
      inputValues.CellFeatureAttributeMatrixPath.createChildPath(filterArgs.value<std::string>(k_MicroTextureRegionFractionOccupiedArrayName_Key));

  return FindMicroTextureRegions(dataStructure, messageHandler, shouldCancel, &inputValues)();
}
} // namespace nx::core
