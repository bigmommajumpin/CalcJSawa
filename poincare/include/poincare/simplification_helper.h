#ifndef SIMPLIFICATOIN_HELPER_H
#define SIMPLIFICATOIN_HELPER_H

#include "expression_node.h"
#include "expression.h"

namespace Poincare {

/* Class holding helper functions (mostly defaults)
* for expression simplication (reduce / beautify). */
class SimplificationHelper {
public:
  static void defaultDeepReduceChildren(Expression e, ExpressionNode::ReductionContext reductionContext);
  static void defaultDeepBeautifyChildren(Expression e, ExpressionNode::ReductionContext reductionContext);
  /* Handle circuit breaker and early break if should be undefined
  * Returns uninitialized handle if nothing was done, the resulting expression otherwise */
  static Expression shallowReduceUndefined(Expression e);
  /* If `e` contains units, replaces with undefined to parent and returns the undefined handle.
  * Returns uninitialized handle otherwise. */
  static Expression shallowReduceBanningUnits(Expression e);
  static Expression defaultShallowReduce(Expression e);
  /* *In place* shallowReduce while keeping the units.
  * The returned expression is the result with the units if units were handled.
  * Otherwise returns unitialized handle. */
  static Expression shallowReduceKeepingUnits(Expression e, ExpressionNode::ReductionContext reductionContext);
  static Expression shallowReduceUndefinedAndUnits(Expression e, ExpressionNode::ReductionContext reductionContext);
};
}


#endif /* SIMPLIFICATOIN_HELPER_H */
