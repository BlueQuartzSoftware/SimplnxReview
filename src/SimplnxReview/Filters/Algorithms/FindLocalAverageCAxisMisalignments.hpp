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
struct SIMPLNXREVIEW_EXPORT FindLocalAverageCAxisMisalignmentsInputValues
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
 * @class FindLocalAverageCAxisMisalignments
 * @brief This filter will...
 */
class SIMPLNXREVIEW_EXPORT FindLocalAverageCAxisMisalignments
{
public:
  FindLocalAverageCAxisMisalignments(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel,
                                     FindLocalAverageCAxisMisalignmentsInputValues* inputValues);
  ~FindLocalAverageCAxisMisalignments() noexcept;

  FindLocalAverageCAxisMisalignments(const FindLocalAverageCAxisMisalignments&) = delete;
  FindLocalAverageCAxisMisalignments(FindLocalAverageCAxisMisalignments&&) noexcept = delete;
  FindLocalAverageCAxisMisalignments& operator=(const FindLocalAverageCAxisMisalignments&) = delete;
  FindLocalAverageCAxisMisalignments& operator=(FindLocalAverageCAxisMisalignments&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const FindLocalAverageCAxisMisalignmentsInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};
} // namespace nx::core
