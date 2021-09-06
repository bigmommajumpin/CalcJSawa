#ifndef GRAPH_INTERVAL_PARAMETER_SELECTOR_CONTROLLER
#define GRAPH_INTERVAL_PARAMETER_SELECTOR_CONTROLLER

#include <apps/shared/interval_parameter_controller.h>
#include <escher/message_table_cell_with_chevron.h>
#include <escher/selectable_list_view_controller.h>
#include "../../shared/continuous_function.h"

namespace Graph {

class IntervalParameterSelectorController : public Escher::SelectableListViewController {
public:
  IntervalParameterSelectorController();
  const char * title() override;
  void viewDidDisappear() override;
  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;
  int numberOfRows() const override;
  KDCoordinate nonMemoizedRowHeight(int j) override;
  int reusableCellCount(int type) override;
  Escher::HighlightCell * reusableCell(int index, int type) override;
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override;
  void setStartEndMessages(Shared::IntervalParameterController * controller, Shared::ContinuousFunction::SymbolType symbolType);
private:
  static constexpr size_t k_numberOfSymbolTypes = Shared::ContinuousFunction::k_numberOfSymbolTypes;
  Shared::ContinuousFunction::SymbolType symbolTypeAtRow(int j) const;
  I18n::Message messageForType(Shared::ContinuousFunction::SymbolType symbolType);
  Escher::MessageTableCellWithChevron m_intervalParameterCell[k_numberOfSymbolTypes];
};

}

#endif
