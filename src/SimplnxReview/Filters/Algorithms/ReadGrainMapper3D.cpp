#include "ReadGrainMapper3D.hpp"

#include "SimplnxReview/utils/GrainMapper3DUtilities.hpp"

#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/DataStructure/Geometry/ImageGeom.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"
#include "simplnx/DataStructure/StringArray.hpp"
#include "simplnx/Filter/IFilter.hpp"
#include "simplnx/Utilities/Parsing/HDF5/H5Support.hpp"
#include "simplnx/Utilities/Parsing/HDF5/Readers/DatasetReader.hpp"

#include "H5Support/H5Lite.h"
#include "H5Support/H5ScopedSentinel.h"
#include "H5Support/H5Utilities.h"

#include <fmt/format.h>

#include <algorithm>
#include <vector>

using namespace nx::core;
using namespace GrainMapper3DUtilities;
namespace GM3DConst = GrainMapper3DUtilities::Constants;

namespace
{

template <typename T>
Result<> ReadDataset(DataStructure& m_DataStructure, ReadGrainMapper3DInputValues* inputValues, DataPath arrayPath, hid_t parentId)
{
  using ArrayType = DataArray<T>;
  auto& dataRef = m_DataStructure.getDataRefAs<ArrayType>(arrayPath);
  auto* dataStorePtr = dataRef.getDataStore();
}
} // namespace

namespace EbsdLib::CrystalStructure
{
inline constexpr uint32_t UnknownCrystalStructure = 999; //!< UnknownCrystalStructure
}

// -----------------------------------------------------------------------------
ReadGrainMapper3D::ReadGrainMapper3D(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, ReadGrainMapper3DInputValues* inputValues)
: m_DataStructure(dataStructure)
, m_InputValues(inputValues)
, m_ShouldCancel(shouldCancel)
, m_MessageHandler(mesgHandler)
{
}

// -----------------------------------------------------------------------------
const std::atomic_bool& ReadGrainMapper3D::getCancel()
{
  return m_ShouldCancel;
}

Result<> ReadGrainMapper3D::copyPhaseData(GrainMapperReader& reader, hid_t fileId)
{

  herr_t error = reader.readPhases(fileId);
  if(error < 0)
  {
    return MakeErrorResult(-39801, fmt::format("Error reading phase info"));
  }

  auto phases = reader.getPhaseInformation();
  DataPath cellEnsembleAMPath = m_InputValues->ImageGeometryPath.createChildPath(m_InputValues->CellEnsembleAttributeMatrixName);

  // These arrays are purposely created using the AngFile constant names for BOTH the Oim and the Esprit readers!
  auto& crystalStructures = m_DataStructure.getDataRefAs<UInt32Array>(cellEnsembleAMPath.createChildPath(GM3DConstants::k_CrystalStructures));
  auto& materialNames = m_DataStructure.getDataRefAs<StringArray>(cellEnsembleAMPath.createChildPath(GM3DConstants::k_MaterialName));
  auto& latticeConstantsArray = m_DataStructure.getDataRefAs<Float32Array>(cellEnsembleAMPath.createChildPath(GM3DConstants::k_LatticeConstants));
  Float32Array::store_type* latticeConstants = latticeConstantsArray.getDataStore();

  crystalStructures[0] = EbsdLib::CrystalStructure::UnknownCrystalStructure;
  materialNames[0] = "Invalid Phase";
  latticeConstants->setComponent(0, 0, 0.0f);
  latticeConstants->setComponent(0, 1, 0.0f);
  latticeConstants->setComponent(0, 2, 0.0f);
  latticeConstants->setComponent(0, 3, 0.0f);
  latticeConstants->setComponent(0, 4, 0.0f);
  latticeConstants->setComponent(0, 5, 0.0f);
  int32 index = 1;
  for(const auto& phase : phases)
  {
    const int32 phaseId = index++;
    crystalStructures[phaseId] = GrainMapper3DUtilities::GetLaueIndexFromSpaceGroup(phase.SpaceGroup);
    materialNames[phaseId] = phase.Name;
    std::vector<double> lc = phase.UnitCell;

    latticeConstants->setComponent(phaseId, 0, static_cast<float32>(lc[0]));
    latticeConstants->setComponent(phaseId, 1, static_cast<float32>(lc[1]));
    latticeConstants->setComponent(phaseId, 2, static_cast<float32>(lc[2]));
    latticeConstants->setComponent(phaseId, 3, static_cast<float32>(lc[3]));
    latticeConstants->setComponent(phaseId, 4, static_cast<float32>(lc[4]));
    latticeConstants->setComponent(phaseId, 5, static_cast<float32>(lc[5]));
  }

  return {};
}

Result<> ReadGrainMapper3D::copyDctData(GrainMapperReader& reader, hid_t fileId)
{
  hid_t labDctGid = H5Gopen(fileId, GrainMapper3DUtilities::Constants::k_LabDCTGroupName.c_str(), H5P_DEFAULT);
  if(labDctGid < 0)
  {
  }
  auto groupSentinel = H5Support::H5ScopedGroupSentinel(labDctGid, true);

  // Now check that each of the known data sets exist
  // Get the Image Geometry Dimensions
  hid_t dataGid = H5Gopen(labDctGid, GrainMapper3DUtilities::Constants::k_DataGroupName.c_str(), H5P_DEFAULT);
  if(dataGid < 0)
  {
  }
  groupSentinel.addGroupId(dataGid);

  const auto& imageGeom = m_DataStructure.getDataRefAs<ImageGeom>(m_InputValues->ImageGeometryPath);
  const usize totalPoints = imageGeom.getNumberOfCells();

  reader.findAvailableDctDatasets(labDctGid);
  auto dctDataSets = reader.getDctDatasetNames();

  std::vector<std::string> floatDataSets = {GM3DConst::k_CompletenessName, GM3DConst::k_RodriguesName, GM3DConst::k_EulerZXZName, GM3DConst::k_EulerZYZName, GM3DConst::k_QuaternionName};
  std::vector<std::string> in32DataSets = {GM3DConst::k_GrainIdName};
  std::vector<std::string> uint8DataSets = {GM3DConst::k_MaskName, GM3DConst::k_PhaseIdName, GM3DConst::k_IPF001Name, GM3DConst::k_IPF010Name, GM3DConst::k_IPF100Name};
  Result<> result;
  for(const auto& dataSetName : dctDataSets)
  {
    DataPath dataArrayPath = m_InputValues->ImageGeometryPath.createChildPath(m_InputValues->CellAttributeMatrixName).createChildPath(dataSetName);

    nx::core::HDF5::DatasetReader datasetReader(dataGid, dataSetName);

    if(std::count(floatDataSets.begin(), floatDataSets.end(), dataSetName) > 0)
    {
      result = nx::core::HDF5::Support::FillDataArray<float32>(m_DataStructure, dataArrayPath, datasetReader);
    }
    else if(std::count(in32DataSets.begin(), in32DataSets.end(), dataSetName) > 0)
    {
      result = nx::core::HDF5::Support::FillDataArray<int32>(m_DataStructure, dataArrayPath, datasetReader);
    }
    else if(std::count(uint8DataSets.begin(), uint8DataSets.end(), dataSetName) > 0)
    {
      result = nx::core::HDF5::Support::FillDataArray<uint8>(m_DataStructure, dataArrayPath, datasetReader);
    }
    if(result.invalid())
    {
      return result;
    }
  }
  return {};
}

// -----------------------------------------------------------------------------
Result<> ReadGrainMapper3D::operator()()
{
  GrainMapperReader reader(m_InputValues->InputFile.string());

  hid_t fileId = H5Support::H5Utilities::openFile(m_InputValues->InputFile, true);
  if(fileId < 0)
  {
    return MakeErrorResult(-39800, fmt::format("Grain Mapper 3D File '{}' could not be opened.", m_InputValues->InputFile.string()));
  }
  auto sentinel = H5Support::H5ScopedFileSentinel(fileId, false);

  // ***********************************************************************
  // Read the Phase Information
  Result<> result = copyPhaseData(reader, fileId);
  if(result.invalid())
  {
    return result;
  }

  result = copyDctData(reader, fileId);

  return {};
}
