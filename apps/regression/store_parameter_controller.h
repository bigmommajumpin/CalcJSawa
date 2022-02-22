#ifndef REGRESSION_STORE_PARAMETER_CONTROLLER_H
#define REGRESSION_STORE_PARAMETER_CONTROLLER_H

#include <escher/message_table_cell_with_chevron_and_expression.h>
#include "regression_controller.h"
#include "store.h"
#include "../shared/store_parameter_controller.h"

namespace Regression {

class StoreController;

class StoreParameterController : public Shared::StoreParameterController {
public:
  StoreParameterController(Escher::Responder * parentResponder, StoreController * storeController);
  bool handleEvent(Ion::Events::Event event) override;
  KDCoordinate nonMemoizedRowHeight(int index) override;
  Escher::HighlightCell * reusableCell(int index, int type) override;
  int reusableCellCount(int type) override { return type == k_regressionCellType ? 1 : Shared::StoreParameterController::reusableCellCount(type); }
  int typeAtIndex(int index) override { return index == k_indexOfRegressionCell ? k_regressionCellType : Shared::StoreParameterController::typeAtIndex(index); }
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override;
protected:
  int numberOfCells() const override { return k_newNumberOfCells; }

private:
  virtual I18n::Message sortMessage() override { return I18n::Message::SortValues; } // TODO : put this in storeParameterController of statistics when created.
  constexpr static int k_newNumberOfCells = Shared::StoreParameterController::k_numberOfCells + 1;
  constexpr static int k_indexOfRegressionCell = Shared::StoreParameterController::k_numberOfCells;
  static constexpr int k_regressionCellType = 2;

  Escher::MessageTableCellWithChevronAndExpression m_changeRegressionCell;
};

}

#endif
