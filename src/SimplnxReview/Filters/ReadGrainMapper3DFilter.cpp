#include "ReadGrainMapper3DFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/ReadGrainMapper3D.hpp"
#include "SimplnxReview/utils/GrainMapper3DUtilities.hpp"

#include "simplnx/DataStructure/AttributeMatrix.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/Geometry/ImageGeom.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Filter/Actions/CreateDataGroupAction.hpp"
#include "simplnx/Filter/Actions/CreateImageGeometryAction.hpp"
#include "simplnx/Filter/Actions/CreateNeighborListAction.hpp"
#include "simplnx/Filter/Actions/CreateStringArrayAction.hpp"
#include "simplnx/Filter/Actions/DeleteDataAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataGroupCreationParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/FileSystemPathParameter.hpp"
#include "simplnx/Parameters/NeighborListSelectionParameter.hpp"

#include <filesystem>

namespace fs = std::filesystem;

using namespace nx::core;
using namespace GrainMapper3DUtilities;

namespace
{

} // namespace

namespace nx::core
{
//------------------------------------------------------------------------------
std::string ReadGrainMapper3DFilter::name() const
{
  return FilterTraits<ReadGrainMapper3DFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string ReadGrainMapper3DFilter::className() const
{
  return FilterTraits<ReadGrainMapper3DFilter>::className;
}

//------------------------------------------------------------------------------
Uuid ReadGrainMapper3DFilter::uuid() const
{
  return FilterTraits<ReadGrainMapper3DFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string ReadGrainMapper3DFilter::humanName() const
{
  return "Read GrainMapper3D File";
}

//------------------------------------------------------------------------------
std::vector<std::string> ReadGrainMapper3DFilter::defaultTags() const
{
  return {className(), "Reader", "XNovo", "GrainMapper", "HDF5"};
}

//------------------------------------------------------------------------------
Parameters ReadGrainMapper3DFilter::parameters() const
{
  Parameters params;
  // Create the parameter descriptors that are needed for this filter
  params.insertSeparator(Parameters::Separator{"Input Parameter(s)"});
  params.insert(std::make_unique<FileSystemPathParameter>(k_InputFile_Key, "Input File", "The input .hdf5 file path", fs::path("input.h5"), FileSystemPathParameter::ExtensionsType{".h5"},
                                                          FileSystemPathParameter::PathType::InputFile));
  params.insertSeparator(Parameters::Separator{"Output Image Geometry"});
  params.insert(std::make_unique<DataGroupCreationParameter>(k_CreatedImageGeometryPath_Key, "Image Geometry", "The path to the created Image Geometry", DataPath({ImageGeom::k_TypeName})));
  params.insertSeparator(Parameters::Separator{"Output Cell Attribute Matrix"});
  params.insert(std::make_unique<DataObjectNameParameter>(k_CellAttributeMatrixName_Key, "Cell Attribute Matrix", "The name of the cell data attribute matrix for the created Image Geometry",
                                                          ImageGeom::k_CellDataName));
  params.insertSeparator(Parameters::Separator{"Output Ensemble Attribute Matrix"});
  params.insert(std::make_unique<DataObjectNameParameter>(k_CellEnsembleAttributeMatrixName_Key, "Ensemble Attribute Matrix", "The Attribute Matrix where the phase information is stored.",
                                                          "Cell Ensemble Data"));
  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer ReadGrainMapper3DFilter::clone() const
{
  return std::make_unique<ReadGrainMapper3DFilter>();
}

//------------------------------------------------------------------------------
IFilter::PreflightResult ReadGrainMapper3DFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                const std::atomic_bool& shouldCancel) const
{
  auto pInputFileValue = filterArgs.value<FileSystemPathParameter::ValueType>(k_InputFile_Key);
  auto pImageGeometryPath = filterArgs.value<DataPath>(k_CreatedImageGeometryPath_Key);
  auto pCellAttributeMatrixNameValue = filterArgs.value<std::string>(k_CellAttributeMatrixName_Key);
  auto pCellEnsembleAttributeMatrixNameValue = filterArgs.value<std::string>(k_CellEnsembleAttributeMatrixName_Key);

  PreflightResult preflightResult;
  nx::core::Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  GrainMapperReader reader(pInputFileValue.string());
  Result<> result = reader.readHeaderOnly();
  if(result.invalid())
  {
    return MakePreflightErrorResult(-99582, fmt::format("An error occurred while reading the header data"));
  }

  // create the Image Geometry and it's attribute matrices
  const std::vector<usize> dims = reader.getDimensions();;
  {
    CreateImageGeometryAction::SpacingType spacing = reader.getSpacing();
    std::vector<float> origin = reader.getOrigin();

    auto createDataGroupAction = std::make_unique<CreateImageGeometryAction>(pImageGeometryPath, dims, origin, spacing, pCellAttributeMatrixNameValue);
    resultOutputActions.value().appendAction(std::move(createDataGroupAction));
  }

  // Reverse the Image Dimensions
  const std::vector<usize> tupleDims = {dims[2], dims[1], dims[0]};

  // Get the available Data sets
  DataPath cellAMPath = pImageGeometryPath.createChildPath(pCellAttributeMatrixNameValue);

  auto nameToDataTypeMap = reader.getNameToDataTypeMap();
  auto nameToCompDimMap = reader.getNameToCompDimMap();
  auto availableDataSets = reader.getDCTDatasetNames();
  for(const auto& dataSetName : availableDataSets)
  {
    resultOutputActions.value().appendAction(std::make_unique<CreateArrayAction>(nameToDataTypeMap[dataSetName], tupleDims, std::vector<usize>{nameToCompDimMap[dataSetName]}, cellAMPath.createChildPath(dataSetName)));
  }

  // read the phase information



  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> ReadGrainMapper3DFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                              const std::atomic_bool& shouldCancel) const
{
  ReadGrainMapper3DInputValues inputValues;

  inputValues.InputFile = filterArgs.value<FileSystemPathParameter::ValueType>(k_InputFile_Key);
  inputValues.ImageGeometryPath = filterArgs.value<DataPath>(k_CreatedImageGeometryPath_Key);
  inputValues.CellAttributeMatrixName = filterArgs.value<std::string>(k_CellAttributeMatrixName_Key);
  inputValues.CellEnsembleAttributeMatrixName = filterArgs.value<std::string>(k_CellEnsembleAttributeMatrixName_Key);

  return ReadGrainMapper3D(dataStructure, messageHandler, shouldCancel, &inputValues)();
}
} // namespace nx::core
