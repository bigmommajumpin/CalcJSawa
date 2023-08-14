#ifndef CALCULATION_ADDITIONAL_OUTPUTS_VECTOR_LIST_CONTROLLER_H
#define CALCULATION_ADDITIONAL_OUTPUTS_VECTOR_LIST_CONTROLLER_H

#include "illustrated_expressions_list_controller.h"
#include "vector_graph_cell.h"
#include "vector_model.h"

namespace Calculation {

class VectorListController : public IllustratedExpressionsListController {
 public:
  VectorListController(EditExpressionController* editExpressionController)
      : IllustratedExpressionsListController(editExpressionController),
        m_graphCell(&m_model) {}
  void computeAdditionalResults(
      Poincare::Expression input, Poincare::Expression exactOutput,
      Poincare::Expression approximateOutput) override;

  constexpr static Poincare::ReductionTarget k_target =
      Poincare::ReductionTarget::SystemForApproximation;
  constexpr static Poincare::SymbolicComputation k_symbolicComputation =
      Poincare::SymbolicComputation::
          ReplaceAllSymbolsWithDefinitionsOrUndefined;

 private:
  constexpr static int k_maxNumberOfOutputRows = 3;

  IllustrationCell* illustrationCell() override { return &m_graphCell; }
  I18n::Message messageAtIndex(int index) override;

  VectorGraphCell m_graphCell;
  VectorModel m_model;
};

}  // namespace Calculation

#endif
