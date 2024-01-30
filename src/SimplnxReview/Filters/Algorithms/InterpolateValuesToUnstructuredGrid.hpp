#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/ChoicesParameter.hpp"
#include "simplnx/Parameters/DataGroupSelectionParameter.hpp"
#include "simplnx/Parameters/DynamicTableParameter.hpp"
#include "simplnx/Parameters/MultiArraySelectionParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"
#include "simplnx/Parameters/VectorParameter.hpp"
#include "simplnx/Utilities/ImageRotationUtilities.hpp"

#include <Eigen/Dense>

namespace nx::core
{

struct SIMPLNXREVIEW_EXPORT InterpolateValuesToUnstructuredGridInputValues
{
  DataPath SourceGeomPath;
  DataPath DestinationGeomPath;
  std::vector<DataPath> InputDataPaths;
  bool UseExistingAttrMatrix;
  DataPath ExistingAttrMatrixPath;
  std::string CreatedAttrMatrixName;
};

/**
 * @class ConditionalSetValue
 * @brief This filter replaces values in the target array with a user specified value
 * where a bool mask array specifies.
 */

class SIMPLNXREVIEW_EXPORT InterpolateValuesToUnstructuredGrid
{
public:
  InterpolateValuesToUnstructuredGrid(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                      InterpolateValuesToUnstructuredGridInputValues* inputValues);
  ~InterpolateValuesToUnstructuredGrid() noexcept;

  InterpolateValuesToUnstructuredGrid(const InterpolateValuesToUnstructuredGrid&) = delete;
  InterpolateValuesToUnstructuredGrid(InterpolateValuesToUnstructuredGrid&&) noexcept = delete;
  InterpolateValuesToUnstructuredGrid& operator=(const InterpolateValuesToUnstructuredGrid&) = delete;
  InterpolateValuesToUnstructuredGrid& operator=(InterpolateValuesToUnstructuredGrid&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

  void sendThreadSafeProgressMessage(usize counter);

private:
  DataStructure& m_DataStructure;
  const InterpolateValuesToUnstructuredGridInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;

  // Thread safe Progress Message
  std::chrono::steady_clock::time_point m_InitialPoint = std::chrono::steady_clock::now();
  mutable std::mutex m_ProgressMessage_Mutex;
  size_t m_TotalElements = 0;
  size_t m_ProgressCounter = 0;
  size_t m_LastProgressInt = 0;
};

} // namespace nx::core
