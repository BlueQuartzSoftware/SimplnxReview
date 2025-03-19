/**
 * This file is auto generated from the original OrientationAnalysis/ComputeMicroTextureRegionsFilter
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
 * When you start working on this unit test remove "[ComputeMicroTextureRegionsFilter][.][UNIMPLEMENTED]"
 * from the TEST_CASE macro. This will enable this unit test to be run by default
 * and report errors.
 */

#include <catch2/catch.hpp>

#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/DataGroupSelectionParameter.hpp"
#include "simplnx/Parameters/StringParameter.hpp"
#include "simplnx/UnitTest/UnitTestCommon.hpp"

#include "SimplnxReview/Filters/ComputeMicroTextureRegionsFilter.hpp"
#include "SimplnxReview/SimplnxReview_test_dirs.hpp"

using namespace nx::core;

TEST_CASE("SimplnxReview::ComputeMicroTextureRegionsFilter: Valid Filter Execution", "[SimplnxReview][ComputeMicroTextureRegionsFilter][.][UNIMPLEMENTED][!mayfail]")
{
  // Instantiate the filter, a DataStructure object and an Arguments Object
  ComputeMicroTextureRegionsFilter filter;
  DataStructure ds;
  Arguments args;

  // Create default Parameters for the filter.
  args.insertOrAssign(ComputeMicroTextureRegionsFilter::k_FeatureIdsArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(ComputeMicroTextureRegionsFilter::k_CellFeatureAttributeMatrixPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(ComputeMicroTextureRegionsFilter::k_MicroTextureRegionNumCellsArrayName_Key, std::make_any<StringParameter::ValueType>("SomeString"));
  args.insertOrAssign(ComputeMicroTextureRegionsFilter::k_MicroTextureRegionFractionOccupiedArrayName_Key, std::make_any<StringParameter::ValueType>("SomeString"));

  // Preflight the filter and check result
  auto preflightResult = filter.preflight(ds, args);
  SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions);

  // Execute the filter and check the result
  auto executeResult = filter.execute(ds, args);
  SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);
}

// TEST_CASE("OrientationAnalysis::ComputeMicroTextureRegionsFilter: InValid Filter Execution")
//{
//
// }
