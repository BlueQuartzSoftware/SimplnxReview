#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{

struct SIMPLNXREVIEW_EXPORT GlavicicTortureInputValues
{
  DataPath ExpectedMTRIdsPath;
  std::vector<DataPath> GroupEulerAnglesPaths;
  DataPath EulerAnglesPath;
  uint64 Seed;
};

/**
 * @class GlavicicTorture
 * @brief This algorithm...
 */

class SIMPLNXREVIEW_EXPORT GlavicicTorture
{
public:
  GlavicicTorture(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, GlavicicTortureInputValues* inputValues);
  ~GlavicicTorture() noexcept;

  GlavicicTorture(const GlavicicTorture&) = delete;
  GlavicicTorture(GlavicicTorture&&) noexcept = delete;
  GlavicicTorture& operator=(const GlavicicTorture&) = delete;
  GlavicicTorture& operator=(GlavicicTorture&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const GlavicicTortureInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};

} // namespace nx::core
