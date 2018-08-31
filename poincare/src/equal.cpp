#include <poincare/equal.h>
#include <poincare/allocation_failure_expression_node.h>
#include <poincare/rational.h>
#include <poincare/addition.h>
#include <poincare/division.h>
#include <poincare/multiplication.h>
#include <poincare/opposite.h>
#include <poincare/power.h>
#include <poincare/square_root.h>
#include <poincare/subtraction.h>
#include <poincare/symbol.h>
#include <poincare/char_layout_node.h>
#include <poincare/horizontal_layout_node.h>
#include <ion.h>
extern "C" {
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
}
namespace Poincare {

EqualNode * EqualNode::FailedAllocationStaticNode() {
  static AllocationFailureExpressionNode<EqualNode> failure;
  TreePool::sharedPool()->registerStaticNodeIfRequired(&failure);
  return &failure;
}

Expression EqualNode::standardEquation(Context & context, Preferences::AngleUnit angleUnit) const {
  Expression sub = Subtraction(Expression(childAtIndex(0)).clone(), Expression(childAtIndex(1)).clone());
  return sub.deepReduce(context, angleUnit);
}

Expression EqualNode::shallowReduce(Context & context, Preferences::AngleUnit angleUnit) {
  return Equal(this).shallowReduce(context, angleUnit);
}

LayoutRef EqualNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  HorizontalLayoutRef result;
  result.addOrMergeChildAtIndex(childAtIndex(0)->createLayout(floatDisplayMode, numberOfSignificantDigits), 0, false);
  result.addChildAtIndex(CharLayoutRef('='), result.numberOfChildren(), result.numberOfChildren(), nullptr);
  result.addOrMergeChildAtIndex(childAtIndex(1)->createLayout(floatDisplayMode, numberOfSignificantDigits), result.numberOfChildren(), false);
  return result;
}

int EqualNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Infix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, "=");
}

template<typename T>
Evaluation<T> EqualNode::templatedApproximate(Context& context, Preferences::AngleUnit angleUnit) const {
  return Complex<T>::Undefined();
}

Expression Equal::shallowReduce(Context & context, Preferences::AngleUnit angleUnit) {
  {
    Expression e = Expression::defaultShallowReduce(context, angleUnit);
    if (e.isUndefinedOrAllocationFailure()) {
      return e;
    }
  }
  if (childAtIndex(0).isIdenticalTo(childAtIndex(1))) {
    Expression result = Rational(1);
    replaceWithInPlace(result);
    return result;
  }
  return *this;
}

}
