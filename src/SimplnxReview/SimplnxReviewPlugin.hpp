#pragma once

#include "SimplnxReview/SimplnxReview_export.hpp"

#include "simplnx/Plugin/AbstractPlugin.hpp"

class SIMPLNXREVIEW_EXPORT SimplnxReviewPlugin : public nx::core::AbstractPlugin
{
public:
  SimplnxReviewPlugin();
  ~SimplnxReviewPlugin() override;

  SimplnxReviewPlugin(const SimplnxReviewPlugin&) = delete;
  SimplnxReviewPlugin(SimplnxReviewPlugin&&) = delete;

  SimplnxReviewPlugin& operator=(const SimplnxReviewPlugin&) = delete;
  SimplnxReviewPlugin& operator=(SimplnxReviewPlugin&&) = delete;

  /**
   * @brief Returns a map of UUIDs as strings, where SIMPL UUIDs are keys to
   * their simplnx counterpart
   * @return std::map<nx::core::Uuid, nx::core::Uuid>
   */
  SIMPLMapType getSimplToSimplnxMap() const override;
};
