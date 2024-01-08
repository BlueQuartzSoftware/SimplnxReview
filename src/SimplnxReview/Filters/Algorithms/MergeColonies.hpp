#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/DataStructure/AttributeMatrix.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/DataPath.hpp"
#include "simplnx/DataStructure/DataStructure.hpp"
#include "simplnx/Filter/IFilter.hpp"
#include "simplnx/Parameters/ArrayCreationParameter.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"

#include "EbsdLib/LaueOps/LaueOps.h"

namespace nx::core
{
struct SIMPLNXREVIEW_EXPORT MergeColoniesInputValues
{
  bool UseNonContiguousNeighbors;
  DataPath NonContiguousNeighborListArrayPath;
  DataPath ContiguousNeighborListArrayPath;
  bool m_PatchGrouping = false;
  float32 AxisTolerance;
  float32 AngleTolerance;
  DataPath FeaturePhasesPath;
  DataPath AvgQuatsPath;
  DataPath FeatureIdsPath;
  DataPath CellPhasesPath;
  DataPath CrystalStructuresPath;
  DataPath CellParentIdsPath;
  DataPath CellFeatureAMPath;
  DataPath FeatureParentIdsPath;
  DataPath ActivePath;
  uint64 SeedValue;
  bool RandomizeParentIds = true;
};

/**
 * @class ConditionalSetValue
 * @brief This filter replaces values in the target array with a user specified value
 * where a bool mask array specifies.
 */

class SIMPLNXREVIEW_EXPORT MergeColonies
{
  using LaueOpsShPtrType = std::shared_ptr<LaueOps>;
  using LaueOpsContainer = std::vector<LaueOpsShPtrType>;

public:
  MergeColonies(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, MergeColoniesInputValues* inputValues);
  ~MergeColonies() noexcept;

  MergeColonies(const MergeColonies&) = delete;
  MergeColonies(MergeColonies&&) noexcept = delete;
  MergeColonies& operator=(const MergeColonies&) = delete;
  MergeColonies& operator=(MergeColonies&&) noexcept = delete;

  Result<> operator()();

  const std::atomic_bool& getCancel();

protected:
  int getSeed(int32 newFid);
  bool determineGrouping(int32 referenceFeature, int32 neighborFeature, int32 newFid);
  void execute();
  bool growPatch(int32 currentPatch);
  bool growGrouping(int32 referenceFeature, int32 neighborFeature, int32 newFid);
  void characterize_colonies();

private:
  DataStructure& m_DataStructure;
  const MergeColoniesInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;

  LaueOpsContainer m_OrientationOps;
  Int32Array& m_FeatureParentIds;
  Int32Array& m_FeaturePhases;
  Float32Array& m_AvgQuats;
  UInt32Array& m_CrystalStructures;
  float32 m_AxisToleranceRad = 0.0;
  float32 m_AngleTolerance = 1.0;
};
} // namespace nx::core
