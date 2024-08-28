#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"

namespace nx::core
{

struct SIMPLNXREVIEW_EXPORT ReadGrainMapper3DInputValues
{
  std::filesystem::path InputFile;
  DataPath ImageGeometryPath;
  std::string CellAttributeMatrixName;
  std::string CellEnsembleAttributeMatrixName;
};

/**
 * @class ReadGrainMapper3D
 * @brief This filter determines the average C-axis location of each Feature.
 */

class SIMPLNXREVIEW_EXPORT ReadGrainMapper3D
{
public:
  ReadGrainMapper3D(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ReadGrainMapper3DInputValues* inputValues);
  ~ReadGrainMapper3D() noexcept = default;

  ReadGrainMapper3D(const ReadGrainMapper3D&) = delete;
  ReadGrainMapper3D(ReadGrainMapper3D&&) noexcept = delete;
  ReadGrainMapper3D& operator=(const ReadGrainMapper3D&) = delete;
  ReadGrainMapper3D& operator=(ReadGrainMapper3D&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

private:
  DataStructure& m_DataStructure;
  const ReadGrainMapper3DInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;
};

} // namespace nx::core
