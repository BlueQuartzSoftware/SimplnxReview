#include "FindLocalAverageCAxisMisalignments.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"

using namespace nx::core;

namespace
{
template <bool CalculateBiasedAverage, bool CalculateUnbiasedAverage>
struct MisalignmentArguments
{
  static inline constexpr bool CalculatingBiasedAverage = CalculateBiasedAverage;
  static inline constexpr bool CalculatingUnbiasedAverage = CalculateUnbiasedAverage;
};

template <class MisalignmentArguments = MisalignmentArguments<false, false>>
class FindLocalAverageMisalignments
{
public:
  FindLocalAverageMisalignments(const std::atomic_bool& shouldCancel, const Int32Array& featureParentIds, const Float32Array& avgCAxisMisalignments, Int32NeighborList& neighborList,
                                Float32NeighborList& cAxisMisalignmentList, Int32Array& numFeaturesPerParent, Float32Array& unbiasedLocalCAxisMisalignments, Float32Array& localCAxisMisalignments)
  : m_ShouldCancel(shouldCancel)
  , m_FeatureParentIds(featureParentIds)
  , m_AvgCAxisMisalignments(avgCAxisMisalignments)
  , m_NeighborList(neighborList)
  , m_CAxisMisalignmentList(cAxisMisalignmentList)
  , m_NumFeaturesPerParent(numFeaturesPerParent)
  , m_UnbiasedLocalCAxisMisalignments(unbiasedLocalCAxisMisalignments)
  , m_LocalCAxisMisalignments(localCAxisMisalignments)
  {
  }
  ~FindLocalAverageMisalignments() noexcept = default;

  FindLocalAverageMisalignments(const FindLocalAverageMisalignments&) = delete;            // Copy Constructor Default Implemented
  FindLocalAverageMisalignments(FindLocalAverageMisalignments&&) = delete;                 // Move Constructor Not Implemented
  FindLocalAverageMisalignments& operator=(const FindLocalAverageMisalignments&) = delete; // Copy Assignment Not Implemented
  FindLocalAverageMisalignments& operator=(FindLocalAverageMisalignments&&) = delete;      // Move Assignment Not Implemented

  Result<> operator()()
  {
    usize numFeatures = m_FeatureParentIds.getNumberOfTuples();
    usize newNumFeatures = m_NumFeaturesPerParent.getNumberOfTuples();

    std::vector<int32> numUnbiasedFeaturesPerParent(1, 0);

    if constexpr(MisalignmentArguments::CalculatingUnbiasedAverage)
    {
      // Default value-initialized to zeroes: https://en.cppreference.com/w/cpp/named_req/DefaultInsertable
      numUnbiasedFeaturesPerParent.resize(numFeatures);
    }

    for(usize i = 1; i < numFeatures; i++)
    {
      if(m_ShouldCancel)
      {
        return {};
      }

      int32 parentId = m_FeatureParentIds[i];
      if constexpr(MisalignmentArguments::CalculatingUnbiasedAverage)
      {
        for(usize j = 0; j < m_NeighborList[i].size(); j++)
        {
          if(m_FeatureParentIds[m_NeighborList[i][j]] == m_FeatureParentIds[i])
          {
            m_UnbiasedLocalCAxisMisalignments[parentId] += m_CAxisMisalignmentList[i][j];
            numUnbiasedFeaturesPerParent[parentId]++;
          }
        }
      }

      if constexpr(MisalignmentArguments::CalculatingBiasedAverage)
      {
        m_NumFeaturesPerParent[parentId]++;
        m_LocalCAxisMisalignments[parentId] += m_AvgCAxisMisalignments[i];
      }
    }

    for(usize i = 1; i < newNumFeatures; i++)
    {
      if(m_ShouldCancel)
      {
        return {};
      }

      if constexpr(MisalignmentArguments::CalculatingBiasedAverage)
      {
        m_LocalCAxisMisalignments[i] /= m_NumFeaturesPerParent[i];
      }

      if constexpr(MisalignmentArguments::CalculatingUnbiasedAverage)
      {
        if(numUnbiasedFeaturesPerParent[i] > 0)
        {
          m_UnbiasedLocalCAxisMisalignments[i] /= static_cast<float32>(numUnbiasedFeaturesPerParent[i]);
        }
        else
        {
          m_UnbiasedLocalCAxisMisalignments[i] = 0.0f;
        }
      }
    }

    return {};
  }

private:
  const std::atomic_bool& m_ShouldCancel;

  // Unmodified Arrays
  const Int32Array& m_FeatureParentIds;
  const Float32Array& m_AvgCAxisMisalignments;
  Int32NeighborList& m_NeighborList;
  Float32NeighborList& m_CAxisMisalignmentList;

  // Modified Arrays
  Float32Array& m_UnbiasedLocalCAxisMisalignments;
  Float32Array& m_LocalCAxisMisalignments;
  Int32Array& m_NumFeaturesPerParent;
};
} // namespace

// -----------------------------------------------------------------------------
FindLocalAverageCAxisMisalignments::FindLocalAverageCAxisMisalignments(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                                                       FindLocalAverageCAxisMisalignmentsInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
FindLocalAverageCAxisMisalignments::~FindLocalAverageCAxisMisalignments() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& FindLocalAverageCAxisMisalignments::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> FindLocalAverageCAxisMisalignments::operator()()
{
  const auto& featureParentIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeatureParentIdsPath);
  const auto& avgCAxisMisalignments = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->AvgCAxisMisalignmentsPath);
  auto& neighborList = m_DataStructure.getDataRefAs<Int32NeighborList>(m_InputValues->NeighborListPath);
  auto& cAxisMisalignmentList = m_DataStructure.getDataRefAs<Float32NeighborList>(m_InputValues->CAxisMisalignmentListPath);
  auto& numFeaturesPerParent = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->NumFeaturesPerParentPath);
  auto& unbiasedLocalCAxisMisalignments = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->UnbiasedLocalCAxisMisalignmentsPath);
  auto& localCAxisMisalignments = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->LocalCAxisMisalignmentsPath);

  if(m_InputValues->CalcBiasedAvg)
  {
    if(m_InputValues->CalcUnbiasedAvg)
    {
      return ::FindLocalAverageMisalignments<MisalignmentArguments<true, true>>(getCancel(), featureParentIds, avgCAxisMisalignments, neighborList, cAxisMisalignmentList, numFeaturesPerParent,
                                                                                unbiasedLocalCAxisMisalignments, localCAxisMisalignments)();
    }
    return ::FindLocalAverageMisalignments<MisalignmentArguments<true, false>>(getCancel(), featureParentIds, avgCAxisMisalignments, neighborList, cAxisMisalignmentList, numFeaturesPerParent,
                                                                               unbiasedLocalCAxisMisalignments, localCAxisMisalignments)();
  }

  // Since we validate that at least one of the bool options is true in preflight we know that
  // by reaching this point in the logic we m_InputValues->CalcUnbiasedAvg must be true
  return ::FindLocalAverageMisalignments<MisalignmentArguments<false, true>>(getCancel(), featureParentIds, avgCAxisMisalignments, neighborList, cAxisMisalignmentList, numFeaturesPerParent,
                                                                             unbiasedLocalCAxisMisalignments, localCAxisMisalignments)();
}
