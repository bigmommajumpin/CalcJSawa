#include <poincare/layout_helper.h>
#include <poincare/code_point_layout.h>
#include <poincare/combined_code_points_layout.h>
#include <poincare/horizontal_layout.h>
#include <poincare/left_parenthesis_layout.h>
#include <poincare/right_parenthesis_layout.h>
#include <poincare/vertical_offset_layout.h>
#include <ion/unicode/utf8_decoder.h>
#include <assert.h>
#include <utility>

namespace Poincare {

Layout LayoutHelper::Infix(const Expression & expression, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits, const char * operatorName) {
  HorizontalLayout result = HorizontalLayout::Builder();
  const size_t operatorLength = strlen(operatorName);
  const int numberOfChildren = expression.numberOfChildren();
  assert(numberOfChildren > 1);
  for (int i = 0; i < numberOfChildren; i++) {
    Layout childLayout = expression.childAtIndex(i).createLayout(floatDisplayMode, numberOfSignificantDigits, false, true);

    if (i > 0) {
      /* Handle the operator */
      if (operatorLength > 0) {
        Layout operatorLayout = String(operatorName, operatorLength);
        assert(operatorLayout.type() == LayoutNode::Type::CodePointLayout);
        CodePointLayoutNode * codePointNode = static_cast<CodePointLayoutNode *>(operatorLayout.node());
        codePointNode->setDisplayType(CodePointLayoutNode::DisplayType::Operator);
        result.addOrMergeChildAtIndex(operatorLayout, result.numberOfChildren(), true);
      } else {
        CodePointLayoutNode * implicitFactorNode = nullptr;
        if (childLayout.type() == LayoutNode::Type::CodePointLayout) {
          implicitFactorNode = static_cast<CodePointLayoutNode *>(childLayout.node());
        } else if (childLayout.type() == LayoutNode::Type::HorizontalLayout && childLayout.childAtIndex(0).type() == LayoutNode::Type::CodePointLayout) {
          implicitFactorNode = static_cast<CodePointLayoutNode *>(childLayout.childAtIndex(0).node());
        }
        if (implicitFactorNode) {
          implicitFactorNode->setDisplayType(CodePointLayoutNode::DisplayType::Implicit);
        }
      }
    }

    result.addOrMergeChildAtIndex(childLayout, result.numberOfChildren(), true);
  }
  return std::move(result);
}

Layout LayoutHelper::Prefix(const Expression & expression, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits, const char * operatorName) {
  HorizontalLayout result = HorizontalLayout::Builder();
  // Add the operator name.
  result.addOrMergeChildAtIndex(String(operatorName, strlen(operatorName)), 0, true);

  // Create the layout of arguments separated by commas.
  HorizontalLayout args = HorizontalLayout::Builder();
  const int numberOfChildren = expression.numberOfChildren();
  for (int i = 0; i < numberOfChildren; i++) {
    if (i > 0) {
      args.addChildAtIndex(CodePointLayout::Builder(','), args.numberOfChildren(), args.numberOfChildren(), nullptr);
    }
    args.addOrMergeChildAtIndex(expression.childAtIndex(i).createLayout(floatDisplayMode, numberOfSignificantDigits, false, true), args.numberOfChildren(), true);
  }
  // Add the parenthesed arguments.
  result.addOrMergeChildAtIndex(Parentheses(args, false), result.numberOfChildren(), true);
  return std::move(result);
}

Layout LayoutHelper::Parentheses(Layout layout, bool cloneLayout) {
  HorizontalLayout result = HorizontalLayout::Builder();
  result.addChildAtIndex(LeftParenthesisLayout::Builder(), 0, 0, nullptr);
  if (!layout.isUninitialized()) {
    result.addOrMergeChildAtIndex(cloneLayout ? layout.clone() : layout, 1, true);
  }
  result.addChildAtIndex(RightParenthesisLayout::Builder(), result.numberOfChildren(), result.numberOfChildren(), nullptr);
  return std::move(result);
}

Layout LayoutHelper::String(const char * buffer, int bufferLen, const KDFont * font) {
  if (bufferLen < 0) {
    bufferLen = strlen(buffer);
  }
  assert(bufferLen > 0);
  HorizontalLayout resultLayout = HorizontalLayout::Builder();
  UTF8Decoder decoder(buffer);
  const char * currentPointer = buffer;
  CodePoint codePoint = decoder.nextCodePoint();
  const char * nextPointer = decoder.stringPosition();
  assert(!codePoint.isCombining());
  int layoutIndex = 0;
  int bufferIndex = 0;
  while (codePoint != UCodePointNull && bufferIndex < bufferLen) {
    CodePoint nextCodePoint = decoder.nextCodePoint();
    Layout nextChild;
    if (nextCodePoint.isCombining()) {
      nextChild = CombinedCodePointsLayout::Builder(codePoint, nextCodePoint, font);
      nextPointer = decoder.stringPosition();
      nextCodePoint = decoder.nextCodePoint();
    } else {
      nextChild = CodePointLayout::Builder(codePoint, font);
    }
    resultLayout.addChildAtIndex(nextChild, layoutIndex, layoutIndex, nullptr);
    layoutIndex++;
    bufferIndex += nextPointer - currentPointer;
    currentPointer = nextPointer;
    codePoint = nextCodePoint;
    nextPointer = decoder.stringPosition();
    assert(!codePoint.isCombining());
  }
  return resultLayout.squashUnaryHierarchyInPlace();
}

Layout LayoutHelper::StringLayoutOfSerialization(const Expression & expression, char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) {
  int length = expression.serialize(buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits);
  assert(length < bufferSize);
  return LayoutHelper::String(buffer, length);
}

Layout LayoutHelper::CodePointString(const CodePoint * buffer, int bufferLen, const KDFont * font) {
  assert(bufferLen > 0);
  HorizontalLayout resultLayout = HorizontalLayout::Builder();
  for (int i = 0; i < bufferLen; i++) {
    assert(!buffer[i].isCombining());
    // TODO support combining code point?
    resultLayout.addChildAtIndex(CodePointLayout::Builder(buffer[i], font), i, i, nullptr);
  }
  return resultLayout.squashUnaryHierarchyInPlace();
}

Layout LayoutHelper::Logarithm(Layout argument, Layout index) {
  Layout logLayout = String("log", 3);
  HorizontalLayout resultLayout = static_cast<HorizontalLayout &>(logLayout);
  VerticalOffsetLayout offsetLayout = VerticalOffsetLayout::Builder(index, VerticalOffsetLayoutNode::Position::Subscript);
  resultLayout.addChildAtIndex(offsetLayout, resultLayout.numberOfChildren(), resultLayout.numberOfChildren(), nullptr);
  resultLayout.addOrMergeChildAtIndex(Parentheses(argument, false), resultLayout.numberOfChildren(), true);
  return std::move(resultLayout);
}

HorizontalLayout LayoutHelper::CodePointSubscriptCodePointLayout(CodePoint base, CodePoint subscript) {
  return HorizontalLayout::Builder(
      CodePointLayout::Builder(base),
      VerticalOffsetLayout::Builder(CodePointLayout::Builder(subscript, KDFont::LargeFont), VerticalOffsetLayoutNode::Position::Subscript));
}

}
