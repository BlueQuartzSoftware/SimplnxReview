#include <CxPybind/CxPybind.hpp>

#include <SimplnxReview/SimplnxReviewPlugin.hpp>

#include "SimplnxReview/SimplnxReviewFilterBinding.hpp"

using namespace nx::core;
using namespace nx::core::CxPybind;
namespace py = pybind11;

using namespace pybind11::literals;

// ############################################################################
// IMPORTANT NOTE
// The below statement must have the name of the plugin IN ALL LOWER CASE!!!
// ############################################################################
PYBIND11_MODULE(simplnxreview, mod) // <== IS THAT IN ALL LOWER CASE?
{
  py::module_::import("simplnx");

  auto& internals = Internals::Instance();

  auto* plugin = internals.addPlugin<SimplnxReviewPlugin>();

  SimplnxReview::BindFilters(mod, internals);

  internals.registerPluginPyFilters(*plugin);
}
