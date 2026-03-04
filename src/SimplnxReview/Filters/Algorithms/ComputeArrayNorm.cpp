#include "ComputeArrayNorm.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/Utilities/DataArrayUtilities.hpp"
#include "simplnx/Utilities/FilterUtilities.hpp"

#include <cmath>

using namespace nx::core;

namespace
{
template <typename T>
class ComputeNormImpl
{
public:
  ComputeNormImpl(const IDataArray& inputArray, Float32Array& normArray, float32 pSpace)
  : m_InputArray(dynamic_cast<const DataArray<T>&>(inputArray))
  , m_NormArray(normArray)
  , m_PSpace(pSpace)
  {
  }

  void operator()() const
  {
    const auto& inputStore = m_InputArray.getDataStoreRef();
    auto& normStore = m_NormArray.getDataStoreRef();

    usize numTuples = m_InputArray.getNumberOfTuples();
    usize numComponents = m_InputArray.getNumberOfComponents();

    for(usize i = 0; i < numTuples; i++)
    {
      float32 normTmp = 0.0f;
      for(usize j = 0; j < numComponents; j++)
      {
        float32 value = static_cast<float32>(inputStore[numComponents * i + j]);
        normTmp += std::pow(std::abs(value), m_PSpace);
      }
      normStore[i] = std::pow(normTmp, 1.0f / m_PSpace);
    }
  }

private:
  const DataArray<T>& m_InputArray;
  Float32Array& m_NormArray;
  float32 m_PSpace;
};

struct ComputeNormFunctor
{
  template <typename T>
  void operator()(const IDataArray& inputArray, Float32Array& normArray, float32 pSpace)
  {
    ComputeNormImpl<T>(inputArray, normArray, pSpace)();
  }
};
} // namespace

// -----------------------------------------------------------------------------
ComputeArrayNorm::ComputeArrayNorm(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ComputeArrayNormInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
ComputeArrayNorm::~ComputeArrayNorm() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& ComputeArrayNorm::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> ComputeArrayNorm::operator()()
{
  const auto& inputArray = m_DataStructure.getDataRefAs<IDataArray>(m_InputValues->SelectedArrayPath);
  auto& normArray = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->NormArrayPath);

  ExecuteDataFunction(ComputeNormFunctor{}, inputArray.getDataType(), inputArray, normArray, m_InputValues->PSpace);

  return {};
}
