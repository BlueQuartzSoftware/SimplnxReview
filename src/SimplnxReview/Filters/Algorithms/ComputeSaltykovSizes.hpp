#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{

struct SIMPLNXREVIEW_EXPORT ComputeSaltykovSizesInputValues
{
  DataPath equivalent_diameters_path;
  DataPath saltykov_equivalent_diameters_path;
  uint64 seed{};
};

/**
 * @class ComputeSaltykovSizes
 * @brief This filter...
 */

class SIMPLNXREVIEW_EXPORT ComputeSaltykovSizes
{
public:
  ComputeSaltykovSizes(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ComputeSaltykovSizesInputValues* inputValues);
  ~ComputeSaltykovSizes() noexcept = default;

  ComputeSaltykovSizes(const ComputeSaltykovSizes&) = delete;
  ComputeSaltykovSizes(ComputeSaltykovSizes&&) noexcept = delete;
  ComputeSaltykovSizes& operator=(const ComputeSaltykovSizes&) = delete;
  ComputeSaltykovSizes& operator=(ComputeSaltykovSizes&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const ComputeSaltykovSizesInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};

} // namespace nx::core
