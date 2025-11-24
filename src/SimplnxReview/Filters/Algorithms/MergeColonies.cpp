#include "MergeColonies.hpp"

#include "simplnx/Common/Array.hpp"
#include "simplnx/Common/Constants.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"
#include "simplnx/Utilities/Math/GeometryMath.hpp"
#include "simplnx/Utilities/MessageHelper.hpp"

#include "EbsdLib/Core/EbsdLibConstants.h"
#include "EbsdLib/Core/Orientation.hpp"
#include "EbsdLib/Orientation/Quaternion.hpp"

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
  constexpr float64 radToDeg = 180.0 / Constants::k_PiD;

  // transpose gBeta so the sample direction is the output when
  // gBeta is multiplied by the crystal directions below
  const ebsdlib::Matrix3X3D gBetaT = betaQuat.toOrientationMatrix().transpose().toGMatrix();
  // transpose gBeta so the sample direction is the output when
  // gBeta is multiplied by the crystal directions below
  ebsdlib::Matrix3X3D gAlphaT = alphaQuat.toOrientationMatrix().transpose().toGMatrix();

  for(int32 i = 0; i < 12; i++)
  {
    ebsdlib::Matrix3X3D mat = gBetaT * crystalDirections[i];

    ebsdlib::Matrix3X1D a(mat[2], mat[5], mat[8]);
    ebsdlib::Matrix3X1D b(gAlphaT[2], gAlphaT[5], gAlphaT[8]);

    dP = a.cosTheta(b);
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
      dP = a.cosTheta(b);
      dP = std::clamp(dP, -1.0, 1.0);
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
      dP = a.cosTheta(b);
      dP = std::clamp(dP, -1.0, 1.0);
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
      dP = a.cosTheta(b);
      dP = std::clamp(dP, -1.0, 1.0);
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
Result<> MergeColonies::execute()
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
    return MakeErrorResult(-99342, "There was an error getting the Non-contiguous neighborlist from the DataStructure");
  }

  std::vector<int32> groupList;

  int32 parentCount = 0;
  int32 featureSeed = 0;
  int32 featureNeighborListSize = 0, nonContigNeighListSize = 0;
  bool patchGrouping = false;

  while(featureSeed >= 0)
  {
    parentCount++;
    featureSeed = getSeed(parentCount);
    if(featureSeed >= 0)
    {
      groupList.push_back(featureSeed);
      // Loop over the current `groupList` vector which can grow during the looping
      for(std::vector<int32>::size_type j = 0; j < groupList.size(); j++)
      {
        int32 firstFeature = groupList[j];
        featureNeighborListSize = static_cast<int32>(featureNeighborListRef[firstFeature].size());
        if(m_InputValues->UseNonContiguousNeighbors)
        {
          nonContigNeighListSize = nonContigNeighListPtr->getListSize(firstFeature);
        }

        // There are 2 kinds of NeighborLists so we loop on both of them
        // k=0: FeatureNeighborList
        // k=1: FeatureNeighborHood (non-contiguous neighbors)
        for(int32 k = 0; k < 2; k++)
        {
          // If we are patchGrouping, then skip the first group
          if(patchGrouping)
          {
            k = 1;
          }
          int32 currentListSize = 0;
          if(k == 0)
          {
            currentListSize = featureNeighborListSize;
          }
          else if(k == 1)
          {
            currentListSize = nonContigNeighListSize;
          }
          // Loop over which ever NeighborList we are currently using
          for(int32 l = 0; l < currentListSize; l++)
          {
            int32 neigh = -1;
            if(k == 0) // Feature Neighbor List
            {
              neigh = featureNeighborListRef[firstFeature][l];
            }
            else if(k == 1 && m_InputValues->UseNonContiguousNeighbors) // Feature NeighborHood (non-contiguous)
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
            int32_t firstfeature = groupList[j];
            int32 currentListSize = static_cast<int32>(featureNeighborListRef[firstfeature].size());
            for(int32_t l = 0; l < currentListSize; l++)
            {
              int32 neigh = featureNeighborListRef[firstfeature][l];
              if(neigh != firstfeature)
              {
                if(growGrouping(firstfeature, neigh, parentCount))
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
Result<> MergeColonies::operator()()
{
  // Initialize the random number generator
  m_Generator = std::mt19937_64(m_InputValues->SeedValue);
  m_Distribution = std::uniform_real_distribution<float32>(0.0f, 1.0f);

  // The main algorithm is in the 'execute()' method
  Result<> result = execute();
  if(result.invalid())
  {
    return result;
  }

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
    // Now adjust all the FeatureId values for each Voxel
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
  const usize numFeatures = m_FeaturePhases.getNumberOfTuples();

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

  // Resize the created Feature Attribute Matrix
  if(featureIdSeed >= 0)
  {
    m_FeatureParentIds[featureIdSeed] = newFid;
    const std::vector<usize> tDims = {static_cast<usize>(newFid + 1)};
    m_DataStructure.getDataRefAs<AttributeMatrix>(m_InputValues->CellFeatureAMPath).resizeTuples(tDims);
  }
  return featureIdSeed;
}

// -----------------------------------------------------------------------------
bool MergeColonies::determineGrouping(int32 referenceFeature, int32 neighborFeature, int32 newFid) const
{
  // The phase is valid for both features and the neighbor feature has not been grouped yet.
  if(m_FeatureParentIds[neighborFeature] == -1 && m_FeaturePhases[referenceFeature] > 0 && m_FeaturePhases[neighborFeature] > 0)
  {
    usize avgQuatIdx = referenceFeature * 4;
    const ebsdlib::QuatD q1(m_AvgQuats[avgQuatIdx], m_AvgQuats[avgQuatIdx + 1], m_AvgQuats[avgQuatIdx + 2], m_AvgQuats[avgQuatIdx + 3]);
    avgQuatIdx = neighborFeature * 4;
    const ebsdlib::QuatD q2(m_AvgQuats[avgQuatIdx], m_AvgQuats[avgQuatIdx + 1], m_AvgQuats[avgQuatIdx + 2], m_AvgQuats[avgQuatIdx + 3]);

    // Make sure both features are of the same Laue class and the Laue class is hexagonal
    const uint32 laueClass1 = m_CrystalStructures[m_FeaturePhases[referenceFeature]];
    const uint32 laueClass2 = m_CrystalStructures[m_FeaturePhases[neighborFeature]];
    if(laueClass1 == laueClass2 && (laueClass1 == ebsdlib::CrystalStructure::Hexagonal_High))
    {
      ebsdlib::AxisAngleDType ax = m_OrientationOps[laueClass1]->calculateMisorientation(q1, q2);

      ebsdlib::Rodrigues<double> rod = ax.toRodrigues();
      rod = m_OrientationOps[laueClass1]->getMDFFZRod(rod);
      ax = rod.toAxisAngle();
      const float32 w = ax[3] * (Constants::k_180OverPiD); // Convert to degrees

      // Test each of the special Axis-Angle relationships
      // c = <0001>
      float angdiff1 = std::fabs(w - 10.529f);
      float axisdiff1 = std::acosf(std::fabs(ax[2]));
      if(angdiff1 < m_AngleTolerance && axisdiff1 < m_AxisToleranceRad)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }

      // a2 = <-12-10>
      float angdiff3 = std::fabs(w - 60.00f);
      float axisdiff3 = std::acosf(std::fabs(ax[0]));
      if(angdiff3 < m_AngleTolerance && axisdiff3 < m_AxisToleranceRad)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }

      // d1 at 80.97 degrees from c in the plane of (d3,c)
      float angdiff4 = std::fabs(w - 60.83f);
      float axisdiff4 = std::acosf(std::fabs(ax[0]) * 0.9834f + std::fabs(ax[1]) * 0.0905f + std::fabs(ax[2]) * 0.1570f);
      if(angdiff4 < m_AngleTolerance && axisdiff4 < m_AxisToleranceRad)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }

      // d2 at 72.73 degrees from c in the plane of (a2,c)
      float angdiff5 = std::fabs(w - 63.26f);
      float axisdiff5 = std::acosf(std::fabs(ax[0]) * 0.9549f + std::fabs(ax[2]) * 0.2969f);
      if(angdiff5 < m_AngleTolerance && axisdiff5 < m_AxisToleranceRad)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }

      // d3 at 5.26 degrees from a2 in the basal plane
      float angdiff2 = std::fabs(w - 90.00f);
      float axisdiff2 = std::acosf(std::fabs(ax[0]) * 0.9958f + std::fabs(ax[1]) * 0.0917f);
      if(angdiff2 < m_AngleTolerance && axisdiff2 < m_AxisToleranceRad)
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }
    }
    else if(ebsdlib::CrystalStructure::Cubic_High == laueClass2 && ebsdlib::CrystalStructure::Hexagonal_High == laueClass1)
    {
      if(check_for_burgers(q2, q1, m_AngleTolerance))
      {
        m_FeatureParentIds[neighborFeature] = newFid;
        return true;
      }
    }
    else if(ebsdlib::CrystalStructure::Cubic_High == laueClass1 && ebsdlib::CrystalStructure::Hexagonal_High == laueClass2)
    {
      if(check_for_burgers(q1, q2, m_AngleTolerance))
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
