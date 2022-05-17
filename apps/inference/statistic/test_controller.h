#ifndef PROBABILITY_CONTROLLERS_TEST_CONTROLLER_H
#define PROBABILITY_CONTROLLERS_TEST_CONTROLLER_H

#include <escher/message_table_cell_with_chevron_and_message.h>
#include <escher/selectable_list_view_controller.h>
#include <escher/stack_view_controller.h>
#include <ion/events.h>

#include "inference/models/statistic/statistic.h"

namespace Inference {

class HypothesisController;
class CategoricalTypeController;
class TypeController;
class InputController;
class InputSlopeController;

class TestController : public Escher::SelectableCellListPage<Escher::MessageTableCellWithChevronAndMessage, Statistic::k_numberOfSignificanceTestType, Escher::MemoizedListViewDataSource> {
public:
  TestController(Escher::StackViewController * parentResponder,
                 HypothesisController * hypothesisController,
                 TypeController * typeController,
                 CategoricalTypeController * categoricalController,
                 InputSlopeController * inputSlopeController,
                 InputController * inputController,
                 Statistic * statistic);
  void stackOpenPage(Escher::ViewController * nextPage) override;
  void viewWillAppear() override;
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event e) override;
  const char * title() override;
  int numberOfRows() const override;
  Escher::HighlightCell * reusableCell(int i, int type) override;

  constexpr static int k_indexOfOneProp = 0;
  constexpr static int k_indexOfOneMean = 1;
  constexpr static int k_indexOfTwoProps = 2;
  constexpr static int k_indexOfTwoMeans = 3;
  constexpr static int k_indexOfCategorical = 4;
  constexpr static int k_indexOfSlope = 5;

private:
  int virtualIndexOfSlope() const { return m_statistic->numberOfSignificancesTestTypes() - 1; }

  HypothesisController * m_hypothesisController;
  TypeController * m_typeController;
  InputController * m_inputController;
  CategoricalTypeController * m_categoricalController;
  InputSlopeController * m_inputSlopeController;
  Statistic * m_statistic;
};

}  // namespace Inference

#endif /* PROBABILITY_CONTROLLERS_TEST_CONTROLLER_H */
