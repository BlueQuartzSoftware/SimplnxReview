#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"
#include "simplnx/Parameters/StringParameter.hpp"

#include <random>

namespace nx::core {

struct SIMPLNXREVIEW_EXPORT GroupMicroTextureRegionsInputValues {
  bool UseNonContiguousNeighbors;
  DataPath NonContiguousNeighborListArrayPath;
  DataPath ContiguousNeighborListArrayPath;
  bool m_PatchGrouping = false;
  bool UseRunningAverage;
  float32 CAxisTolerance;
  DataPath FeatureIdsArrayPath;
  DataPath FeaturePhasesArrayPath;
  DataPath VolumesArrayPath;
  DataPath AvgQuatsArrayPath;
  DataPath CrystalStructuresArrayPath;
  DataPath NewCellFeatureAttributeMatrixName;
  DataPath CellParentIdsArrayName;
  DataPath FeatureParentIdsArrayName;
  uint64 SeedValue;
};

/**
 * @class GroupMicroTextureRegions
 * @brief This filter ...
 */
class SIMPLNXREVIEW_EXPORT GroupMicroTextureRegions {
public:
  GroupMicroTextureRegions(DataStructure &dataStructure,
                           const IFilter::MessageHandler &mesgHandler,
                           const std::atomic_bool &shouldCancel,
                           GroupMicroTextureRegionsInputValues *inputValues);
  ~GroupMicroTextureRegions() noexcept;

  GroupMicroTextureRegions(const GroupMicroTextureRegions &) = delete;
  GroupMicroTextureRegions(GroupMicroTextureRegions &&) noexcept = delete;
  GroupMicroTextureRegions &
  operator=(const GroupMicroTextureRegions &) = delete;
  GroupMicroTextureRegions &
  operator=(GroupMicroTextureRegions &&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool &getCancel();

protected:
  int getSeed(int32 newFid);
  bool determineGrouping(int32 referenceFeature, int32 neighborFeature,
                         int32 newFid);
  void execute();
  bool growPatch(int32 currentPatch);
  bool growGrouping(int32 referenceFeature, int32 neighborFeature,
                    int32 newFid);

private:
  DataStructure &m_DataStructure;
  const GroupMicroTextureRegionsInputValues *m_InputValues = nullptr;
  const std::atomic_bool &m_ShouldCancel;
  const IFilter::MessageHandler &m_MessageHandler;

  usize m_NumTuples = 0;
  std::array<float32, 3> m_AvgCAxes = {0.0f, 0.0f, 0.0f};
  std::mt19937_64 m_Generator = {};
  std::uniform_real_distribution<float32> m_Distribution = {};
};
} // namespace nx::core
