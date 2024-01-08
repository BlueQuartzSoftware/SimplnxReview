#include <CxPybind/CxPybind.hpp>

#include <SimplnxReview/SimplnxReviewPlugin.hpp>

#include "SimplnxReview/SimplnxReviewFilterBinding.hpp"

using namespace nx::core;
using namespace nx::core::CxPybind;
namespace py = pybind11;

using namespace pybind11::literals;

PYBIND11_MODULE(SimplnxReview, mod)
{
  py::module_::import("simplnx");

  auto& internals = Internals::Instance();

  auto* plugin = internals.addPlugin<SimplnxReviewPlugin>();

  SimplnxReview::BindFilters(mod, internals);

  internals.registerPluginPyFilters(*plugin);
}
