#ifndef INPUT_GOODNESS_CONTROLLER_H
#define INPUT_GOODNESS_CONTROLLER_H

#include <apps/probability/gui/vertical_layout.h>
#include <apps/shared/button_with_separator.h>
#include <escher/buffer_table_cell.h>
#include <escher/even_odd_message_text_cell.h>
#include <escher/even_odd_editable_text_cell.h>
#include <escher/highlight_cell.h>
#include <escher/message_table_cell_with_editable_text_with_message.h>
#include <escher/responder.h>
#include <escher/selectable_table_view.h>
#include <escher/selectable_table_view_data_source.h>
#include <escher/stack_view_controller.h>
#include <escher/view.h>
#include <escher/view_controller.h>
#include <ion/events.h>
#include <kandinsky/coordinate.h>

#include "../abstract/button_delegate.h"
#include "../gui/page_controller.h"

using namespace Escher;

namespace Probability {

class InputGoodnessView : public VerticalLayout<3>, public ButtonDelegate, public Responder {
 public:
  InputGoodnessView(Responder * parentResponder, TableViewDataSource * dataSource,
                    InputEventHandlerDelegate * inputEventHandlerDelegate, TextFieldDelegate * textFieldDelegate);

  SelectableTableView * selectableTableView() { return &m_tableView; }
  MessageTableCellWithEditableTextWithMessage * significanceLevelView() { return &m_significance; }
  Shared::ButtonWithSeparator * nextButton() { return &m_next; }
  // Responder
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;

 private:
  void buttonAction() override {}
  Responder * responderForRow(int row);
  void setResponderForSelectedRow();
  void highlightViewForSelectedRow();

  constexpr static int k_indexOfTable = 0;
  constexpr static int k_indexOfSignificance = 1;
  constexpr static int k_indexOfNext = 2;

  SelectableTableView m_tableView;
  MessageTableCellWithEditableTextWithMessage m_significance;
  Shared::ButtonWithSeparator m_next;

  SelectableTableViewDataSource m_tableSelection;
  SelectableTableViewDataSource m_viewSelection;
};

// TODO member
class InputGoodnessDataSource : public TableViewDataSource {
 public:
  InputGoodnessDataSource(Responder * parent, SelectableTableView * tableView,
                          InputEventHandlerDelegate * inputEventHandlerDelegate, TextFieldDelegate * delegate);
  int numberOfRows() const override { return 4; };
  int numberOfColumns() const override { return 2; }
  int reusableCellCount(int type) override { return numberOfRows() * numberOfColumns(); }
  HighlightCell * reusableCell(int i, int type) override;
  int typeAtLocation(int i, int j) override { return 0; }
  void willDisplayCellAtLocation(HighlightCell * cell, int i, int j) override;

  // TODO needed ?
  KDCoordinate columnWidth(int i) override { return 150; }
  KDCoordinate rowHeight(int j) override { return 20; }

 private:
  struct IntRepr {
    char m_content[10];
  };

  IntRepr m_observed[5];
  IntRepr m_expected[5];
  EvenOddMessageTextCell m_header[2];
  EvenOddEditableTextCell m_cells[8];  // TODO should it contain views?
};

class InputGoodnessController : public Page, public InputGoodnessDataSource {
 public:
  InputGoodnessController(StackViewController * parent, InputEventHandlerDelegate * inputEventHandlerDelegate,
                          TextFieldDelegate * textFieldDelegate);
  View * view() override { return &m_contentView; }
  void didBecomeFirstResponder() override;

 private:
  InputGoodnessView m_contentView;
};

}  // namespace Probability

#endif /* INPUT_GOODNESS_CONTROLLER_H */
