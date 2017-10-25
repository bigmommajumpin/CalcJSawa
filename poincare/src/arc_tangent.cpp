#include <poincare/arc_tangent.h>
#include <poincare/trigonometry.h>
extern "C" {
#include <assert.h>
}
#include <cmath>

namespace Poincare {

Expression::Type ArcTangent::type() const {
  return Type::ArcTangent;
}

Expression * ArcTangent::clone() const {
  ArcTangent * a = new ArcTangent(m_operands, true);
  return a;
}

Expression * ArcTangent::immediateSimplify(Context& context, AngleUnit angleUnit) {
  return Trigonometry::immediateSimplifyInverseFunction(this, context, angleUnit);
}

template<typename T>
Complex<T> ArcTangent::computeOnComplex(const Complex<T> c, AngleUnit angleUnit) {
  assert(angleUnit != AngleUnit::Default);
  if (c.b() != 0) {
    return Complex<T>::Float(NAN);
  }
  T result = std::atan(c.a());
  if (angleUnit == AngleUnit::Degree) {
    return Complex<T>::Float(result*180/M_PI);
  }
  return Complex<T>::Float(result);
}

}
