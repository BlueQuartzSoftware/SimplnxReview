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

#include <random>

namespace nx::core
{
struct SIMPLNXREVIEW_EXPORT MergeColoniesInputValues
{
  bool use_non_contiguous_neighbors{};
  DataPath non_contiguous_neighbor_list_array_path;
  DataPath contiguous_neighbor_list_array_path;
  bool m_patch_grouping = false;
  float32 axis_tolerance{};
  float32 angle_tolerance{};
  DataPath feature_phases_path;
  DataPath avg_quats_path;
  DataPath feature_ids_path;
  DataPath cell_phases_path;
  DataPath crystal_structures_path;
  DataPath cell_parent_ids_path;
  DataPath cell_feature_am_path;
  DataPath feature_parent_ids_path;
  DataPath active_path;
  uint64 seed_value{};
  bool randomize_parent_ids = true;
};

/**
 * @class MergeColonies

 */

class SIMPLNXREVIEW_EXPORT MergeColonies
{
  using LaueOpsShPtrType = std::shared_ptr<ebsdlib::LaueOps>;
  using LaueOpsContainerType = std::vector<LaueOpsShPtrType>;

public:
  MergeColonies(DataStructure& dataStructure, const IFilter::MessageHandler& mesgHandler, const std::atomic_bool& shouldCancel, MergeColoniesInputValues* inputValues);
  ~MergeColonies() noexcept;

  MergeColonies(const MergeColonies&) = delete;
  MergeColonies(MergeColonies&&) noexcept = delete;
  MergeColonies& operator=(const MergeColonies&) = delete;
  MergeColonies& operator=(MergeColonies&&) noexcept = delete;

  Result<> operator()();

protected:
  int getSeed(int32 newFid);
  bool determineGrouping(int32 referenceFeature, int32 neighborFeature, int32 newFid) const;
  Result<> execute();
  static bool GrowPatch(int32 currentPatch);
  static bool GrowGrouping(int32 referenceFeature, int32 neighborFeature, int32 newFid);
  void characterizeColonies();

private:
  DataStructure& m_DataStructure;
  const MergeColoniesInputValues* m_InputValues = nullptr;
  const std::atomic_bool& m_ShouldCancel;
  const IFilter::MessageHandler& m_MessageHandler;

  LaueOpsContainerType m_OrientationOps;
  Int32ArrayType& m_FeatureParentIds;
  Int32ArrayType& m_FeaturePhases;
  Float32ArrayType& m_AvgQuats;
  UInt32ArrayType& m_CrystalStructures;
  float32 m_AxisToleranceRad = 0.0;
  float32 m_AngleTolerance = 1.0;

  std::mt19937_64 m_Generator;
  std::uniform_real_distribution<float32> m_Distribution;
};
} // namespace nx::core
