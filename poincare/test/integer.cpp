#include <quiz.h>
#include <poincare.h>
#include <assert.h>

QUIZ_CASE(poincare_integer) {
  assert(Integer(123) == Integer(123));
  assert(Integer("123") == Integer(123));
  assert(!(Integer("-123") == Integer(123)));
  assert(Integer("-123") == Integer(-123));
  //assert(Integer("0123") == Integer(123));
  assert(Integer("0x2BABE") == Integer(178878));
  assert(Integer("0b1011") == Integer(11));
}

QUIZ_CASE(poincare_integer_compare) {
  assert(Integer(123) < Integer(456));
  assert(!(Integer(123) < Integer(123)));
  assert(!(Integer(-123) < Integer(-123)));
  assert(Integer(-100) < Integer(2));
  assert(Integer(-200) < Integer(-100));
  assert(Integer(123) < Integer("123456789123456789"));
  assert(Integer("123456789123456788") < Integer("123456789123456789"));
  assert(!(Integer("123456789123456789") < Integer("123456789123456788")));
}

QUIZ_CASE(poincare_integer_add) {
  assert(Integer((uint32_t)0) + Integer((uint32_t)0) == Integer((uint32_t)0));
  assert(Integer(123) + Integer(456) == Integer(579));
  assert(Integer("123456789123456789") + Integer(1) == Integer("123456789123456790"));
}

QUIZ_CASE(poincare_integer_subtract) {
  assert(Integer(123) - Integer(23) == Integer(100));
  assert(Integer("123456789123456789") - Integer("9999999999") == Integer("123456779123456790"));
  assert(Integer(23) - Integer(100) == Integer(-77));
}

QUIZ_CASE(poincare_integer_multiply) {
  assert(Integer(12) * Integer(34) == Integer(408));
  assert(Integer(56) * Integer(0) == Integer(0));
  assert(Integer(-12) * Integer(34) == Integer(-408));
  assert(Integer(-12) * Integer(-34) == Integer(408));
  assert(Integer(999999) * Integer(999999) == Integer("999998000001"));
  assert(Integer("9999999999") * Integer("9999999999") == Integer("99999999980000000001"));
}

QUIZ_CASE(poincare_integer_divide) {
  assert(Integer(8) / Integer(4) == Integer(2));
  assert(Integer("3293920983029832")/Integer("38928") == Integer("84615726033"));
  assert(Integer("3293920983029832")/Integer("389090928") == Integer("8465684"));
}

QUIZ_CASE(poincare_integer_approximate) {
  assert(Integer(1).approximate() == 1.0f);
  assert(Integer("12345678").approximate() == 12345678.0f);
}
