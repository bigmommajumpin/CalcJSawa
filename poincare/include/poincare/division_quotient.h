#ifndef POINCARE_DIVISION_QUOTIENT_H
#define POINCARE_DIVISION_QUOTIENT_H

#include <poincare/approximation_helper.h>
#include <poincare/expression.h>
#include <poincare/layout_helper.h>

namespace Poincare {

class DivisionQuotientNode : public ExpressionNode {
public:
  static DivisionQuotientNode * FailedAllocationStaticNode();
  DivisionQuotientNode * failedAllocationStaticNode() override { return FailedAllocationStaticNode(); }

  // TreeNode
  size_t size() const override { return sizeof(DivisionQuotientNode); }
  int numberOfChildren() const override { return 2; }
#if POINCARE_TREE_LOG
  virtual void logNodeName(std::ostream & stream) const override {
    stream << "DivisionQuotient";
  }
#endif

  // ExpressionNode
  Type type() const override { return Type::DivisionQuotient; }
private:
  // Layout
  LayoutReference createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  const char * name() const { return "quo"; }
  // Simplification
  Expression shallowReduce(Context & context, Preferences::AngleUnit angleUnit) const override;
  // Evaluation
  Evaluation<float> approximate(SinglePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override { return templatedApproximate<float>(context, angleUnit); }
  Evaluation<double> approximate(DoublePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override { return templatedApproximate<double>(context, angleUnit); }
  template<typename T> Evaluation<T> templatedApproximate(Context& context, Preferences::AngleUnit angleUnit) const;
};

class DivisionQuotient : public Expression {
public:
  DivisionQuotient() : Expression(TreePool::sharedPool()->createTreeNode<DivisionQuotientNode>()) {}
  DivisionQuotient(const DivisionQuotientNode * n) : Expression(n) {}
  DivisionQuotient(Expression child1, Expression child2) : DivisionQuotient() {
    replaceChildAtIndexInPlace(0, child1);
    replaceChildAtIndexInPlace(1, child2);
  }

  // Expression
  Expression shallowReduce(Context& context, Preferences::AngleUnit angleUnit) const;
};

}

#endif

