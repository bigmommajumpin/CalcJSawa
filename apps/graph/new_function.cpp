#include "new_function.h"
#include <escher/palette.h>
#include <poincare/subtraction.h>
#include <poincare/symbol.h>
#include <poincare/undefined.h>
#include <poincare/polynomial.h>
#include <poincare/zoom.h>
#include <poincare/integral.h>
#include <poincare/float.h>
#include <poincare/serialization_helper.h>
#include "../shared/poincare_helpers.h"
#include <algorithm>

using namespace Poincare;

namespace Graph {

NewFunction NewFunction::NewModel(Ion::Storage::Record::ErrorStatus * error, const char * baseName) {
  static int s_colorIndex = 0;
  // Create the record
  RecordDataBuffer data(Escher::Palette::nextDataColor(&s_colorIndex));
  if (baseName == nullptr) {
    // Return if error
    return NewFunction();
  }

  *error = Ion::Storage::sharedStorage()->createRecordWithExtension(baseName, Ion::Storage::funcExtension, &data, sizeof(data));
  if (*error != Ion::Storage::Record::ErrorStatus::None) {
    assert(false); // TOCHECK Hugo
    // Return if error
    return NewFunction();
  }

  // Return the NewFunction with the new record
  return NewFunction(Ion::Storage::sharedStorage()->recordBaseNamedWithExtension(baseName, Ion::Storage::funcExtension));
}

bool NewFunction::isNamed() const {
  return fullName() != nullptr;
}

bool NewFunction::drawAbove() const {
  EquationSymbol eqSymbol = equationSymbol();
  return eqSymbol == EquationSymbol::Greater || eqSymbol == EquationSymbol::GreaterOrEqual || eqSymbol == EquationSymbol::Inequal;
}

bool NewFunction::drawBelow() const {
  EquationSymbol eqSymbol = equationSymbol();
  return eqSymbol == EquationSymbol::Less || eqSymbol == EquationSymbol::LessOrEqual || eqSymbol == EquationSymbol::Inequal;
}

bool NewFunction::drawCurve() const {
  EquationSymbol eqSymbol = equationSymbol();
  return eqSymbol == EquationSymbol::GreaterOrEqual || eqSymbol == EquationSymbol::LessOrEqual || eqSymbol == EquationSymbol::Equal;
}

int NewFunction::yDegree(Context * context) const {
  return expressionEquation(context).polynomialDegree(context, "y");
}

int NewFunction::xDegree(Context * context) const {
  return expressionEquation(context).polynomialDegree(context, "x");
}

I18n::Message NewFunction::functionCategory() const {
  assert(static_cast<size_t>(plotType()) < k_numberOfPlotTypes);
  static const I18n::Message category[k_numberOfPlotTypes] = {
    I18n::Message::CartesianType, I18n::Message::PolarType, I18n::Message::ParametricType, I18n::Message::ParametricType, I18n::Message::ParametricType,
    I18n::Message::CartesianType, I18n::Message::PolarType, I18n::Message::ParametricType, I18n::Message::ParametricType, I18n::Message::ParametricType,
    I18n::Message::CartesianType, I18n::Message::PolarType, I18n::Message::ParametricType, I18n::Message::ParametricType, I18n::Message::ParametricType,
    I18n::Message::CartesianType, I18n::Message::PolarType, I18n::Message::ParametricType, I18n::Message::ParametricType, I18n::Message::ParametricType,
  };
  return category[static_cast<size_t>(plotType())];
}

CodePoint NewFunction::symbol() const {
  // TODO Hugo : Rotate available symbols
  switch (plotType()) {
  case PlotType::Parametric:
    return 't';
  case PlotType::Polar:
    return UCodePointGreekSmallLetterTheta;
  default:
    return 'x';
  }
}

Expression NewFunction::Model::expressionEquation(const Ion::Storage::Record * record, Context * context) const {
  // TODO Hugo : Add todo expressionReduced on circularity ?
  if (record->fullName() != nullptr && isCircularlyDefined(record, context)) {
    return Undefined::Builder();
  }
  Expression result = Expression::ExpressionFromAddress(expressionAddress(record), expressionSize(record));
  // TODO Hugo : Handle function assignement from outside
  // TODO Hugo : Handle other than equal
  assert(result.type() == ExpressionNode::Type::Equal);
  if (result.childAtIndex(0).type() == ExpressionNode::Type::Function && result.childAtIndex(0).childAtIndex(0).isIdenticalTo(Symbol::Builder(UCodePointUnknown))) {
    // Named function
    result = result.childAtIndex(1);
    // TOCHECK Hugo
    assert(isNamed());
  } else {
    result = Subtraction::Builder(result.childAtIndex(0), result.childAtIndex(1));
    // ExpressionNode::ReductionContext reductionContext = ExpressionNode::ReductionContext(
    //   context, Preferences::sharedPreferences()->complexFormat(),
    //   Preferences::sharedPreferences()->angleUnit(),
    //   GlobalPreferences::sharedGlobalPreferences()->unitFormat(),
    //   ExpressionNode::ReductionTarget::SystemForAnalysis,
    //   ExpressionNode::SymbolicComputation::DoNotReplaceAnySymbol);
    // result = result.reduce(reductionContext);
  }
  /* 'Simplify' routine might need to call expressionReduced on the very
    * same function. So we need to keep a valid result while executing
    * 'Simplify'. Thus, we use a temporary expression. */
  Expression tempExpression = result.clone();
  Shared::PoincareHelpers::Simplify(&tempExpression, context, ExpressionNode::ReductionTarget::SystemForApproximation);
  // simplify might return an uninitialized Expression if interrupted
  if (!tempExpression.isUninitialized()) {
    result = tempExpression;
  }
  // TODO Hugo : Memoize it ?
  return result;
}

Expression NewFunction::Model::expressionReduced(const Ion::Storage::Record * record, Context * context) const {
  // TODO Hugo : Fix this
  // Get expression degree on y
  if (m_expression.isUninitialized()) {
    // Retrieve the expression from the equation
    m_expression = expressionEquation(record, context);
    if (record->fullName() != nullptr) {
      // Transform the solution by solving the equation in y
      int degree = m_expression.polynomialDegree(context, "y");
      if (degree <= 0 || degree >= 3) {
        // TODO Hugo : handle Vertical line  here
        m_expression = Undefined::Builder();
      } else {
        Expression coefficients[Expression::k_maxNumberOfPolynomialCoefficients];
        int d = m_expression.getPolynomialReducedCoefficients("y", coefficients, context, Preferences::ComplexFormat::Cartesian, Preferences::sharedPreferences()->angleUnit(), Preferences::UnitFormat::Metric, ExpressionNode::SymbolicComputation::DoNotReplaceAnySymbol);
        assert(d == degree);
        if (d == 1) {
          Expression root;
          Polynomial::LinearPolynomialRoots(coefficients[1], coefficients[0], &root, context, Preferences::ComplexFormat::Real, Preferences::sharedPreferences()->angleUnit());
          m_expression = root;
        } else {
          Expression root1, root2, delta;
          int solutions = Polynomial::QuadraticPolynomialRoots(coefficients[2], coefficients[1], coefficients[0], &root1, &root2, &delta, context, Preferences::ComplexFormat::Real, Preferences::sharedPreferences()->angleUnit());
          if (solutions <= 1) {
            m_expression = root1;
          } else {
            // TODO HUgo : Plot two curves instead
            m_expression = root1;
            // Expression alternator = DivisionRemainder::Builder(Floor::Builder(Multiplication::Builder(Symbol::Builder(UCodePointUnknown), Rational::Builder(10))), Rational::Builder(2));
            // m_expression = Addition::Builder(Multiplication::Builder(alternator, Subtraction::Builder(root1, root2.clone())), root2);
          }
        }
      }
    }
  }
  return m_expression;
}

void NewFunction::updatePlotType(Preferences::AngleUnit angleUnit, Context * context) {
  // Compute plot type from expressions
  /* If of format f(...) = ... , handle this -> Cartesian, Polar, Parametric or Undefined
   * | y  | x  | Status
   * | 0  | 0  | Undefined
   * | 0  | 1  | Vertical Line
   * | 0  | 2  | Unhandled (Multiple vertical lines ? # TODO)
   * | 0  | +  | Unhandled (Multiple vertical lines ? # TODO)
   * | 1  | 0  | Horizontal Line
   * | 1  | 1  | Line
   * | 1  | 2  | Cartesian
   * | 1  | +  | Cartesian
   * | 2  | 0  | Unhandled (Multiple horizontal lines ? # TODO)
   * | 2  | 1  | Circle, Ellipsis, Hyperbola, Parabolla # TODO differentiate
   * | 2  | 2  | Circle, Ellipsis, Hyperbola, Parabolla # TODO differentiate
   * | 2  | +  | Other
   * | +  | 0  | Unhandled (Multiple horizontal lines ? # TODO)
   * | +  | 1  | Unhandled (Swap x and y ? # TODO)
   * | +  | 2  | Unhandled (Swap x and y ? # TODO)
   * | +  | +  | Unhandled
   */
  // TODO Hugo : proprify returns here
  if (isNamed()) {
    // TODO Hugo :retrieve symbol, -> Polar, Parametric or Cartsian
    return recordData()->setPlotType(PlotType::Cartesian);
  }
  int yDeg = yDegree(context);
  int xDeg = xDegree(context);
  if (yDeg == 0 && xDeg == 0) {
    return recordData()->setPlotType(PlotType::Undefined);
  }
  if (yDeg == 0 && xDeg == 1) {
    return recordData()->setPlotType(PlotType::VerticalLine);
  }
  if (yDeg == 0 && xDeg == 1) {
    return recordData()->setPlotType(PlotType::HorizontalLine);
  }
  if (yDeg == 1 && xDeg == 1) {
    return recordData()->setPlotType(PlotType::Line);
  }
  if (yDeg == 1) {
    return recordData()->setPlotType(PlotType::Cartesian);
  }
  if (yDeg == 2 && xDeg == 1 || xDeg == 2) {
    // TODO Hugo : Compute delta ...
    // + assert that no x^2 * y or y^2 * x !! -> Unhandled otherwise
    /* Ax2+Bxy+Cy2+Dx+Ey+F=0 with A,B,C not all zero
     * Discriminant B^2-4AC :
     *  - Negative, A=C and B=0 : Circle
     *  - Negative : Ellipse
     *  - Null : Parabola
     *  - Positive and A + C = 0 : Rectangular Hyperbola
     *  - Positive : Hyperbola
     */
    return recordData()->setPlotType(PlotType::Conics);
  }
  if (yDeg == 2) {
    return recordData()->setPlotType(PlotType::Other);
  }
  return recordData()->setPlotType(PlotType::Unhandled);
  /*
   * Eccentricity e formula :
   *  1 if parabola, sqrt(...) otherwise
   * Cercle :
   *  r : Rayon :
   *    sqrt(-F/A + (E^2+D^2)/((2A)^2))
   *  (x,y) : Coordonnées du centre :
   *     x = (2CD-BE)/(B^2-4AC)
   *     y = (2AE-BD)/(B^2-4AC)
   *
   * Ellipse :
   *  a : Demi grand axe
   *  b : Demi petit axe
   *  c : Distance centre-foyer
   *  e : excentircité
   *    sqrt(...)
   *  (x,y) : Coordonnées du centre :
   *     x = (2CD-BE)/(B^2-4AC)
   *     y = (2AE-BD)/(B^2-4AC)
   *
   * Hyperbole :
   *  a : Distance centre-sommet
   *  b : Demi axe transverse
   *  c : Distance centre-foyer
   *  e : excentircité
   *    sqrt(...)
   *  (x,y) : Coordonnées du centre :
   *     x = (2CD-BE)/(B^2-4AC)
   *     y = (2AE-BD)/(B^2-4AC)
   *
   * Parabole :
   *  p : Parametre
   *  (x,y) : Coordonnées du sommet :
   */
}

I18n::Message NewFunction::ParameterMessageForPlotType(PlotType plotType) {
  if (plotType == PlotType::Parametric) {
    return I18n::Message::T;
  }
  if (plotType == PlotType::Polar) {
    return I18n::Message::Theta;
  }
  return I18n::Message::X;
}

bool NewFunction::displayDerivative() const {
  // TODO Hugo : Re-implement derivative
  return false;
}

void NewFunction::setDisplayDerivative(bool display) {
  // TODO Hugo : Re-implement derivative
}

int NewFunction::derivativeNameWithArgument(char * buffer, size_t bufferSize) {
  // TODO Hugo : Re-implement derivative
  return 0;
}

double NewFunction::approximateDerivative(double x, Context * context) const {
  // TODO Hugo : Re-implement derivative
  return 0.0;
}

int NewFunction::printValue(double cursorT, double cursorX, double cursorY, char * buffer, int bufferSize, int precision, Context * context) {
  // TODO Hugo : Re-check
  PlotType type = plotType();
  if (type == PlotType::Parametric) {
    int result = 0;
    result += UTF8Decoder::CodePointToChars('(', buffer+result, bufferSize-result);
    result += Shared::PoincareHelpers::ConvertFloatToText<double>(cursorX, buffer+result, bufferSize-result, precision);
    result += UTF8Decoder::CodePointToChars(';', buffer+result, bufferSize-result);
    result += Shared::PoincareHelpers::ConvertFloatToText<double>(cursorY, buffer+result, bufferSize-result, precision);
    result += UTF8Decoder::CodePointToChars(')', buffer+result, bufferSize-result);
    return result;
  }
  if (type == PlotType::Polar) {
    return Shared::PoincareHelpers::ConvertFloatToText<double>(evaluate2DAtParameter(cursorT, context).x2(), buffer, bufferSize, precision);
  }
  return Shared::PoincareHelpers::ConvertFloatToText<double>(cursorY, buffer, bufferSize, precision);
}

bool NewFunction::shouldClipTRangeToXRange() const {
  // TODO Hugo : Re-check
  return plotType() != PlotType::Parametric && plotType() != PlotType::Polar;
}

void NewFunction::protectedFullRangeForDisplay(float tMin, float tMax, float tStep, float * min, float * max, Context * context, bool xRange) const {
  // TODO Hugo : Re-check
  Zoom::ValueAtAbscissa evaluation;
  if (xRange) {
    evaluation = [](float x, Context * context, const void * auxiliary) {
      return static_cast<const NewFunction *>(auxiliary)->evaluateXYAtParameter(x, context).x1();
    };
  } else {
    evaluation = [](float x, Context * context, const void * auxiliary) {
      return static_cast<const NewFunction *>(auxiliary)->evaluateXYAtParameter(x, context).x2();
    };
  }

  Zoom::FullRange(evaluation, tMin, tMax, tStep, min, max, context, this);
}

float NewFunction::tMin() const {
  // TODO Hugo : Re-check
  return recordData()->tMin();
}

float NewFunction::tMax() const {
  // TODO Hugo : Re-check
  return recordData()->tMax();
}

void NewFunction::setTMin(float tMin) {
  // TODO Hugo : Re-check
  recordData()->setTMin(tMin);
  // TODO Hugo : Re-check cache
  // setCache(nullptr);
}

void NewFunction::setTMax(float tMax) {
  // TODO Hugo : Re-check
  recordData()->setTMax(tMax);
  // TODO Hugo : Re-check cache
  // setCache(nullptr);
}

float NewFunction::rangeStep() const {
  // TODO Hugo : Re-check
  return !(plotType() == PlotType::Parametric || plotType() == PlotType::Polar) ? NAN : (tMax() - tMin())/k_polarParamRangeSearchNumberOfPoints;
}

bool NewFunction::basedOnCostlyAlgorithms(Context * context) const {
  // TODO Hugo : Re-implement
  return true;
}

void NewFunction::xRangeForDisplay(float xMinLimit, float xMaxLimit, float * xMin, float * xMax, float * yMinIntrinsic, float * yMaxIntrinsic, Context * context) const {
  // TODO Hugo : Re-check
  if (plotType() == PlotType::Parametric || plotType() == PlotType::Polar) {
    assert(std::isfinite(tMin()) && std::isfinite(tMax()) && std::isfinite(rangeStep()) && rangeStep() > 0);
    protectedFullRangeForDisplay(tMin(), tMax(), rangeStep(), xMin, xMax, context, true);
    *yMinIntrinsic = FLT_MAX;
    *yMaxIntrinsic = -FLT_MAX;
    return;
  }

  *xMin = NAN;
  *xMax = NAN;
  *yMinIntrinsic = NAN;
  *yMaxIntrinsic = NAN;

  if (basedOnCostlyAlgorithms(context)) {
    /* The function makes use of some costly algorithms, such as integration or
     * sequences, and cannot be computed in a timely manner. */
    return;
  }

  Zoom::ValueAtAbscissa evaluation = [](float x, Context * context, const void * auxiliary) {
    /* When evaluating sin(x)/x close to zero using the standard sine function,
     * one can detect small variations, while the cardinal sine is supposed to
     * be locally monotonous. To smooth out such variations, we round the
     * result of the evaluations. As we are not interested in precise results
     * but only in ordering, this approximation is sufficient. */
    constexpr float precision = 1e-5;
    return precision * std::round(static_cast<const NewFunction *>(auxiliary)->evaluateXYAtParameter(x, context).x2() / precision);
  };
  Zoom::InterestingRangesForDisplay(evaluation, xMin, xMax, yMinIntrinsic, yMaxIntrinsic, std::max(tMin(), xMinLimit), std::min(tMax(), xMaxLimit), context, this);
}

void NewFunction::yRangeForDisplay(float xMin, float xMax, float yMinForced, float yMaxForced, float ratio, float * yMin, float * yMax, Context * context) const {
  // TODO Hugo : Re-check
  if (plotType() == PlotType::Parametric || plotType() == PlotType::Polar) {
    assert(std::isfinite(tMin()) && std::isfinite(tMax()) && std::isfinite(rangeStep()) && rangeStep() > 0);
    protectedFullRangeForDisplay(tMin(), tMax(), rangeStep(), yMin, yMax, context, false);
    return;
  }

  *yMin = NAN;
  *yMax = NAN;

  if (basedOnCostlyAlgorithms(context)) {
    /* The function makes use of some costly algorithms, such as integration or
     * sequences, and cannot be computed in a timely manner. */
    return;
  }

  Zoom::ValueAtAbscissa evaluation = [](float x, Context * context, const void * auxiliary) {
    return static_cast<const NewFunction *>(auxiliary)->evaluateXYAtParameter(x, context).x2();
  };

  if (yMaxForced - yMinForced <= ratio * (xMax - xMin)) {
    Zoom::RangeWithRatioForDisplay(evaluation, ratio, xMin, xMax, yMinForced, yMaxForced, yMin, yMax, context, this);
    if (*yMin < *yMax) {
      return;
    }
  }

  *yMin = NAN;
  *yMax = NAN;

  Zoom::RefinedYRangeForDisplay(evaluation, xMin, xMax, yMin, yMax, context, this);
}

void * NewFunction::Model::expressionAddress(const Ion::Storage::Record * record) const {
  // TODO Hugo : Re-check
  return (char *)record->value().buffer+sizeof(RecordDataBuffer);
}

size_t NewFunction::Model::expressionSize(const Ion::Storage::Record * record) const {
  // TODO Hugo : Re-check
  return record->value().size-sizeof(RecordDataBuffer);
}

template<typename T>
Coordinate2D<T> NewFunction::templatedApproximateAtParameter(T t, Context * context) const {
  if (t < tMin() || t > tMax()) {
    return Coordinate2D<T>(plotType() == PlotType::Cartesian ? t : NAN, NAN);
  }
  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknown[bufferSize];
  SerializationHelper::CodePoint(unknown, bufferSize, UCodePointUnknown);
  PlotType type = plotType();
  Expression e = expressionReduced(context);
  if (type != PlotType::Parametric) {
    assert(type == PlotType::Cartesian || type == PlotType::Polar);
    return Coordinate2D<T>(t, Shared::PoincareHelpers::ApproximateWithValueForSymbol(e, unknown, t, context));
  }
  if (e.type() == ExpressionNode::Type::Dependency) {
    e = e.childAtIndex(0);
  }
  assert(e.type() == ExpressionNode::Type::Matrix);
  assert(static_cast<Matrix&>(e).numberOfRows() == 2);
  assert(static_cast<Matrix&>(e).numberOfColumns() == 1);
  return Coordinate2D<T>(
      Shared::PoincareHelpers::ApproximateWithValueForSymbol(e.childAtIndex(0), unknown, t, context),
      Shared::PoincareHelpers::ApproximateWithValueForSymbol(e.childAtIndex(1), unknown, t, context));
}

Coordinate2D<double> NewFunction::nextMinimumFrom(double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) const {
  // TODO Hugo : Re-check
  return nextPointOfInterestFrom(start, max, context, [](Expression e, char * symbol, double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) { return Shared::PoincareHelpers::NextMinimum(e, symbol, start, max, context, relativePrecision, minimalStep, maximalStep); }, relativePrecision, minimalStep, maximalStep);
}

Coordinate2D<double> NewFunction::nextMaximumFrom(double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) const {
  // TODO Hugo : Re-check
  return nextPointOfInterestFrom(start, max, context, [](Expression e, char * symbol, double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) { return Shared::PoincareHelpers::NextMaximum(e, symbol, start, max, context, relativePrecision, minimalStep, maximalStep); }, relativePrecision, minimalStep, maximalStep);
}

Coordinate2D<double> NewFunction::nextRootFrom(double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) const {
  // TODO Hugo : Re-check
  return nextPointOfInterestFrom(start, max, context, [](Expression e, char * symbol, double start, double max, Context * context, double relativePrecision, double minimalStep, double maximalStep) { return Coordinate2D<double>(Shared::PoincareHelpers::NextRoot(e, symbol, start, max, context, relativePrecision, minimalStep, maximalStep), 0.0); }, relativePrecision, minimalStep, maximalStep);
}

Coordinate2D<double> NewFunction::nextIntersectionFrom(double start, double max, Context * context, Expression e, double relativePrecision, double minimalStep, double maximalStep, double eDomainMin, double eDomainMax) const {
  // TODO Hugo : Re-check
  assert(plotType() == PlotType::Cartesian);
  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknownX[bufferSize];
  SerializationHelper::CodePoint(unknownX, bufferSize, UCodePointUnknown);
  double tmin = std::max<double>(tMin(), eDomainMin), tmax = std::min<double>(tMax(), eDomainMax);
  start = start < tmin ? tmin : start > tmax ? tmax : start;
  max = max < tmin ? tmin : max > tmax ? tmax : max;
  if (start == max) {
    return NAN;
  }
  return Shared::PoincareHelpers::NextIntersection(expressionReduced(context), unknownX, start, max, context, e, relativePrecision, minimalStep, maximalStep);
}

Coordinate2D<double> NewFunction::nextPointOfInterestFrom(double start, double max, Context * context, ComputePointOfInterest compute, double relativePrecision, double minimalStep, double maximalStep) const {
  // TODO Hugo : Re-check
  assert(plotType() == PlotType::Cartesian);
  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknownX[bufferSize];
  SerializationHelper::CodePoint(unknownX, bufferSize, UCodePointUnknown);
  double tmin = tMin(), tmax = tMax();
  start = start < tmin ? tmin : start > tmax ? tmax : start;
  max = max < tmin ? tmin : max > tmax ? tmax : max;
  if (start == max) {
    return NAN;
  }
  return compute(expressionReduced(context), unknownX, start, max, context, relativePrecision, minimalStep, maximalStep);
}

Expression NewFunction::sumBetweenBounds(double start, double end, Context * context) const {
  // TODO Hugo : Re-check
  assert(plotType() == PlotType::Cartesian);
  start = std::max<double>(start, tMin());
  end = std::min<double>(end, tMax());
  return Integral::Builder(expressionReduced(context).clone(), Symbol::Builder(UCodePointUnknown), Float<double>::Builder(start), Float<double>::Builder(end)); // Integral takes ownership of args
  /* TODO: when we approximate integral, we might want to simplify the integral
   * here. However, we might want to do it once for all x (to avoid lagging in
   * the derivative table. */
}

void NewFunction::fullXYRange(float * xMin, float * xMax, float * yMin, float * yMax, Context * context) const {
  // TODO Hugo : Re-implement
}

template Coordinate2D<float> NewFunction::templatedApproximateAtParameter<float>(float, Context *) const;
template Coordinate2D<double> NewFunction::templatedApproximateAtParameter<double>(double, Context *) const;

template Coordinate2D<float> NewFunction::privateEvaluateXYAtParameter<float>(float, Context *) const;
template Coordinate2D<double> NewFunction::privateEvaluateXYAtParameter<double>(double, Context *) const;


} // namespace Graph
