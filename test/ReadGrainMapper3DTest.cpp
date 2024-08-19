#include <catch2/catch.hpp>

#include "simplnx/Parameters/ArrayCreationParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/UnitTest/UnitTestCommon.hpp"

#include "SimplnxReview/Filters/ReadGrainMapper3DFilter.hpp"
#include "SimplnxReview/SimplnxReview_test_dirs.hpp"

using namespace nx::core;

TEST_CASE("SimplnxReview::ReadGrainMapper3D: Valid Filter Execution", "[SimplnxReview][ReadGrainMapper3D]")
{
  // Instantiate the filter, a DataStructure object and an Arguments Object
  ReadGrainMapper3DFilter filter;
  DataStructure dataStructure;
  Arguments args;

  // Create default Parameters for the filter.

  // Preflight the filter and check result
  auto preflightResult = filter.preflight(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions);

  // Execute the filter and check the result
  auto executeResult = filter.execute(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);
}
