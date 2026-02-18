#include <catch2/catch.hpp>

#include "simplnx/DataStructure/AttributeMatrix.hpp"
#include "simplnx/DataStructure/DataArray.hpp"
#include "simplnx/DataStructure/Geometry/ImageGeom.hpp"
#include "simplnx/DataStructure/NeighborList.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Parameters/NeighborListSelectionParameter.hpp"
#include "simplnx/UnitTest/UnitTestCommon.hpp"

#include "SimplnxReview/Filters/ComputeGroupingDensityFilter.hpp"
#include "SimplnxReview/SimplnxReview_test_dirs.hpp"

using namespace nx::core;

namespace
{
// DataStructure path constants
const std::string k_ImageGeomName = "ImageGeom";
const std::string k_FeatureAMName = "CellFeatureData";
const std::string k_ParentAMName = "ParentFeatureData";
const std::string k_VolumesName = "Volumes";
const std::string k_ParentIdsName = "ParentIds";
const std::string k_ContiguousNLName = "ContiguousNeighborList";
const std::string k_NonContiguousNLName = "NonContiguousNeighborList";
const std::string k_ParentVolumesName = "ParentVolumes";
const std::string k_GroupingDensitiesName = "GroupingDensities";
const std::string k_CheckedFeaturesName = "CheckedFeatures";

const DataPath k_VolumesPath = DataPath({k_ImageGeomName, k_FeatureAMName, k_VolumesName});
const DataPath k_ParentIdsPath = DataPath({k_ImageGeomName, k_FeatureAMName, k_ParentIdsName});
const DataPath k_ContiguousNLPath = DataPath({k_ImageGeomName, k_FeatureAMName, k_ContiguousNLName});
const DataPath k_NonContiguousNLPath = DataPath({k_ImageGeomName, k_FeatureAMName, k_NonContiguousNLName});
const DataPath k_ParentVolumesPath = DataPath({k_ImageGeomName, k_ParentAMName, k_ParentVolumesName});
const DataPath k_GroupingDensitiesPath = DataPath({k_ImageGeomName, k_ParentAMName, k_GroupingDensitiesName});
const DataPath k_CheckedFeaturesPath = DataPath({k_ImageGeomName, k_FeatureAMName, k_CheckedFeaturesName});

// Test data dimensions
// 6 features (index 0 = placeholder, features 1-5)
// 4 parents  (index 0 = placeholder, parents 1-3)
// Features 1,2,3 → Parent 1
// Features 4,5   → Parent 2
// Parent 3 has NO features (tests density == -1.0 path)
constexpr usize k_NumFeatures = 6;
constexpr usize k_NumParents = 4;

/**
 * @brief Builds a DataStructure with all input data needed for the ComputeGroupingDensity filter.
 * Optionally includes a non-contiguous neighbor list.
 */
DataStructure createTestDataStructure(bool includeNonContiguousNL)
{
  DataStructure dataStructure;

  // Create ImageGeom (just a container for the AMs)
  auto* imageGeom = ImageGeom::Create(dataStructure, k_ImageGeomName);
  imageGeom->setDimensions({1, 1, 1});

  // Feature-level AttributeMatrix (6 tuples: indices 0-5)
  auto* featureAM = AttributeMatrix::Create(dataStructure, k_FeatureAMName, {k_NumFeatures}, imageGeom->getId());

  // Parent-level AttributeMatrix (4 tuples: indices 0-3)
  auto* parentAM = AttributeMatrix::Create(dataStructure, k_ParentAMName, {k_NumParents}, imageGeom->getId());

  // --- Feature-level arrays ---

  // Feature Volumes: [0, 10, 20, 15, 25, 30]
  auto* featureVolumes = UnitTest::CreateTestDataArray<float32>(dataStructure, k_VolumesName, {k_NumFeatures}, {1}, featureAM->getId());
  auto& featureVolumesRef = featureVolumes->getDataStoreRef();
  featureVolumesRef[0] = 0.0f;
  featureVolumesRef[1] = 10.0f;
  featureVolumesRef[2] = 20.0f;
  featureVolumesRef[3] = 15.0f;
  featureVolumesRef[4] = 25.0f;
  featureVolumesRef[5] = 30.0f;

  // Parent IDs: [0, 1, 1, 1, 2, 2]
  auto* parentIds = UnitTest::CreateTestDataArray<int32>(dataStructure, k_ParentIdsName, {k_NumFeatures}, {1}, featureAM->getId());
  auto& parentIdsRef = parentIds->getDataStoreRef();
  parentIdsRef[0] = 0;
  parentIdsRef[1] = 1;
  parentIdsRef[2] = 1;
  parentIdsRef[3] = 1;
  parentIdsRef[4] = 2;
  parentIdsRef[5] = 2;

  // Contiguous Neighbor List
  // Feature 0: {}
  // Feature 1: {2}
  // Feature 2: {1, 3}
  // Feature 3: {2, 4}
  // Feature 4: {3, 5}
  // Feature 5: {4}
  auto* contiguousNL = NeighborList<int32>::Create(dataStructure, k_ContiguousNLName, ShapeType{k_NumFeatures}, featureAM->getId());
  contiguousNL->setList(0, std::make_shared<std::vector<int32>>(std::vector<int32>{}));
  contiguousNL->setList(1, std::make_shared<std::vector<int32>>(std::vector<int32>{2}));
  contiguousNL->setList(2, std::make_shared<std::vector<int32>>(std::vector<int32>{1, 3}));
  contiguousNL->setList(3, std::make_shared<std::vector<int32>>(std::vector<int32>{2, 4}));
  contiguousNL->setList(4, std::make_shared<std::vector<int32>>(std::vector<int32>{3, 5}));
  contiguousNL->setList(5, std::make_shared<std::vector<int32>>(std::vector<int32>{4}));

  // Non-Contiguous Neighbor List (optional)
  // Feature 0: {}
  // Feature 1: {4}
  // Feature 2: {5}
  // Feature 3: {}
  // Feature 4: {1}
  // Feature 5: {2}
  if(includeNonContiguousNL)
  {
    auto* nonContiguousNL = NeighborList<int32>::Create(dataStructure, k_NonContiguousNLName, ShapeType{k_NumFeatures}, featureAM->getId());
    nonContiguousNL->setList(0, std::make_shared<std::vector<int32>>(std::vector<int32>{}));
    nonContiguousNL->setList(1, std::make_shared<std::vector<int32>>(std::vector<int32>{4}));
    nonContiguousNL->setList(2, std::make_shared<std::vector<int32>>(std::vector<int32>{5}));
    nonContiguousNL->setList(3, std::make_shared<std::vector<int32>>(std::vector<int32>{}));
    nonContiguousNL->setList(4, std::make_shared<std::vector<int32>>(std::vector<int32>{1}));
    nonContiguousNL->setList(5, std::make_shared<std::vector<int32>>(std::vector<int32>{2}));
  }

  // --- Parent-level arrays ---

  // Parent Volumes: [0, 200, 100, 50]
  auto* parentVolumes = UnitTest::CreateTestDataArray<float32>(dataStructure, k_ParentVolumesName, {k_NumParents}, {1}, parentAM->getId());
  auto& parentVolumesRef = parentVolumes->getDataStoreRef();
  parentVolumesRef[0] = 0.0f;
  parentVolumesRef[1] = 200.0f;
  parentVolumesRef[2] = 100.0f;
  parentVolumesRef[3] = 50.0f;

  return dataStructure;
}

/**
 * @brief Creates the filter Arguments for the given boolean option combination.
 */
Arguments createFilterArgs(bool useNonContiguous, bool findCheckedFeatures)
{
  Arguments args;
  args.insertOrAssign(ComputeGroupingDensityFilter::k_FeatureVolumesArrayPath_Key, std::make_any<DataPath>(k_VolumesPath));
  args.insertOrAssign(ComputeGroupingDensityFilter::k_ContiguousNeighborListArrayPath_Key, std::make_any<DataPath>(k_ContiguousNLPath));
  args.insertOrAssign(ComputeGroupingDensityFilter::k_UseNonContiguousNeighbors_Key, std::make_any<bool>(useNonContiguous));
  args.insertOrAssign(ComputeGroupingDensityFilter::k_NonContiguousNeighborListArrayPath_Key, std::make_any<DataPath>(useNonContiguous ? k_NonContiguousNLPath : DataPath{}));
  args.insertOrAssign(ComputeGroupingDensityFilter::k_ParentIdsPath_Key, std::make_any<DataPath>(k_ParentIdsPath));
  args.insertOrAssign(ComputeGroupingDensityFilter::k_ParentVolumesPath_Key, std::make_any<DataPath>(k_ParentVolumesPath));
  args.insertOrAssign(ComputeGroupingDensityFilter::k_FindCheckedFeatures_Key, std::make_any<bool>(findCheckedFeatures));
  args.insertOrAssign(ComputeGroupingDensityFilter::k_CheckedFeaturesName_Key, std::make_any<std::string>(k_CheckedFeaturesName));
  args.insertOrAssign(ComputeGroupingDensityFilter::k_GroupingDensitiesName_Key, std::make_any<std::string>(k_GroupingDensitiesName));
  return args;
}
} // namespace

// =============================================================================
// Execution Tests - Exercise all 4 template specializations
// =============================================================================

TEST_CASE("SimplnxReview::ComputeGroupingDensityFilter: Basic Density (no non-contiguous, no checked features)", "[SimplnxReview][ComputeGroupingDensityFilter]")
{
  UnitTest::LoadPlugins();

  DataStructure dataStructure = createTestDataStructure(false);
  ComputeGroupingDensityFilter filter;
  Arguments args = createFilterArgs(false, false);

  auto executeResult = filter.execute(dataStructure, args, nullptr, IFilter::MessageHandler{[](const IFilter::Message& message) { fmt::print("{}\n", message.message); }});
  SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);

  // Verify grouping densities
  // Parent 1: features {1,2,3}, contiguous neighbors add feature 4
  //   totalCheckVolume = 10 + 20 + 15 + 25 = 70
  //   density = 200 / 70 ≈ 2.857143
  // Parent 2: features {4,5}, contiguous neighbors add feature 3
  //   totalCheckVolume = 25 + 15 + 30 = 70
  //   density = 100 / 70 ≈ 1.428571
  // Parent 3: no features → density = -1.0
  REQUIRE_NOTHROW(dataStructure.getDataRefAs<Float32Array>(k_GroupingDensitiesPath));
  const auto& groupingDensities = dataStructure.getDataRefAs<Float32Array>(k_GroupingDensitiesPath);

  REQUIRE(groupingDensities[1] == Approx(200.0f / 70.0f).epsilon(0.0001f));
  REQUIRE(groupingDensities[2] == Approx(100.0f / 70.0f).epsilon(0.0001f));
  REQUIRE(groupingDensities[3] == Approx(-1.0f));
}

TEST_CASE("SimplnxReview::ComputeGroupingDensityFilter: With Non-Contiguous Neighbors", "[SimplnxReview][ComputeGroupingDensityFilter]")
{
  UnitTest::LoadPlugins();

  DataStructure dataStructure = createTestDataStructure(true);
  ComputeGroupingDensityFilter filter;
  Arguments args = createFilterArgs(true, false);

  auto executeResult = filter.execute(dataStructure, args, nullptr, IFilter::MessageHandler{[](const IFilter::Message& message) { fmt::print("{}\n", message.message); }});
  SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);

  // With non-contiguous neighbors, all 5 features get checked for each parent
  // Parent 1: totalCheckVolume = 10+20+25+15+30 = 100, density = 200/100 = 2.0
  // Parent 2: totalCheckVolume = 25+15+30+10+20 = 100, density = 100/100 = 1.0
  // Parent 3: no features → density = -1.0
  REQUIRE_NOTHROW(dataStructure.getDataRefAs<Float32Array>(k_GroupingDensitiesPath));
  const auto& groupingDensities = dataStructure.getDataRefAs<Float32Array>(k_GroupingDensitiesPath);

  REQUIRE(groupingDensities[1] == Approx(2.0f).epsilon(0.0001f));
  REQUIRE(groupingDensities[2] == Approx(1.0f).epsilon(0.0001f));
  REQUIRE(groupingDensities[3] == Approx(-1.0f));
}

TEST_CASE("SimplnxReview::ComputeGroupingDensityFilter: With Checked Features", "[SimplnxReview][ComputeGroupingDensityFilter]")
{
  UnitTest::LoadPlugins();

  DataStructure dataStructure = createTestDataStructure(false);
  ComputeGroupingDensityFilter filter;
  Arguments args = createFilterArgs(false, true);

  auto executeResult = filter.execute(dataStructure, args, nullptr, IFilter::MessageHandler{[](const IFilter::Message& message) { fmt::print("{}\n", message.message); }});
  SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);

  // Densities same as basic case
  REQUIRE_NOTHROW(dataStructure.getDataRefAs<Float32Array>(k_GroupingDensitiesPath));
  const auto& groupingDensities = dataStructure.getDataRefAs<Float32Array>(k_GroupingDensitiesPath);

  REQUIRE(groupingDensities[1] == Approx(200.0f / 70.0f).epsilon(0.0001f));
  REQUIRE(groupingDensities[2] == Approx(100.0f / 70.0f).epsilon(0.0001f));
  REQUIRE(groupingDensities[3] == Approx(-1.0f));

  // Checked features: each feature is assigned to the parent with the largest volume that checked it
  // Parent 1 (vol=200) processes first and checks features 1,2,3,4 (feature 4 via neighbor of feature 3)
  // Parent 2 (vol=100) processes second and checks features 3,4,5 (features 3,4 via neighbors of feature 4)
  //   But parent 2 vol=100 < parent 1 vol=200, so features 3,4 stay assigned to parent 1
  //   Feature 5 is only checked by parent 2
  // Expected: [0, 1, 1, 1, 1, 2]
  REQUIRE_NOTHROW(dataStructure.getDataRefAs<Int32Array>(k_CheckedFeaturesPath));
  const auto& checkedFeatures = dataStructure.getDataRefAs<Int32Array>(k_CheckedFeaturesPath);

  REQUIRE(checkedFeatures[0] == 0);
  REQUIRE(checkedFeatures[1] == 1);
  REQUIRE(checkedFeatures[2] == 1);
  REQUIRE(checkedFeatures[3] == 1);
  REQUIRE(checkedFeatures[4] == 1);
  REQUIRE(checkedFeatures[5] == 2);
}

TEST_CASE("SimplnxReview::ComputeGroupingDensityFilter: Both Options Enabled", "[SimplnxReview][ComputeGroupingDensityFilter]")
{
  UnitTest::LoadPlugins();

  DataStructure dataStructure = createTestDataStructure(true);
  ComputeGroupingDensityFilter filter;
  Arguments args = createFilterArgs(true, true);

  auto executeResult = filter.execute(dataStructure, args, nullptr, IFilter::MessageHandler{[](const IFilter::Message& message) { fmt::print("{}\n", message.message); }});
  SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);

  // With non-contiguous neighbors, all features get checked in parent 1's pass
  // Parent 1: density = 200/100 = 2.0
  // Parent 2: density = 100/100 = 1.0
  // Parent 3: density = -1.0
  REQUIRE_NOTHROW(dataStructure.getDataRefAs<Float32Array>(k_GroupingDensitiesPath));
  const auto& groupingDensities = dataStructure.getDataRefAs<Float32Array>(k_GroupingDensitiesPath);

  REQUIRE(groupingDensities[1] == Approx(2.0f).epsilon(0.0001f));
  REQUIRE(groupingDensities[2] == Approx(1.0f).epsilon(0.0001f));
  REQUIRE(groupingDensities[3] == Approx(-1.0f));

  // Parent 1 (vol=200) processes first and checks ALL features (1-5) via non-contiguous links
  // Parent 2 (vol=100) can't override any since 100 < 200
  // Expected: [0, 1, 1, 1, 1, 1]
  REQUIRE_NOTHROW(dataStructure.getDataRefAs<Int32Array>(k_CheckedFeaturesPath));
  const auto& checkedFeatures = dataStructure.getDataRefAs<Int32Array>(k_CheckedFeaturesPath);

  REQUIRE(checkedFeatures[0] == 0);
  REQUIRE(checkedFeatures[1] == 1);
  REQUIRE(checkedFeatures[2] == 1);
  REQUIRE(checkedFeatures[3] == 1);
  REQUIRE(checkedFeatures[4] == 1);
  REQUIRE(checkedFeatures[5] == 1);
}

// =============================================================================
// Preflight Error Tests
// =============================================================================

TEST_CASE("SimplnxReview::ComputeGroupingDensityFilter: Preflight Error - Feature tuple count mismatch", "[SimplnxReview][ComputeGroupingDensityFilter]")
{
  UnitTest::LoadPlugins();

  // Build a DataStructure where ParentIds has a different tuple count than Volumes
  DataStructure dataStructure;
  auto* imageGeom = ImageGeom::Create(dataStructure, k_ImageGeomName);
  imageGeom->setDimensions({1, 1, 1});

  auto* featureAM = AttributeMatrix::Create(dataStructure, k_FeatureAMName, {k_NumFeatures}, imageGeom->getId());
  auto* parentAM = AttributeMatrix::Create(dataStructure, k_ParentAMName, {k_NumParents}, imageGeom->getId());

  // Volumes with 6 tuples
  UnitTest::CreateTestDataArray<float32>(dataStructure, k_VolumesName, {k_NumFeatures}, {1}, featureAM->getId());
  // Contiguous NL with 6 tuples
  NeighborList<int32>::Create(dataStructure, k_ContiguousNLName, ShapeType{k_NumFeatures}, featureAM->getId());
  // Parent Volumes
  UnitTest::CreateTestDataArray<float32>(dataStructure, k_ParentVolumesName, {k_NumParents}, {1}, parentAM->getId());

  // ParentIds in a DIFFERENT AM with a different tuple count (mismatch!)
  auto* mismatchAM = AttributeMatrix::Create(dataStructure, "MismatchAM", {10}, imageGeom->getId());
  UnitTest::CreateTestDataArray<int32>(dataStructure, k_ParentIdsName, {10}, {1}, mismatchAM->getId());

  DataPath mismatchParentIdsPath = DataPath({k_ImageGeomName, "MismatchAM", k_ParentIdsName});

  ComputeGroupingDensityFilter filter;
  Arguments args = createFilterArgs(false, false);
  args.insertOrAssign(ComputeGroupingDensityFilter::k_ParentIdsPath_Key, std::make_any<DataPath>(mismatchParentIdsPath));

  auto preflightResult = filter.preflight(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_INVALID(preflightResult.outputActions);
}

TEST_CASE("SimplnxReview::ComputeGroupingDensityFilter: Preflight Error - Volumes not in AttributeMatrix", "[SimplnxReview][ComputeGroupingDensityFilter]")
{
  UnitTest::LoadPlugins();

  // Build a DataStructure where Volumes is NOT inside an AttributeMatrix
  DataStructure dataStructure;
  auto* imageGeom = ImageGeom::Create(dataStructure, k_ImageGeomName);
  imageGeom->setDimensions({1, 1, 1});

  // Create volumes directly under the ImageGeom (not in an AM)
  UnitTest::CreateTestDataArray<float32>(dataStructure, k_VolumesName, {k_NumFeatures}, {1}, imageGeom->getId());
  UnitTest::CreateTestDataArray<int32>(dataStructure, k_ParentIdsName, {k_NumFeatures}, {1}, imageGeom->getId());
  NeighborList<int32>::Create(dataStructure, k_ContiguousNLName, ShapeType{k_NumFeatures}, imageGeom->getId());

  auto* parentAM = AttributeMatrix::Create(dataStructure, k_ParentAMName, {k_NumParents}, imageGeom->getId());
  UnitTest::CreateTestDataArray<float32>(dataStructure, k_ParentVolumesName, {k_NumParents}, {1}, parentAM->getId());

  DataPath volumesNoAMPath = DataPath({k_ImageGeomName, k_VolumesName});
  DataPath parentIdsNoAMPath = DataPath({k_ImageGeomName, k_ParentIdsName});
  DataPath contiguousNLNoAMPath = DataPath({k_ImageGeomName, k_ContiguousNLName});

  ComputeGroupingDensityFilter filter;
  Arguments args = createFilterArgs(false, false);
  args.insertOrAssign(ComputeGroupingDensityFilter::k_FeatureVolumesArrayPath_Key, std::make_any<DataPath>(volumesNoAMPath));
  args.insertOrAssign(ComputeGroupingDensityFilter::k_ParentIdsPath_Key, std::make_any<DataPath>(parentIdsNoAMPath));
  args.insertOrAssign(ComputeGroupingDensityFilter::k_ContiguousNeighborListArrayPath_Key, std::make_any<DataPath>(contiguousNLNoAMPath));

  auto preflightResult = filter.preflight(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_INVALID(preflightResult.outputActions);
}

TEST_CASE("SimplnxReview::ComputeGroupingDensityFilter: Preflight Error - Parent Volumes not in AttributeMatrix", "[SimplnxReview][ComputeGroupingDensityFilter]")
{
  UnitTest::LoadPlugins();

  DataStructure dataStructure;
  auto* imageGeom = ImageGeom::Create(dataStructure, k_ImageGeomName);
  imageGeom->setDimensions({1, 1, 1});

  auto* featureAM = AttributeMatrix::Create(dataStructure, k_FeatureAMName, {k_NumFeatures}, imageGeom->getId());
  UnitTest::CreateTestDataArray<float32>(dataStructure, k_VolumesName, {k_NumFeatures}, {1}, featureAM->getId());
  UnitTest::CreateTestDataArray<int32>(dataStructure, k_ParentIdsName, {k_NumFeatures}, {1}, featureAM->getId());
  NeighborList<int32>::Create(dataStructure, k_ContiguousNLName, ShapeType{k_NumFeatures}, featureAM->getId());

  // Parent Volumes directly under ImageGeom (not in AM)
  UnitTest::CreateTestDataArray<float32>(dataStructure, k_ParentVolumesName, {k_NumParents}, {1}, imageGeom->getId());

  DataPath parentVolumesNoAMPath = DataPath({k_ImageGeomName, k_ParentVolumesName});

  ComputeGroupingDensityFilter filter;
  Arguments args = createFilterArgs(false, false);
  args.insertOrAssign(ComputeGroupingDensityFilter::k_ParentVolumesPath_Key, std::make_any<DataPath>(parentVolumesNoAMPath));

  auto preflightResult = filter.preflight(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_INVALID(preflightResult.outputActions);
}
