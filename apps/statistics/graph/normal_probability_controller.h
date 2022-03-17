#ifndef STATISTICS_NORMAL_PROBABILITY_CONTROLLER_H
#define STATISTICS_NORMAL_PROBABILITY_CONTROLLER_H

#include "plot_controller.h"

namespace Statistics {

class NormalProbabilityController : public PlotController {
public:
  using PlotController::PlotController;
  // PlotControllerDelegate
  int totalValues(int series, int * sortedIndex) const override { return m_store->totalNormalProbabilityValues(series); }
  double valueAtIndex(int series, int * sortedIndex, int i) const override { return m_store->normalProbabilityValueAtIndex(series, sortedIndex, i); }
  double resultAtIndex(int series, int * sortedIndex, int i) const override { return m_store->normalProbabilityResultAtIndex(series, i); }
  void computeYBounds(float * yMin, float *yMax) const override;
  void computeXBounds(float * xMin, float *xMax) const override;
  bool drawSeriesZScoreLine(int series, float * x, float * y, float * u, float * v, KDColor * color) const override;

  TELEMETRY_ID("NormalProbability");
private:
  const char * resultMessageTemplate() const override { return "%s : %*.*ed"; }
  I18n::Message resultMessage() const override { return I18n::Message::StatisticsNormalProbabilityZScore; }
};

}  // namespace Statistics

#endif /* STATISTICS_NORMAL_PROBABILITY_CONTROLLER_H */
