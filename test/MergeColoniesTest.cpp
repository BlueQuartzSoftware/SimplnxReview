#include <catch2/catch.hpp>

#include "simplnx/Core/Application.hpp"
#include "simplnx/Parameters/ArrayCreationParameter.hpp"
#include "simplnx/Parameters/BoolParameter.hpp"
#include "simplnx/Parameters/DataObjectNameParameter.hpp"
#include "simplnx/Pipeline/Pipeline.hpp"
#include "simplnx/Pipeline/PipelineFilter.hpp"
#include "simplnx/UnitTest/UnitTestCommon.hpp"

#include "SimplnxReview/Filters/MergeColoniesFilter.hpp"
#include "SimplnxReview/SimplnxReview_test_dirs.hpp"

#include <fstream>

using namespace nx::core;

TEST_CASE("SimplnxReview::MergeColoniesFilter: Valid Filter Execution", "[SimplnxReview][MergeColoniesFilter]")
{
  // Instantiate the filter, a DataStructure object and an Arguments Object
  MergeColoniesFilter filter;
  DataStructure dataStructure;
  Arguments args;

  // Create default Parameters for the filter.
  args.insertOrAssign(MergeColoniesFilter::k_UseNonContiguousNeighbors_Key, std::make_any<bool>(false));
  args.insertOrAssign(MergeColoniesFilter::k_NonContiguousNeighborListArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(MergeColoniesFilter::k_ContiguousNeighborListArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(MergeColoniesFilter::k_AxisTolerance_Key, std::make_any<float32>(1.23345f));
  args.insertOrAssign(MergeColoniesFilter::k_AngleTolerance_Key, std::make_any<float32>(1.23345f));
  args.insertOrAssign(MergeColoniesFilter::k_FeaturePhasesArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(MergeColoniesFilter::k_AvgQuatsArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(MergeColoniesFilter::k_FeatureIdsArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(MergeColoniesFilter::k_CellPhasesArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(MergeColoniesFilter::k_CrystalStructuresArrayPath_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(MergeColoniesFilter::k_CellParentIdsArrayName_Key, std::make_any<DataObjectNameParameter::ValueType>(""));
  args.insertOrAssign(MergeColoniesFilter::k_NewCellFeatureAttributeMatrixName_Key, std::make_any<DataPath>(DataPath{}));
  args.insertOrAssign(MergeColoniesFilter::k_FeatureParentIdsArrayName_Key, std::make_any<DataObjectNameParameter::ValueType>(""));
  args.insertOrAssign(MergeColoniesFilter::k_ActiveArrayName_Key, std::make_any<DataObjectNameParameter::ValueType>(""));
}

TEST_CASE("SimplnxReview::MergeColoniesFilter: SIMPL Backwards Compatibility", "[SimplnxReview][MergeColoniesFilter][BackwardsCompatibility]")
{
  auto app = Application::GetOrCreateInstance();
  UnitTest::LoadPlugins();
  auto filterList = app->getFilterList();

  const fs::path conversionDir = fs::path(nx::core::unit_test::k_SourceDir.view()) / "test" / "simpl_conversion";

  const std::vector<std::pair<std::string, fs::path>> fixtures = {
      {"SIMPL 6.5 (UUID)", conversionDir / "6_5" / "MergeColoniesFilter.json"},
      {"SIMPL 6.4 (Filter_Name)", conversionDir / "6_4" / "MergeColoniesFilter.json"},
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
      REQUIRE(filter->uuid() == FilterTraits<MergeColoniesFilter>::uuid);

      CHECK(pipelineFilter->getComments().empty());

      const Arguments args = pipelineFilter->getArguments();
      CHECK(args.value<std::string>(MergeColoniesFilter::k_ActiveArrayName_Key) == "TestName");
      CHECK(args.value<float32>(MergeColoniesFilter::k_AngleTolerance_Key) == 2.5f);
      CHECK(args.value<DataPath>(MergeColoniesFilter::k_AvgQuatsArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<float32>(MergeColoniesFilter::k_AxisTolerance_Key) == 2.5f);
      CHECK(args.value<std::string>(MergeColoniesFilter::k_CellParentIdsArrayName_Key) == "TestName");
      CHECK(args.value<DataPath>(MergeColoniesFilter::k_CellPhasesArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<DataPath>(MergeColoniesFilter::k_ContiguousNeighborListArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<DataPath>(MergeColoniesFilter::k_CrystalStructuresArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<DataPath>(MergeColoniesFilter::k_FeatureIdsArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<std::string>(MergeColoniesFilter::k_FeatureParentIdsArrayName_Key) == "TestName");
      CHECK(args.value<DataPath>(MergeColoniesFilter::k_FeaturePhasesArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      // AMPathBuilderFilterParameterConverter - verified by successful pipeline loading
      CHECK(args.value<DataPath>(MergeColoniesFilter::k_NewCellFeatureAttributeMatrixName_Key) == DataPath({"DataContainer", "TestName"}));
      CHECK(args.value<DataPath>(MergeColoniesFilter::k_NonContiguousNeighborListArrayPath_Key) == DataPath({"DataContainer", "CellData", "TestArray"}));
      CHECK(args.value<bool>(MergeColoniesFilter::k_UseNonContiguousNeighbors_Key) == true);
    }
  }
}
