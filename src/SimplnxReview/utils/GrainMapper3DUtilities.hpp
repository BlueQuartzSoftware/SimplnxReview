#pragma once

#include "simplnx/Common/Array.hpp"
#include "simplnx/Common/Result.hpp"
#include "simplnx/DataStructure/Geometry/ImageGeom.hpp"
#include "simplnx/Common/Types.hpp"

#include <string>
#include <vector>
#include <cstdint>

namespace GrainMapper3DUtilities
{

class GrainMapperReader
{
public:
  GrainMapperReader(const std::string& filePath);
  ~GrainMapperReader();


  typedef struct
  {
    std::string Name;
    int32_t SpaceGroup;
    std::vector<double> UnitCell; //  ABC, Alpha, Beta, Gamma
    std::string UniversalHermannMauguin;
  } GrainMapperPhase;


  nx::core::Result<> readHeaderOnly();

  std::vector<size_t> getDimensions() const;
  std::vector<float> getSpacing() const;
  std::vector<float> getOrigin() const;

  std::vector<std::string> getDCTDatasetNames() const;
  std::map<std::string, nx::core::DataType> getNameToDataTypeMap() const;
  const std::map<std::string, size_t> getNameToCompDimMap() const;
  std::vector<GrainMapperPhase> getPhaseInformation() const;

private:
  std::string m_ErrorMessage = {};
  std::string m_FileName = {};
  std::string m_HDF5Path = {};
  std::string m_OINAVersion = {};

  std::vector<size_t> m_Dimensions;
  std::vector<double>  m_Spacing = {1.0, 1.0, 1.0};
  std::vector<double>  m_Origin = {0.0, 0.0, 0.0};

  std::vector<std::string> m_AvailableDCTDatasets;
  std::vector<GrainMapperPhase> m_PhaseInfos;
};

}; // namespace GrainMapper3DUtilities
