#include "GlavicicTortureFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/GlavicicTorture.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/Geometry/ImageGeom.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/GeometrySelectionParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"

#include <fmt/format.h>

#include <chrono>
#include <random>

using namespace nx::core;

namespace nx::core
{
//------------------------------------------------------------------------------
std::string GlavicicTortureFilter::name() const
{
  return FilterTraits<GlavicicTortureFilter>::name.str();
}

//------------------------------------------------------------------------------
std::string GlavicicTortureFilter::className() const
{
  return FilterTraits<GlavicicTortureFilter>::className;
}

//------------------------------------------------------------------------------
Uuid GlavicicTortureFilter::uuid() const
{
  return FilterTraits<GlavicicTortureFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string GlavicicTortureFilter::humanName() const
{
  return "Glavicic Torture";
}

//------------------------------------------------------------------------------
std::vector<std::string> GlavicicTortureFilter::defaultTags() const
{
  return {className(), "Experimental"};
}

//------------------------------------------------------------------------------
Parameters GlavicicTortureFilter::parameters() const
{
  Parameters params;

  params.insertSeparator(Parameters::Separator{"Optional Parameters"});
  params.insertLinkableParameter(std::make_unique<BoolParameter>(k_UseSeed_Key, "Use Seed for Random Generation", "When true the user will be able to put in a seed for random generation", false));
  params.insert(std::make_unique<NumberParameter<uint64>>(k_SeedValue_Key, "Seed", "The seed fed into the random generator", std::mt19937::default_seed));
  params.linkParameters(k_UseSeed_Key, k_SeedValue_Key, true);

  params.insertSeparator(Parameters::Separator{"Input Data Objects"});
  params.insert(std::make_unique<GeometrySelectionParameter>(k_ImageGeometryPath_Key, "Image Geometry", "The input Image Geometry", DataPath{},
                                                             GeometrySelectionParameter::AllowedTypes{IGeometry::Type::Image}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_ExpectedMTRIdsArrayPath_Key, "Expected MTR Ids", "The expected MTR Ids for each cell", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::uint8}, ArraySelectionParameter::AllowedComponentShapes{{1}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_Group0ArrayPath_Key, "Group 0", "The Group 0 Euler Angles", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::float32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{3}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_Group1ArrayPath_Key, "Group 1", "The Group 1 Euler Angles", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::float32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{3}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_Group2ArrayPath_Key, "Group 2", "The Group 2 Euler Angles", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::float32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{3}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_Group3ArrayPath_Key, "Group 3", "The Group 3 Euler Angles", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::float32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{3}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_Group4ArrayPath_Key, "Group 4", "The Group 4 Euler Angles", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::float32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{3}}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_Group6ArrayPath_Key, "Group 6", "The Group 6 Euler Angles", DataPath{}, ArraySelectionParameter::AllowedTypes{DataType::float32},
                                                          ArraySelectionParameter::AllowedComponentShapes{{3}}));

  params.insertSeparator(Parameters::Separator{"Output Cell Data"});
  params.insert(std::make_unique<DataObjectNameParameter>(k_EulerAnglesArrayName_Key, "Euler Angles", "The name of the created cell level Euler Angles data array", "EulerAngles"));

  return params;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer GlavicicTortureFilter::clone() const
{
  return std::make_unique<GlavicicTortureFilter>();
}

//------------------------------------------------------------------------------
IFilter::VersionType GlavicicTortureFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::PreflightResult GlavicicTortureFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                              const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  auto pImageGeometryPathValue = filterArgs.value<DataPath>(k_ImageGeometryPath_Key);
  auto pExpectedMTRIdsArrayPathValue = filterArgs.value<DataPath>(k_ExpectedMTRIdsArrayPath_Key);
  auto pEulerAnglesArrayNameValue = filterArgs.value<std::string>(k_EulerAnglesArrayName_Key);

  nx::core::Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  const auto& imageGeom = dataStructure.getDataRefAs<ImageGeom>(pImageGeometryPathValue);
  const auto& expectedMTRIdsRef = dataStructure.getDataRefAs<UInt8Array>(pExpectedMTRIdsArrayPathValue);
  usize numCells = imageGeom.getNumberOfCells();
  if(expectedMTRIdsRef.getNumberOfTuples() != numCells)
  {
    return {MakeErrorResult<OutputActions>(-64521, fmt::format("Expected MTR Ids array '{}' has {} tuples but the Image Geometry '{}' has {} cells", pExpectedMTRIdsArrayPathValue.toString(),
                                                               expectedMTRIdsRef.getNumberOfTuples(), pImageGeometryPathValue.toString(), numCells))};
  }

  // The output Euler Angles array is created next to the input Expected MTR Ids array so it lands in the same Attribute Matrix
  DataPath eulerAnglesPath = pExpectedMTRIdsArrayPathValue.replaceName(pEulerAnglesArrayNameValue);
  {
    auto createArrayAction = std::make_unique<CreateArrayAction>(DataType::float32, expectedMTRIdsRef.getTupleShape(), std::vector<usize>{3}, eulerAnglesPath);
    resultOutputActions.value().appendAction(std::move(createArrayAction));
  }

  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> GlavicicTortureFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                            const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  auto seed = filterArgs.value<std::mt19937_64::result_type>(k_SeedValue_Key);
  if(!filterArgs.value<bool>(k_UseSeed_Key))
  {
    seed = static_cast<std::mt19937_64::result_type>(std::chrono::steady_clock::now().time_since_epoch().count());
  }

  GlavicicTortureInputValues inputValues;

  inputValues.Seed = seed;
  inputValues.ExpectedMTRIdsPath = filterArgs.value<DataPath>(k_ExpectedMTRIdsArrayPath_Key);
  inputValues.GroupEulerAnglesPaths = {filterArgs.value<DataPath>(k_Group0ArrayPath_Key), filterArgs.value<DataPath>(k_Group1ArrayPath_Key), filterArgs.value<DataPath>(k_Group2ArrayPath_Key),
                                       filterArgs.value<DataPath>(k_Group3ArrayPath_Key), filterArgs.value<DataPath>(k_Group4ArrayPath_Key), filterArgs.value<DataPath>(k_Group6ArrayPath_Key)};

  inputValues.EulerAnglesPath = inputValues.ExpectedMTRIdsPath.replaceName(filterArgs.value<std::string>(k_EulerAnglesArrayName_Key));

  return GlavicicTorture(dataStructure, messageHandler, shouldCancel, &inputValues)();
}
} // namespace nx::core
