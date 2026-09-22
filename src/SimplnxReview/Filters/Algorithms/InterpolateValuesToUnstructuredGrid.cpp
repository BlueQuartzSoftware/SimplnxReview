#include "InterpolateValuesToUnstructuredGrid.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/Geometry/INodeGeometry0D.hpp"
#include "simplnx/Utilities/FilterUtilities.hpp"
#include "simplnx/Utilities/ParallelAlgorithmUtilities.hpp"
#include "simplnx/Utilities/ParallelDataAlgorithm.hpp"
#include "simplnx/Utilities/ParallelTaskAlgorithm.hpp"

using namespace nx::core;

namespace
{
float32 distBetweenPoints(const Vec3<float32>& srcPoint, const Vec3<float32>& destPoint)
{
  return powf(destPoint[0] - srcPoint[0], 2.0f) + pow(destPoint[1] - srcPoint[1], 2.0f) + pow(destPoint[2] - srcPoint[2], 2.0f);
}

/**
 * @brief The CalculateClosestVerticesImpl class implements a threaded algorithm that computes the normal of each
 * triangle for a set of triangles
 */
class CalculateClosestVerticesImpl
{
public:
  CalculateClosestVerticesImpl(InterpolateValuesToUnstructuredGrid* filter, const INodeGeometry0D& srcGeom, const INodeGeometry0D& destGeom, std::vector<usize>& closestSrcIds,
                               const IFilter::MessageHandler& messageHandler, const std::atomic_bool& shouldCancel)
  : m_Filter(filter)
  , m_SrcGeometry(srcGeom)
  , m_DestGeometry(destGeom)
  , m_ClosestSrcIds(closestSrcIds)
  , m_MessageHandler(messageHandler)
  , m_ShouldCancel(shouldCancel)
  {
  }
  virtual ~CalculateClosestVerticesImpl() = default;

  void generate(size_t start, size_t end) const
  {
    usize counter = 0;
    // A count stride keeps the seam out of the per-vertex hot path. The throttle behind the seam
    // decides when a message is actually due, so this loop never reads the clock.
    const usize increment = std::max<usize>(1, (end - start) / 100);
    for(usize destVertexId = start; destVertexId < end; destVertexId++)
    {
      if(m_ShouldCancel)
      {
        m_Filter->sendThreadSafeProgressMessage(counter);
        return;
      }

      if(counter > increment)
      {
        m_Filter->sendThreadSafeProgressMessage(counter);
        counter = 0;
      }

      Vec3<float32> destVertexCoord = m_DestGeometry.getVertexCoordinate(destVertexId);
      usize closestVertexId;
      float32 closestDist = std::numeric_limits<float32>::max();
      for(usize srcVertexId = 0; srcVertexId < m_SrcGeometry.getNumberOfVertices(); srcVertexId++)
      {
        Vec3<float32> srcVertexCoord = m_SrcGeometry.getVertexCoordinate(srcVertexId);
        float32 dist = distBetweenPoints(srcVertexCoord, destVertexCoord);
        closestVertexId = (dist < closestDist) ? srcVertexId : closestVertexId;
        closestDist = (dist < closestDist) ? dist : closestDist;
      }
      m_ClosestSrcIds[destVertexId] = closestVertexId;
      counter++;
    }

    m_Filter->sendThreadSafeProgressMessage(counter);
  }

  void operator()(const Range& range) const
  {
    generate(range.min(), range.max());
  }

private:
  InterpolateValuesToUnstructuredGrid* m_Filter = nullptr;
  const INodeGeometry0D& m_SrcGeometry;
  const INodeGeometry0D& m_DestGeometry;
  std::vector<usize>& m_ClosestSrcIds;
  const IFilter::MessageHandler& m_MessageHandler;
  const std::atomic_bool& m_ShouldCancel;
};

struct ExecuteInterpolation
{
  template <typename Type>
  void operator()(const IDataArray& iSrcDataArray, IDataArray& iDestDataArray, const std::vector<usize>& closestSrcIds)
  {
    const auto& srcDataArray = dynamic_cast<const DataArray<Type>&>(iSrcDataArray);
    auto& destDataArray = dynamic_cast<DataArray<Type>&>(iDestDataArray);

    for(usize i = 0; i < closestSrcIds.size(); i++)
    {
      destDataArray[i] = srcDataArray[closestSrcIds[i]];
    }
  }
};
} // namespace

// -----------------------------------------------------------------------------
InterpolateValuesToUnstructuredGrid::InterpolateValuesToUnstructuredGrid(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                                                         InterpolateValuesToUnstructuredGridInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
, m_Throttle(mesgHandler)
{
}

// -----------------------------------------------------------------------------
InterpolateValuesToUnstructuredGrid::~InterpolateValuesToUnstructuredGrid() noexcept = default;

// -----------------------------------------------------------------------------
const std::atomic_bool& InterpolateValuesToUnstructuredGrid::getCancel()
{
  return m_ShouldCancel;
}

// -----------------------------------------------------------------------------
Result<> InterpolateValuesToUnstructuredGrid::operator()()
{
  auto& srcGeometry = m_DataStructure.getDataRefAs<INodeGeometry0D>(m_InputValues->SourceGeomPath);
  auto& destGeometry = m_DataStructure.getDataRefAs<INodeGeometry0D>(m_InputValues->DestinationGeomPath);

  std::vector<usize> closestSrcIds(destGeometry.getNumberOfVertices());

  // set up thread-safe messenger
  m_TotalElements = destGeometry.getNumberOfVertices();
  m_Throttle.reset(m_TotalElements, "Calculating Closest Vertices");

  // Parallel algorithm to calculate closest vertices
  ParallelDataAlgorithm dataAlg;
  dataAlg.setRange(0ULL, static_cast<usize>(destGeometry.getNumberOfVertices()));
  dataAlg.execute(CalculateClosestVerticesImpl(this, srcGeometry, destGeometry, closestSrcIds, m_MessageHandler, m_ShouldCancel));
  if(m_ShouldCancel)
  {
    return {};
  }
  m_MessageHandler.sendProgressCount("Calculating Closest Vertices", m_TotalElements, m_TotalElements);

  DataPath interpolatedAttrMatrixPath;
  if(m_InputValues->UseExistingAttrMatrix)
  {
    interpolatedAttrMatrixPath = m_InputValues->ExistingAttrMatrixPath;
  }
  else
  {
    interpolatedAttrMatrixPath = m_InputValues->DestinationGeomPath.createChildPath(m_InputValues->CreatedAttrMatrixName);
  }

  ProgressEstimator arrayEstimator;
  for(usize i = 0; i < m_InputValues->InputDataPaths.size(); i++)
  {
    const auto& dataPath = m_InputValues->InputDataPaths[i];

    if(m_ShouldCancel)
    {
      return {};
    }

    m_MessageHandler.sendInfoMessage(fmt::format("Interpolating \"{}\" Array Values", dataPath.getTargetName()));

    const auto& srcDataArray = m_DataStructure.getDataRefAs<IDataArray>(dataPath);
    auto& destDataArray = m_DataStructure.getDataRefAs<IDataArray>(interpolatedAttrMatrixPath.createChildPath(dataPath.getTargetName()));

    ExecuteDataFunction(ExecuteInterpolation{}, srcDataArray.getDataType(), srcDataArray, destDataArray, closestSrcIds);

    m_MessageHandler.sendProgressCount("Interpolating Array Values", i + 1, m_InputValues->InputDataPaths.size(), arrayEstimator.estimate(i + 1, m_InputValues->InputDataPaths.size()));
  }

  return {};
}

// -----------------------------------------------------------------------------
void InterpolateValuesToUnstructuredGrid::sendThreadSafeProgressMessage(usize counter)
{
  std::lock_guard<std::mutex> guard(m_ProgressMessage_Mutex);
  m_Throttle.incrementPercent(counter);
}
