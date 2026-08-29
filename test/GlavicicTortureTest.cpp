#include <catch2/catch.hpp>

#include "simplnx/Core/Application.hpp"
#include "simplnx/DataStructure/Geometry/ImageGeom.hpp"
#include "simplnx/UnitTest/UnitTestCommon.hpp"

#include "SimplnxReview/Filters/GlavicicTortureFilter.hpp"
#include "SimplnxReview/SimplnxReview_test_dirs.hpp"

using namespace nx::core;

namespace
{
const std::string k_ImageGeometryName = "Image Geometry";
const std::string k_CellDataName = "Cell Data";
const std::string k_ExpectedMTRIdsName = "Expected MTR Ids";
const std::string k_EulerAnglesName = "EulerAngles";

const DataPath k_ImageGeometryPath({k_ImageGeometryName});
const DataPath k_ExpectedMTRIdsPath = k_ImageGeometryPath.createChildPath(k_CellDataName).createChildPath(k_ExpectedMTRIdsName);
const DataPath k_EulerAnglesPath = k_ImageGeometryPath.createChildPath(k_CellDataName).createChildPath(k_EulerAnglesName);

const std::vector<std::string> k_GroupArrayNames = {"Group 0 Euler Angles", "Group 1 Euler Angles", "Group 2 Euler Angles", "Group 3 Euler Angles", "Group 4 Euler Angles", "Group 6 Euler Angles"};
const std::vector<int32> k_GroupIds = {0, 1, 2, 3, 4, 6};
constexpr usize k_NumGroupTuples = 4;

const SizeVec3 k_ImageDims = {100, 100, 1}; // X, Y, Z

// Encodes a unique, non-zero value for every component of every tuple of every group array
float32 GroupComponentValue(usize groupIndex, usize tupleIndex, usize componentIndex)
{
  return static_cast<float32>(1000 * (groupIndex + 1) + 10 * tupleIndex + componentIndex);
}
} // namespace

TEST_CASE("SimplnxReview::GlavicicTortureFilter: Valid Filter Execution", "[SimplnxReview][GlavicicTortureFilter]")
{
  auto app = Application::GetOrCreateInstance();
  UnitTest::LoadPlugins();

  DataStructure dataStructure;
  ImageGeom* imageGeomPtr = ImageGeom::Create(dataStructure, k_ImageGeometryName);
  imageGeomPtr->setDimensions(k_ImageDims);
  imageGeomPtr->setOrigin({0.0f, 0.0f, 0.0f});
  imageGeomPtr->setSpacing({1.0f, 1.0f, 1.0f});

  const std::vector<usize> tupleShape = {k_ImageDims[2], k_ImageDims[1], k_ImageDims[0]};
  AttributeMatrix* cellDataPtr = AttributeMatrix::Create(dataStructure, k_CellDataName, tupleShape, imageGeomPtr->getId());
  imageGeomPtr->setCellData(*cellDataPtr);

  UInt8Array* expectedMTRIdsPtr = UnitTest::CreateTestDataArray<uint8>(dataStructure, k_ExpectedMTRIdsName, tupleShape, {1}, cellDataPtr->getId());
  auto& expectedMTRIdsRef = expectedMTRIdsPtr->getDataStoreRef();
  usize numCells = imageGeomPtr->getNumberOfCells();
  for(usize tupleIndex = 0; tupleIndex < numCells; tupleIndex++)
  {
    expectedMTRIdsRef[tupleIndex] = static_cast<uint8>(k_GroupIds[tupleIndex % k_GroupIds.size()]);
  }

  for(usize groupIndex = 0; groupIndex < k_GroupArrayNames.size(); groupIndex++)
  {
    Float32Array* groupArrayPtr = UnitTest::CreateTestDataArray<float32>(dataStructure, k_GroupArrayNames[groupIndex], {k_NumGroupTuples}, {3});
    auto& groupStoreRef = groupArrayPtr->getDataStoreRef();
    for(usize tupleIndex = 0; tupleIndex < k_NumGroupTuples; tupleIndex++)
    {
      for(usize componentIndex = 0; componentIndex < 3; componentIndex++)
      {
        groupStoreRef[tupleIndex * 3 + componentIndex] = GroupComponentValue(groupIndex, tupleIndex, componentIndex);
      }
    }
  }

  GlavicicTortureFilter filter;
  Arguments args;
  args.insertOrAssign(GlavicicTortureFilter::k_UseSeed_Key, std::make_any<bool>(true));
  args.insertOrAssign(GlavicicTortureFilter::k_SeedValue_Key, std::make_any<uint64>(5489));
  args.insertOrAssign(GlavicicTortureFilter::k_ImageGeometryPath_Key, std::make_any<DataPath>(k_ImageGeometryPath));
  args.insertOrAssign(GlavicicTortureFilter::k_ExpectedMTRIdsArrayPath_Key, std::make_any<DataPath>(k_ExpectedMTRIdsPath));
  args.insertOrAssign(GlavicicTortureFilter::k_Group0ArrayPath_Key, std::make_any<DataPath>(DataPath({k_GroupArrayNames[0]})));
  args.insertOrAssign(GlavicicTortureFilter::k_Group1ArrayPath_Key, std::make_any<DataPath>(DataPath({k_GroupArrayNames[1]})));
  args.insertOrAssign(GlavicicTortureFilter::k_Group2ArrayPath_Key, std::make_any<DataPath>(DataPath({k_GroupArrayNames[2]})));
  args.insertOrAssign(GlavicicTortureFilter::k_Group3ArrayPath_Key, std::make_any<DataPath>(DataPath({k_GroupArrayNames[3]})));
  args.insertOrAssign(GlavicicTortureFilter::k_Group4ArrayPath_Key, std::make_any<DataPath>(DataPath({k_GroupArrayNames[4]})));
  args.insertOrAssign(GlavicicTortureFilter::k_Group6ArrayPath_Key, std::make_any<DataPath>(DataPath({k_GroupArrayNames[5]})));
  args.insertOrAssign(GlavicicTortureFilter::k_EulerAnglesArrayName_Key, std::make_any<std::string>(k_EulerAnglesName));

  auto preflightResult = filter.preflight(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_VALID(preflightResult.outputActions);

  auto executeResult = filter.execute(dataStructure, args);
  SIMPLNX_RESULT_REQUIRE_VALID(executeResult.result);

  REQUIRE_NOTHROW(dataStructure.getDataRefAs<Float32Array>(k_EulerAnglesPath));
  const auto& eulerAnglesRef = dataStructure.getDataRefAs<Float32Array>(k_EulerAnglesPath);
  REQUIRE(eulerAnglesRef.getNumberOfTuples() == numCells);
  REQUIRE(eulerAnglesRef.getNumberOfComponents() == 3);

  // Every cell received a randomly picked Euler angle from the group matching its Expected MTR Id
  const auto& eulerAnglesStoreRef = eulerAnglesRef.getDataStoreRef();
  usize mismatchCount = 0;
  for(usize cellIndex = 0; cellIndex < numCells; cellIndex++)
  {
    usize groupIndex = cellIndex % k_GroupIds.size();
    bool matchedATuple = false;
    for(usize tupleIndex = 0; tupleIndex < k_NumGroupTuples; tupleIndex++)
    {
      if(eulerAnglesStoreRef[cellIndex * 3 + 0] == GroupComponentValue(groupIndex, tupleIndex, 0) && eulerAnglesStoreRef[cellIndex * 3 + 1] == GroupComponentValue(groupIndex, tupleIndex, 1) &&
         eulerAnglesStoreRef[cellIndex * 3 + 2] == GroupComponentValue(groupIndex, tupleIndex, 2))
      {
        matchedATuple = true;
        break;
      }
    }
    if(!matchedATuple)
    {
      mismatchCount++;
    }
  }
  REQUIRE(mismatchCount == 0);

  UnitTest::CheckArraysInheritTupleDims(dataStructure);
}
