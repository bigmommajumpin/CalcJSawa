#ifndef PROBABILITY_ABSTRACT_INPUT_CATEGORICAL_CONTROLLER_H
#define PROBABILITY_ABSTRACT_INPUT_CATEGORICAL_CONTROLLER_H

#include <escher/selectable_table_view.h>

#include <escher/button_delegate.h>
#include "input_categorical_view.h"
#include <apps/shared/page_controller.h>
#include "probability/models/statistic/chi2_statistic.h"

namespace Probability {

/* This is the common Controller between Homogeneity and Goodness input controllers. It parses
 * significance level input and own the content view. */
class InputCategoricalController : public Shared::Page,
                                   public Shared::ParameterTextFieldDelegate,
                                   public Escher::ButtonDelegate,
                                   public Escher::SelectableTableViewDelegate {
public:
  InputCategoricalController(Escher::StackViewController * parent,
                             Shared::Page * resultsController,
                             Chi2Statistic * statistic,
                             Escher::InputEventHandlerDelegate * inputEventHandlerDelegate);

  virtual TableViewController * tableViewController() = 0;

  // TextFieldDelegate
  bool textFieldShouldFinishEditing(Escher::TextField * textField, Ion::Events::Event event) override;
  bool textFieldDidFinishEditing(Escher::TextField * textField,
                                 const char * text,
                                 Ion::Events::Event event) override;

  // Responder
  void didEnterResponderChain(Responder * previousResponder) override;
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;
  // ButtonDelegate
  bool buttonAction() override;

  // ViewController
  Escher::View * view() override { return &m_contentView; }
  Escher::ViewController::TitlesDisplay titlesDisplay() override {
    return Escher::ViewController::TitlesDisplay::DisplayLastTitle;
  }

  // SelectableTableViewDelegate
  void tableViewDidChangeSelectionAndDidScroll(Escher::SelectableTableView * t,
                                               int previousSelectedCellX,
                                               int previousSelectedCellY,
                                               bool withinTemporarySelection) override;

protected:
  int stackTitleStyleStep() const override { return 0; }
  Chi2Statistic * m_statistic;
  Shared::Page * m_resultsController;
  InputCategoricalView m_contentView;
};

}  // namespace Probability

#endif /* PROBABILITY_ABSTRACT_INPUT_CATEGORICAL_CONTROLLER_H */
