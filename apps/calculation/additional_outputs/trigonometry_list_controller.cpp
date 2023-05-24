#include "trigonometry_list_controller.h"

#include <apps/shared/expression_display_permissions.h>
#include <apps/shared/poincare_helpers.h>
#include <poincare/layout_helper.h>
#include <poincare/trigonometry.h>
#include <poincare_expressions.h>

#include "../app.h"

using namespace Poincare;

namespace Calculation {

void TrigonometryListController::setExactAndApproximateExpression(
    Poincare::Expression exactExpression,
    Poincare::Expression approximateExpression) {
  IllustratedExpressionsListController::setExactAndApproximateExpression(
      exactExpression, approximateExpression);

  Preferences* preferences = Preferences::sharedPreferences;
  Preferences::AngleUnit userAngleUnit = preferences->angleUnit();
  Context* context = App::app()->localContext();
  size_t index = 0;
  Expression e = exactExpression;

  Expression unit;
  Shared::PoincareHelpers::CloneAndReduceAndRemoveUnit(
      &e, context, ReductionTarget::User, &unit);

  if (!unit.isUninitialized()) {
    assert(unit.isPureAngleUnit() &&
           static_cast<Unit&>(unit).representative() ==
               Unit::k_angleRepresentatives +
                   Unit::k_radianRepresentativeIndex);
    /* After a reduction, all angle units are converted to radians, so we
     * convert e again here to fit the angle unit that will be used in
     * reductions below. */
    e = Multiplication::Builder(
        e, Trigonometry::UnitConversionFactor(Preferences::AngleUnit::Radian,
                                              userAngleUnit));
  }

  Expression period = Multiplication::Builder(
      Rational::Builder(2),
      Trigonometry::PiExpressionInAngleUnit(userAngleUnit));
  // Use the reduction of frac part to compute mod 1 on rationals
  Expression simplifiedAngle = Multiplication::Builder(
      FracPart::Builder(Division::Builder(e, period.clone())), period.clone());
  Shared::PoincareHelpers::CloneAndSimplify(&simplifiedAngle, context,
                                            ReductionTarget::User);
  /* Approximate the angle if the fractional part could not be reduced (because
   * the angle is not a multiple of pi), or if displaying the exact expression
   * is forbidden. */
  if (simplifiedAngle.recursivelyMatches(
          [](const Expression e, Context* context) {
            return e.type() == ExpressionNode::Type::FracPart;
          }) ||
      Shared::ExpressionDisplayPermissions::ShouldNeverDisplayExactOutput(
          simplifiedAngle, context)) {
    Expression angleApproximate = approximateExpression;
    if (angleApproximate.isUninitialized()) {
      /* In case of direct trigonometry, the approximate expression of the angle
       * is not yet computed, so it needs to be computed here.
       * Do not approximate the FracPart, which could lead to truncation error
       * for large angles (e.g. frac(1e17/2pi) = 0). Instead find the angle with
       * the same sine and cosine. */
      angleApproximate = ArcCosine::Builder(Cosine::Builder(e));
      angleApproximate = Shared::PoincareHelpers::Approximate<double>(
          angleApproximate, context, preferences);
      /* acos has its values in [0,π[, use the sign of the sine to find the
       * right semicircle. */
      if (Shared::PoincareHelpers::ApproximateToScalar<double>(
              Sine::Builder(e), context, preferences) < 0) {
        angleApproximate = Shared::PoincareHelpers::Approximate<double>(
            Subtraction::Builder(period.clone(), angleApproximate), context,
            preferences);
      }
    } else {
      angleApproximate = Shared::PoincareHelpers::Approximate<double>(
          angleApproximate, context, preferences);
      // Set the angle in [0, 2π] if it was in [-π, π]
      if (angleApproximate.isPositive(context) == TrinaryBoolean::False) {
        angleApproximate = Shared::PoincareHelpers::Approximate<double>(
            Addition::Builder(period.clone(), angleApproximate), context,
            preferences);
      }
    }
    e = angleApproximate;
    m_isStrictlyEqual[index] = false;
  } else {
    e = simplifiedAngle;
    m_isStrictlyEqual[index] = true;
  }

  m_layouts[index] = LayoutHelper::String("θ");

  Expression withAngleUnit = Multiplication::Builder(
      e.clone(),
      Unit::Builder(
          UnitNode::AngleRepresentative::DefaultRepresentativeForAngleUnit(
              userAngleUnit)));

  Expression radians = Unit::Builder(Unit::k_angleRepresentatives +
                                     Unit::k_radianRepresentativeIndex);
  m_exactLayouts[index] = getLayoutFromExpression(
      UnitConvert::Builder(withAngleUnit.clone(), radians), context,
      preferences);

  Expression degrees = Unit::Builder(Unit::k_angleRepresentatives +
                                     Unit::k_degreeRepresentativeIndex);
  m_approximatedLayouts[index] = getLayoutFromExpression(
      UnitConvert::Builder(withAngleUnit.clone(), degrees), context,
      preferences);

  Expression theta = Symbol::Builder(k_symbol);
  setLineAtIndex(++index, Cosine::Builder(theta), Cosine::Builder(e.clone()),
                 context, preferences);
  updateIsStrictlyEqualAtIndex(index, context);
  setLineAtIndex(++index, Sine::Builder(theta), Sine::Builder(e.clone()),
                 context, preferences);
  updateIsStrictlyEqualAtIndex(index, context);
  setLineAtIndex(++index, Tangent::Builder(theta), Tangent::Builder(e.clone()),
                 context, preferences);
  updateIsStrictlyEqualAtIndex(index, context);

  // Set illustration
  float angle = Shared::PoincareHelpers::ApproximateToScalar<float>(e, context);
  // Convert angle to radians
  angle = angle * M_PI / Trigonometry::PiInAngleUnit(userAngleUnit);
  assert(std::isfinite(angle));
  m_model.setAngle(angle);
  setShowIllustration(true);
}

void TrigonometryListController::updateIsStrictlyEqualAtIndex(
    int index, Context* context) {
  if (m_approximatedLayouts[index].isUninitialized()) {
    /* Only one layout is displayed, so there is no equal sign. */
    return;
  }
  char approximateBuffer[::Constant::MaxSerializedExpressionSize];
  m_approximatedLayouts[index].serializeForParsing(
      approximateBuffer, ::Constant::MaxSerializedExpressionSize);
  if (strcmp(approximateBuffer, Undefined::Name()) == 0) {
    // Hide exact result if approximation is undef (e.g tan(1.5707963267949))
    m_exactLayouts[index] = Layout();
    return;
  }
  char exactBuffer[::Constant::MaxSerializedExpressionSize];
  m_exactLayouts[index].serializeForParsing(
      exactBuffer, ::Constant::MaxSerializedExpressionSize);
  assert(strcmp(exactBuffer, approximateBuffer) != 0);
  m_isStrictlyEqual[index] = Expression::ExactAndApproximateExpressionsAreEqual(
      Expression::Parse(exactBuffer, context),
      Expression::Parse(approximateBuffer, context));
}

void TrigonometryListController::willDisplayCellForIndex(
    Escher::HighlightCell* cell, int index) {
  if (typeAtIndex(index) == k_expressionCellType) {
    int expressionIndex = index - showIllustration();
    assert(0 <= expressionIndex && expressionIndex < k_numberOfExpressionRows);
    static_cast<AdditionnalResultCell*>(cell)
        ->label()
        ->setExactAndApproximateAreStriclyEqual(
            m_isStrictlyEqual[expressionIndex]);
  }
  return IllustratedExpressionsListController::willDisplayCellForIndex(cell,
                                                                       index);
}

KDCoordinate TrigonometryListController::nonMemoizedRowHeight(int j) {
  if (typeAtIndex(j) == k_illustrationCellType) {
    return k_illustrationHeight;
  }
  return IllustratedExpressionsListController::nonMemoizedRowHeight(j);
}

I18n::Message TrigonometryListController::messageAtIndex(int index) {
  if (index == 0) {
    return I18n::Message::AngleInZeroTwoPi;
  }
  return I18n::Message::Default;
}

}  // namespace Calculation
