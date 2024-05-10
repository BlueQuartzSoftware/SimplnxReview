#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/DataGroupSelectionParameter.hpp"
#include "simplnx/Parameters/StringParameter.hpp"

namespace nx::core
{
struct SIMPLNXREVIEW_EXPORT ComputeLocalAverageCAxisMisalignmentsInputValues
{
  bool CalcBiasedAvg;
  bool CalcUnbiasedAvg;
  DataPath NeighborListPath;
  DataPath CAxisMisalignmentListPath;
  DataPath AvgCAxisMisalignmentsPath;
  DataPath FeatureParentIdsPath;
  DataPath NumFeaturesPerParentPath;
  DataPath LocalCAxisMisalignmentsPath;
  DataPath UnbiasedLocalCAxisMisalignmentsPath;
};

/**
 * @class ComputeLocalAverageCAxisMisalignments
 * @brief This filter will...
 */
class SIMPLNXREVIEW_EXPORT ComputeLocalAverageCAxisMisalignments
{
public:
  ComputeLocalAverageCAxisMisalignments(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                        ComputeLocalAverageCAxisMisalignmentsInputValues* inputValues);
  ~ComputeLocalAverageCAxisMisalignments() noexcept;

  ComputeLocalAverageCAxisMisalignments(const ComputeLocalAverageCAxisMisalignments&) = delete;
  ComputeLocalAverageCAxisMisalignments(ComputeLocalAverageCAxisMisalignments&&) noexcept = delete;
  ComputeLocalAverageCAxisMisalignments& operator=(const ComputeLocalAverageCAxisMisalignments&) = delete;
  ComputeLocalAverageCAxisMisalignments& operator=(ComputeLocalAverageCAxisMisalignments&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const ComputeLocalAverageCAxisMisalignmentsInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};
} // namespace nx::core
