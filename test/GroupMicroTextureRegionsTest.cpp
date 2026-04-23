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

#include "simplnx/Core/Application.hpp"
#include "simplnx/Parameters/ArraySelectionParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/NumberParameter.hpp"
#include "simplnx/Parameters/StringParameter.hpp"
#include "simplnx/Pipeline/Pipeline.hpp"
#include "simplnx/Pipeline/PipelineFilter.hpp"
#include "simplnx/UnitTest/UnitTestCommon.hpp"

#include "SimplnxReview/Filters/GroupMicroTextureRegionsFilter.hpp"
#include "SimplnxReview/SimplnxReview_test_dirs.hpp"

#include <fstream>

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
  SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions);

  // Execute the filter and check the result
  auto executeResult = filter.execute(ds, args);
  SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);
}

// TEST_CASE("OrientationAnalysis::GroupMicroTextureRegionsFilter: InValid Filter Execution")
//{
//
// }

TEST_CASE("SimplnxReview::GroupMicroTextureRegionsFilter: SIMPL Backwards Compatibility", "[SimplnxReview][GroupMicroTextureRegionsFilter][BackwardsCompatibility]")
{
  auto app = Application::GetOrCreateInstance();
  UnitTest::LoadPlugins();
  auto filterList = app->getFilterList();

  const fs::path conversionDir = fs::path(nx::core::unit_test::k_SourceDir.view()) / "test" / "simpl_conversion";

  const std::vector<std::pair<std::string, fs::path>> fixtures = {
      {"SIMPL 6.5 (UUID)", conversionDir / "6_5" / "GroupMicroTextureRegionsFilter.json"},
      {"SIMPL 6.4 (Filter_Name)", conversionDir / "6_4" / "GroupMicroTextureRegionsFilter.json"},
  };

  for(const auto& [label, fixturePath] : fixtures)
  {
    DYNAMIC_SECTION(label)
    {
      auto pipelineResult = Pipeline::FromSIMPLFile(fixturePath, filterList);
      REQUIRE(pipelineResult.valid());

      auto& pipeline = pipelineResult.value();
      REQUIRE(pipeline.size() == 1);

      auto* pipelineFilter = dynamic_cast<PipelineFilter*>(pipeline.at(0));
      REQUIRE(pipelineFilter != nullptr);

      const IFilter* filter = pipelineFilter->getFilter();
      REQUIRE(filter != nullptr);
      REQUIRE(filter->uuid() == FilterTraits<GroupMicroTextureRegionsFilter>::uuid);

      CHECK(pipelineFilter->getComments().empty());

      const Arguments args = pipelineFilter->getArguments();
      CHECK(args.value<std::string>(GroupMicroTextureRegionsFilter::k_ActiveArrayName_Key) == "TestName");
      CHECK(args.value<DataPath>(GroupMicroTextureRegionsFilter::k_AvgQuatsArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<float32>(GroupMicroTextureRegionsFilter::k_CAxisTolerance_Key) == 2.5f);
      CHECK(args.value<std::string>(GroupMicroTextureRegionsFilter::k_CellParentIdsArrayName_Key) == "TestName");
      CHECK(args.value<DataPath>(GroupMicroTextureRegionsFilter::k_ContiguousNeighborListArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<DataPath>(GroupMicroTextureRegionsFilter::k_CrystalStructuresArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<DataPath>(GroupMicroTextureRegionsFilter::k_FeatureIdsArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<std::string>(GroupMicroTextureRegionsFilter::k_FeatureParentIdsArrayName_Key) == "TestName");
      CHECK(args.value<DataPath>(GroupMicroTextureRegionsFilter::k_FeaturePhasesArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<DataPath>(GroupMicroTextureRegionsFilter::k_NewCellFeatureAttributeMatrixName_Key) == DataPath({"TestName"}));
      CHECK(args.value<DataPath>(GroupMicroTextureRegionsFilter::k_NonContiguousNeighborListArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<bool>(GroupMicroTextureRegionsFilter::k_UseNonContiguousNeighbors_Key) == true);
      CHECK(args.value<bool>(GroupMicroTextureRegionsFilter::k_UseRunningAverage_Key) == true);
      CHECK(args.value<DataPath>(GroupMicroTextureRegionsFilter::k_VolumesArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
    }
  }
}
