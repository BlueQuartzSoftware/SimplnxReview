//
// Created by Michael Jackson on 8/19/24.
//

#include "GrainMapper3DUtilities.hpp"

#include "H5Support/H5Lite.h"
#include "H5Support/H5ScopedSentinel.h"
#include "H5Support/H5Utilities.h"

#include <fmt/format.h>

using namespace nx::core;
using namespace H5Support;
namespace GrainMapper3DUtilities
{

namespace Constants
{
const std::string k_LabDCTGroupName("LabDCT");
const std::string k_AbsorptionCTName("k_AbsorptionCT");
const std::string k_ProjectInfoName("ProjectInfo");
const std::string k_VersionName("Version");

const std::string k_ExtentName("Extent");
const std::string k_SpacingName("Spacing");
const std::string k_CenterName("Center");

const std::string k_DataGroupName("Data");
const std::string k_CompletenessName("Completeness");
const std::string k_GrainIdName("GrainId");
const std::string k_MaskName("Mask");
const std::string k_PhaseIdName("PhaseId");
const std::string k_RodriguesName("Rodrigues");

const std::string k_EulerZXZName("EulerZXZ");
const std::string k_EulerZYZName("EulerZYZ");
const std::string k_QuaternionName("Quaternion");
const std::string k_IPF001Name("IPF001");
const std::string k_IPF010Name("IPF010");
const std::string k_IPF100Name("IPF100");

const std::map<std::string, DataType> k_NameToDataTypeMap = {{k_CompletenessName, DataType::float32}, {k_GrainIdName, DataType::int32},      {k_MaskName, DataType::uint8},
                                                             {k_PhaseIdName, DataType::uint8},        {k_RodriguesName, DataType::float32},  {k_EulerZXZName, DataType::float32},
                                                             {k_EulerZYZName, DataType::float32},     {k_QuaternionName, DataType::float32}, {k_IPF001Name, DataType::uint8},
                                                             {k_IPF010Name, DataType::uint8},         {k_IPF100Name, DataType::uint8}};
const std::map<std::string, size_t> k_NameToCompDimMap = {{k_CompletenessName, 1}, {k_GrainIdName, 1},    {k_MaskName, 1},   {k_PhaseIdName, 1}, {k_RodriguesName, 3}, {k_EulerZXZName, 3},
                                                          {k_EulerZYZName, 3},     {k_QuaternionName, 4}, {k_IPF001Name, 3}, {k_IPF010Name, 3},  {k_IPF100Name, 3}};

// ****************************************************************************
// Phase Constants
const std::string k_PhaseInfoName("PhaseInfo");
const std::string k_Name("Name");
const std::string k_SpaceGroupName("SpaceGroup");
const std::string k_UnitCellName("UnitCell");
const std::string k_UniversalHermannMauguinName("UniversalHermannMauguin");

} // namespace Constants

GrainMapperReader::GrainMapperReader(const std::string& filePath)
: m_FileName(filePath)
{
}

GrainMapperReader::~GrainMapperReader()
{
}

std::vector<size_t> GrainMapperReader::getDimensions() const
{
  return m_Dimensions;
}

std::vector<float> GrainMapperReader::getSpacing() const
{
  return {static_cast<float>(m_Spacing[0]), static_cast<float>(m_Spacing[1]), static_cast<float>(m_Spacing[2])};
}

std::vector<float> GrainMapperReader::getOrigin() const
{
  return {static_cast<float>(m_Origin[0]), static_cast<float>(m_Origin[1]), static_cast<float>(m_Origin[2])};
}

std::map<std::string, DataType> GrainMapperReader::getNameToDataTypeMap() const
{
  return Constants::k_NameToDataTypeMap;
}

const std::map<std::string, size_t> GrainMapperReader::getNameToCompDimMap() const
{
  return Constants::k_NameToCompDimMap;
}

std::vector<std::string> GrainMapperReader::getDCTDatasetNames() const
{
  return m_AvailableDCTDatasets;
}

std::vector<GrainMapperReader::GrainMapperPhase> GrainMapperReader::getPhaseInformation() const
{
  return m_PhaseInfos;
}

Result<> GrainMapperReader::readHeaderOnly()
{

  Result<> result;

  hid_t fileId = H5Support::H5Utilities::openFile(m_FileName, true);
  if(fileId < 0)
  {
    MakeErrorResult(-39800, fmt::format("Grain Mapper 3D File '{}' could not be opened.", m_FileName));
  }
  auto sentinel = H5Support::H5ScopedFileSentinel(fileId, false);

  // Get the Image Geometry Dimensions
  hid_t labDctGid = H5Gopen(fileId, Constants::k_LabDCTGroupName.c_str(), H5P_DEFAULT);
  if(labDctGid < 0)
  {
  }
  sentinel.addGroupId(labDctGid);
  std::vector<double> extents;
  herr_t error = H5Lite::readVectorDataset(labDctGid, Constants::k_ExtentName.c_str(), extents);
  if(error < 0)
  {
  }

  error = H5Lite::readVectorDataset(labDctGid, Constants::k_SpacingName.c_str(), m_Spacing);
  if(error < 0)
  {
  }

  m_Dimensions = std::vector<size_t>{static_cast<size_t>(extents[0] / m_Spacing[0]), static_cast<size_t>(extents[1] / m_Spacing[1]), static_cast<size_t>(extents[2] / m_Spacing[2])};

  error = H5Lite::readVectorDataset(labDctGid, Constants::k_CenterName.c_str(), m_Origin);
  if(error < 0)
  {
  }

  fmt::print("Dims: {}\n", fmt::join(m_Dimensions, ","));
  fmt::print("Origin: {}\n", fmt::join(m_Origin, ","));
  fmt::print("Spacing: {}\n", fmt::join(m_Spacing, ","));

  // Now check that each of the known data sets exist
  // Get the Image Geometry Dimensions
  hid_t dataGid = H5Gopen(labDctGid, Constants::k_DataGroupName.c_str(), H5P_DEFAULT);
  if(dataGid < 0)
  {
  }
  sentinel.addGroupId(dataGid);

  for(const auto& entry : Constants::k_NameToDataTypeMap)
  {
    if(H5Lite::datasetExists(dataGid, entry.first))
    {
      fmt::print("{}\n", entry.first);
      m_AvailableDCTDatasets.push_back(entry.first);
    }
  }

  // Get the Phase Information
  hid_t phaseGid = H5Gopen(fileId, Constants::k_PhaseInfoName.c_str(), H5P_DEFAULT);
  if(phaseGid < 0)
  {
  }
  sentinel.addGroupId(phaseGid);
  std::list<std::string> phaseNames;
  error = H5Utilities::getGroupObjects(phaseGid, H5Utilities::CustomHDFDataTypes::Group, phaseNames);
  if(error < 0)
  {
  }
  // Now we know how many phases we have, we need to programmatically generate those phase names
  // in order to keep them consistent. Yep, someone didn't really think through the parsing of this
  // or assumptions are being made about the order that HDF5 is going to give them back to you. Either
  // is bad.
  for(int i = 0; i < phaseNames.size(); i++)
  {
    std::string name = fmt::format("Phase{:02}", i + 1);
    std::cout << name << std::endl;
  }

  return result;
}

} // namespace GrainMapper3DUtilities
