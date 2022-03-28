#ifndef STATISTICS_FREQUENCY_CONTROLLER_H
#define STATISTICS_FREQUENCY_CONTROLLER_H

#include "plot_controller.h"
#include <apps/shared/round_cursor_view.h>

namespace Statistics {

class FrequencyController : public PlotController {
public:
  using PlotController::PlotController;

  // PlotControllerDelegate
  int totalValues(int series) const override { return m_store->totalCumulatedFrequencyValues(series); }
  double valueAtIndex(int series, int i) const override { return m_store->cumulatedFrequencyValueAtIndex(series, i); }
  double resultAtIndex(int series, int i) const override { return m_store->cumulatedFrequencyResultAtIndex(series, i); }
  void computeYBounds(float * yMin, float *yMax) const override;
  void computeXBounds(float * xMin, float *xMax) const override;
  bool connectPoints() const override { return true; }
  // Append '%' to vertical axis labels.
  void appendLabelSuffix(Shared::CurveView::Axis axis, char * labelBuffer, int maxSize, int glyphLength, int maxGlyphLength) const override;

  TELEMETRY_ID("Frequency");
private:
  constexpr static float k_numberOfCursorStepsInGradUnit = 5.0f;

  void switchCursor(bool seriesChanged);
  const char * resultMessageTemplate() const override { return "%s%s%*.*ed%%"; }
  I18n::Message resultMessage() const override { return I18n::Message::StatisticsFrequencyFcc; }
  // MultipleDataViewController
  void viewWillAppearBeforeReload() override;
  bool moveSelectionHorizontally(int deltaIndex) override;
  bool moveSelectionVertically(int deltaIndex) override;

  Shared::RoundCursorView m_roundCursorView;
  bool m_continuousCursor = false;
};

}  // namespace Statistics

#endif /* STATISTICS_FREQUENCY_CONTROLLER_H */
