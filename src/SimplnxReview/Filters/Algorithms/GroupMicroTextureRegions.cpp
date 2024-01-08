#include "GroupMicroTextureRegions.hpp"

#include "simplnx/Common/Constants.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataGroup.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"
#include "simplnx/Utilities/Math/GeometryMath.hpp"
#include "simplnx/Utilities/Math/MatrixMath.hpp"

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
  NeighborList<int32>& neighborlist = m_DataStructure.getDataRefAs<NeighborList<int32>>(m_InputValues->ContiguousNeighborListArrayPath);
  NeighborList<int32>* nonContigNeighList = nullptr;
  if(m_InputValues->UseNonContiguousNeighbors)
  {
    nonContigNeighList = m_DataStructure.getDataAs<NeighborList<int32>>(m_InputValues->NonContiguousNeighborListArrayPath);
  }

  std::vector<int32> grouplist;

  int32 parentcount = 0;
  int32 seed = 0;
  int32 list1size = 0, list2size = 0, listsize = 0;
  int32 neigh = 0;
  bool patchGrouping = false;

  while(seed >= 0)
  {
    parentcount++;
    seed = getSeed(parentcount);
    if(seed >= 0)
    {
      grouplist.push_back(seed);
      for(std::vector<int32>::size_type j = 0; j < grouplist.size(); j++)
      {
        int32 firstfeature = grouplist[j];
        list1size = int32(neighborlist[firstfeature].size());
        if(m_InputValues->UseNonContiguousNeighbors)
        {
          list2size = nonContigNeighList->getListSize(firstfeature);
        }
        for(int32 k = 0; k < 2; k++)
        {
          if(patchGrouping)
          {
            k = 1;
          }
          if(k == 0)
          {
            listsize = list1size;
          }
          else if(k == 1)
          {
            listsize = list2size;
          }
          for(int32 l = 0; l < listsize; l++)
          {
            if(k == 0)
            {
              neigh = neighborlist[firstfeature][l];
            }
            else if(k == 1)
            {
              neigh = nonContigNeighList->getListReference(firstfeature)[l];
            }
            if(neigh != firstfeature)
            {
              if(determineGrouping(firstfeature, neigh, parentcount))
              {
                if(!patchGrouping)
                {
                  grouplist.push_back(neigh);
                }
              }
            }
          }
        }
      }
      if(patchGrouping)
      {
        if(growPatch(parentcount))
        {
          for(std::vector<int32_t>::size_type j = 0; j < grouplist.size(); j++)
          {
            int32_t firstfeature = grouplist[j];
            listsize = int32_t(neighborlist[firstfeature].size());
            for(int32_t l = 0; l < listsize; l++)
            {
              neigh = neighborlist[firstfeature][l];
              if(neigh != firstfeature)
              {
                if(growGrouping(firstfeature, neigh, parentcount))
                {
                  grouplist.push_back(neigh);
                }
              }
            }
          }
        }
      }
    }
    grouplist.clear();
  }
}

// -----------------------------------------------------------------------------
Result<> GroupMicroTextureRegions::operator()()
{
  m_Generator = std::mt19937_64(m_InputValues->SeedValue);
  m_Distribution = std::uniform_real_distribution<float32>(0.0f, 1.0f);

  m_AvgCAxes[0] = 0.0f;
  m_AvgCAxes[1] = 0.0f;
  m_AvgCAxes[2] = 0.0f;
  auto& featureParentIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeatureParentIdsArrayName);
  featureParentIds.fill(-1);

  execute();

  // handle active array resize
  if(m_NumTuples < 2)
  {
    return MakeErrorResult(-87000, fmt::format("The number of grouped Features was {} which means no grouped Features were detected. A grouping value may be set too high", m_NumTuples));
  }
  m_DataStructure.getDataRefAs<AttributeMatrix>(m_InputValues->NewCellFeatureAttributeMatrixName).resizeTuples(AttributeMatrix::ShapeType{m_NumTuples});

  auto& cellParentIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->CellParentIdsArrayName);
  auto& featureIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeatureIdsArrayPath);
  usize totalPoints = featureIds.getNumberOfTuples();
  for(usize k = 0; k < totalPoints; k++)
  {
    cellParentIds[k] = featureParentIds[featureIds[k]];
  }

  // By default we randomize grains !!! COMMENT OUT FOR DEMONSTRATION !!!
  m_MessageHandler(IFilter::Message::Type::Info, "Randomizing Parent Ids");
  RandomizeFeatureIds(totalPoints, m_NumTuples, cellParentIds, featureParentIds, featureIds, m_InputValues->SeedValue);

  return {};
}

// -----------------------------------------------------------------------------
int GroupMicroTextureRegions::getSeed(int32 newFid)
{
  auto& featureParentIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeatureParentIdsArrayName);
  auto& featurePhases = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeaturePhasesArrayPath);

  usize numFeatures = featurePhases.getNumberOfTuples();

  float32 g1[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float32 g1t[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  int32 voxelSeed = -1;

  // Precalculate some constants
  int32 totalFMinus1 = numFeatures - 1;

  usize counter = 0;
  auto randFeature = static_cast<int32>(m_Distribution(m_Generator) * static_cast<float32>(totalFMinus1));
  while(voxelSeed == -1 && counter < numFeatures)
  {
    if(randFeature > totalFMinus1)
    {
      randFeature = randFeature - numFeatures;
    }
    if(featureParentIds[randFeature] == -1)
    {
      voxelSeed = randFeature;
    }
    randFeature++;
    counter++;
  }

  //  // Used for debugging and demonstration
  //  if(newFid == 1)
  //  {
  //    auto& centroids = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->VolumesArrayPath.getParent().createChildPath("Centroids"));
  //    std::ofstream fout ("/tmp/GroupMicroTextureInitialVoxelSeeds.txt", std::ios_base::out | std::ios_base::app);
  //    fout << fmt::format("Feature Parent Id: {} | X: {}, Y: {}\n", voxelSeed, centroids.getComponent(voxelSeed, 0), centroids.getComponent(voxelSeed, 1));
  //  }

  if(voxelSeed >= 0)
  {
    featureParentIds[voxelSeed] = newFid;
    m_NumTuples = newFid + 1;

    if(m_InputValues->UseRunningAverage)
    {
      auto& volumes = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->VolumesArrayPath);
      auto& avgQuats = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->AvgQuatsArrayPath);

      usize index = voxelSeed * 4;
      OrientationTransformation::qu2om<QuatF, OrientationF>({avgQuats[index + 0], avgQuats[index + 1], avgQuats[index + 2], avgQuats[index + 3]}).toGMatrix(g1);

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
      MatrixMath::Multiply3x1withConstant(m_AvgCAxes.data(), volumes[voxelSeed]);
    }
  }

  return voxelSeed;
}

// -----------------------------------------------------------------------------
bool GroupMicroTextureRegions::determineGrouping(int32 referenceFeature, int32 neighborFeature, int32 newFid)
{
  uint32 phase1 = 0;
  float32 g1[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float32 g2[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float32 g1t[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  float32 g2t[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  std::array<float32, 3> c1 = {0.0f, 0.0f, 0.0f};
  std::array<float32, 3> caxis = {0.0f, 0.0f, 1.0f};

  auto& featurePhases = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeaturePhasesArrayPath);
  auto& featureParentIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeatureParentIdsArrayName);
  auto& crystalStructures = m_DataStructure.getDataRefAs<UInt32Array>(m_InputValues->CrystalStructuresArrayPath);
  if(featureParentIds[neighborFeature] == -1 && featurePhases[referenceFeature] > 0 && featurePhases[neighborFeature] > 0)
  {
    auto& avgQuats = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->AvgQuatsArrayPath);
    if(!m_InputValues->UseRunningAverage)
    {
      usize index = referenceFeature * 4;
      phase1 = crystalStructures[featurePhases[referenceFeature]];
      OrientationTransformation::qu2om<QuatF, Orientation<float32>>({avgQuats[index + 0], avgQuats[index + 1], avgQuats[index + 2], avgQuats[index + 3]}).toGMatrix(g1);

      // transpose the g matrix so when c-axis is multiplied by it,
      // it will give the sample direction that the c-axis is along
      MatrixMath::Transpose3x3(g1, g1t);
      MatrixMath::Multiply3x3with3x1(g1t, caxis.data(), c1.data());
      // normalize so that the dot product can be taken below without
      // dividing by the magnitudes (they would be 1)
      MatrixMath::Normalize3x1(c1.data());
    }
    uint32 phase2 = crystalStructures[featurePhases[neighborFeature]];
    if(phase1 == phase2 && (phase1 == EbsdLib::CrystalStructure::Hexagonal_High))
    {
      usize index = neighborFeature * 4;
      OrientationTransformation::qu2om<QuatF, OrientationF>({avgQuats[index + 0], avgQuats[index + 1], avgQuats[index + 2], avgQuats[index + 3]}).toGMatrix(g2);

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

      if(w < -1.0f)
      {
        w = -1.0f;
      }
      else if(w > 1.0f)
      {
        w = 1.0f;
      }
      w = std::acos(w);

      // Convert user defined tolerance to radians.
      float32 cAxisToleranceRad = m_InputValues->CAxisTolerance * nx::core::Constants::k_PiD / 180.0f;
      if(w <= cAxisToleranceRad || (nx::core::Constants::k_PiD - w) <= cAxisToleranceRad)
      {
        featureParentIds[neighborFeature] = newFid;
        if(m_InputValues->UseRunningAverage)
        {
          auto& volumes = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->VolumesArrayPath);
          MatrixMath::Multiply3x1withConstant(c2.data(), volumes[neighborFeature]);
          MatrixMath::Add3x1s(m_AvgCAxes.data(), c2.data(), m_AvgCAxes.data());
        }
        return true;
      }
    }
  }
  return false;
}
