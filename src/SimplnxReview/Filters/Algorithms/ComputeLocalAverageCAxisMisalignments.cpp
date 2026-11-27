#include "ComputeLocalAverageCAxisMisalignments.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"

using namespace nx::core;

namespace
{
template <bool CalculateBiasedAverage, bool CalculateUnbiasedAverage>
struct MisalignmentArguments
{
  static constexpr bool k_CalculatingBiasedAverage = CalculateBiasedAverage;
  static constexpr bool k_CalculatingUnbiasedAverage = CalculateUnbiasedAverage;
};

template <class MisalignmentArguments = MisalignmentArguments<false, false>>
class FindLocalAverageMisalignments
{
public:
  FindLocalAverageMisalignments(const std::atomic_bool& shouldCancel, ThrottledMessageHandler& progressThrottle, const Int32ArrayType& featureParentIds, const Float32ArrayType& avgCAxisMisalignments,
                                Int32NeighborListType& neighborList, Float32NeighborListType& cAxisMisalignmentList, Int32ArrayType& numFeaturesPerParent, Float32ArrayType& unbiasedLocalCAxisMisalignments,
                                Float32ArrayType& localCAxisMisalignments)
  : m_ShouldCancel(shouldCancel)
  , m_Throttle(progressThrottle)
  , m_FeatureParentIds(featureParentIds)
  , m_AvgCAxisMisalignments(avgCAxisMisalignments)
  , m_NeighborList(neighborList)
  , m_CAxisMisalignmentList(cAxisMisalignmentList)
  , m_UnbiasedLocalCAxisMisalignments(unbiasedLocalCAxisMisalignments)
  , m_LocalCAxisMisalignments(localCAxisMisalignments)
  , m_NumFeaturesPerParent(numFeaturesPerParent)
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

    if constexpr(MisalignmentArguments::k_CalculatingUnbiasedAverage)
    {
      // Default value-initialized to zeroes: https://en.cppreference.com/w/cpp/named_req/DefaultInsertable
      numUnbiasedFeaturesPerParent.resize(numFeatures);
    }

    m_Throttle.reset(numFeatures > 1 ? numFeatures - 1 : 0, "Computing Local Average CAxis Misalignments");
    for(usize i = 1; i < numFeatures; i++)
    {
      if(m_ShouldCancel)
      {
        return {};
      }
      m_Throttle.updatePercent(i - 1);

      int32 parentId = m_FeatureParentIds[i];
      if constexpr(MisalignmentArguments::k_CalculatingUnbiasedAverage)
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

      if constexpr(MisalignmentArguments::k_CalculatingBiasedAverage)
      {
        m_NumFeaturesPerParent[parentId].inc();
        m_LocalCAxisMisalignments[parentId] += m_AvgCAxisMisalignments[i];
      }
    }

    m_Throttle.reset(newNumFeatures > 1 ? newNumFeatures - 1 : 0, "Normalizing Parent Misalignments");
    for(usize i = 1; i < newNumFeatures; i++)
    {
      if(m_ShouldCancel)
      {
        return {};
      }
      m_Throttle.updatePercent(i - 1);

      if constexpr(MisalignmentArguments::k_CalculatingBiasedAverage)
      {
        m_LocalCAxisMisalignments[i] /= m_NumFeaturesPerParent[i];
      }

      if constexpr(MisalignmentArguments::k_CalculatingUnbiasedAverage)
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
  ThrottledMessageHandler& m_Throttle;

  // Unmodified Arrays
  const Int32ArrayType& m_FeatureParentIds;
  const Float32ArrayType& m_AvgCAxisMisalignments;
  Int32NeighborListType& m_NeighborList;
  Float32NeighborListType& m_CAxisMisalignmentList;

  // Modified Arrays
  Float32ArrayType& m_UnbiasedLocalCAxisMisalignments;
  Float32ArrayType& m_LocalCAxisMisalignments;
  Int32ArrayType& m_NumFeaturesPerParent;
};
} // namespace

// -----------------------------------------------------------------------------
ComputeLocalAverageCAxisMisalignments::ComputeLocalAverageCAxisMisalignments(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                                                             ComputeLocalAverageCAxisMisalignmentsInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
, m_Throttle(mesgHandler)
{
}

// -----------------------------------------------------------------------------
ComputeLocalAverageCAxisMisalignments::~ComputeLocalAverageCAxisMisalignments() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& ComputeLocalAverageCAxisMisalignments::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> ComputeLocalAverageCAxisMisalignments::operator()()
{
  const auto& featureParentIds = m_DataStructure.getDataRefAs<Int32ArrayType>(m_InputValues->feature_parent_ids_path);
  const auto& avgCAxisMisalignments = m_DataStructure.getDataRefAs<Float32ArrayType>(m_InputValues->avg_c_axis_misalignments_path);
  auto& neighborList = m_DataStructure.getDataRefAs<Int32NeighborListType>(m_InputValues->neighbor_list_path);
  auto& cAxisMisalignmentList = m_DataStructure.getDataRefAs<Float32NeighborListType>(m_InputValues->c_axis_misalignment_list_path);
  auto& numFeaturesPerParent = m_DataStructure.getDataRefAs<Int32ArrayType>(m_InputValues->num_features_per_parent_path);
  auto& unbiasedLocalCAxisMisalignments = m_DataStructure.getDataRefAs<Float32ArrayType>(m_InputValues->unbiased_local_c_axis_misalignments_path);
  auto& localCAxisMisalignments = m_DataStructure.getDataRefAs<Float32ArrayType>(m_InputValues->local_c_axis_misalignments_path);

  if(m_InputValues->calc_biased_avg)
  {
    if(m_InputValues->calc_unbiased_avg)
    {
      return ::FindLocalAverageMisalignments<MisalignmentArguments<true, true>>(getCancel(), m_Throttle, featureParentIds, avgCAxisMisalignments, neighborList, cAxisMisalignmentList,
                                                                                numFeaturesPerParent, unbiasedLocalCAxisMisalignments, localCAxisMisalignments)();
    }
    return ::FindLocalAverageMisalignments<MisalignmentArguments<true, false>>(getCancel(), m_Throttle, featureParentIds, avgCAxisMisalignments, neighborList, cAxisMisalignmentList,
                                                                               numFeaturesPerParent, unbiasedLocalCAxisMisalignments, localCAxisMisalignments)();
  }

  // Since we validate that at least one of the bool options is true in preflight we know that
  // by reaching this point in the logic we m_InputValues->CalcUnbiasedAvg must be true
  return ::FindLocalAverageMisalignments<MisalignmentArguments<false, true>>(getCancel(), m_Throttle, featureParentIds, avgCAxisMisalignments, neighborList, cAxisMisalignmentList,
                                                                             numFeaturesPerParent, unbiasedLocalCAxisMisalignments, localCAxisMisalignments)();
}
