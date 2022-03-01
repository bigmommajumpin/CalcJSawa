#ifndef STATISTICS_NORMAL_PROBABILITY_VIEW_H
#define STATISTICS_NORMAL_PROBABILITY_VIEW_H

#include <apps/shared/curve_view.h>
#include <apps/shared/curve_view_cursor.h>
#include <escher/view.h>
#include "plot_banner_view.h"
#include "normal_probability_curve_view.h"
#include "plot_range.h"
#include "multiple_data_view.h"
#include "../store.h"

namespace Statistics {

/* TODO : Add an intermediary class between MultipleDataView and View, as well
 *        as between MultipleDataViewController and ViewController +
 *        GraphButtonRowDelegate to avoid having to completely re-implement
 *        MultipleDataView's methods.
 *        Move some logic in the controller so the store doesn't need to be
 *        passed down. */
class NormalProbabilityView : public MultipleDataView {
public:
  NormalProbabilityView(Store * store);

  // MultipleDataView
  int numberOfSubviews() const override { return 2; } // CurveView and Banner
  Escher::View * subviewAtIndex(int index) override;
  // All series are displayed in the same curve view
  Shared::CurveView * dataViewAtIndex(int index) override { return &m_curveView; }
  int seriesOfSubviewAtIndex(int index) override { return index; }
  PlotBannerView * bannerView() override { return &m_bannerView; }
  void moveCursorTo(int i, int series) { m_curveView.moveCursorTo(i, series); }
  void reload() override;

private:
  void layoutDataSubviews(bool force) override;

  NormalProbabilityCurveView m_curveView;
  PlotRange m_graphRange;
  Shared::CurveViewCursor m_cursor;
  PlotBannerView m_bannerView;
  Shared::CursorView m_cursorView;
};

}  // namespace Statistics

#endif /* STATISTICS_NORMAL_PROBABILITY_VIEW_H */
