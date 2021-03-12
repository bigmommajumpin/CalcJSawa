#ifndef SHARED_BANNER_VIEW_H
#define SHARED_BANNER_VIEW_H

#include <escher/metric.h>
#include <escher/view.h>
#include <escher/palette.h>

namespace Shared {

class BannerView : public Escher::View {
public:
  static KDCoordinate HeightGivenNumberOfLines(int linesCount);
  void drawRect(KDContext * ctx, KDRect rect) const override;
  KDSize minimalSizeForOptimalDisplay() const override;
  KDCoordinate minimalHeightForOptimalDisplayGivenWidth(KDCoordinate width) const;
  void reload() { layoutSubviews(); }
  static constexpr const KDFont * Font() { return KDFont::SmallFont; }
  static constexpr KDColor TextColor() { return KDColorBlack; }
  static constexpr KDColor BackgroundColor() { return Escher::Palette::GrayMiddle; }
private:
  static constexpr KDCoordinate LineSpacing = Escher::Metric::BannerTextMargin;
  int numberOfSubviews() const override = 0;
  View * subviewAtIndex(int index) override = 0;
  void layoutSubviews(bool force = false) override;
  int numberOfLinesGivenWidth(KDCoordinate width) const;
};

}

#endif
