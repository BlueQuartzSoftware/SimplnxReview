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

namespace GM3DConst = GrainMapper3DUtilities::Constants;

namespace GrainMapper3DUtilities
{

const std::map<std::string, DataType> k_NameToDataTypeMap = {
    {GM3DConst::k_CompletenessName, DataType::float32}, {GM3DConst::k_GrainIdName, DataType::int32},      {GM3DConst::k_MaskName, DataType::uint8},
    {GM3DConst::k_PhaseIdName, DataType::uint8},        {GM3DConst::k_RodriguesName, DataType::float32},  {GM3DConst::k_EulerZXZName, DataType::float32},
    {GM3DConst::k_EulerZYZName, DataType::float32},     {GM3DConst::k_QuaternionName, DataType::float32}, {GM3DConst::k_IPF001Name, DataType::uint8},
    {GM3DConst::k_IPF010Name, DataType::uint8},         {GM3DConst::k_IPF100Name, DataType::uint8}};

const std::map<std::string, size_t> k_NameToCompDimMap = {{GM3DConst::k_CompletenessName, 1}, {GM3DConst::k_GrainIdName, 1},  {GM3DConst::k_MaskName, 1},     {GM3DConst::k_PhaseIdName, 1},
                                                          {GM3DConst::k_RodriguesName, 3},    {GM3DConst::k_EulerZXZName, 3}, {GM3DConst::k_EulerZYZName, 3}, {GM3DConst::k_QuaternionName, 4},
                                                          {GM3DConst::k_IPF001Name, 3},       {GM3DConst::k_IPF010Name, 3},   {GM3DConst::k_IPF100Name, 3}};

int32_t GetLaueIndexFromSpaceGroup(int32_t spaceGroupId)
{
  // clang-format off
  std::array<size_t, 32> sgpg =   {1, 2, 3, 6, 10, 16, 25, 47, 75, 81, 83, 89, 99, 111, 123, 143, 147, 149, 156, 162, 168, 174, 175, 177, 183, 187, 191, 195, 200, 207, 215, 221};
  std::array<size_t, 32> pgLaue = {1, 1, 2, 2, 2,  22, 22, 22, 4,  4,  4,  42, 42, 42,  42,  3,   3,   32,  32,  32,  6,   6,   6,   62,  62,  62,  62,  23,  23,  43,  43,  43};
  // clang-format on
  size_t pgIndex = sgpg.size() - 1;
  for(size_t i = 0; i < sgpg.size(); i++)
  {
    if(sgpg[i] > spaceGroupId)
    {
      pgIndex = i - 1;
      break;
    }
  }

  size_t value = pgLaue.at(pgIndex);
  switch(value)
  {
  case 1: // TriclinicOps
    return 4;
  case 2: // MonoclinicOps
    return 5;
  case 22: // OrthoRhombicOps
    return 6;
  case 4: // TetragonalLowOps
    return 7;
  case 42: // TetragonalOps
    return 8;
  case 3: // TrigonalLowOps
    return 9;
  case 32: // TrigonalOps
    return 10;
  case 6: // HexagonalLowOps
    return 2;
  case 62: // HexagonalOps
    return 0;
  case 23: // CubicLowOps
    return 3;
  case 43: // CubicOps
    return 1;
  default:
    return 999;
  }
}

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
  return GrainMapper3DUtilities::k_NameToDataTypeMap;
}

const std::map<std::string, size_t> GrainMapperReader::getNameToCompDimMap() const
{
  return GrainMapper3DUtilities::k_NameToCompDimMap;
}

std::vector<std::string> GrainMapperReader::getDctDatasetNames() const
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
    return MakeErrorResult(-39800, fmt::format("Grain Mapper 3D File '{}' could not be opened.", m_FileName));
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

  error = findAvailableDctDatasets(labDctGid);
  if(error < 0)
  {
  }
  error = readPhases(fileId);
  if(error < 0)
  {
  }

  return result;
}

herr_t GrainMapperReader::findAvailableDctDatasets(hid_t labDctGid)
{
  // Now check that each of the known data sets exist
  // Get the Image Geometry Dimensions
  hid_t dataGid = H5Gopen(labDctGid, Constants::k_DataGroupName.c_str(), H5P_DEFAULT);
  if(dataGid < 0)
  {
    return -1;
  }
  auto groupSentinel = H5Support::H5ScopedGroupSentinel(dataGid, true);

  for(const auto& entry : GrainMapper3DUtilities::k_NameToDataTypeMap)
  {
    if(H5Lite::datasetExists(dataGid, entry.first))
    {
      fmt::print("{}\n", entry.first);
      m_AvailableDCTDatasets.push_back(entry.first);
    }
  }
  return 0;
}

herr_t GrainMapperReader::readPhases(hid_t parentId)
{
  // Get the Phase Information
  hid_t phaseInfoGid = H5Gopen(parentId, Constants::k_PhaseInfoName.c_str(), H5P_DEFAULT);
  if(phaseInfoGid < 0)
  {
    return phaseInfoGid;
  }
  auto groupSentinel = H5Support::H5ScopedGroupSentinel(phaseInfoGid, true);
  std::list<std::string> phaseNames;
  herr_t error = H5Utilities::getGroupObjects(phaseInfoGid, H5Utilities::CustomHDFDataTypes::Group, phaseNames);
  if(error < 0)
  {
    return error;
  }
  m_PhaseInfos.clear();

  // Now we know how many phases we have, we need to programmatically generate those phase names
  // in order to keep them consistent. Yep, someone didn't really think through the parsing of this
  // or assumptions are being made about the order that HDF5 is going to give them back to you. Either
  // is bad.
  for(int i = 0; i < phaseNames.size(); i++)
  {
    std::string phaseName = fmt::format("Phase{:02}", i + 1);
    std::cout << phaseName << std::endl;

    hid_t phaseGid = H5Gopen(phaseInfoGid, phaseName.c_str(), H5P_DEFAULT);
    auto phaseDGidSentinel = H5Support::H5ScopedGroupSentinel(phaseGid, true);

    GrainMapperPhase phase;
    error = H5Lite::readStringDataset(phaseGid, Constants::k_Name, phase.Name);
    if(error < 0)
    {
      return error;
    }

    error = H5Lite::readStringDataset(phaseGid, Constants::k_Name, phase.UniversalHermannMauguin);
    if(error < 0)
    {
      return error;
    }

    error = H5Lite::readScalarDataset(phaseGid, Constants::k_SpaceGroupName, phase.SpaceGroup);
    if(error < 0)
    {
      return error;
    }

    error = H5Lite::readVectorDataset(phaseGid, Constants::k_UnitCellName, phase.UnitCell);
    if(error < 0)
    {
      return error;
    }

    m_PhaseInfos.push_back(phase);
  }
  return 0;
}

} // namespace GrainMapper3DUtilities
