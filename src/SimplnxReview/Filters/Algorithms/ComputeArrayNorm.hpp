#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{

struct SIMPLNXREVIEW_EXPORT ComputeArrayNormInputValues
{
  float32 PSpace;
  DataPath SelectedArrayPath;
  DataPath NormArrayPath;
};

/**
 * @class ComputeArrayNorm
 * @brief This algorithm computes the p-th norm of an Attribute Array.
 */
class SIMPLNXREVIEW_EXPORT ComputeArrayNorm
{
public:
  ComputeArrayNorm(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ComputeArrayNormInputValues* inputValues);
  ~ComputeArrayNorm() noexcept;

  ComputeArrayNorm(const ComputeArrayNorm&) = delete;
  ComputeArrayNorm(ComputeArrayNorm&&) noexcept = delete;
  ComputeArrayNorm& operator=(const ComputeArrayNorm&) = delete;
  ComputeArrayNorm& operator=(ComputeArrayNorm&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const ComputeArrayNormInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};

} // namespace nx::core
