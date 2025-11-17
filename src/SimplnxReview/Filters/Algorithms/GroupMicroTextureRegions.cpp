#include "GroupMicroTextureRegions.hpp"

#include "simplnx/Common/Constants.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"
#include "simplnx/Utilities/Math/GeometryMath.hpp"
#include "simplnx/Utilities/Math/MatrixMath.hpp"
#include "simplnx/Utilities/MessageHelper.hpp"

#include "EbsdLib/LaueOps/LaueOps.h"

#include <random>

using namespace nx::core;

// -----------------------------------------------------------------------------
GroupMicroTextureRegions::GroupMicroTextureRegions(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                                   GroupMicroTextureRegionsInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
, m_FeaturePhases(m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeaturePhasesArrayPath))
, m_FeatureParentIds(m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeatureParentIdsArrayName))
, m_CrystalStructures(m_DataStructure.getDataRefAs<UInt32Array>(m_InputValues->CrystalStructuresArrayPath))
, m_AvgQuats(m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->AvgQuatsArrayPath))
, m_Volumes(m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->VolumesArrayPath))
{
}

// -----------------------------------------------------------------------------
GroupMicroTextureRegions::~GroupMicroTextureRegions() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& GroupMicroTextureRegions::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
bool GroupMicroTextureRegions::growPatch(int32_t currentPatch)
{
  return false;
}

// -----------------------------------------------------------------------------
bool GroupMicroTextureRegions::growGrouping(int32_t referenceFeature, int32_t neighborFeature, int32_t newFid)
{
  return false;
}

// -----------------------------------------------------------------------------
Result<> GroupMicroTextureRegions::execute()
{
  MessageHelper messageHelper(m_MessageHandler);
  ThrottledMessenger throttledMessenger = messageHelper.createThrottledMessenger();

  NeighborList<int32>& featureNeighborListRef = m_DataStructure.getDataRefAs<NeighborList<int32>>(m_InputValues->ContiguousNeighborListArrayPath);
  NeighborList<int32>* nonContigNeighListPtr = nullptr;
  if(m_InputValues->UseNonContiguousNeighbors)
  {
    nonContigNeighListPtr = m_DataStructure.getDataAs<NeighborList<int32>>(m_InputValues->NonContiguousNeighborListArrayPath);
  }
  if(nullptr == nonContigNeighListPtr)
  {
    return MakeErrorResult(-99345, "There was an error getting the Non-contiguous neighborlist from the DataStructure");
  }

  std::vector<int32> groupList;

  int32 parentCount = 0;
  int32 featureSeed = 0;
  int32 list1size = 0, list2size = 0, listSize = 0;

  bool patchGrouping = false;

  while(featureSeed >= 0)
  {
    parentCount++;
    featureSeed = getSeed(parentCount);
    if(featureSeed >= 0)
    {
      groupList.push_back(featureSeed);
      for(std::vector<int32>::size_type j = 0; j < groupList.size(); j++)
      {
        int32 firstFeature = groupList[j];
        list1size = static_cast<int32>(featureNeighborListRef[firstFeature].size());
        if(m_InputValues->UseNonContiguousNeighbors)
        {
          list2size = nonContigNeighListPtr->getListSize(firstFeature);
        }
        for(int32 k = 0; k < 2; k++)
        {
          if(patchGrouping)
          {
            k = 1;
          }
          if(k == 0)
          {
            listSize = list1size;
          }
          else if(k == 1)
          {
            listSize = list2size;
          }
          for(int32 l = 0; l < listSize; l++)
          {
            int32 neigh = -1;
            if(k == 0)
            {
              neigh = featureNeighborListRef[firstFeature][l];
            }
            else if(k == 1 && m_InputValues->UseNonContiguousNeighbors)
            {
              bool ok = false;
              neigh = nonContigNeighListPtr->getValue(firstFeature, l, ok);
            }
            if(neigh >= 0 && neigh != firstFeature)
            {
              if(determineGrouping(firstFeature, neigh, parentCount))
              {
                if(!patchGrouping)
                {
                  groupList.push_back(neigh);
                }
              }
            }
          }
        }
      }
      if(patchGrouping)
      {
        if(growPatch(parentCount))
        {
          for(std::vector<int32_t>::size_type j = 0; j < groupList.size(); j++)
          {
            int32_t firstFeature = groupList[j];
            listSize = static_cast<int32_t>(featureNeighborListRef[firstFeature].size());
            for(int32_t l = 0; l < listSize; l++)
            {
              int32 neigh = featureNeighborListRef[firstFeature][l];
              if(neigh != firstFeature)
              {
                if(growGrouping(firstFeature, neigh, parentCount))
                {
                  groupList.push_back(neigh);
                }
              }
            }
          }
        }
      }

      throttledMessenger.sendThrottledMessage([&]() { return fmt::format("Parent Count: {}", parentCount); });
    }
    groupList.clear();
  }
  return {};
}

// -----------------------------------------------------------------------------
Result<> GroupMicroTextureRegions::operator()()
{
  MessageHelper messageHelper(m_MessageHandler);

  m_Generator = std::mt19937_64(std::mt19937::default_seed);
  m_Distribution = std::uniform_real_distribution<float32>(0.0f, 1.0f);

  // Initialize Data
  m_AvgCAxes[0] = 0.0f;
  m_AvgCAxes[1] = 0.0f;
  m_AvgCAxes[2] = 0.0f;
  m_FeatureParentIds.fill(-1);

  // Execute the main grouping algorithm
  messageHelper.sendMessage(fmt::format("Starting Grouping....."));

  // Execute the grouping algorithm
  Result<> result = execute();
  if(result.invalid())
  {
    return result;
  }

  // handle active array resize
  if(m_NumTuples < 2)
  {
    return MakeErrorResult(-87000, fmt::format("The number of grouped Features was {} which means no grouped Features were detected. A grouping value may be set too high", m_NumTuples));
  }
  m_DataStructure.getDataRefAs<AttributeMatrix>(m_InputValues->NewCellFeatureAttributeMatrixName).resizeTuples(ShapeType{m_NumTuples});

  auto& cellParentIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->CellParentIdsArrayName);
  auto& featureIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeatureIdsArrayPath);
  usize totalPoints = featureIds.getNumberOfTuples();
  for(usize k = 0; k < totalPoints; k++)
  {
    cellParentIds[k] = m_FeatureParentIds[featureIds[k]];
  }

  // By default, we randomize grains !!! COMMENT OUT FOR DEMONSTRATION !!!
  // m_MessageHandler(IFilter::Message::Type::Info, "Randomizing Parent Ids");
  // RandomizeFeatureIds(totalPoints, m_NumTuples, cellParentIds, m_FeatureParentIds, featureIds, m_InputValues->SeedValue);

  return {};
}

// -----------------------------------------------------------------------------
int GroupMicroTextureRegions::getSeed(int32 newFid)
{
  usize numFeatures = m_FeaturePhases.getNumberOfTuples();

  int32 featureIdSeed = -1;

  // Precalculate some constants
  const int32 totalFMinus1 = static_cast<int32>(numFeatures) - 1;

  usize counter = 0;
  // This section finds a feature id that has not been grouped yet. It starts by
  // randomly selecting a feature id between 0 and numFeatures-1. We then start
  // looping. If the initial random value is valid then we exit the loop after
  // a single iteration. If that feature has already been grouped, then we add one
  // to the `randFeature` value and try again. If we get to the end of the range of
  // featureIds then the algorithm will loop back to featureId = 0 and start incrementing
  // from there. This is reasonably efficient as we only generate random numbers
  // as needed.
  auto randFeature = static_cast<int32>(m_Distribution(m_Generator) * static_cast<float32>(totalFMinus1));
  while(featureIdSeed == -1 && counter < numFeatures)
  {
    if(randFeature > totalFMinus1)
    {
      randFeature = randFeature - numFeatures;
    }
    if(m_FeatureParentIds.getValue(randFeature) == -1)
    {
      featureIdSeed = randFeature;
    }
    randFeature++;
    counter++;
  }

  //  // Used for debugging and demonstration
  //  if(newFid == 1)
  //  {
  //    auto& centroids = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->VolumesArrayPath.replaceName("Centroids"));
  //    std::ofstream fout ("/tmp/GroupMicroTextureInitialVoxelSeeds.txt", std::ios_base::out | std::ios_base::app);
  //    fout << fmt::format("Feature Parent Id: {} | X: {}, Y: {}\n", voxelSeed, centroids.getComponent(voxelSeed, 0), centroids.getComponent(voxelSeed, 1));
  //  }

  if(featureIdSeed >= 0)
  {
    m_FeatureParentIds[featureIdSeed] = newFid;
    m_NumTuples = newFid + 1;

    if(m_InputValues->UseRunningAverage)
    {
      usize index = featureIdSeed * 4;
      // Get the orientation matrix (which is passive) and then transpose it to make it active transform
      EbsdLib::Matrix3X3F g1t =
          OrientationTransformation::qu2om<QuatF, OrientationF>({m_AvgQuats.getValue(index + 0), m_AvgQuats.getValue(index + 1), m_AvgQuats.getValue(index + 2), m_AvgQuats.getValue(index + 3)})
              .toGMatrixObj()
              .transpose();

      EbsdLib::Matrix3X1F cAxis(0.0f, 0.0f, 1.0f);
      // normalize so that the dot product can be taken below without
      // dividing by the magnitudes (they would be 1)
      const EbsdLib::Matrix3X1F c1 = (g1t * cAxis).normalize();

      m_AvgCAxes = c1 * m_Volumes.getValue(featureIdSeed);
    }
  }

  return featureIdSeed;
}

// -----------------------------------------------------------------------------
bool GroupMicroTextureRegions::determineGrouping(int32 referenceFeature, int32 neighborFeature, int32 newFid)
{
  const int32 neighborParentId = m_FeatureParentIds.getValue(neighborFeature);
  const int32 referenceFeaturePhase = m_FeaturePhases.getValue(referenceFeature);
  const int32 neighborFeaturePhase = m_FeaturePhases.getValue(neighborFeature);

  if(neighborParentId == -1 && referenceFeaturePhase > 0 && neighborFeaturePhase > 0)
  {
    EbsdLib::Matrix3X1F c1 = {0.0f, 0.0f, 0.0f};
    EbsdLib::Matrix3X1F cAxis(0.0f, 0.0f, 1.0f);

    if(!m_InputValues->UseRunningAverage)
    {
      const usize index = referenceFeature * 4;
      // Get the orientation matrix (which is passive) and then transpose it to make it active transform
      // transpose the g matrix so when c-axis is multiplied by it,
      // it will give the sample direction that the c-axis is along
      EbsdLib::Matrix3X3F g1t =
          OrientationTransformation::qu2om<QuatF, Orientation<float32>>({m_AvgQuats[index + 0], m_AvgQuats[index + 1], m_AvgQuats[index + 2], m_AvgQuats[index + 3]}).toGMatrixObj().transpose();
      c1 = (g1t * cAxis).normalize();
    }
    uint32 phase1 = m_CrystalStructures.getValue(referenceFeaturePhase);
    uint32 phase2 = m_CrystalStructures.getValue(neighborFeaturePhase);
    if(phase1 == phase2 && (phase1 == EbsdLib::CrystalStructure::Hexagonal_High))
    {
      const usize index = neighborFeature * 4;
      // Get the orientation matrix (which is passive) and then transpose it to make it active transform
      // transpose the g matrix so when c-axis is multiplied by it,
      // it will give the sample direction that the c-axis is along
      EbsdLib::Matrix3X3F g2t =
          OrientationTransformation::qu2om<QuatF, Orientation<float32>>({m_AvgQuats[index + 0], m_AvgQuats[index + 1], m_AvgQuats[index + 2], m_AvgQuats[index + 3]}).toGMatrixObj().transpose();
      EbsdLib::Matrix3X1F c2 = (g2t * cAxis).normalize();

      float32 w;
      if(m_InputValues->UseRunningAverage)
      {
        w = m_AvgCAxes.cosTheta(c2);
      }
      else
      {
        w = c1.cosTheta(c2);
      }
      w = std::acos(std::clamp(w, -1.0f, 1.0f));

      // Convert user defined tolerance to radians.
      float32 cAxisToleranceRad = m_InputValues->CAxisTolerance * nx::core::Constants::k_PiF / 180.0f;
      if(w <= cAxisToleranceRad || (nx::core::Constants::k_PiD - w) <= cAxisToleranceRad)
      {
        m_FeatureParentIds.setValue(neighborFeature, newFid);
        if(m_InputValues->UseRunningAverage)
        {
          c2 = c2 * m_Volumes.getValue(neighborFeature);
          m_AvgCAxes = m_AvgCAxes + c2;
        }
        return true;
      }
    }
  }
  return false;
}
