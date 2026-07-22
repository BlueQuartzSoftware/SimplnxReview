#include "GlavicicTorture.hpp"

#include "simplnx/DataStructure/DataArray.hpp"

#include <fmt/format.h>

#include <array>
#include <map>
#include <random>

using namespace nx::core;

namespace
{
// The Euler group id that each entry of GlavicicTortureInputValues::GroupEulerAnglesPaths represents.
// There is intentionally no group 5.
constexpr std::array<int32, 6> k_GroupIds = {0, 1, 2, 3, 4, 6};
} // namespace

// -----------------------------------------------------------------------------
GlavicicTorture::GlavicicTorture(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, GlavicicTortureInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
GlavicicTorture::~GlavicicTorture() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& GlavicicTorture::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> GlavicicTorture::operator()()
{
  const auto& expectedMTRIdsRef = m_DataStructure.getDataRefAs<UInt8Array>(m_InputValues->ExpectedMTRIdsPath).getDataStoreRef();
  auto& eulerAnglesRef = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->EulerAnglesPath).getDataStoreRef();

  // Map each Euler group id to the DataStore of its selected Group Euler Angles array
  std::map<int32, const AbstractDataStore<float32>*> groupStores;
  for(usize groupIndex = 0; groupIndex < m_InputValues->GroupEulerAnglesPaths.size(); groupIndex++)
  {
    groupStores[k_GroupIds[groupIndex]] = &(m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->GroupEulerAnglesPaths[groupIndex]).getDataStoreRef());
  }

  std::mt19937_64 generator(m_InputValues->Seed);

  usize numCells = expectedMTRIdsRef.getNumberOfTuples();
  usize progressInterval = numCells / 10;
  for(usize cellIndex = 0; cellIndex < numCells; cellIndex++)
  {
    if(m_ShouldCancel)
    {
      return {};
    }
    if(progressInterval > 0 && cellIndex % progressInterval == 0)
    {
      m_MessageHandler({IFilter::Message::Type::Info, fmt::format("Assigning Euler Angles: {}%", (cellIndex * 100) / numCells)});
    }

    int32 mtrId = static_cast<int32>(expectedMTRIdsRef[cellIndex]);
    auto groupIter = groupStores.find(mtrId);
    if(groupIter == groupStores.end())
    {
      return MakeErrorResult(-64530, fmt::format("Cell {} has an Expected MTR Id of {} which does not match any Euler group ({})", cellIndex, mtrId, fmt::join(k_GroupIds, ", ")));
    }
    const AbstractDataStore<float32>& groupStoreRef = *(groupIter->second);

    std::uniform_int_distribution<usize> tupleDist(0, groupStoreRef.getNumberOfTuples() - 1);
    usize pickedTupleIndex = tupleDist(generator);

    eulerAnglesRef[cellIndex * 3 + 0] = groupStoreRef[pickedTupleIndex * 3 + 0];
    eulerAnglesRef[cellIndex * 3 + 1] = groupStoreRef[pickedTupleIndex * 3 + 1];
    eulerAnglesRef[cellIndex * 3 + 2] = groupStoreRef[pickedTupleIndex * 3 + 2];
  }

  return {};
}
