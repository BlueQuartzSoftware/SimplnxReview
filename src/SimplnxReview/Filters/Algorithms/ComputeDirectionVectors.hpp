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
  ebsdlib::orientations::Type InputRepType;
  DataPath InputOrientationsArrayPath;
  LatticeConstantsInputType LatticeConstantsInputType;
  DataPath LatticeConstantsArrayPath;
  FloatVec3 ManualLatticeConstantsLengths;
  FloatVec3 ManualLatticeConstantsAngles;
  std::string OutputDirectionVectorsArrayName;
};

/**
 * @class
 */
class SIMPLNXREVIEW_EXPORT ComputeDirectionVectors
{
public:
  ComputeDirectionVectors(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ComputeDirectionVectorsInputValues* inputValues);
  ~ComputeDirectionVectors() noexcept;

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
