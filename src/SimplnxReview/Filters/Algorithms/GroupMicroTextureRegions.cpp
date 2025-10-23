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

namespace
{
void RandomizeFeatureIds(usize totalPoints, usize totalFeatures, Int32Array& cellParentIds, Int32Array& featureParentIds, const Int32Array& featureIds, uint64 seed)
{
  // Generate an even distribution of numbers between the min and max range
  std::mt19937_64 gen(seed);
  std::uniform_int_distribution<int64> dist(0, totalFeatures - 1);

  std::vector<int32> gid(totalFeatures);
  std::iota(gid.begin(), gid.end(), 0);

  //--- Shuffle elements by randomly exchanging each with one other.
  for(usize i = 1; i < totalFeatures; i++)
  {
    auto r = static_cast<int32>(dist(gen)); // Random remaining position.
    if(r >= totalFeatures)
    {
      continue;
    }

    int32 temp = gid[i];
    gid[i] = gid[r];
    gid[r] = temp;
  }

  // Now adjust all the Grain id values for each Voxel
  for(usize i = 0; i < totalPoints; ++i)
  {
    cellParentIds[i] = gid[cellParentIds[i]];
    featureParentIds[featureIds[i]] = cellParentIds[i];
  }
}
} // namespace

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
void GroupMicroTextureRegions::execute()
{
  MessageHelper messageHelper(m_MessageHandler);
  ThrottledMessenger throttledMessenger = messageHelper.createThrottledMessenger();

  NeighborList<int32>& featureNeighborListRef = m_DataStructure.getDataRefAs<NeighborList<int32>>(m_InputValues->ContiguousNeighborListArrayPath);
  NeighborList<int32>* nonContigNeighList = nullptr;
  if(m_InputValues->UseNonContiguousNeighbors)
  {
    nonContigNeighList = m_DataStructure.getDataAs<NeighborList<int32>>(m_InputValues->NonContiguousNeighborListArrayPath);
  }

  std::vector<int32> groupList;

  int32 parentCount = 0;
  int32 featureSeed = 0;
  int32 list1size = 0, list2size = 0, listSize = 0;
  int32 neigh = 0;
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
          list2size = nonContigNeighList->getListSize(firstFeature);
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
            if(k == 0)
            {
              neigh = featureNeighborListRef[firstFeature][l];
            }
            else if(k == 1)
            {
              bool ok = false;
              neigh = nonContigNeighList->getValue(firstFeature, l, ok);
            }
            if(neigh != firstFeature)
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
              neigh = featureNeighborListRef[firstFeature][l];
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
}

// -----------------------------------------------------------------------------
Result<> GroupMicroTextureRegions::operator()()
{
  MessageHelper messageHelper(m_MessageHandler);

  m_Generator = std::mt19937_64(std::mt19937::default_seed);
  m_Distribution = std::uniform_real_distribution<float32>(0.0f, 1.0f);

  m_AvgCAxes[0] = 0.0f;
  m_AvgCAxes[1] = 0.0f;
  m_AvgCAxes[2] = 0.0f;
  m_FeatureParentIds.fill(-1);

  // Execute the main grouping algorithm
  messageHelper.sendMessage(fmt::format("Starting Grouping....."));

  execute();

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

  // By default we randomize grains !!! COMMENT OUT FOR DEMONSTRATION !!!
  // m_MessageHandler(IFilter::Message::Type::Info, "Randomizing Parent Ids");
  // RandomizeFeatureIds(totalPoints, m_NumTuples, cellParentIds, m_FeatureParentIds, featureIds, m_InputValues->SeedValue);

  return {};
}

// -----------------------------------------------------------------------------
int GroupMicroTextureRegions::getSeed(int32 newFid)
{
  usize numFeatures = m_FeaturePhases.getNumberOfTuples();

  float32 g1[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float32 g1t[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
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
      OrientationTransformation::qu2om<QuatF, OrientationF>({m_AvgQuats.getValue(index + 0), m_AvgQuats.getValue(index + 1), m_AvgQuats.getValue(index + 2), m_AvgQuats.getValue(index + 3)})
          .toGMatrix(g1);

      std::array<float32, 3> c1 = {0.0f, 0.0f, 0.0f};
      std::array<float32, 3> cAxis = {0.0f, 0.0f, 1.0f};
      // transpose the g matrix so when c-axis is multiplied by it,
      // it will give the sample direction that the c-axis is along
      MatrixMath::Transpose3x3(g1, g1t);
      MatrixMath::Multiply3x3with3x1(g1t, cAxis.data(), c1.data());
      // normalize so that the dot product can be taken below without
      // dividing by the magnitudes (they would be 1)
      MatrixMath::Normalize3x1(c1.data());
      MatrixMath::Copy3x1(c1.data(), m_AvgCAxes.data());
      MatrixMath::Multiply3x1withConstant(m_AvgCAxes.data(), m_Volumes.getValue(featureIdSeed));
    }
  }

  return featureIdSeed;
}

// -----------------------------------------------------------------------------
bool GroupMicroTextureRegions::determineGrouping(int32 referenceFeature, int32 neighborFeature, int32 newFid)
{
  uint32 phase1 = 0;
  /**
   * referenceFeature is Cubic Phase
   * neighborFeature is Hex Phase
   * m_InputValues->UseRunningAverage = TRUE
   * First `if` check is passed
   * Second `if` check is passed, `phase1` stays at HEX
   * Third `if` check will pass because the 2nd phase is HEX
   * Probably should not be happening?
   * Solution: Properly initialize the `phase` outside of all checks or just before the `phase2` initialization
   * Bug introduced JAN 30, 2014 by J. Tucker commit `7e49e52f362005e44ea9bf21b7a717277b2af04e` in Original DREAM3D repository

  */

  float32 g1[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float32 g2[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float32 g1t[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float32 g2t[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  std::array<float32, 3> c1 = {0.0f, 0.0f, 0.0f};
  std::array<float32, 3> caxis = {0.0f, 0.0f, 1.0f};

  int32 neighborParentId = m_FeatureParentIds.getValue(neighborFeature);
  int32 referenceFeaturePhase = m_FeaturePhases.getValue(referenceFeature);
  int32 neighborFeaturePhase = m_FeaturePhases.getValue(neighborFeature);

  if(neighborParentId == -1 && referenceFeaturePhase > 0 && neighborFeaturePhase > 0)
  {
    if(!m_InputValues->UseRunningAverage)
    {
      usize index = referenceFeature * 4;
      OrientationTransformation::qu2om<QuatF, Orientation<float32>>({m_AvgQuats[index + 0], m_AvgQuats[index + 1], m_AvgQuats[index + 2], m_AvgQuats[index + 3]}).toGMatrix(g1);

      // transpose the g matrix so when c-axis is multiplied by it,
      // it will give the sample direction that the c-axis is along
      MatrixMath::Transpose3x3(g1, g1t);
      MatrixMath::Multiply3x3with3x1(g1t, caxis.data(), c1.data());
      // normalize so that the dot product can be taken below without
      // dividing by the magnitudes (they would be 1)
      MatrixMath::Normalize3x1(c1.data());
    }
    phase1 = m_CrystalStructures.getValue(referenceFeaturePhase);
    uint32 phase2 = m_CrystalStructures.getValue(neighborFeaturePhase);
    if(phase1 == phase2 && (phase1 == EbsdLib::CrystalStructure::Hexagonal_High))
    {
      usize index = neighborFeature * 4;
      OrientationTransformation::qu2om<QuatF, OrientationF>({m_AvgQuats[index + 0], m_AvgQuats[index + 1], m_AvgQuats[index + 2], m_AvgQuats[index + 3]}).toGMatrix(g2);

      std::array<float32, 3> c2 = {0.0f, 0.0f, 0.0f};
      // transpose the g matrix so when c-axis is multiplied by it,
      // it will give the sample direction that the c-axis is along
      MatrixMath::Transpose3x3(g2, g2t);
      MatrixMath::Multiply3x3with3x1(g2t, caxis.data(), c2.data());
      // normalize so that the dot product can be taken below without
      // dividing by the magnitudes (they would be 1)
      MatrixMath::Normalize3x1(c2.data());

      float32 w;
      if(m_InputValues->UseRunningAverage)
      {
        w = GeometryMath::CosThetaBetweenVectors(Point3Df{m_AvgCAxes}, Point3Df{c2});
      }
      else
      {
        w = GeometryMath::CosThetaBetweenVectors(Point3Df{c1}, Point3Df{c2});
      }

      w = std::acos(std::clamp(w, -1.0f, 1.0f));

      // Convert user defined tolerance to radians.
      float32 cAxisToleranceRad = m_InputValues->CAxisTolerance * nx::core::Constants::k_PiD / 180.0f;
      if(w <= cAxisToleranceRad || (nx::core::Constants::k_PiD - w) <= cAxisToleranceRad)
      {
        m_FeatureParentIds.setValue(neighborFeature, newFid);
        if(m_InputValues->UseRunningAverage)
        {
          MatrixMath::Multiply3x1withConstant(c2.data(), m_Volumes.getValue(neighborFeature));
          MatrixMath::Add3x1s(m_AvgCAxes.data(), c2.data(), m_AvgCAxes.data());
        }
        return true;
      }
    }
  }
  return false;
}
