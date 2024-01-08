#pragma once

#include "simplnx/Plugin/AbstractPlugin.hpp"

#include <nlohmann/json.hpp>

// clang-format off
#include "SimplnxReview/Filters/GroupMicroTextureRegionsFilter.hpp"
// @@__HEADER__TOKEN__DO__NOT__DELETE__@@

#include <map>
#include <string>

namespace nx::core
{
  static const AbstractPlugin::SIMPLMapType k_SIMPL_to_SimplnxReview
  {
    // syntax std::make_pair {Dream3d UUID , Dream3dnx UUID, {}}}, // dream3d-class-name
    {nx::core::Uuid::FromString("5e18a9e2-e342-56ac-a54e-3bd0ca8b9c53").value(), {nx::core::FilterTraits<GroupMicroTextureRegionsFilter>::uuid}}, // GroupMicroTextureRegions

    // @@__MAP__UPDATE__TOKEN__DO__NOT__DELETE__@@
  };

} // namespace nx::core
// clang-format on
