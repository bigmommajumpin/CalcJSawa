#ifndef POINCARE_BASELINE_RELATIVE_LAYOUT_H
#define POINCARE_BASELINE_RELATIVE_LAYOUT_H

#include <poincare/static_layout_hierarchy.h>

namespace Poincare {

class BaselineRelativeLayout : public StaticLayoutHierarchy<2> {
public:
  enum class Type {
    Subscript,
    Superscript
  };
  BaselineRelativeLayout(ExpressionLayout * base, ExpressionLayout * indice, Type type, bool cloneOperands);
  ExpressionLayout * clone() const override;
  bool moveLeft(ExpressionLayoutCursor * cursor) override;
  bool moveRight(ExpressionLayoutCursor * cursor) override;
protected:
  ExpressionLayout * baseLayout();
  ExpressionLayout * indiceLayout();
  void render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) override;
  KDSize computeSize() override;
  KDPoint positionOfChild(ExpressionLayout * child) override;
  Type m_type;
private:
  constexpr static KDCoordinate k_indiceHeight = 5;
};

}

#endif
