#include "median_model.h"
#include "../store.h"
#include <poincare/layout_helper.h>
#include <math.h>
#include <assert.h>

using namespace Poincare;

namespace Regression {

Layout MedianModel::layout() {
  if (m_layout.isUninitialized()) {
    static constexpr const char * s = "a·X+b";
    m_layout = LayoutHelper::String(s, strlen(s), k_layoutFont);
  }
  return m_layout;
}

double MedianModel::evaluate(double * modelCoefficients, double x) const {
  double a = modelCoefficients[0];
  double b = modelCoefficients[1];
  return a*x+b;
}

double MedianModel::levelSet(double * modelCoefficients, double xMin, double xMax, double y, Poincare::Context * context) {
  double a = modelCoefficients[0];
  double b = modelCoefficients[1];
  if (a == 0) {
    return NAN;
  }
  return (y-b)/a;
}

double MedianModel::partialDerivate(double * modelCoefficients, int derivateCoefficientIndex, double x) const {
  if (derivateCoefficientIndex == 0) {
    // Derivate with respect to a: x
    return x;
  }
  assert(derivateCoefficientIndex == 1);
  // Derivate with respect to b: 1
  return 1.0;
}

void MedianModel::fit(Store * store, int series, double * modelCoefficients, Poincare::Context * context) {
  int numberOfDots = store->numberOfPairsOfSeries(series);
  if (numberOfDots < 3) {
    modelCoefficients[0] = NAN;
    modelCoefficients[1] = NAN;
    return;
  }

  int sortedIndex[Store::k_maxNumberOfPairs];
  for (int i = 0; i < numberOfDots; i++) {
    sortedIndex[i] = i;
  }
  store->sortIndexByColumn(sortedIndex, series, 0, 0, numberOfDots);

  int sizeOfMiddleGroup = numberOfDots / 3 + (numberOfDots % 3 == 1 ? 1 : 0);
  int sizeOfRightLeftGroup = numberOfDots / 3 + (numberOfDots % 3 == 2 ? 1 : 0);

  double leftPoint[2];
  double middlePoint[2];
  double rightPoint[2];

  leftPoint[0] = getMedianValue(store, sortedIndex, series, 0, 0, sizeOfRightLeftGroup);
  middlePoint[0] = getMedianValue(store, sortedIndex, series, 0, sizeOfRightLeftGroup, sizeOfRightLeftGroup + sizeOfMiddleGroup);
  rightPoint[0] = getMedianValue(store, sortedIndex, series, 0, sizeOfRightLeftGroup + sizeOfMiddleGroup, numberOfDots);

  if (rightPoint[0] == leftPoint[0]) {
    modelCoefficients[0] = NAN;
    modelCoefficients[1] = NAN;
    return;
  }

  store->sortIndexByColumn(sortedIndex, series, 1, 0, sizeOfRightLeftGroup);
  store->sortIndexByColumn(sortedIndex, series, 1, sizeOfRightLeftGroup, sizeOfRightLeftGroup + sizeOfMiddleGroup);
  store->sortIndexByColumn(sortedIndex, series, 1, sizeOfRightLeftGroup + sizeOfMiddleGroup, numberOfDots);

  leftPoint[1] = getMedianValue(store, sortedIndex, series, 1, 0, sizeOfRightLeftGroup);
  middlePoint[1] = getMedianValue(store, sortedIndex, series, 1, sizeOfRightLeftGroup, sizeOfRightLeftGroup + sizeOfMiddleGroup);
  rightPoint[1] = getMedianValue(store, sortedIndex, series, 1, sizeOfRightLeftGroup + sizeOfMiddleGroup, numberOfDots);

  double a = (rightPoint[1] - leftPoint[1]) / (rightPoint[0] - leftPoint[0]);
  modelCoefficients[0] = a;
  modelCoefficients[1] = ((leftPoint[1] - a * leftPoint[0]) + (middlePoint[1] - a * middlePoint[0]) + (rightPoint[1] - a * rightPoint[0])) / 3;
}

double MedianModel::getMedianValue(Store * store, int * sortedIndex, int series, int column, int startIndex, int endIndex) {
  assert(endIndex != startIndex);
  if ((endIndex - startIndex) % 2 == 1) {
    return store->get(series, column, sortedIndex[startIndex + (endIndex - startIndex) / 2]);
  } else {
    return (store->get(series, column, sortedIndex[startIndex + (endIndex - startIndex) / 2]) + (store->get(series, column, sortedIndex[startIndex + (endIndex - startIndex) / 2 - 1])))/ 2;
  }
}

}
