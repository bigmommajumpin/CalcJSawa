#ifndef POINCARE_LAYOUT_REFERENCE_H
#define POINCARE_LAYOUT_REFERENCE_H

#include <omg/directions.h>
#include <poincare/context.h>
#include <poincare/layout_node.h>
#include <poincare/tree_handle.h>
#include <poincare/trinary_boolean.h>
#include <escher/palette.h>

namespace Poincare {

class Expression;

class Layout : public TreeHandle {
  friend class AdditionNode;
  friend class GridLayoutNode;
  friend class HorizontalLayoutNode;
  friend class InputBeautification;
  friend class LayoutNode;
  friend class VerticalOffsetLayoutNode;
public:
  Layout() : TreeHandle() {}
  Layout(const LayoutNode * node) : TreeHandle(node) {}
  Layout clone() const;
  LayoutNode * node() const {
    assert(isUninitialized() || !TreeHandle::node()->isGhost());
    return static_cast<LayoutNode *>(TreeHandle::node());
  }
  static Layout LayoutFromAddress(const void * address, size_t size);

  // Properties
  LayoutNode::Type type() const { return node()->type(); }
  bool isHorizontal() const { return node()->isHorizontal(); }
  bool isIdenticalTo(Layout l, bool makeEditable = false) { return isUninitialized() ? l.isUninitialized() : node()->isIdenticalTo(l, makeEditable); }
  bool hasTopLevelEquationSymbol() const { return privateHasTopLevelComparisonSymbol(false); }
  bool hasTopLevelComparisonSymbol() const { return privateHasTopLevelComparisonSymbol(true); }

  // Rendering
  void draw(KDContext * ctx, KDPoint p, KDFont::Size font, KDColor expressionColor, KDColor backgroundColor, LayoutSelection selection, KDColor selectionColor = Escher::Palette::Select);
  void draw(KDContext * ctx, KDPoint p, KDFont::Size font, KDColor expressionColor, KDColor backgroundColor);
  void render(KDContext * ctx, KDPoint p, KDFont::Size font, KDColor expressionColor, KDColor backgroundColor) {
    return node()->render(ctx, p, font, expressionColor, backgroundColor);
  }
  KDSize layoutSize(KDFont::Size font) const { return node()->layoutSize(font); }
  KDPoint absoluteOrigin(KDFont::Size font) const { return node()->absoluteOrigin(font); }
  KDCoordinate baseline(KDFont::Size font) { return node()->baseline(font); }
  void invalidAllSizesPositionsAndBaselines() { return node()->invalidAllSizesPositionsAndBaselines(); }

  // Serialization
  int serializeForParsing(char * buffer, int bufferSize) const { return node()->serialize(buffer, bufferSize); }
  int serializeParsedExpression(char * buffer, int bufferSize, Context * context) const;

  // Layout properties
  /* Return True if the layout succeeded the test, Unknown if its children
   * could succeed, and False if the recursion should stop. */
  typedef TrinaryBoolean (*LayoutTest)(const Layout l);
  Layout recursivelyMatches(LayoutTest test) const;
  bool isEmpty() const { return node()->isEmpty(); }
  bool isCollapsable(int * numberOfOpenParenthesis, bool goingLeft) const { return const_cast<Layout *>(this)->node()->isCollapsable(numberOfOpenParenthesis, goingLeft); }
  int leftCollapsingAbsorbingChildIndex() const { return const_cast<Layout *>(this)->node()->leftCollapsingAbsorbingChildIndex(); }
  int rightCollapsingAbsorbingChildIndex() const { return const_cast<Layout *>(this)->node()->rightCollapsingAbsorbingChildIndex(); }
  bool hasText() { return node()->hasText(); }
  Layout XNTLayout() const;

  // Layout modification
  bool removeGraySquaresFromAllGridAncestors() { return node()->removeGraySquaresFromAllGridAncestors(); }
  bool removeGraySquaresFromAllGridChildren() { return node()->removeGraySquaresFromAllGridChildren(); }
  bool addGraySquaresToAllGridAncestors() { return node()->addGraySquaresToAllGridAncestors(); }
  Layout layoutToPointWhenInserting(Expression * correspondingExpression, bool * forceCursorLeftOfText = nullptr) {
    // Pointer to correspondingExpr because expression.h includes layout.h
    assert(correspondingExpression != nullptr);
    return Layout(node()->layoutToPointWhenInserting(correspondingExpression, forceCursorLeftOfText));
  }
  void setMargin(bool hasMargin) { node()->setMargin(hasMargin); }
  void lockMargin(bool lock) { node()->lockMargin(lock); }

  // Tree
  Layout childAtIndex(int i) const;
  Layout root() const {
    assert(!isUninitialized());
    return Layout(node()->root());
  }
  Layout parent() const {
    assert(!isUninitialized());
    return Layout(node()->parent());
  }

  // Replace strings with codepoints
  Layout makeEditable() { return node()->makeEditable(); }

  // Cursor move
  int indexOfNextChildToPointToAfterHorizontalCursorMove(OMG::HorizontalDirection direction, int currentIndex) const { return node()->indexOfNextChildToPointToAfterHorizontalCursorMove(direction, currentIndex); }

private:
  bool privateHasTopLevelComparisonSymbol(bool includingNotEqualSymbol) const;
};

template<typename T, typename U, int N, typename Parent>
class LayoutBuilder : public Parent {
public:
#ifndef PLATFORM_DEVICE
  static_assert(std::is_base_of<Layout, Parent>::value);
#endif
  static T Builder() {
    static_assert(N == 0);
    TreeHandle h = TreeHandle::BuilderWithChildren(Initializer<U>, sizeof(U), {});
    return static_cast<T&>(h);
  }
  static T Builder(Layout child) {
    static_assert(N == 1);
    TreeHandle h = TreeHandle::BuilderWithChildren(Initializer<U>, sizeof(U), {child});
    return static_cast<T&>(h);
  }
  static T Builder(Layout child1, Layout child2) {
    static_assert(N == 2);
    TreeHandle h = TreeHandle::BuilderWithChildren(Initializer<U>, sizeof(U), {child1, child2});
    return static_cast<T&>(h);
  }
  static T Builder(Layout child1, Layout child2, Layout child3) {
    static_assert(N == 3);
    TreeHandle h = TreeHandle::BuilderWithChildren(Initializer<U>, sizeof(U), {child1, child2, child3});
    return static_cast<T&>(h);
  }
  static T Builder(Layout child1, Layout child2, Layout child3, Layout child4) {
    static_assert(N == 4);
    TreeHandle h = TreeHandle::BuilderWithChildren(Initializer<U>, sizeof(U), {child1, child2, child3, child4});
    return static_cast<T&>(h);
  }
};

template<typename T, typename U, typename P = Layout> using LayoutNoChildren = LayoutBuilder<T,U,0,P>;
template<typename T, typename U, typename P = Layout> using LayoutOneChild = LayoutBuilder<T,U,1,P>;
template<typename T, typename U, typename P = Layout> using LayoutTwoChildren = LayoutBuilder<T,U,2,P>;
template<typename T, typename U, typename P = Layout> using LayoutThreeChildren = LayoutBuilder<T,U,3,P>;
template <typename T, typename U, typename P = Layout>
using LayoutFourChildren = LayoutBuilder<T, U, 4, P>;

}

#endif
