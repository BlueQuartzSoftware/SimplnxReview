#include "SimplnxReviewPlugin.hpp"
#include "SimplnxReviewLegacyUUIDMapping.hpp"

#include "SimplnxReview/SimplnxReview_filter_registration.hpp"

using namespace nx::core;

namespace
{
// Plugin Uuid
constexpr AbstractPlugin::IdType k_ID = *Uuid::FromString("abcdef1b-014e-5adb-84eb-bad76fc79fb1");
} // namespace

SimplnxReviewPlugin::SimplnxReviewPlugin()
: AbstractPlugin(k_ID, "SimplnxReview", "Plugin to hold highly experimental filters", "BlueQuartz Software, LLC")
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
