/**
 * This file is auto generated from the original OrientationAnalysis/GroupMicroTextureRegionsFilter
 * runtime information. These are the steps that need to be taken to utilize this
 * unit test in the proper way.
 *
 * 1: Validate each of the default parameters that gets created.
 * 2: Inspect the actual filter to determine if the filter in its default state
 * would pass or fail BOTH the preflight() and execute() methods
 * 3: UPDATE the ```REQUIRE(result.result.valid());``` code to have the proper
 *
 * 4: Add additional unit tests to actually test each code path within the filter
 *
 * There are some example Catch2 ```TEST_CASE``` sections for your inspiration.
 *
 * NOTE the format of the ```TEST_CASE``` macro. Please stick to this format to
 * allow easier parsing of the unit tests.
 *
 * When you start working on this unit test remove "[GroupMicroTextureRegionsFilter][.][UNIMPLEMENTED]"
 * from the TEST_CASE macro. This will enable this unit test to be run by default
 * and report errors.
 */

#include <catch2/catch.hpp>

#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"
#include "simplnx/Parameters/StringParameter.hpp"

#include "SimplnxReview/Filters/GroupMicroTextureRegionsFilter.hpp"
#include "SimplnxReview/SimplnxReview_test_dirs.hpp"

using namespace nx::core;

TEST_CASE("SimplnxReview::GroupMicroTextureRegionsFilter: Valid Filter Execution", "[OrientationAnalysis][GroupMicroTextureRegionsFilter][.][UNIMPLEMENTED][!mayfail]")
{
  // Instantiate the filter, a DataStructure object and an Arguments Object
  GroupMicroTextureRegionsFilter filter;
  DataStructure ds;
  Arguments args;

  // Create default Parameters for the filter.
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_UseNonContiguousNeighbors_Key, std::make_any<bool>(false));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_NonContiguousNeighborListArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_ContiguousNeighborListArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_UseRunningAverage_Key, std::make_any<bool>(false));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_CAxisTolerance_Key, std::make_any<float32>(1.23345f));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_FeatureIdsArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_FeaturePhasesArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_VolumesArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_AvgQuatsArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_NewCellFeatureAttributeMatrixName_Key, std::make_any<StringParameter::ValueType>("SomeString"));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_CellParentIdsArrayName_Key, std::make_any<StringParameter::ValueType>("SomeString"));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_FeatureParentIdsArrayName_Key, std::make_any<StringParameter::ValueType>("SomeString"));
  args.insertOrAssign(GroupMicroTextureRegionsFilter::k_ActiveArrayName_Key, std::make_any<StringParameter::ValueType>("SomeString"));

  // Preflight the filter and check result
  auto preflightResult = filter.preflight(ds, args);
  REQUIRE(preflightResult.outputActions.valid());

  // Execute the filter and check the result
  auto executeResult = filter.execute(ds, args);
  REQUIRE(executeResult.result.valid());
}

// TEST_CASE("OrientationAnalysis::GroupMicroTextureRegionsFilter: InValid Filter Execution")
//{
//
// }
