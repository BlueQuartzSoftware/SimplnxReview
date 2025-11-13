#include "MergeColonies.hpp"

#include "simplnx/Common/Array.hpp"
#include "simplnx/Common/Constants.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataGroup.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"
#include "simplnx/Utilities/Math/GeometryMath.hpp"
#include "simplnx/Utilities/Math/MatrixMath.hpp"

#include <EbsdLib/Core/EbsdLibConstants.h>
#include <EbsdLib/Core/Orientation.hpp>
#include <EbsdLib/Math/Matrix3X3.hpp>
#include <EbsdLib/Orientation/OrientationFwd.hpp>
#include <EbsdLib/Orientation/Quaternion.hpp>

using namespace nx::core;
using LaueOpsShPtrType = std::shared_ptr<ebsdlib::LaueOps>;
using LaueOpsContainer = std::vector<LaueOpsShPtrType>;

namespace
{
const float64 unit110 = 1.0 / std::sqrt(2.0);
const float64 unit111 = 1.0 / std::sqrt(3.0);
const float64 unit112_1 = 1.0 / std::sqrt(6.0);
const float64 unit112_2 = 2.0 / std::sqrt(6.0);

std::vector<ebsdlib::Matrix3X3D> crystalDirections = {
    {unit111, unit112_1, unit110, -unit111, -unit112_1, unit110, unit111, -unit112_2, 0}, {-unit111, unit112_1, unit110, unit111, -unit112_1, unit110, unit111, unit112_2, 0},
    {unit111, -unit112_1, unit110, unit111, -unit112_1, -unit110, unit111, unit112_2, 0}, {unit111, unit112_1, unit110, unit111, unit112_1, -unit110, -unit111, unit112_2, 0},
    {unit111, unit112_1, unit110, unit111, -unit112_2, 0, unit111, unit112_1, -unit110},  {unit111, -unit112_1, unit110, -unit111, -unit112_2, 0, unit111, -unit112_1, -unit110},
    {unit111, -unit112_1, unit110, unit111, unit112_2, 0, -unit111, unit112_1, unit110},  {-unit111, -unit112_1, unit110, unit111, -unit112_2, 0, unit111, unit112_1, unit110},
    {unit111, -unit112_2, 0, unit111, unit112_1, unit110, -unit111, -unit112_1, unit110}, {unit111, unit112_2, 0, -unit111, unit112_1, unit110, unit111, -unit112_1, unit110},
    {unit111, unit112_2, 0, unit111, -unit112_1, unit110, unit111, -unit112_1, -unit110}, {-unit111, unit112_2, 0, unit111, unit112_1, unit110, unit111, unit112_1, -unit110}};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool check_for_burgers(const ebsdlib::QuatD& betaQuat, const ebsdlib::QuatD& alphaQuat, float64 angleTolerance)
{
  float64 dP = 0.0;
  float64 angle = 0.0;
  float64 radToDeg = 180.0 / nx::core::Constants::k_PiD;

  // float64 gBeta[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  // float64 gBetaT[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
  ebsdlib::Matrix3X3D gBeta = ebsdlib::QuaternionDType(betaQuat).toOrientationMatrix().toGMatrix();
  // transpose gBeta so the sample direction is the output when
  // gBeta is multiplied by the crystal directions below
  ebsdlib::Matrix3X3D gBetaT = gBeta.transpose();

  ebsdlib::Matrix3X3D gAlpha = ebsdlib::QuaternionDType(alphaQuat).toOrientationMatrix().toGMatrix();

  // transpose gBeta so the sample direction is the output when
  // gBeta is multiplied by the crystal directions below
  ebsdlib::Matrix3X3D gAlphaT = gAlpha.transpose();

  ebsdlib::Matrix3X3D mat;
  for(int32 i = 0; i < 12; i++)
  {
    ebsdlib::Matrix3X3D crystalDirection(crystalDirections[i]);
    mat = gBetaT * crystalDirection;

    Point3Dd a = Point3Dd(mat[2], mat[5], mat[8]);
    Point3Dd b = Point3Dd(gAlphaT[2], gAlphaT[5], gAlphaT[8]);
    dP = GeometryMath::CosThetaBetweenVectors(a, b);
    dP = std::clamp(dP, -1.0, 1.0);
    angle = std::acos(dP);
    if((angle * radToDeg) < angleTolerance || (180.0f - (angle * radToDeg)) < angleTolerance)
    {
      a[0] = mat[0];
      a[1] = mat[3];
      a[2] = mat[6];
      b[0] = gAlphaT[0];
      b[1] = gAlphaT[3];
      b[2] = gAlphaT[6];
      dP = GeometryMath::CosThetaBetweenVectors(a, b);
      angle = std::acos(dP);
      if((angle * radToDeg) < angleTolerance)
      {
        return true;
      }
      if((180.0 - (angle * radToDeg)) < angleTolerance)
      {
        return true;
      }
      b[0] = -0.5 * gAlphaT[0] + 0.866025 * gAlphaT[1];
      b[1] = -0.5 * gAlphaT[3] + 0.866025 * gAlphaT[4];
      b[2] = -0.5 * gAlphaT[6] + 0.866025 * gAlphaT[7];
      dP = GeometryMath::CosThetaBetweenVectors(a, b);
      angle = std::acos(dP);
      if((angle * radToDeg) < angleTolerance)
      {
        return true;
      }
      if((180.0 - (angle * radToDeg)) < angleTolerance)
      {
        return true;
      }
      b[0] = -0.5 * gAlphaT[0] - 0.866025 * gAlphaT[1];
      b[1] = -0.5 * gAlphaT[3] - 0.866025 * gAlphaT[4];
      b[2] = -0.5 * gAlphaT[6] - 0.866025 * gAlphaT[7];
      dP = GeometryMath::CosThetaBetweenVectors(a, b);
      angle = std::acos(dP);
      if((angle * radToDeg) < angleTolerance)
      {
        return true;
      }
      if((180.0 - (angle * radToDeg)) < angleTolerance)
      {
        return true;
      }
    }
  }
  return false;
}
} // namespace

// -----------------------------------------------------------------------------
MergeColonies::MergeColonies(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, MergeColoniesInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
, m_OrientationOps(ebsdlib::LaueOps::GetAllOrientationOps())
, m_FeatureParentIds(dataStructure.getDataRefAs<Int32Array>(inputValues->FeatureParentIdsPath))
, m_FeaturePhases(dataStructure.getDataRefAs<Int32Array>(inputValues->FeaturePhasesPath))
, m_AvgQuats(dataStructure.getDataRefAs<Float32Array>(inputValues->AvgQuatsPath))
, m_CrystalStructures(dataStructure.getDataRefAs<UInt32Array>(inputValues->CrystalStructuresPath))
, m_AxisToleranceRad(inputValues->AxisTolerance * Constants::k_PiF / 180.0f)
, m_AngleTolerance(inputValues->AngleTolerance)
{
}

// -----------------------------------------------------------------------------
MergeColonies::~MergeColonies() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& MergeColonies::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
bool MergeColonies::growPatch(int32_t currentPatch)
{
  return false;
}

// -----------------------------------------------------------------------------
bool MergeColonies::growGrouping(int32_t referenceFeature, int32_t neighborFeature, int32_t newFid)
{
  return false;
}

// -----------------------------------------------------------------------------
void MergeColonies::execute()
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
              bool ok = false;
              neigh = nonContigNeighList->getValue(firstfeature, l, ok);
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
Result<> MergeColonies::operator()()
{
  execute();

  auto active = m_DataStructure.getDataRefAs<BoolArray>(m_InputValues->ActivePath);

  usize totalFeatures = active.getNumberOfTuples();
  if(totalFeatures < 2)
  {
    return MakeErrorResult(-87000, "The number of Grouped Features was 0 or 1 which means no grouped "
                                   "Features were detected. A grouping value may be set too high");
  }

  auto featureIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeatureIdsPath);
  auto cellParentIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->CellParentIdsPath);

  int32 numParents = 0;
  usize totalPoints = featureIds.getNumberOfTuples();
  for(usize k = 0; k < totalPoints; k++)
  {
    int32 featurename = featureIds[k];
    cellParentIds[k] = m_FeatureParentIds[featurename];
    if(m_FeatureParentIds[featurename] > numParents)
    {
      numParents = m_FeatureParentIds[featurename];
    }
  }
  numParents += 1;

  m_MessageHandler({IFilter::Message::Type::Info, "Characterizing Colonies Starting"});
  characterize_colonies();
  m_MessageHandler({IFilter::Message::Type::Info, "Characterizing Colonies Complete"});

  if(m_InputValues->RandomizeParentIds)
  {
    m_MessageHandler({IFilter::Message::Type::Info, "Randomizing Parent Ids...."});
    // Generate all the numbers up front
    const int32 rangeMin = 1;
    const int32 rangeMax = numParents - 1;
    std::mt19937 generator(m_InputValues->SeedValue); // Standard mersenne_twister_engine seeded
    std::uniform_int_distribution<int32> distribution(rangeMin, rangeMax);

    std::vector<int32> pid(numParents);
    pid.push_back(0);
    std::set<int32> parentIdSet;
    parentIdSet.insert(0);
    for(int32 i = 1; i < numParents; ++i)
    {
      pid.push_back(i); // numberGenerator();
      parentIdSet.insert(pid[i]);
    }

    int32 r = 0;
    int32 temp = 0;

    m_MessageHandler({IFilter::Message::Type::Info, "Shuffle elements ...."});
    //--- Shuffle elements by randomly exchanging each with one other.
    for(int32 i = 1; i < numParents; i++)
    {
      r = distribution(generator); // Random remaining position.
      if(r >= numParents)
      {
        continue;
      }
      temp = pid[i];
      pid[i] = pid[r];
      pid[r] = temp;
    }

    m_MessageHandler({IFilter::Message::Type::Info, "Adjusting Feature Ids Array...."});
    // Now adjust all the Feature Id values for each Voxel
    for(usize i = 0; i < totalPoints; ++i)
    {
      cellParentIds[i] = pid[cellParentIds[i]];
      m_FeatureParentIds[featureIds[i]] = cellParentIds[i];
    }
  }

  return {};
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int32 MergeColonies::getSeed(int32 newFid)
{
  usize numFeatures = m_FeaturePhases.getNumberOfTuples();

  std::mt19937 generator(m_InputValues->SeedValue); // Standard mersenne_twister_engine seeded
  std::uniform_real_distribution<float32> distribution(0, 1);
  int32 seed = -1;
  int32 randFeature = 0;

  // Precalculate some constants
  usize totalFMinus1 = numFeatures - 1;

  usize counter = 0;
  randFeature = int32(distribution(generator) * float32(totalFMinus1));
  while(seed == -1 && counter < numFeatures)
  {
    if(randFeature > totalFMinus1)
    {
      randFeature = randFeature - numFeatures;
    }
    if(m_FeatureParentIds[randFeature] == -1)
    {
      seed = randFeature;
    }
    randFeature++;
    counter++;
  }
  if(seed >= 0)
  {
    m_FeatureParentIds[seed] = newFid;
    std::vector<usize> tDims(1, newFid + 1);
    m_DataStructure.getDataRefAs<AttributeMatrix>(m_InputValues->CellFeatureAMPath).resizeTuples(tDims);
  }
  return seed;
}

// -----------------------------------------------------------------------------
bool MergeColonies::determineGrouping(int32 referenceFeature, int32 neighborFeature, int32 newFid)
{
  float64 w = std::numeric_limits<float64>::max();
  bool colony = false;

  if(m_FeatureParentIds[neighborFeature] == -1 && m_FeaturePhases[referenceFeature] > 0 && m_FeaturePhases[neighborFeature] > 0)
  {
    usize avgQuatIdx = referenceFeature * 4;
    ebsdlib::QuatD q1(m_AvgQuats[avgQuatIdx], m_AvgQuats[avgQuatIdx + 1], m_AvgQuats[avgQuatIdx + 2], m_AvgQuats[avgQuatIdx + 3]);
    avgQuatIdx = neighborFeature * 4;
    ebsdlib::QuatD q2(m_AvgQuats[avgQuatIdx], m_AvgQuats[avgQuatIdx + 1], m_AvgQuats[avgQuatIdx + 2], m_AvgQuats[avgQuatIdx + 3]);

    uint32 laueClass1 = m_CrystalStructures[m_FeaturePhases[referenceFeature]];
    uint32 laueClass2 = m_CrystalStructures[m_FeaturePhases[neighborFeature]];
    if(laueClass1 == laueClass2 && (laueClass1 == ebsdlib::CrystalStructure::Hexagonal_High))
    {
      ebsdlib::AxisAngleDType ax = m_OrientationOps[laueClass1]->calculateMisorientation(q1, q2);

      auto rod = ax.toRodrigues();
      rod = m_OrientationOps[laueClass1]->getMDFFZRod(rod);
      ax = rod.toAxisAngle();

      w = ax[3] * (Constants::k_180OverPiD);
      float angdiff1 = std::fabs(w - 10.53f);
      float axisdiff1 = std::acos(
          /*std::fabs(n1) * 0.0000f + std::fabs(n2) * 0.0000f +*/ std::fabs(ax[2]) /* * 1.0000f */);
      if(angdiff1 < m_AngleTolerance && axisdiff1 < m_AxisToleranceRad)
      {
        colony = true;
      }
      float angdiff2 = std::fabs(w - 90.00f);
      float axisdiff2 = std::acos(std::fabs(ax[0]) * 0.9958f + std::fabs(ax[1]) * 0.0917f /* + std::fabs(n3) * 0.0000f */);
      if(angdiff2 < m_AngleTolerance && axisdiff2 < m_AxisToleranceRad)
      {
        colony = true;
      }
      float angdiff3 = std::fabs(w - 60.00f);
      float axisdiff3 = std::acos(std::fabs(ax[0]) /* * 1.0000f + std::fabs(n2) * 0.0000f + std::fabs(n3) * 0.0000f*/);
      if(angdiff3 < m_AngleTolerance && axisdiff3 < m_AxisToleranceRad)
      {
        colony = true;
      }
      float angdiff4 = std::fabs(w - 60.83f);
      float axisdiff4 = std::acos(std::fabs(ax[0]) * 0.9834f + std::fabs(ax[1]) * 0.0905f + std::fabs(ax[2]) * 0.1570f);
      if(angdiff4 < m_AngleTolerance && axisdiff4 < m_AxisToleranceRad)
      {
        colony = true;
      }
      float angdiff5 = std::fabs(w - 63.26f);
      float axisdiff5 = std::acos(std::fabs(ax[0]) * 0.9549f /* + std::fabs(n2) * 0.0000f */ + std::fabs(ax[2]) * 0.2969f);
      if(angdiff5 < m_AngleTolerance && axisdiff5 < m_AxisToleranceRad)
      {
        colony = true;
      }
      if(colony)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }
    }
    else if(ebsdlib::CrystalStructure::Cubic_High == laueClass2 && ebsdlib::CrystalStructure::Hexagonal_High == laueClass1)
    {
      colony = check_for_burgers(q2, q1, m_AngleTolerance);
      if(colony)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }
    }
    else if(ebsdlib::CrystalStructure::Cubic_High == laueClass1 && ebsdlib::CrystalStructure::Hexagonal_High == laueClass2)
    {
      colony = check_for_burgers(q1, q2, m_AngleTolerance);
      if(colony)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }
    }
  }
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MergeColonies::characterize_colonies()
{
}
