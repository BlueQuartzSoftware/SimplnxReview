#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{

struct SIMPLNXREVIEW_EXPORT FindSaltykovSizesInputValues
{
  DataPath EquivalentDiametersPath;
  DataPath SaltykovEquivalentDiametersPath;
  uint64 Seed;
};

/**
 * @class ConditionalSetValue
 * @brief This filter...
 */

class SIMPLNXREVIEW_EXPORT FindSaltykovSizes
{
public:
  FindSaltykovSizes(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, FindSaltykovSizesInputValues* inputValues);
  ~FindSaltykovSizes() noexcept;

  FindSaltykovSizes(const FindSaltykovSizes&) = delete;
  FindSaltykovSizes(FindSaltykovSizes&&) noexcept = delete;
  FindSaltykovSizes& operator=(const FindSaltykovSizes&) = delete;
  FindSaltykovSizes& operator=(FindSaltykovSizes&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const FindSaltykovSizesInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};

} // namespace nx::core
