#include "FindMicroTextureRegions.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/Geometry/ImageGeom.hpp"

#include <algorithm>

using namespace nx::core;

// -----------------------------------------------------------------------------
FindMicroTextureRegions::FindMicroTextureRegions(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                                 FindMicroTextureRegionsInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
FindMicroTextureRegions::~FindMicroTextureRegions() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& FindMicroTextureRegions::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> FindMicroTextureRegions::operator()()
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

  std::for_each(featureIds.begin(), featureIds.end(), [&microTextureRegionNumCells](const int32 id) mutable { microTextureRegionNumCells[id]++; });

  float32 x, y, z;
  usize zStride, yStride;
  for(usize i = 0; i < zPoints; i++)
  {
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
  }

  auto& microTextureRegionFractionOccupied = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->MicroTextureRegionFractionOccupiedArrayPath);
  for(usize i = 1; i < numMicroTextureRegions; i++)
  {
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
  }

  return {};
}
