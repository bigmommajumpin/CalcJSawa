#include <quiz.h>
#include <apps/shared/global_context.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <cmath>
#include "../equation_store.h"

#include "helpers.h"

using namespace Solver;
using namespace Poincare;

// Private sub-helpers

template <typename T>
void solve_and_process_error(std::initializer_list<const char *> equations, T && lambda) {
  Shared::GlobalContext globalContext;
  EquationStore equationStore;
  for (const char * equation : equations) {
    Ion::Storage::Record::ErrorStatus err = equationStore.addEmptyModel();
    quiz_assert_print_if_failure(err == Ion::Storage::Record::ErrorStatus::None, equation);
    Ion::Storage::Record record = equationStore.recordAtIndex(equationStore.numberOfModels()-1);
    Shared::ExpiringPointer<Equation> model = equationStore.modelForRecord(record);
    model->setContent(equation, &globalContext);
  }
  bool replaceFunctionsButNotSymbols = false;
  EquationStore::Error err = equationStore.exactSolve(&globalContext, &replaceFunctionsButNotSymbols);
  lambda(&equationStore, err);
  equationStore.removeAll();
}

template <typename T>
void solve_and(std::initializer_list<const char *> equations, T && lambda) {
  solve_and_process_error(equations, [lambda](EquationStore * store, EquationStore::Error error) {
    quiz_assert(error == NoError);
    lambda(store);
  });
}


// Helpers

void assert_solves_to_error(const char * equation, EquationStore::Error error) {
  solve_and_process_error({equation},[error](EquationStore * store, EquationStore::Error e){
    quiz_assert(e == error);
  });
}

void assert_solves_to_infinite_solutions(std::initializer_list<const char *> equations) {
  solve_and(equations, [](EquationStore * store){
    quiz_assert(store->numberOfSolutions() == INT_MAX);
  });
}

void assert_solves_to(std::initializer_list<const char *> equations, std::initializer_list<const char *> solutions) {
  solve_and(equations, [solutions](EquationStore * store){
    Shared::GlobalContext globalContext;
    int i = 0;
    for (const char * solution : solutions) {
      // Solutions are specified under the form "foo=bar"
      constexpr int maxSolutionLength = 100;
      char editableSolution[maxSolutionLength];
      strlcpy(editableSolution, solution, maxSolutionLength);

      char * equal = strchr(editableSolution, '=');
      quiz_assert(equal != nullptr);
      *equal = 0;

      const char * expectedVariable = editableSolution;
      if (store->type() != EquationStore::Type::PolynomialMonovariable) {
        /* For some reason the EquationStore returns up to 3 results but always
         * just one variable, so we don't check variable name...
         * TODO: Change this poor behavior. */
        const char * obtainedVariable = store->variableAtIndex(i);
        quiz_assert(strcmp(obtainedVariable, expectedVariable) == 0);
      }

      /* Now for the ugly part!
       * At the moment, the EquationStore doesn't let us retrieve solutions as
       * Expression. We can only get Layout. It somewhat makes sense for how it
       * is used in the app, but it's a nightmare to test, so changing this
       * behavior is a TODO. */

      const char * expectedValue = equal + 1;

      Preferences::ComplexFormat complexFormat = Preferences::sharedPreferences()->complexFormat();

      /* We want to give complex results to equations that explicitely use 𝐢
       * As a result, we need to enforce a non-real complex format here. */
      if (complexFormat == Preferences::ComplexFormat::Real) {
        complexFormat = Preferences::ComplexFormat::Cartesian;
      }

      /* We're pretty much reinventing ParseAndSimplify here.
       * But for some reason, we really need to call simplifyAndApproximate,
       * otherwise simplification of Polar numbers don't work. For instance,
       * ParseAndSimplify("𝐢") will yield "𝐢", even in Polar mode!
       * We're using the same weird trick as in assert_parsed_expression_simplify_to
       * TODO: Fix ParseAndSimplify */

      Expression expectedExpression = Expression::Parse(expectedValue, &globalContext, false);
      quiz_assert(!expectedExpression.isUninitialized());
      expectedExpression.simplifyAndApproximate(
        &expectedExpression,
        nullptr,
        &globalContext,
        complexFormat,
        Preferences::sharedPreferences()->angleUnit(),
        ExpressionNode::SymbolicComputation::ReplaceAllDefinedSymbolsWithDefinition
      );

      Layout expectedLayout = expectedExpression.createLayout(Preferences::PrintFloatMode::Decimal, 5);
      Layout obtainedLayout = store->exactSolutionLayoutAtIndex(i, true);
#if 0
      // Uncomment this if you need to see why a test fails using a debugger
      constexpr int bufferSize = 200;
      char debugExpectedLayout[bufferSize];
      char debugObtainedLayout[bufferSize];
      expectedLayout.serializeForParsing(debugExpectedLayout, bufferSize);
      obtainedLayout.serializeForParsing(debugObtainedLayout, bufferSize);
#endif
      quiz_assert(obtainedLayout.isIdenticalTo(expectedLayout));

      i++;
    }
    quiz_assert(store->numberOfSolutions() == i);
  });
}

void assert_solves_numerically_to(const char * equation, double min, double max, std::initializer_list<double> solutions, const char * variable) {
  solve_and_process_error({equation},[min,max,solutions,variable](EquationStore * store, EquationStore::Error e){
    Shared::GlobalContext globalContext;
    quiz_assert(e == RequireApproximateSolution);
    store->setIntervalBound(0, min);
    store->setIntervalBound(1, max);
    store->approximateSolve(&globalContext, false);

    quiz_assert(strcmp(store->variableAtIndex(0), variable)== 0);
    int i = 0;
    for (double solution : solutions) {
      quiz_assert(std::fabs(store->approximateSolutionAtIndex(i++) - solution) < 1E-5);
    }
    quiz_assert(store->numberOfSolutions() == i);
  });
}

void set_complex_format(Preferences::ComplexFormat format) {
  Preferences::sharedPreferences()->setComplexFormat(format);
}
void reset_complex_format() {
  Preferences defaultPreferences;
  Preferences::sharedPreferences()->setComplexFormat(defaultPreferences.complexFormat());
}

void set(const char * variable, const char * value) {
  const char * assign = "→";

  char buffer[32];
  assert(strlen(value) + strlen(assign) + strlen(variable) < sizeof(buffer));

  buffer[0] = 0;
  strlcat(buffer, value, sizeof(buffer));
  strlcat(buffer, assign, sizeof(buffer));
  strlcat(buffer, variable, sizeof(buffer));

  Shared::GlobalContext globalContext;
  Expression::ParseAndSimplify(
    buffer,
    &globalContext,
    Preferences::sharedPreferences()->complexFormat(),
    Preferences::sharedPreferences()->angleUnit()
  );
}

void unset(const char * variable) {
  // The variable is either an expression or a function
  Ion::Storage::sharedStorage()->destroyRecordWithBaseNameAndExtension(variable, "exp");
  Ion::Storage::sharedStorage()->destroyRecordWithBaseNameAndExtension(variable, "func");
}
