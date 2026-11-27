#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/DataGroupSelectionParameter.hpp"
#include "simplnx/Parameters/StringParameter.hpp"
#include "simplnx/Utilities/ThrottledMessageHandler.hpp"

namespace nx::core
{
struct SIMPLNXREVIEW_EXPORT ComputeLocalAverageCAxisMisalignmentsInputValues
{
  bool calc_biased_avg{};
  bool calc_unbiased_avg{};
  DataPath neighbor_list_path;
  DataPath c_axis_misalignment_list_path;
  DataPath avg_c_axis_misalignments_path;
  DataPath feature_parent_ids_path;
  DataPath num_features_per_parent_path;
  DataPath local_c_axis_misalignments_path;
  DataPath unbiased_local_c_axis_misalignments_path;
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
  ThrottledMessageHandler m_Throttle;
};
} // namespace nx::core
