#include <poincare/init.h>
#include <poincare/exception_checkpoint.h>
#include <ion.h>
#include <cmath>
#include <assert.h>
#include "tree/helpers.h"
#include "helper.h"

using namespace Poincare;

void assert_tokenizes_as(const Token::Type * tokenTypes, const char * string) {
  Tokenizer tokenizer(string);
  while (true) {
    Token token = tokenizer.popToken();
    quiz_assert(token.type() == *tokenTypes);
    if (token.type() == Token::EndOfStream) {
      return;
    }
    tokenTypes++;
  }
}

void assert_tokenizes_as_number(const char * string) {
  const Token::Type types[] = {Token::Number, Token::EndOfStream};
  assert_tokenizes_as(types, string);
}

void assert_tokenizes_as_undefined_token(const char * string) {
  Tokenizer tokenizer(string);
  while (true) {
    Token token = tokenizer.popToken();
    if (token.type() == Token::Undefined) {
      return;
    }
    if (token.type() == Token::EndOfStream) {
      quiz_assert(false);
    }
  }
}

void assert_raises_parsing_error(const char * text) {
  Parser p(text);
  Expression result = p.parse();
  quiz_assert(p.getStatus() != Parser::Status::Success);
}

QUIZ_CASE(poincare_parser_tokenize_numbers) {
  assert_tokenizes_as_number("1");
  assert_tokenizes_as_number("12");
  assert_tokenizes_as_number("123");
  assert_tokenizes_as_number("1.3");
  assert_tokenizes_as_number(".3");
  assert_tokenizes_as_number("1.3ᴇ3");
  assert_tokenizes_as_number("12.34ᴇ56");
  assert_tokenizes_as_number(".3ᴇ-32");
  assert_tokenizes_as_number("12.");
  assert_tokenizes_as_number(".999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999");
  assert_tokenizes_as_number("0.");
  assert_tokenizes_as_number("1.ᴇ-4");
  assert_tokenizes_as_number("1.ᴇ9999");

  assert_tokenizes_as_undefined_token("1ᴇ");
  assert_tokenizes_as_undefined_token("1..");
  assert_tokenizes_as_undefined_token("..");
  assert_tokenizes_as_undefined_token("1.ᴇᴇ");
  assert_tokenizes_as_undefined_token("1.ᴇ");
  assert_tokenizes_as_undefined_token("1ᴇ--4");
  assert_tokenizes_as_undefined_token("1.ᴇᴇ4");
  assert_tokenizes_as_undefined_token("1ᴇ2ᴇ4");
}

QUIZ_CASE(poincare_parser_parse_numbers) {
  int initialPoolSize = pool_size();
  assert_parsed_expression_type("2+3", ExpressionNode::Type::Addition);
  assert_pool_size(initialPoolSize);

  // Parse digits
  assert_parsed_expression_is("0", Rational::Builder(0));
  assert_parsed_expression_is("0.1", Decimal::Builder(0.1));
  assert_parsed_expression_is("1.", Rational::Builder(1));
  assert_parsed_expression_is(".1", Decimal::Builder(0.1));
  assert_parsed_expression_is("0ᴇ2", Decimal::Builder(0.0));
  assert_parsed_expression_is("0.1ᴇ2", Decimal::Builder(10.0));
  assert_parsed_expression_is("1.ᴇ2", Decimal::Builder(100.0));
  assert_parsed_expression_is(".1ᴇ2", Decimal::Builder(10.0));
  assert_parsed_expression_is("0ᴇ-2", Decimal::Builder(0.0));
  assert_parsed_expression_is("0.1ᴇ-2", Decimal::Builder(0.001));
  assert_parsed_expression_is("1.ᴇ-2", Decimal::Builder(0.01));
  assert_parsed_expression_is(".1ᴇ-2", Decimal::Builder(0.001));
}

QUIZ_CASE(poincare_parser_memory_exhaustion) {
  int initialPoolSize = pool_size();
  int memoryFailureHasBeenHandled = false;
  {
    Poincare::ExceptionCheckpoint ecp;
    if (ExceptionRun(ecp)) {
      Addition a = Addition::Builder();
      while (true) {
        Expression e = Expression::Parse("1+2+3+4+5+6+7+8+9+10");
        a.addChildAtIndexInPlace(e, 0, a.numberOfChildren());
      }
    } else {
      Poincare::Tidy();
      memoryFailureHasBeenHandled = true;
    }
  }

  quiz_assert(memoryFailureHasBeenHandled);
  assert_pool_size(initialPoolSize);
  Expression e = Expression::Parse("1+1");
  /* Stupid check to make sure the global variable generated by Bison is not
   * ruining everything */
}

QUIZ_CASE(poincare_parser_parse) {
  assert_parsed_expression_is("1", Rational::Builder(1));
  assert_parsed_expression_is("(1)", Parenthesis::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("((1))", Parenthesis::Builder((Expression)Parenthesis::Builder(Rational::Builder(1))));
  assert_parsed_expression_is("1+2", Addition::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("(1)+2", Addition::Builder(Parenthesis::Builder(Rational::Builder(1)),Rational::Builder(2)));
  assert_parsed_expression_is("(1+2)", Parenthesis::Builder(Addition::Builder(Rational::Builder(1),Rational::Builder(2))));
  assert_parsed_expression_is("1+2+3", Addition::Builder(Addition::Builder(Rational::Builder(1),Rational::Builder(2)),Rational::Builder(3)));
  assert_parsed_expression_is("1+2+(3+4)", Addition::Builder(Addition::Builder(Rational::Builder(1),Rational::Builder(2)),Parenthesis::Builder(Addition::Builder(Rational::Builder(3),Rational::Builder(4)))));
  assert_parsed_expression_is("1×2", Multiplication::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("1×2×3", Multiplication::Builder(Multiplication::Builder(Rational::Builder(1),Rational::Builder(2)),Rational::Builder(3)));
  assert_parsed_expression_is("1+2×3", Addition::Builder(Rational::Builder(1), Multiplication::Builder(Rational::Builder(2), Rational::Builder(3))));
  assert_parsed_expression_is("1/2", Division::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("(1/2)", Parenthesis::Builder(Division::Builder(Rational::Builder(1),Rational::Builder(2))));
  assert_parsed_expression_is("1/2/3", Division::Builder(Division::Builder(Rational::Builder(1),Rational::Builder(2)),Rational::Builder(3)));
  assert_parsed_expression_is("1/2×3", Multiplication::Builder(Division::Builder(Rational::Builder(1),Rational::Builder(2)),Rational::Builder(3)));
  assert_parsed_expression_is("(1/2×3)", Parenthesis::Builder(Multiplication::Builder(Division::Builder(Rational::Builder(1),Rational::Builder(2)),Rational::Builder(3))));
  assert_parsed_expression_is("1×2/3", Multiplication::Builder(Rational::Builder(1),Division::Builder(Rational::Builder(2),Rational::Builder(3))));
  assert_parsed_expression_is("(1×2/3)", Parenthesis::Builder(Multiplication::Builder(Rational::Builder(1),Division::Builder(Rational::Builder(2),Rational::Builder(3)))));
  assert_parsed_expression_is("(1/2/3)", Parenthesis::Builder(Division::Builder(Division::Builder(Rational::Builder(1),Rational::Builder(2)),Rational::Builder(3))));
  assert_parsed_expression_is("1^2", Power::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("1^2^3", Power::Builder(Rational::Builder(1),Power::Builder(Rational::Builder(2),Rational::Builder(3))));
  assert_parsed_expression_is("1=2", Equal::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_raises_parsing_error("=5");
  assert_raises_parsing_error("1=2=3");
  assert_parsed_expression_is("-1", Opposite::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("(-1)", Parenthesis::Builder(Opposite::Builder(Rational::Builder(1))));
  assert_parsed_expression_is("1-2", Subtraction::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("-1-2", Subtraction::Builder(Opposite::Builder(Rational::Builder(1)),Rational::Builder(2)));
  assert_parsed_expression_is("1-2-3", Subtraction::Builder(Subtraction::Builder(Rational::Builder(1),Rational::Builder(2)),Rational::Builder(3)));
  assert_parsed_expression_is("(1-2)", Parenthesis::Builder(Subtraction::Builder(Rational::Builder(1),Rational::Builder(2))));
  assert_parsed_expression_is("1+-2", Addition::Builder(Rational::Builder(1),Opposite::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("--1", Opposite::Builder((Expression)Opposite::Builder(Rational::Builder(1))));
  assert_parsed_expression_is("(1+2)-3", Subtraction::Builder(Parenthesis::Builder(Addition::Builder(Rational::Builder(1),Rational::Builder(2))),Rational::Builder(3)));
  assert_parsed_expression_is("(2×-3)", Parenthesis::Builder(Multiplication::Builder(Rational::Builder(2),Opposite::Builder(Rational::Builder(3)))));
  assert_parsed_expression_is("1^(2)-3", Subtraction::Builder(Power::Builder(Rational::Builder(1),Parenthesis::Builder(Rational::Builder(2))),Rational::Builder(3)));
  assert_parsed_expression_is("1^2-3", Subtraction::Builder(Power::Builder(Rational::Builder(1),Rational::Builder(2)),Rational::Builder(3)));
  assert_parsed_expression_is("2^-3", Power::Builder(Rational::Builder(2),Opposite::Builder(Rational::Builder(3))));
  assert_parsed_expression_is("2--2+-1", Addition::Builder(Subtraction::Builder(Rational::Builder(2),Opposite::Builder(Rational::Builder(2))),Opposite::Builder(Rational::Builder(1))));
  assert_parsed_expression_is("2--2×-1", Subtraction::Builder(Rational::Builder(2),Opposite::Builder(Multiplication::Builder(Rational::Builder(2),Opposite::Builder(Rational::Builder(1))))));
  assert_parsed_expression_is("-1^2", Opposite::Builder(Power::Builder(Rational::Builder(1),Rational::Builder(2))));
  assert_parsed_expression_is("2/-3/-4", Division::Builder(Division::Builder(Rational::Builder(2),Opposite::Builder(Rational::Builder(3))),Opposite::Builder(Rational::Builder(4))));
  assert_parsed_expression_is("1×2-3×4", Subtraction::Builder(Multiplication::Builder(Rational::Builder(1),Rational::Builder(2)),Multiplication::Builder(Rational::Builder(3),Rational::Builder(4))));
  assert_parsed_expression_is("-1×2", Opposite::Builder(Multiplication::Builder(Rational::Builder(1), Rational::Builder(2))));
  assert_parsed_expression_is("1!", Factorial::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("1+2!", Addition::Builder(Rational::Builder(1),Factorial::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("1!+2", Addition::Builder(Factorial::Builder(Rational::Builder(1)),Rational::Builder(2)));
  assert_parsed_expression_is("1!+2!", Addition::Builder(Factorial::Builder(Rational::Builder(1)),Factorial::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("1×2!", Multiplication::Builder(Rational::Builder(1),Factorial::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("1!×2", Multiplication::Builder(Factorial::Builder(Rational::Builder(1)),Rational::Builder(2)));
  assert_parsed_expression_is("1!×2!", Multiplication::Builder(Factorial::Builder(Rational::Builder(1)),Factorial::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("1-2!", Subtraction::Builder(Rational::Builder(1),Factorial::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("1!-2", Subtraction::Builder(Factorial::Builder(Rational::Builder(1)),Rational::Builder(2)));
  assert_parsed_expression_is("1!-2!", Subtraction::Builder(Factorial::Builder(Rational::Builder(1)),Factorial::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("1/2!", Division::Builder(Rational::Builder(1),Factorial::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("1!/2", Division::Builder(Factorial::Builder(Rational::Builder(1)),Rational::Builder(2)));
  assert_parsed_expression_is("1!/2!", Division::Builder(Factorial::Builder(Rational::Builder(1)),Factorial::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("1^2!", Power::Builder(Rational::Builder(1),Factorial::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("1!^2", Power::Builder(Factorial::Builder(Rational::Builder(1)),Rational::Builder(2)));
  assert_parsed_expression_is("1!^2!", Power::Builder(Factorial::Builder(Rational::Builder(1)),Factorial::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("(1)!", Factorial::Builder(Parenthesis::Builder(Rational::Builder(1))));
  assert_raises_parsing_error("1+");
  assert_raises_parsing_error(")");
  assert_raises_parsing_error(")(");
  assert_raises_parsing_error("()");
  assert_raises_parsing_error("(1");
  assert_raises_parsing_error("1)");
  assert_raises_parsing_error("1++2");
  assert_raises_parsing_error("1//2");
  assert_raises_parsing_error("×1");
  assert_raises_parsing_error("1^^2");
  assert_raises_parsing_error("^1");
  assert_raises_parsing_error("t0000000");
  assert_raises_parsing_error("[[t0000000[");
  assert_raises_parsing_error("0→x=0");
  assert_raises_parsing_error("0=0→x");
  assert_raises_parsing_error("1ᴇ2ᴇ3");
}

Matrix BuildMatrix(int rows, int columns, Expression entries[]) {
  Matrix m = Matrix::Builder();
  int position = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      m.addChildAtIndexInPlace(entries[position], position, position);
      position++;
    }
  }
  m.setDimensions(rows, columns);
  return m;
}

QUIZ_CASE(poincare_parser_matrices) {
  Expression m1[] = {Rational::Builder(1)};
  assert_parsed_expression_is("[[1]]", BuildMatrix(1,1,m1));
  Expression m2[] = {Rational::Builder(1),Rational::Builder(2),Rational::Builder(3)};
  assert_parsed_expression_is("[[1,2,3]]", BuildMatrix(1,3,m2));
  Expression m3[] = {Rational::Builder(1),Rational::Builder(2),Rational::Builder(3),Rational::Builder(4),Rational::Builder(5),Rational::Builder(6)};
  assert_parsed_expression_is("[[1,2,3][4,5,6]]", BuildMatrix(2,3,m3));
  Expression m4[] = {Rational::Builder(1), BuildMatrix(1,1,m1)};
  assert_parsed_expression_is("[[1,[[1]]]]", BuildMatrix(1,2,m4));
  assert_raises_parsing_error("[");
  assert_raises_parsing_error("]");
  assert_raises_parsing_error("[[");
  assert_raises_parsing_error("][");
  assert_raises_parsing_error("[]");
  assert_raises_parsing_error("[1]");
  assert_raises_parsing_error("[[1,2],[3]]");
  assert_raises_parsing_error("[[]");
  assert_raises_parsing_error("[[1]");
  assert_raises_parsing_error("[1]]");
  assert_raises_parsing_error("[[,]]");
  assert_raises_parsing_error("[[1,]]");
  assert_raises_parsing_error(",");
  assert_raises_parsing_error("[,]");
}

QUIZ_CASE(poincare_parser_symbols_and_functions) {
  // User-defined symbols
  assert_parsed_expression_is("a", Symbol::Builder("a", 1));
  assert_parsed_expression_is("x", Symbol::Builder("x", 1));
  assert_parsed_expression_is("toot", Symbol::Builder("toot", 4));
  assert_parsed_expression_is("toto_", Symbol::Builder("toto_", 5));
  assert_parsed_expression_is("t_toto", Symbol::Builder("t_toto", 6));
  assert_parsed_expression_is("tot12", Symbol::Builder("tot12", 5));
  assert_parsed_expression_is("TOto", Symbol::Builder("TOto", 4));
  assert_parsed_expression_is("TO12_Or", Symbol::Builder("TO12_Or", 7));
  assert_raises_parsing_error("_a");
  assert_raises_parsing_error("abcdefgh");

  // User-defined functions
  assert_parsed_expression_is("f(x)", Function::Builder("f", 1, Symbol::Builder("x",1)));
  assert_parsed_expression_is("f(1)", Function::Builder("f", 1, Rational::Builder(1)));
  assert_parsed_expression_is("ab12AB_(x)", Function::Builder("ab12AB_", 7, Symbol::Builder("x",1)));
  assert_parsed_expression_is("ab12AB_(1)", Function::Builder("ab12AB_", 7, Rational::Builder(1)));
  assert_parsed_expression_is("f(g(x))", Function::Builder("f", 1, Function::Builder("g", 1, Symbol::Builder("x",1))));
  assert_parsed_expression_is("f(g(1))", Function::Builder("f", 1, Function::Builder("g", 1, Rational::Builder(1))));
  assert_parsed_expression_is("f((1))", Function::Builder("f", 1, Parenthesis::Builder(Rational::Builder(1))));
  assert_raises_parsing_error("f(1,2)");
  assert_raises_parsing_error("f(f)");
  assert_raises_parsing_error("abcdefgh(1)");

  // Reserved symbols
  assert_parsed_expression_is("ans", Symbol::Builder("ans", 3));
  assert_parsed_expression_is("𝐢", Constant::Builder(KDCodePointMathematicalBoldSmallI));
  assert_parsed_expression_is("π", Constant::Builder(KDCodePointGreekSmallLetterPi));
  assert_parsed_expression_is("ℯ", Constant::Builder(KDCodePointScriptSmallE));
  assert_parsed_expression_is(Infinity::Name(), Infinity::Builder(false));
  assert_parsed_expression_is(Undefined::Name(), Undefined::Builder());

  assert_raises_parsing_error("u");
  assert_raises_parsing_error("v");

  // Reserved functions
  assert_parsed_expression_is("acos(1)", ArcCosine::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("acosh(1)", HyperbolicArcCosine::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("abs(1)", AbsoluteValue::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("arg(1)", ComplexArgument::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("asin(1)", ArcSine::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("asinh(1)", HyperbolicArcSine::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("atan(1)", ArcTangent::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("atanh(1)", HyperbolicArcTangent::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("binomial(2,1)", BinomialCoefficient::Builder(Rational::Builder(2),Rational::Builder(1)));
  assert_parsed_expression_is("ceil(1)", Ceiling::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("confidence(1,2)", ConfidenceInterval::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_raises_parsing_error("diff(1,2,3)");
  assert_parsed_expression_is("diff(1,x,3)", Derivative::Builder(Rational::Builder(1),Symbol::Builder("x",1),Rational::Builder(3)));
  assert_parsed_expression_is("dim(1)", MatrixDimension::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("conj(1)", Conjugate::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("det(1)", Determinant::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("cos(1)", Cosine::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("cosh(1)", HyperbolicCosine::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("factor(1)", Factor::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("floor(1)", Floor::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("frac(1)", FracPart::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("gcd(1,2)", GreatCommonDivisor::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("im(1)", ImaginaryPart::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("int(1,x,2,3)", Integral::Builder(Rational::Builder(1),Symbol::Builder("x",1),Rational::Builder(2),Rational::Builder(3)));
  assert_raises_parsing_error("int(1,2,3,4)");
  assert_parsed_expression_is("inverse(1)", MatrixInverse::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("lcm(1,2)", LeastCommonMultiple::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("ln(1)", NaperianLogarithm::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("log(1)", CommonLogarithm::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("log(1,2)", Logarithm::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("log_{2}(1)", Logarithm::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("permute(2,1)", PermuteCoefficient::Builder(Rational::Builder(2),Rational::Builder(1)));
  assert_parsed_expression_is("prediction95(1,2)", PredictionInterval::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("prediction(1,2)", SimplePredictionInterval::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("product(1,n,2,3)", Product::Builder(Rational::Builder(1),Symbol::Builder("n",1),Rational::Builder(2),Rational::Builder(3)));
  assert_raises_parsing_error("product(1,2,3,4)");
  assert_parsed_expression_is("quo(1,2)", DivisionQuotient::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("random()", Random::Builder());
  assert_parsed_expression_is("randint(1,2)", Randint::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("re(1)", RealPart::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("rem(1,2)", DivisionRemainder::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("root(1,2)", NthRoot::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("round(1,2)", Round::Builder(Rational::Builder(1),Rational::Builder(2)));
  assert_parsed_expression_is("sin(1)", Sine::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("sinh(1)", HyperbolicSine::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("sum(1,n,2,3)", Sum::Builder(Rational::Builder(1),Symbol::Builder("n",1),Rational::Builder(2),Rational::Builder(3)));
  assert_raises_parsing_error("sum(1,2,3,4)");
  assert_parsed_expression_is("tan(1)", Tangent::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("tanh(1)", HyperbolicTangent::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("trace(1)", MatrixTrace::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("transpose(1)", MatrixTranspose::Builder(Rational::Builder(1)));
  assert_parsed_expression_is("√(1)", SquareRoot::Builder(Rational::Builder(1)));
  assert_raises_parsing_error("cos(1,2)");
  assert_raises_parsing_error("log(1,2,3)");
}

QUIZ_CASE(poincare_parser_parse_store) {
  assert_parsed_expression_is("1→a", Store::Builder(Rational::Builder(1),Symbol::Builder("a",1)));
  assert_parsed_expression_is("1→e", Store::Builder(Rational::Builder(1),Symbol::Builder("e",1)));
  assert_parsed_expression_is("1→f(x)", Store::Builder(Rational::Builder(1),Function::Builder("f",1,Symbol::Builder("x",1))));
  assert_parsed_expression_is("x→f(x)", Store::Builder(Symbol::Builder("x",1),Function::Builder("f",1,Symbol::Builder("x",1))));
  assert_parsed_expression_is("n→f(x)", Store::Builder(Symbol::Builder("n",1),Function::Builder("f",1,Symbol::Builder("x",1))));
  Expression m0[] = {Symbol::Builder('x')};
  assert_parsed_expression_is("[[x]]→f(x)", Store::Builder(BuildMatrix(1,1,m0), Function::Builder("f", 1, Symbol::Builder('x'))));
  assert_raises_parsing_error("a→b→c");
  assert_raises_parsing_error("1→2");
  assert_raises_parsing_error("1→");
  assert_raises_parsing_error("→2");
  assert_raises_parsing_error("(1→a)");
  assert_raises_parsing_error("1→u(n)");
  assert_raises_parsing_error("1→u(n+1)");
  assert_raises_parsing_error("1→v(n)");
  assert_raises_parsing_error("1→v(n+1)");
  assert_raises_parsing_error("1→u_{n}");
  assert_raises_parsing_error("1→u_{n+1}");
  assert_raises_parsing_error("1→v_{n}");
  assert_raises_parsing_error("1→v_{n+1}");
  assert_raises_parsing_error("1→inf");
  assert_raises_parsing_error("1→undef");
  assert_raises_parsing_error("1→π");
  assert_raises_parsing_error("1→𝐢");
  assert_raises_parsing_error("1→ℯ");
  assert_raises_parsing_error("1→\1"); // UnknownX
  assert_raises_parsing_error("1→acos");
  assert_raises_parsing_error("1→f(2)");
  assert_raises_parsing_error("1→f(f)");
  assert_raises_parsing_error("1→ans");
  assert_raises_parsing_error("ans→ans");
}

QUIZ_CASE(poincare_parser_implicit_multiplication) {
  assert_raises_parsing_error(".1.2");
  assert_raises_parsing_error("1 2");
  assert_parsed_expression_is("1x", Multiplication::Builder(Rational::Builder(1),Symbol::Builder("x", 1)));
  assert_parsed_expression_is("1ans", Multiplication::Builder(Rational::Builder(1),Symbol::Builder("ans", 3)));
  assert_parsed_expression_is("x1", Symbol::Builder("x1", 2));
  assert_parsed_expression_is("1x+2", Addition::Builder(Multiplication::Builder(Rational::Builder(1),Symbol::Builder("x", 1)),Rational::Builder(2)));
  assert_parsed_expression_is("1π", Multiplication::Builder(Rational::Builder(1),Constant::Builder(KDCodePointGreekSmallLetterPi)));
  assert_parsed_expression_is("1x-2", Subtraction::Builder(Multiplication::Builder(Rational::Builder(1),Symbol::Builder("x", 1)),Rational::Builder(2)));
  assert_parsed_expression_is("-1x", Opposite::Builder(Multiplication::Builder(Rational::Builder(1),Symbol::Builder("x", 1))));
  assert_parsed_expression_is("2×1x", Multiplication::Builder(Rational::Builder(2),Multiplication::Builder(Rational::Builder(1),Symbol::Builder("x", 1))));
  assert_parsed_expression_is("2^1x", Multiplication::Builder(Power::Builder(Rational::Builder(2),Rational::Builder(1)),Symbol::Builder("x", 1)));
  assert_parsed_expression_is("1x^2", Multiplication::Builder(Rational::Builder(1),Power::Builder(Symbol::Builder("x", 1),Rational::Builder(2))));
  assert_parsed_expression_is("2/1x", Division::Builder(Rational::Builder(2),Multiplication::Builder(Rational::Builder(1),Symbol::Builder("x", 1))));
  assert_parsed_expression_is("1x/2", Division::Builder(Multiplication::Builder(Rational::Builder(1),Symbol::Builder("x", 1)),Rational::Builder(2)));
  assert_parsed_expression_is("(1)2", Multiplication::Builder(Parenthesis::Builder(Rational::Builder(1)),Rational::Builder(2)));
  assert_parsed_expression_is("1(2)", Multiplication::Builder(Rational::Builder(1),Parenthesis::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("sin(1)2", Multiplication::Builder(Sine::Builder(Rational::Builder(1)),Rational::Builder(2)));
  assert_parsed_expression_is("1cos(2)", Multiplication::Builder(Rational::Builder(1),Cosine::Builder(Rational::Builder(2))));
  assert_parsed_expression_is("1!2", Multiplication::Builder(Factorial::Builder(Rational::Builder(1)),Rational::Builder(2)));
  assert_parsed_expression_is("2ℯ^(3)", Multiplication::Builder(Rational::Builder(2),Power::Builder(Constant::Builder(KDCodePointScriptSmallE),Parenthesis::Builder(Rational::Builder(3)))));
  Expression m1[] = {Rational::Builder(1)}; Matrix M1 = BuildMatrix(1,1,m1);
  Expression m2[] = {Rational::Builder(2)}; Matrix M2 = BuildMatrix(1,1,m2);
  assert_parsed_expression_is("[[1]][[2]]", Multiplication::Builder(M1,M2));
}

QUIZ_CASE(poincare_parser_expression_evaluation) {
  assert_parsed_expression_evaluates_to<float>("-0", "0");
  assert_parsed_expression_evaluates_to<float>("-0.1", "-0.1");
  assert_parsed_expression_evaluates_to<float>("-1.", "-1");
  assert_parsed_expression_evaluates_to<float>("-.1", "-0.1");
  assert_parsed_expression_evaluates_to<float>("-0ᴇ2", "0");
  assert_parsed_expression_evaluates_to<float>("-0.1ᴇ2", "-10");
  assert_parsed_expression_evaluates_to<float>("-1.ᴇ2", "-100");
  assert_parsed_expression_evaluates_to<float>("-.1ᴇ2", "-10");
  assert_parsed_expression_evaluates_to<float>("-0ᴇ-2", "0");
  assert_parsed_expression_evaluates_to<float>("-0.1ᴇ-2", "-0.001");
  assert_parsed_expression_evaluates_to<float>("-1.ᴇ-2", "-0.01");
  assert_parsed_expression_evaluates_to<float>("-.1ᴇ-2", "-0.001");

  assert_parsed_expression_evaluates_to<float>("-2-3", "-5");
  assert_parsed_expression_evaluates_to<float>("1.2×ℯ^(1)", "3.261938");
  assert_parsed_expression_evaluates_to<float>("2ℯ^(3)", "40.1711", System, Radian, Cartesian, 6); // WARNING: the 7th significant digit is wrong on blackbos simulator
  assert_parsed_expression_evaluates_to<float>("ℯ^2×ℯ^(1)", "20.0855", System, Radian, Cartesian, 6); // WARNING: the 7th significant digit is wrong on simulator
  assert_parsed_expression_evaluates_to<double>("ℯ^2×ℯ^(1)", "20.085536923188");
  assert_parsed_expression_evaluates_to<double>("2×3^4+2", "164");
  assert_parsed_expression_evaluates_to<float>("-2×3^4+2", "-160");
  assert_parsed_expression_evaluates_to<double>("-sin(3)×2-3", "-3.2822400161197", System, Radian);
  assert_parsed_expression_evaluates_to<float>("-.003", "-0.003");
  assert_parsed_expression_evaluates_to<double>(".02ᴇ2", "2");
  assert_parsed_expression_evaluates_to<float>("5-2/3", "4.333333");
  assert_parsed_expression_evaluates_to<double>("2/3-5", "-4.3333333333333");
  assert_parsed_expression_evaluates_to<float>("-2/3-5", "-5.666667");
  assert_parsed_expression_evaluates_to<double>("sin(3)2(4+2)", "1.6934400967184", System, Radian);
  assert_parsed_expression_evaluates_to<float>("4/2×(2+3)", "10");
  assert_parsed_expression_evaluates_to<double>("4/2×(2+3)", "10");
}
