#ifndef POINCARE_PARENTHESIS_LEFT_LAYOUT_H
#define POINCARE_PARENTHESIS_LEFT_LAYOUT_H

#include <poincare/src/layout/parenthesis_left_right_layout.h>

namespace Poincare {

class ParenthesisLeftLayout : public ParenthesisLeftRightLayout {
public:
  using ParenthesisLeftRightLayout::ParenthesisLeftRightLayout;
  ExpressionLayout * clone() const override;
protected:
  void render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) override;
};

}

#endif
