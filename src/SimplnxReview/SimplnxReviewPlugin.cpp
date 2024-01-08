#include "SimplnxReviewPlugin.hpp"
#include "SimplnxReviewLegacyUUIDMapping.hpp"

#include "SimplnxReview/SimplnxReview_filter_registration.hpp"

using namespace nx::core;

namespace
{
// This maps previous filters from DREAM.3D Version 6.x to DREAM.3D Version 7.x
std::map<nx::core::Uuid, nx::core::Uuid> k_SimplToComplexFilterMapping = {

};
// Plugin Uuid
constexpr AbstractPlugin::IdType k_ID = *Uuid::FromString("c09cf01b-014e-5adb-84eb-ea76fc79eeb1");
} // namespace

SimplnxReviewPlugin::SimplnxReviewPlugin()
: AbstractPlugin(k_ID, "SimplnxReview", "<<--Description was not read-->>", "BlueQuartz Software, LLC")
{
  std::vector<::FilterCreationFunc> filterFuncs = ::GetPluginFilterList();
  for(const auto& filterFunc : filterFuncs)
  {
    addFilter(filterFunc);
  }
}

SimplnxReviewPlugin::~SimplnxReviewPlugin() = default;

AbstractPlugin::SIMPLMapType SimplnxReviewPlugin::getSimplToSimplnxMap() const
{
  return nx::core::k_SIMPL_to_SimplnxReview;
}

SIMPLNX_DEF_PLUGIN(SimplnxReviewPlugin)
