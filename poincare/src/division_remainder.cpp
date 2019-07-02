#include <poincare/division_remainder.h>
#include <poincare/infinity.h>
#include <poincare/layout_helper.h>
#include <poincare/rational.h>
#include <poincare/serialization_helper.h>
#include <poincare/undefined.h>
#include <cmath>

namespace Poincare {

constexpr Expression::FunctionHelper DivisionRemainder::s_functionHelper;

int DivisionRemainderNode::numberOfChildren() const { return DivisionRemainder::s_functionHelper.numberOfChildren(); }

Layout DivisionRemainderNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(DivisionRemainder(this), floatDisplayMode, numberOfSignificantDigits, DivisionRemainder::s_functionHelper.name());
}

int DivisionRemainderNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, DivisionRemainder::s_functionHelper.name());
}

Expression DivisionRemainderNode::shallowReduce(ReductionContext reductionContext) {
  return DivisionRemainder(this).shallowReduce();
}

template<typename T>
Evaluation<T> DivisionRemainderNode::templatedApproximate(Context * context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) const {
  Evaluation<T> f1Input = childAtIndex(0)->approximate(T(), context, complexFormat, angleUnit);
  Evaluation<T> f2Input = childAtIndex(1)->approximate(T(), context, complexFormat, angleUnit);
  T f1 = f1Input.toScalar();
  T f2 = f2Input.toScalar();
  if (std::isnan(f1) || std::isnan(f2) || f1 != (int)f1 || f2 != (int)f2) {
    return Complex<T>::Undefined();
  }
  return Complex<T>::Builder(std::round(f1-f2*std::floor(f1/f2)));
}


Expression DivisionRemainder::shallowReduce() {
  {
    Expression e = Expression::defaultShallowReduce();
    if (e.isUndefined()) {
      return e;
    }
  }
  Expression c0 = childAtIndex(0);
  Expression c1 = childAtIndex(1);
  if (c0.type() == ExpressionNode::Type::Matrix || c1.type() == ExpressionNode::Type::Matrix) {
    Expression result = Undefined::Builder();
    replaceWithInPlace(result);
    return result;
  }
  if (c0.type() == ExpressionNode::Type::Rational) {
    Rational r0 = static_cast<Rational &>(c0);
    if (!r0.integerDenominator().isOne()) {
      Expression result = Undefined::Builder();
      replaceWithInPlace(result);
      return result;
    }
  }
  if (c1.type() == ExpressionNode::Type::Rational) {
    Rational r1 = static_cast<Rational &>(c1);
    if (!r1.integerDenominator().isOne()) {
      Expression result = Undefined::Builder();
      replaceWithInPlace(result);
      return result;
    }
  }
  if (c0.type() != ExpressionNode::Type::Rational || c1.type() != ExpressionNode::Type::Rational) {
    return *this;
  }
  Rational r0 = static_cast<Rational &>(c0);
  Rational r1 = static_cast<Rational &>(c1);

  Integer a = r0.signedIntegerNumerator();
  Integer b = r1.signedIntegerNumerator();
  if (b.isZero()) {
    Expression result = Infinity::Builder(a.isNegative());
    replaceWithInPlace(result);
    return result;
  }
  Integer result = Integer::Division(a, b).remainder;
  assert(!result.isOverflow());
  Expression rationalResult = Rational::Builder(result);
  replaceWithInPlace(rationalResult);
  return rationalResult;
}

}
