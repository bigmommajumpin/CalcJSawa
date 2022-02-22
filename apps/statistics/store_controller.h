#ifndef STATISTICS_STORE_CONTROLLER_H
#define STATISTICS_STORE_CONTROLLER_H

#include "store.h"
#include "statistics_context.h"
#include "../shared/store_controller.h"

namespace Statistics {

class StoreController : public Shared::StoreController {
public:
  StoreController(Escher::Responder * parentResponder, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, Store * store, Escher::ButtonRowController * header, Poincare::Context * parentContext);
  Shared::StoreContext * storeContext() override { return &m_statisticsContext; }
  bool fillColumnWithFormula(Poincare::Expression formula) override;
  void fillColumnName(int columnIndex, char * buffer) override;

private:
  Shared::ColumnParameterController * columnParameterController() override { return &m_storeParameterController; }
  bool setDataAtLocation(double floatBody, int columnIndex, int rowIndex) override;
  Store * m_store;
  StatisticsContext m_statisticsContext;
  Shared::StoreParameterController m_storeParameterController;
};

}

#endif
