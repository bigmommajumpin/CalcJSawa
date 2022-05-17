#include <poincare/list_mean.h>
#include <poincare/statistics_dataset.h>
#include <poincare/layout_helper.h>
#include <poincare/list_complex.h>
#include <poincare/list_sum.h>
#include <poincare/multiplication.h>
#include <poincare/power.h>
#include <poincare/rational.h>
#include <poincare/serialization_helper.h>

namespace Poincare {

template<int U>
int ListMeanNode<U>::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, ListMean::k_functionName);
}

template<int U>
Layout ListMeanNode<U>::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(ListMean(this), floatDisplayMode, numberOfSignificantDigits, ListMean::k_functionName);
}

template<int U>
Expression ListMeanNode<U>::shallowReduce(ReductionContext reductionContext) {
  return ListMean(this).shallowReduce(reductionContext);
}

template<int U>
template<typename T> Evaluation<T> ListMeanNode<U>::templatedApproximate(ApproximationContext approximationContext) const {
  ListComplex<T> evaluationArray[2];
  StatisticsDataset<T> dataset = StatisticsDataset<T>::BuildFromChildren(this, approximationContext, evaluationArray);
  if (dataset.isUndefined()) {
    return Complex<T>::Undefined();
  }
  return Complex<T>::Builder(dataset.mean());
}

Expression ListMean::shallowReduce(ExpressionNode::ReductionContext reductionContext) {
  int n = numberOfChildren();
  assert(n <= 2);
  Expression children[2];
  for (int i = 0; i < n; i++) {
    children[i] = childAtIndex(i);
    if (children[i].type() != ExpressionNode::Type::List || children[i].numberOfChildren() == 0) {
      return replaceWithUndefinedInPlace();
    }
  }
  List weights = static_cast<List &>(children[1]);
  if (n > 1 && !weights.allChildrenArePositive(reductionContext.context())) {
    // All weights need to be positive.
    return *this;
  }
  int numberOfElementsInList = children[0].numberOfChildren();
  Expression listToSum;
  if (n == 1) {
    listToSum = children[0];
  } else {
    assert(n == 2);
    listToSum = Multiplication::Builder(children[0], children[1].clone());
  }
  ListSum sum = ListSum::Builder(listToSum);
  listToSum.shallowReduce(reductionContext);
  Expression inverseOfTotalWeights;
  if (n == 1) {
    inverseOfTotalWeights = Rational::Builder(1, numberOfElementsInList);
  } else {
    ListSum sumOfWeights = ListSum::Builder(children[1]);
    inverseOfTotalWeights = Power::Builder(sumOfWeights, Rational::Builder(-1, 1));
    sumOfWeights.shallowReduce(reductionContext);
  }
  Multiplication result = Multiplication::Builder(sum, inverseOfTotalWeights);
  sum.shallowReduce(reductionContext);
  inverseOfTotalWeights.shallowReduce(reductionContext);
  replaceWithInPlace(result);
  return result.shallowReduce(reductionContext);
}

template Evaluation<float> ListMeanNode<1>::templatedApproximate<float>(ApproximationContext approximationContext) const;
template Evaluation<float> ListMeanNode<2>::templatedApproximate<float>(ApproximationContext approximationContext) const;
template Evaluation<double> ListMeanNode<1>::templatedApproximate<double>(ApproximationContext approximationContext) const;
template Evaluation<double> ListMeanNode<2>::templatedApproximate<double>(ApproximationContext approximationContext) const;

template class ListMeanNode<1>;
template class ListMeanNode<2>;
}
