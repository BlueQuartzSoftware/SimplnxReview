#include "ComputeDirectionVectors.hpp"

#include "EbsdLib/Core/Orientation.hpp"
#include "EbsdLib/Orientation/AxisAngle.hpp"
#include "EbsdLib/Orientation/Euler.hpp"
#include "EbsdLib/Orientation/OrientationMatrix.hpp"
#include "EbsdLib/Orientation/Quaternion.hpp"
#include "EbsdLib/Orientation/Rodrigues.hpp"

#include <Eigen/Core>

#include "simplnx/Common/Constants.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/Utilities/DataArrayUtilities.hpp"
#include "simplnx/Utilities/ParallelTaskAlgorithm.hpp"

using namespace nx::core;

namespace
{
/**
 *
 * @param latticeParameters The lattice Parameters in the order, a, b, c, alpha, beta, gamma. Note that alpha, beta, gamma are all stored as degrees.
 * @return
 */
template <typename T>
ebsdlib::Matrix3X3<T> DirectStructureMatrix(const Vec3<float32>& latticeParametersLengths, const Vec3<float32>& latticeParametersAngles)
{
  /* This code is taken from EMsoftOO/mod_crystallography.f90 - computeMatrices() function */

  T a = latticeParametersLengths[0];
  T b = latticeParametersLengths[1];
  T c = latticeParametersLengths[2];
  T alpha = latticeParametersAngles[0];
  T beta = latticeParametersAngles[1];
  T gamma = latticeParametersAngles[2];

  // auxiliary variables for the various tensors
  T pirad = Constants::k_PiOver180F;
  T ca = std::cos(pirad * alpha);
  T cb = std::cos(pirad * beta);
  T cg = std::cos(pirad * gamma);
  T sg = std::sin(pirad * gamma);

  // cell volume via the determinant of dmt
  T det = (a * b * c) * (a * b * c) * (1.0f - ca * ca - cb * cb - cg * cg + 1.0f * ca * cb * cg);
  T vol = std::sqrt(det);

  ebsdlib::Matrix3X3<T> dsm;
  dsm[0] = a;
  dsm[1] = b * cg;
  dsm[2] = c * cb;
  dsm[3] = 0.0;
  dsm[4] = b * sg;
  dsm[5] = -c * (cb * cg - ca) / sg;
  dsm[6] = 0.0;
  dsm[7] = 0.0;
  dsm[8] = vol / (a * b * sg);
  return dsm;
}

template <typename T>
class ComputeDirectionVectorsImpl
{
public:
  ComputeDirectionVectorsImpl(const DataArray<T>& inputOrientationsArray, ebsdlib::orientations::Type inputRepType, const ebsdlib::Matrix3X1<T>& cartesian, Float32Array& outputArray,
                              const std::atomic_bool& shouldCancel)
  : m_InputOrientationsArray(inputOrientationsArray)
  , m_InputRepType(inputRepType)
  , m_Cartesian(cartesian)
  , m_OutputArray(outputArray)
  , m_ShouldCancel(shouldCancel)
  {
  }
  ~ComputeDirectionVectorsImpl() = default;

  ComputeDirectionVectorsImpl(const ComputeDirectionVectorsImpl&) = default;
  ComputeDirectionVectorsImpl(ComputeDirectionVectorsImpl&&) noexcept = default;
  ComputeDirectionVectorsImpl& operator=(const ComputeDirectionVectorsImpl&) = delete;
  ComputeDirectionVectorsImpl& operator=(ComputeDirectionVectorsImpl&&) noexcept = delete;

  void convert(usize start, usize end) const
  {
    // For each orientation matrix, convert to gmatrix, transpose, and multiply by the cartesian point.
    // The result is the direction vector that will be stored in the output array.

    const auto& inputOrientationsDataStore = m_InputOrientationsArray.template getIDataStoreRefAs<AbstractDataStore<T>>();
    auto& outputDataStore = m_OutputArray.template getIDataStoreRefAs<AbstractDataStore<float32>>();
    ebsdlib::OrientationMatrix<T> om;
    for(usize i = start; i < end; i++)
    {
      if(m_ShouldCancel)
      {
        return;
      }

      switch(m_InputRepType)
      {
      case ebsdlib::orientations::Type::Euler: {
        // Euler has 3 components
        om = ebsdlib::Euler<T>(inputOrientationsDataStore.getValue(i * 3), inputOrientationsDataStore.getValue(i * 3 + 1), inputOrientationsDataStore.getValue(i * 3 + 2)).toOrientationMatrix();
        break;
      }
      case ebsdlib::orientations::Type::OrientationMatrix: {
        // OrientationMatrix has 9 components
        om = ebsdlib::OrientationMatrix<T>(inputOrientationsDataStore.getValue(i * 3), inputOrientationsDataStore.getValue(i * 3 + 1), inputOrientationsDataStore.getValue(i * 3 + 2),
                                           inputOrientationsDataStore.getValue(i * 3 + 3), inputOrientationsDataStore.getValue(i * 3 + 4), inputOrientationsDataStore.getValue(i * 3 + 5),
                                           inputOrientationsDataStore.getValue(i * 3 + 6), inputOrientationsDataStore.getValue(i * 3 + 7), inputOrientationsDataStore.getValue(i * 3 + 8));
        break;
      }
      case ebsdlib::orientations::Type::Rodrigues: {
        // Rodrigues has 4 components
        om = ebsdlib::Rodrigues<T>(inputOrientationsDataStore.getValue(i * 3), inputOrientationsDataStore.getValue(i * 3 + 1), inputOrientationsDataStore.getValue(i * 3 + 2),
                                   inputOrientationsDataStore.getValue(i * 3 + 3))
                 .toOrientationMatrix();
        break;
      }
      case ebsdlib::orientations::Type::Quaternion: {
        // Quaternion has 4 components
        om = ebsdlib::Quaternion<T>(inputOrientationsDataStore.getValue(i * 3), inputOrientationsDataStore.getValue(i * 3 + 1), inputOrientationsDataStore.getValue(i * 3 + 2),
                                    inputOrientationsDataStore.getValue(i * 3 + 3))
                 .toOrientationMatrix();
        break;
      }
      case ebsdlib::orientations::Type::AxisAngle: {
        // AxisAngle has 4 components
        om = ebsdlib::AxisAngle<T>(inputOrientationsDataStore.getValue(i * 3), inputOrientationsDataStore.getValue(i * 3 + 1), inputOrientationsDataStore.getValue(i * 3 + 2),
                                   inputOrientationsDataStore.getValue(i * 3 + 3))
                 .toOrientationMatrix();
        break;
      }
      case ebsdlib::orientations::Type::Homochoric: {
        // Homochoric has 3 components
        om = ebsdlib::Homochoric<T>(inputOrientationsDataStore.getValue(i * 3), inputOrientationsDataStore.getValue(i * 3 + 1), inputOrientationsDataStore.getValue(i * 3 + 2)).toOrientationMatrix();
        break;
      }
      case ebsdlib::orientations::Type::Cubochoric: {
        // Cubochoric has 3 components
        om = ebsdlib::Cubochoric<T>(inputOrientationsDataStore.getValue(i * 3), inputOrientationsDataStore.getValue(i * 3 + 1), inputOrientationsDataStore.getValue(i * 3 + 2)).toOrientationMatrix();
        break;
      }
      case ebsdlib::orientations::Type::Stereographic: {
        // Stereographic has 3 components
        om =
            ebsdlib::Stereographic<T>(inputOrientationsDataStore.getValue(i * 3), inputOrientationsDataStore.getValue(i * 3 + 1), inputOrientationsDataStore.getValue(i * 3 + 2)).toOrientationMatrix();
        break;
      }
      case ebsdlib::orientations::Type::Unknown: {
        throw std::runtime_error("Unknown Orientation Representation Type.  This should not happen, contact the developers.");
      }
      }

      ebsdlib::Matrix3X1<T> point = om.toGMatrix().transpose() * m_Cartesian;
      std::copy(point.data(), point.data() + 3, outputDataStore.begin() + i * outputDataStore.getNumberOfComponents());

      // std::cout << fmt::format("Cartesian: ({}, {}, {})", m_Cartesian[0], m_Cartesian[1], m_Cartesian[2]) << std::endl;
      // std::cout << fmt::format("Point: ({}, {}, {})", point[0], point[1], point[2]) << std::endl;
    }
  }

  void operator()() const
  {
    convert(0, m_InputOrientationsArray.getNumberOfTuples());
  }

private:
  const DataArray<T>& m_InputOrientationsArray;
  Float32Array& m_OutputArray;
  const ebsdlib::Matrix3X1<T>& m_Cartesian;
  ebsdlib::orientations::Type m_InputRepType;
  const std::atomic_bool& m_ShouldCancel;
};

template <typename T, typename = std::enable_if_t<std::is_same_v<T, float32> || std::is_same_v<T, float64>>>
Result<> ExecuteComputeDirectionVectors(DataStructure& dataStructure, const DataPath& inputOrientationsArrayPath, const DataPath& outputArrayPath, ebsdlib::orientations::Type inputRepType,
                                        const Vec3<float32>& latticeParametersLengths, const Vec3<float32>& latticeParametersAngles, const IFilter::MessageHandler& msgHandler,
                                        const std::atomic_bool& shouldCancel)
{
  msgHandler("Computing Direction Vectors...");

  // Convert the lattice parameters to a direct structure matrix, and calculate the cartesian point
  ebsdlib::Matrix3X3<T> dsm = DirectStructureMatrix<T>(latticeParametersLengths, latticeParametersAngles);
  ebsdlib::Matrix3X1<T> latticePoint(0.0f, 0.0f, 1.0f);
  auto cartesian = dsm * latticePoint;

  // Parallelize the implementation method
  ParallelTaskAlgorithm taskRunner;
  taskRunner.setParallelizationEnabled(false);
  auto& inputOrientationsArray = dataStructure.getDataRefAs<DataArray<T>>(inputOrientationsArrayPath);
  auto& outputArray = dataStructure.getDataRefAs<Float32Array>(outputArrayPath);
  taskRunner.template execute<>(ComputeDirectionVectorsImpl<T>(inputOrientationsArray, inputRepType, cartesian, outputArray, shouldCancel));

  return {};
}
} // namespace

// -----------------------------------------------------------------------------
ComputeDirectionVectors::ComputeDirectionVectors(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                                 ComputeDirectionVectorsInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
ComputeDirectionVectors::~ComputeDirectionVectors() noexcept = default;

// -----------------------------------------------------------------------------
Result<> ComputeDirectionVectors::operator()()
{
  Vec3<float32> latticeParametersLengths;
  Vec3<float32> latticeParametersAngles;
  switch(m_InputValues->LatticeConstantsInputType)
  {
  case LatticeConstantsInputType::DataArrayPath: {
    size_t phaseId = 1;
    auto& latticeConstantsArray = m_DataStructure.getDataRefAs<Float32Array>(m_InputValues->LatticeConstantsArrayPath);
    latticeParametersLengths[0] = latticeConstantsArray[phaseId * 6 + 0];
    latticeParametersLengths[1] = latticeConstantsArray[phaseId * 6 + 1];
    latticeParametersLengths[2] = latticeConstantsArray[phaseId * 6 + 2];
    latticeParametersAngles[0] = latticeConstantsArray[phaseId * 6 + 3];
    latticeParametersAngles[1] = latticeConstantsArray[phaseId * 6 + 4];
    latticeParametersAngles[2] = latticeConstantsArray[phaseId * 6 + 5];
    break;
  }
  case LatticeConstantsInputType::Manual: {
    latticeParametersLengths = m_InputValues->ManualLatticeConstantsLengths;
    latticeParametersAngles = m_InputValues->ManualLatticeConstantsAngles;
    break;
  }
  }

  DataPath orientationsParentPath = m_InputValues->InputOrientationsArrayPath.getParent();
  DataPath outputDirectionVectorsPath = orientationsParentPath.createChildPath(m_InputValues->OutputDirectionVectorsArrayName);

  auto& inputOrientationsArray = m_DataStructure.getDataRefAs<IDataArray>(m_InputValues->InputOrientationsArrayPath);
  DataType inputDataType = inputOrientationsArray.getDataType();
  switch(inputDataType)
  {
  case DataType::float32: {
    return ExecuteComputeDirectionVectors<float32>(m_DataStructure, m_InputValues->InputOrientationsArrayPath, outputDirectionVectorsPath, m_InputValues->InputRepType, latticeParametersLengths,
                                                   latticeParametersAngles, m_MessageHandler, m_ShouldCancel);
  }
  case DataType::float64: {
    return ExecuteComputeDirectionVectors<float64>(m_DataStructure, m_InputValues->InputOrientationsArrayPath, outputDirectionVectorsPath, m_InputValues->InputRepType, latticeParametersLengths,
                                                   latticeParametersAngles, m_MessageHandler, m_ShouldCancel);
  }
  default: {
    return MakeErrorResult(
        -2300, fmt::format("Input Orientations array has incompatible data type: the data type is {} but this filter only supports float32 and float64 data types.", DataTypeToString(inputDataType)));
  }
  }
}
