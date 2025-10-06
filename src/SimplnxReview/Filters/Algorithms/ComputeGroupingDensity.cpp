#include "ComputeGroupingDensity.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"
#include "simplnx/Utilities/MessageHelper.hpp"

using namespace nx::core;

namespace
{
template <bool UseNonContiguousNeighbors, bool FindCheckedFeatures>
struct FindDensitySpecializations
{
  static inline constexpr bool UsingNonContiguousNeighbors = UseNonContiguousNeighbors;
  static inline constexpr bool FindingCheckedFeatures = FindCheckedFeatures;
};

template <class FindDensitySpecializations = FindDensitySpecializations<true, true>>
class FindDensityGrouping
{
public:
  FindDensityGrouping(const std::atomic_bool& shouldCancel, const IFilter::MessageHandler& mesgHandler, const Int32Array& parentIds, const Float32Array& parentVolumes, const Float32Array& volumes,
                      const Int32NeighborList& contiguousNL, Float32Array& groupingDensities, Int32NeighborList& nonContiguousNL, Int32Array& checkedFeatures)
  : m_ShouldCancel(shouldCancel)
  , m_MessageHandler(mesgHandler)
  , m_ParentIds(parentIds)
  , m_ParentVolumes(parentVolumes)
  , m_Volumes(volumes)
  , m_ContiguousNL(contiguousNL)
  , m_GroupingDensities(groupingDensities)
  , m_NonContiguousNL(nonContiguousNL)
  , m_CheckedFeatures(checkedFeatures)
  {
  }
  ~FindDensityGrouping() noexcept = default;

  FindDensityGrouping(const FindDensityGrouping&) = delete;            // Copy Constructor Default Implemented
  FindDensityGrouping(FindDensityGrouping&&) = delete;                 // Move Constructor Not Implemented
  FindDensityGrouping& operator=(const FindDensityGrouping&) = delete; // Copy Assignment Not Implemented
  FindDensityGrouping& operator=(FindDensityGrouping&&) = delete;      // Move Assignment Not Implemented

  Result<> operator()()
  {
    const auto& parentIds = m_ParentIds.getDataStoreRef();
    const auto& parentVolumes = m_ParentVolumes.getDataStoreRef();
    const auto& volumes = m_Volumes.getDataStoreRef();

    auto& checkedFeatures = m_CheckedFeatures.getDataStoreRef();
    auto& groupingDensities = m_GroupingDensities.getDataStoreRef();

    usize numFeatures = volumes.getNumberOfTuples();
    usize numParents = parentVolumes.getNumberOfTuples();

    int kMax = 1;
    if constexpr(FindDensitySpecializations::UsingNonContiguousNeighbors)
    {
      kMax = 2;
    }

    int32 numNeighbors, numNeighborhoods, numCurNeighborList, neigh;
    float32 totalCheckVolume, curParentVolume;
    std::set<int32> totalCheckList = {};

    std::vector<float32> checkedFeatureVolumes(1, 0.0f);
    if constexpr(FindDensitySpecializations::FindingCheckedFeatures)
    {
      // Default value-initialized to zeroes: https://en.cppreference.com/w/cpp/named_req/DefaultInsertable
      checkedFeatureVolumes.resize(numFeatures);
    }
    MessageHelper messageHelper(m_MessageHandler);
    ThrottledMessenger throttledMessenger = messageHelper.createThrottledMessenger();

    for(usize parentIdx = 1; parentIdx < numParents; parentIdx++)
    {
      throttledMessenger.sendThrottledMessage([&]() { return fmt::format("[{}%]", CalculatePercentComplete(parentIdx, numParents)); });

      if(m_ShouldCancel)
      {
        return {};
      }
      for(usize j = 1; j < numFeatures; j++)
      {
        if(parentIds[j] == parentIdx)
        {
          if(totalCheckList.find(j) == totalCheckList.end())
          // if(std::find(totalCheckList.begin(), totalCheckList.end(), j) == totalCheckList.end())
          {
            totalCheckVolume += m_Volumes[j];
            totalCheckList.insert(static_cast<int32>(j));
            if constexpr(FindDensitySpecializations::FindingCheckedFeatures)
            {
              if(parentVolumes[parentIdx] > checkedFeatureVolumes[j])
              {
                checkedFeatureVolumes[j] = parentVolumes[parentIdx];
                checkedFeatures[j] = static_cast<int32>(parentIdx);
              }
            }
          }
          numNeighbors = m_ContiguousNL.getListSize(static_cast<int32>(j));
          if constexpr(FindDensitySpecializations::UsingNonContiguousNeighbors)
          {
            numNeighborhoods = static_cast<int32>(m_NonContiguousNL[j].size());
          }
          for(int k = 0; k < kMax; k++)
          {
            if(k == 0)
            {
              numCurNeighborList = numNeighbors;
            }
            if constexpr(FindDensitySpecializations::UsingNonContiguousNeighbors)
            {
              if(k == 1)
              {
                numCurNeighborList = numNeighborhoods;
              }
            }
            for(int32_t l = 0; l < numCurNeighborList; l++)
            {
              if(k == 0)
              {
                bool ok = false;
                neigh = m_ContiguousNL.getValue(static_cast<int32>(j), l, ok);
              }
              else if(k == 1)
              {
                neigh = m_NonContiguousNL[j][l];
              }
              if(totalCheckList.find(neigh) == totalCheckList.end())
              // if(std::find(totalCheckList.begin(), totalCheckList.end(), neigh) == totalCheckList.end())
              {
                totalCheckVolume += m_Volumes[neigh];
                totalCheckList.insert(neigh);
                if constexpr(FindDensitySpecializations::FindingCheckedFeatures)
                {
                  if(parentVolumes[parentIdx] > checkedFeatureVolumes[neigh])
                  {
                    checkedFeatureVolumes[neigh] = parentVolumes[parentIdx];
                    checkedFeatures[neigh] = static_cast<int32>(parentIdx);
                  }
                }
              }
            }
          }
        }
      }
      curParentVolume = parentVolumes[parentIdx];
      if(totalCheckVolume == 0.0f)
      {
        groupingDensities[parentIdx] = -1.0f;
      }
      else
      {
        groupingDensities[parentIdx] = (curParentVolume / totalCheckVolume);
      }
      totalCheckList.clear();
      totalCheckVolume = 0.0f;
    }

    return {};
  }

private:
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
  const Int32Array& m_ParentIds;
  const Float32Array& m_ParentVolumes;
  const Float32Array& m_Volumes;
  const Int32NeighborList& m_ContiguousNL;
  Float32Array& m_GroupingDensities;
  Int32NeighborList& m_NonContiguousNL;
  Int32Array& m_CheckedFeatures;
};
} // namespace

// -----------------------------------------------------------------------------
ComputeGroupingDensity::ComputeGroupingDensity(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                               ComputeGroupingDensityInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
const std::atomic_bool& ComputeGroupingDensity::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> ComputeGroupingDensity::operator()()
{
  auto& parentIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->ParentIdsPath);
  auto& parentVolumes = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->ParentVolumesPath);
  auto& volumes = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->VolumesPath);
  auto& contiguousNL = m_DataStructure.getDataRefAs<NeighborList<int32>>(m_InputValues->ContiguousNLPath);
  auto& groupingDensities = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->GroupingDensitiesPath);

  // These may or may not be empty depending on the parameters
  auto& nonContiguousNL = m_DataStructure.getDataRefAs<NeighborList<int32>>(m_InputValues->NonContiguousNLPath);
  auto& checkedFeatures = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->CheckedFeaturesPath);

  if(m_InputValues->UseNonContiguousNeighbors)
  {
    if(m_InputValues->FindCheckedFeatures)
    {
      return ::FindDensityGrouping<FindDensitySpecializations<true, true>>(getCancel(), m_MessageHandler, parentIds, parentVolumes, volumes, contiguousNL, groupingDensities, nonContiguousNL,
                                                                           checkedFeatures)();
    }
    return ::FindDensityGrouping<FindDensitySpecializations<true, false>>(getCancel(), m_MessageHandler, parentIds, parentVolumes, volumes, contiguousNL, groupingDensities, nonContiguousNL,
                                                                          checkedFeatures)();
  }
  else if(m_InputValues->FindCheckedFeatures)
  {
    return ::FindDensityGrouping<FindDensitySpecializations<false, true>>(getCancel(), m_MessageHandler, parentIds, parentVolumes, volumes, contiguousNL, groupingDensities, nonContiguousNL,
                                                                          checkedFeatures)();
  }
  else
  {
    return ::FindDensityGrouping<FindDensitySpecializations<false, false>>(getCancel(), m_MessageHandler, parentIds, parentVolumes, volumes, contiguousNL, groupingDensities, nonContiguousNL,
                                                                           checkedFeatures)();
  }
}
