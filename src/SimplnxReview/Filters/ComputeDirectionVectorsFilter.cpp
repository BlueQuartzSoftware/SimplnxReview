#include "ComputeDirectionVectorsFilter.hpp"
#include "Algorithms/ComputeDirectionVectors.hpp"

#include "EbsdLib/Core/EbsdDataArray.hpp"
#include "EbsdLib/OrientationMath/OrientationConverter.hpp"

#include "simplnx/DataStructure/IDataArray.hpp"
#include "simplnx/Filter/Actions/CreateArrayAction.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/VectorParameter.hpp"

namespace nx::core
{
namespace
{
// constexpr int32 k_MissingInputArray = -567;
// constexpr int32 k_ComponentCountMismatchError = -90003;
// constexpr int32 k_InvalidNumTuples = -90004;
} // namespace

//------------------------------------------------------------------------------
std::string ComputeDirectionVectorsFilter::name() const
{
  return FilterTraits<ComputeDirectionVectorsFilter>::name;
}

//------------------------------------------------------------------------------
std::string ComputeDirectionVectorsFilter::className() const
{
  return FilterTraits<ComputeDirectionVectorsFilter>::className;
}

//------------------------------------------------------------------------------
Uuid ComputeDirectionVectorsFilter::uuid() const
{
  return FilterTraits<ComputeDirectionVectorsFilter>::uuid;
}

//------------------------------------------------------------------------------
std::string ComputeDirectionVectorsFilter::humanName() const
{
  return "Compute Direction Vectors";
}

//------------------------------------------------------------------------------
std::vector<std::string> ComputeDirectionVectorsFilter::defaultTags() const
{
  return {className(), "Statistics", "SimplnxCore", "Find"};
}

//------------------------------------------------------------------------------
Parameters ComputeDirectionVectorsFilter::parameters() const
{
  Parameters params;

  using OrientationConverterType = ebsdlib::OrientationConverter<EbsdDataArray<float32>, float32>;

  params.insertSeparator(Parameters::Separator{"Input Parameter(s)"});
  params.insert(std::make_unique<ChoicesParameter>(k_InputType_Key, "Input Orientation Type", "Specifies the incoming orientation representation enumeration index", 0,
                                                   OrientationConverterType::GetOrientationTypeStrings<ChoicesParameter::Choices>()));
  params.insert(std::make_unique<ArraySelectionParameter>(k_InputOrientationArrayPath_Key, "Input Orientations", "The complete path to the incoming orientation representation data array", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::float32, DataType::float64}, ArraySelectionParameter::AllowedComponentShapes{{3}, {4}, {9}}));
  params.insertLinkableParameter(std::make_unique<ChoicesParameter>(k_LatticeConstantsInputType_Key, "Lattice Constants Input Type",
                                                                    "Specifies the method that the lattice constants will be input into this filter.", 0,
                                                                    ChoicesParameter::Choices{"Existing Array", "Manual Entry"}));
  params.insert(std::make_unique<ArraySelectionParameter>(k_LatticeConstantsArrayPath_Key, "Lattice Constants",
                                                          "The complete path to the lattice constants data array that will be imported and used by this filter.", DataPath{},
                                                          ArraySelectionParameter::AllowedTypes{DataType::float32}, ArraySelectionParameter::AllowedComponentShapes{{6}}));
  params.insert(std::make_unique<VectorFloat32Parameter>(k_LatticeConstantsLength_Key, "Lattice Constants Lengths (A, B, C)", "The manual lattice constants that will be used by this filter.",
                                                         VectorFloat32Parameter::ValueType{0.0f, 0.0f, 0.0f}, std::vector<std::string>{"a", "b", "c"}));
  params.insert(std::make_unique<VectorFloat32Parameter>(k_LatticeConstantsAngles_Key, "Lattice Constants Angles (Alpha, Beta, Gamma)",
                                                         "The manual lattice constants that will be used by this filter.", VectorFloat32Parameter::ValueType{0.0f, 0.0f, 0.0f},
                                                         std::vector<std::string>{"alpha", "beta", "gamma"}));

  params.insertSeparator(Parameters::Separator{"Output Parameter(s)"});
  params.insert(std::make_unique<DataObjectNameParameter>(k_OutputDirectionVectorsArrayName_Key, "Output Direction Vectors Array Name",
                                                          "The name of the output direction vectors array that will be created.", "Direction Vectors"));

  params.linkParameters(k_LatticeConstantsInputType_Key, k_LatticeConstantsArrayPath_Key, std::make_any<uint64>(0));
  params.linkParameters(k_LatticeConstantsInputType_Key, k_LatticeConstantsLength_Key, std::make_any<uint64>(1));
  params.linkParameters(k_LatticeConstantsInputType_Key, k_LatticeConstantsAngles_Key, std::make_any<uint64>(1));

  return params;
}

//------------------------------------------------------------------------------
IFilter::VersionType ComputeDirectionVectorsFilter::parametersVersion() const
{
  return 1;
}

//------------------------------------------------------------------------------
IFilter::UniquePointer ComputeDirectionVectorsFilter::clone() const
{
  return std::make_unique<ComputeDirectionVectorsFilter>();
}

//------------------------------------------------------------------------------
IFilter::PreflightResult ComputeDirectionVectorsFilter::preflightImpl(const DataStructure& dataStructure, const Arguments& filterArgs, const MessageHandler& messageHandler,
                                                                      const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  auto inputRepType = static_cast<ebsdlib::orientations::Type>(filterArgs.value<ChoicesParameter::ValueType>(k_InputType_Key));
  auto inputOrientationsArrayPath = filterArgs.value<ArraySelectionParameter::ValueType>(k_InputOrientationArrayPath_Key);
  auto latticeConstantsType = static_cast<LatticeConstantsInputType>(filterArgs.value<ChoicesParameter::ValueType>(k_LatticeConstantsInputType_Key));
  auto latticeConstantsArrayPath = filterArgs.value<ArraySelectionParameter::ValueType>(k_LatticeConstantsArrayPath_Key);
  auto latticeConstantsLengths = filterArgs.value<VectorFloat32Parameter::ValueType>(k_LatticeConstantsLength_Key);
  auto latticeConstantsAngles = filterArgs.value<VectorFloat32Parameter::ValueType>(k_LatticeConstantsAngles_Key);
  auto outputDirectionVectorsArrayName = filterArgs.value<DataObjectNameParameter::ValueType>(k_OutputDirectionVectorsArrayName_Key);

  DataPath orientationsParentPath = inputOrientationsArrayPath.getParent();
  DataPath outputDirectionVectorsPath = orientationsParentPath.createChildPath(outputDirectionVectorsArrayName);
  auto& orientationsArray = dataStructure.getDataRefAs<IDataArray>(inputOrientationsArrayPath);

  nx::core::Result<OutputActions> resultOutputActions;
  auto action = std::make_unique<CreateArrayAction>(DataType::float32, orientationsArray.getTupleShape(), std::vector<usize>{3}, outputDirectionVectorsPath);
  resultOutputActions.value().appendAction(std::move(action));

  return {std::move(resultOutputActions)};
}

//------------------------------------------------------------------------------
Result<> ComputeDirectionVectorsFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                                    const std::atomic_bool& shouldCancel, const ExecutionContext& executionContext) const
{
  ComputeDirectionVectorsInputValues inputValues;

  inputValues.InputRepType = static_cast<ebsdlib::orientations::Type>(filterArgs.value<ChoicesParameter::ValueType>(k_InputType_Key));
  inputValues.InputOrientationsArrayPath = filterArgs.value<ArraySelectionParameter::ValueType>(k_InputOrientationArrayPath_Key);
  inputValues.LatticeConstantsInputType = static_cast<LatticeConstantsInputType>(filterArgs.value<ChoicesParameter::ValueType>(k_LatticeConstantsInputType_Key));
  inputValues.LatticeConstantsArrayPath = filterArgs.value<ArraySelectionParameter::ValueType>(k_LatticeConstantsArrayPath_Key);
  inputValues.ManualLatticeConstantsLengths = filterArgs.value<VectorFloat32Parameter::ValueType>(k_LatticeConstantsLength_Key);
  inputValues.ManualLatticeConstantsAngles = filterArgs.value<VectorFloat32Parameter::ValueType>(k_LatticeConstantsAngles_Key);
  inputValues.OutputDirectionVectorsArrayName = filterArgs.value<DataObjectNameParameter::ValueType>(k_OutputDirectionVectorsArrayName_Key);

  return ComputeDirectionVectors(dataStructure, messageHandler, shouldCancel, &inputValues)();
}
} // namespace nx::core
