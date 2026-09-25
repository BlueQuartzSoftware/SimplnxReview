#include "ComputeMicroTextureRegions.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/Geometry/ImageGeom.hpp"
#include "simplnx/Utilities/ThrottledMessageHandler.hpp"

#include <algorithm>

using namespace nx::core;

// -----------------------------------------------------------------------------
ComputeMicroTextureRegions::ComputeMicroTextureRegions(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                                       ComputeMicroTextureRegionsInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
ComputeMicroTextureRegions::~ComputeMicroTextureRegions() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& ComputeMicroTextureRegions::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> ComputeMicroTextureRegions::operator()()
{
  auto& imageGeom = m_DataStructure.getDataRefAs<ImageGeom>(m_InputValues->ImageGeomPath);
  auto& featureIds = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->FeatureIdsArrayPath);
  auto& microTextureRegionNumCells = m_DataStructure.getDataRefAs<Int32Array>(m_InputValues->MicroTextureRegionNumCellsArrayPath);

  usize numMicroTextureRegions = microTextureRegionNumCells.getNumberOfTuples();

  usize xPoints = imageGeom.getNumXCells();
  usize yPoints = imageGeom.getNumYCells();
  usize zPoints = imageGeom.getNumZCells();
  FloatVec3 spacing = imageGeom.getSpacing();

  std::vector<float32> microTextureRegionXMins(numMicroTextureRegions, spacing[0] * static_cast<float32>(xPoints));
  std::vector<float32> microTextureRegionXMaxs(numMicroTextureRegions, 0.0f);
  std::vector<float32> microTextureRegionYMins(numMicroTextureRegions, spacing[1] * static_cast<float32>(yPoints));
  std::vector<float32> microTextureRegionYMaxs(numMicroTextureRegions, 0.0f);
  std::vector<float32> microTextureRegionZMins(numMicroTextureRegions, spacing[2] * static_cast<float32>(zPoints));
  std::vector<float32> microTextureRegionZMaxs(numMicroTextureRegions, 0.0f);

  std::for_each(featureIds.begin(), featureIds.end(), [&microTextureRegionNumCells](const int32 id) mutable { microTextureRegionNumCells[id].inc(); });

  ThrottledMessageHandler progressThrottle(m_MessageHandler);
  progressThrottle.reset(zPoints, "Finding MicroTexture Region Bounds");

  float32 x, y, z;
  usize zStride, yStride;
  for(usize i = 0; i < zPoints; i++)
  {
    if(m_ShouldCancel)
    {
      return {};
    }
    zStride = i * xPoints * yPoints;
    for(usize j = 0; j < yPoints; j++)
    {
      yStride = j * xPoints;
      for(usize k = 0; k < xPoints; k++)
      {
        int32 mtNum = featureIds[zStride + yStride + k];
        x = static_cast<float32>(k) * spacing[0];
        y = static_cast<float32>(j) * spacing[1];
        z = static_cast<float32>(i) * spacing[2];
        if(x > microTextureRegionXMaxs[mtNum])
        {
          microTextureRegionXMaxs[mtNum] = x;
        }
        if(y > microTextureRegionYMaxs[mtNum])
        {
          microTextureRegionYMaxs[mtNum] = y;
        }
        if(z > microTextureRegionZMaxs[mtNum])
        {
          microTextureRegionZMaxs[mtNum] = z;
        }
        if(x < microTextureRegionXMins[mtNum])
        {
          microTextureRegionXMins[mtNum] = x;
        }
        if(y < microTextureRegionYMins[mtNum])
        {
          microTextureRegionYMins[mtNum] = y;
        }
        if(z < microTextureRegionZMins[mtNum])
        {
          microTextureRegionZMins[mtNum] = z;
        }
      }
    }
    progressThrottle.updateCount(i + 1);
  }

  auto& microTextureRegionFractionOccupied = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->MicroTextureRegionFractionOccupiedArrayPath);
  progressThrottle.reset(numMicroTextureRegions > 1 ? numMicroTextureRegions - 1 : 0, "Computing MicroTexture Region Fractions");
  for(usize i = 1; i < numMicroTextureRegions; i++)
  {
    if(m_ShouldCancel)
    {
      return {};
    }
    float32 xLength = (microTextureRegionXMaxs[i] - microTextureRegionXMins[i]) + spacing[0];
    float32 yLength = (microTextureRegionYMaxs[i] - microTextureRegionYMins[i]) + spacing[1];
    if(zPoints == 1)
    {
      float32 zLength = (microTextureRegionZMaxs[i] - microTextureRegionZMins[i]) + spacing[2];
      float32 prismVolume = xLength * yLength * zLength;
      microTextureRegionFractionOccupied[i] = (static_cast<float32>(microTextureRegionNumCells[i]) * spacing[0] * spacing[1] * spacing[2]) / prismVolume;
    }
    else
    {
      float32 rectangleVolume = xLength * yLength;
      microTextureRegionFractionOccupied[i] = (static_cast<float32>(microTextureRegionNumCells[i]) * spacing[0] * spacing[1]) / rectangleVolume;
    }
    progressThrottle.updateCount(i);
  }

  return {};
}
