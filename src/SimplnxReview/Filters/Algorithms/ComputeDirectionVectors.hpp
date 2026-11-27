#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "EbsdLib/Orientation/OrientationFwd.hpp"

#include "simplnx/Common/Array.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/Filter/IFilter.hpp"
#include "simplnx/Parameters/ChoicesParameter.hpp"

namespace nx::core
{
enum LatticeConstantsInputType : uint8
{
  DataArrayPath = 0,
  Manual = 1
};

struct SIMPLNXREVIEW_EXPORT ComputeDirectionVectorsInputValues
{
  ebsdlib::orientations::Type input_rep_type = {};
  DataPath input_orientations_array_path;
  LatticeConstantsInputType lattice_constants_input_type = {};
  DataPath lattice_constants_array_path;
  FloatVec3Type manual_lattice_constants_lengths;
  FloatVec3Type manual_lattice_constants_angles;
  std::string output_direction_vectors_array_name;
};

/**
 * @class
 */
class SIMPLNXREVIEW_EXPORT ComputeDirectionVectors
{
public:
  ComputeDirectionVectors(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ComputeDirectionVectorsInputValues* inputValues);
  ~ComputeDirectionVectors() noexcept = default;

  ComputeDirectionVectors(const ComputeDirectionVectors&) = delete;
  ComputeDirectionVectors(ComputeDirectionVectors&&) noexcept = delete;
  ComputeDirectionVectors& operator=(const ComputeDirectionVectors&) = delete;
  ComputeDirectionVectors& operator=(ComputeDirectionVectors&&) noexcept = delete;

  Result<> operator()();

private:
  DataStructure& m_DataStructure;
  const ComputeDirectionVectorsInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};
} // namespace nx::core
