#pragma once

#include "simplnx/Plugin/AbstractPlugin.hpp"

#include <nlohmann/json.hpp>

// clang-format off
#include "SimplnxReview/Filters/GroupMicroTextureRegionsFilter.hpp"
#include "SimplnxReview/Filters/MergeColoniesFilter.hpp"
#include "SimplnxReview/Filters/FindSaltykovSizesFilter.hpp"
#include "SimplnxReview/Filters/FindMicroTextureRegionsFilter.hpp"
#include "SimplnxReview/Filters/FindLocalAverageCAxisMisalignmentsFilter.hpp"
#include "SimplnxReview/Filters/FindGroupingDensityFilter.hpp"

// @@__HEADER__TOKEN__DO__NOT__DELETE__@@

#include <map>
#include <string>

namespace nx::core
{
  static const AbstractPlugin::SIMPLMapType k_SIMPL_to_SimplnxReview
  {
    // syntax std::make_pair {Dream3d UUID , Dream3dnx UUID, {}}}, // dream3d-class-name
    {nx::core::Uuid::FromString("5e18a9e2-e342-56ac-a54e-3bd0ca8b9c53").value(), {nx::core::FilterTraits<GroupMicroTextureRegionsFilter>::uuid}}, // GroupMicroTextureRegions
    {nx::core::Uuid::FromString("2c4a6d83-6a1b-56d8-9f65-9453b28845b9").value(), {nx::core::FilterTraits<MergeColoniesFilter>::uuid}}, // GroupMicroTextureRegions
    {nx::core::Uuid::FromString("cc76cffe-81ad-5ece-be2a-ce127c5fa6d7").value(), {nx::core::FilterTraits<FindSaltykovSizesFilter>::uuid}}, // GroupMicroTextureRegions
    {nx::core::Uuid::FromString("90f8e3b1-2460-5862-95a1-a9e06f5ee759").value(), {nx::core::FilterTraits<FindMicroTextureRegionsFilter>::uuid}}, // GroupMicroTextureRegions
    {nx::core::Uuid::FromString("49b2dd47-bb29-50d4-a051-5bad9b6b9f80").value(), {nx::core::FilterTraits<FindLocalAverageCAxisMisalignmentsFilter>::uuid}}, // GroupMicroTextureRegions
    {nx::core::Uuid::FromString("708be082-8b08-4db2-94be-52781ed4d53d").value(), {nx::core::FilterTraits<FindGroupingDensityFilter>::uuid}}, // GroupMicroTextureRegions
    // @@__MAP__UPDATE__TOKEN__DO__NOT__DELETE__@@
  };

} // namespace nx::core
// clang-format on
