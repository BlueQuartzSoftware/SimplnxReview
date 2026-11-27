#include "ComputeEquivalentDiameterCircles.hpp"

#include "simplnx/Common/Constants.hpp"
#include "simplnx/Common/Result.hpp"
#include "simplnx/Common/Types.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/DataStructure/Geometry/EdgeGeom.hpp"
#include "simplnx/Filter/IFilter.hpp"

#include <atomic>
#include <cmath>

using namespace nx::core;

// -----------------------------------------------------------------------------
ComputeEquivalentDiameterCircles::ComputeEquivalentDiameterCircles(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                                                   ComputeEquivalentDiameterCirclesInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
const std::atomic_bool& ComputeEquivalentDiameterCircles::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> ComputeEquivalentDiameterCircles::operator()()
{
  auto& centroidsArray = m_DataStructure.getDataRefAs<Float32ArrayType>(m_InputValues->centroids_array_path);
  auto& equivalentDiametersArray = m_DataStructure.getDataRefAs<Float32ArrayType>(m_InputValues->equivalent_diameters_array_path);
  const DataPath featureIdsPath = m_InputValues->output_edge_geometry_path.createChildPath(m_InputValues->edge_attribute_matrix_name).createChildPath(m_InputValues->feature_ids_array_name);
  auto& featureIdsArray = m_DataStructure.getDataRefAs<Int32ArrayType>(featureIdsPath);
  auto& outputEdgeGeom = m_DataStructure.getDataRefAs<EdgeGeom>(m_InputValues->output_edge_geometry_path);
  Float32ArrayType& verticesArray = outputEdgeGeom.getVerticesRef();
  UInt64ArrayType& edgesArray = outputEdgeGeom.getEdgesRef();

  const usize circleResolution = m_InputValues->circle_resolution;
  const usize verticesPerCircle = circleResolution + 1;
  usize edgeIdx = 0;

  for(usize i = 1; i < centroidsArray.getNumberOfTuples(); ++i)
  {
    const float32 r = equivalentDiametersArray[i] / 2.0f;
    const float32 cx = centroidsArray.getComponent(i, 0);
    const float32 cy = centroidsArray.getComponent(i, 1);

    const usize circleVertexStart = (i - 1) * verticesPerCircle;

    for(usize v = 0; v < verticesPerCircle; ++v)
    {
      const usize vertexIdx = circleVertexStart + v;

      const float32 theta = 2.0f * nx::core::constants::k_PiF * static_cast<float32>(v) / static_cast<float32>(circleResolution);

      const float32 x = cx + r * std::cos(theta);
      const float32 y = cy + r * std::sin(theta);

      verticesArray.setComponent(vertexIdx, 0, x);
      verticesArray.setComponent(vertexIdx, 1, y);
      verticesArray.setComponent(vertexIdx, 2, static_cast<float32>(m_InputValues->z_plane));
    }

    for(usize e = 0; e < circleResolution; ++e)
    {
      const usize v0 = circleVertexStart + e;
      const usize v1 = circleVertexStart + e + 1;

      featureIdsArray.setValue(edgeIdx, static_cast<int32>(i));

      edgesArray.setComponent(edgeIdx, 0, v0);
      edgesArray.setComponent(edgeIdx, 1, v1);
      edgeIdx++;
    }
  }

  return {};
}
