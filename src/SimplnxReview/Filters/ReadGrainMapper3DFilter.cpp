#include "ReadGrainMapper3DFilter.hpp"

#include "SimplnxReview/Filters/Algorithms/ReadGrainMapper3D.hpp"

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

  Result<OutputActions> resultOutputActions;
  std::vector<PreflightValue> preflightUpdatedValues;

  return {std::move(resultOutputActions), std::move(preflightUpdatedValues)};
}

//------------------------------------------------------------------------------
Result<> ReadGrainMapper3DFilter::executeImpl(DataStructure& dataStructure, const Arguments& filterArgs, const PipelineFilter* pipelineNode, const MessageHandler& messageHandler,
                                              const std::atomic_bool& shouldCancel) const
{
  ReadGrainMapper3DInputValues inputValues;

  return ReadGrainMapper3D(dataStructure, messageHandler, shouldCancel, &inputValues)();
}
} // namespace nx::core
