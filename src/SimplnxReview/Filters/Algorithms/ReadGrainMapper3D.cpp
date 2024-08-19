#include "ReadGrainMapper3D.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"

using namespace nx::core;

namespace
{
} // namespace

// -----------------------------------------------------------------------------
ReadGrainMapper3D::ReadGrainMapper3D(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ReadGrainMapper3DInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
const std::atomic_bool& ReadGrainMapper3D::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> ReadGrainMapper3D::operator()()
{
  return {};
}
