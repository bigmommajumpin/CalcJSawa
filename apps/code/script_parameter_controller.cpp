#include "script_parameter_controller.h"

#include "menu_controller.h"

using namespace Escher;

namespace Code {

ScriptParameterController::ScriptParameterController(
    Responder *parentResponder, I18n::Message title,
    MenuController *menuController)
    : SelectableListViewController(parentResponder),
      m_pageTitle(title),
      m_executeScript(I18n::Message::ExecuteScript),
      m_renameScript(I18n::Message::Rename),
      m_deleteScript(I18n::Message::DeleteScript),
      m_script(Ion::Storage::Record()),
      m_menuController(menuController) {
  m_autoImportScript.label()->setMessage(I18n::Message::AutoImportScript);
  m_autoImportScript.subLabel()->setMessage(
      I18n::Message::AutoImportScriptSubLabel);
}

void ScriptParameterController::setScript(Script script) { m_script = script; }

void ScriptParameterController::dismissScriptParameterController() {
  m_script = Script(Ion::Storage::Record());
  stackViewController()->pop();
}

const char *ScriptParameterController::title() {
  return I18n::translate(m_pageTitle);
}

bool ScriptParameterController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE) {
    Script s = m_script;
    switch (selectedRow()) {
      case 0:
        dismissScriptParameterController();
        m_menuController->openConsoleWithScript(s);
        return true;
      case 1:
        dismissScriptParameterController();
        m_menuController->renameSelectedScript();
        return true;
      case 2:
        m_script.toggleAutoImportation();
        resetMemoization();
        m_selectableListView.reloadData();
        m_menuController->reloadConsole();
        Container::activeApp()->setFirstResponder(&m_selectableListView);
        return true;
      case 3:
        dismissScriptParameterController();
        m_menuController->deleteScript(s);
        m_menuController->reloadConsole();
        return true;
      default:
        assert(false);
        return false;
    }
  }
  return false;
}

void ScriptParameterController::viewWillAppear() {
  ViewController::viewWillAppear();
  resetMemoization();
  m_selectableListView.reloadData();
  m_selectableListView.selectCell(0);
}

void ScriptParameterController::didBecomeFirstResponder() {
  selectCell(0);
  SelectableListViewController<
      MemoizedListViewDataSource>::didBecomeFirstResponder();
}

HighlightCell *ScriptParameterController::reusableCell(int index, int type) {
  assert(index >= 0);
  assert(index < k_totalNumberOfCell);
  HighlightCell *cells[k_totalNumberOfCell] = {
      &m_executeScript, &m_renameScript, &m_autoImportScript, &m_deleteScript};
  return cells[index];
}

void ScriptParameterController::willDisplayCellForIndex(HighlightCell *cell,
                                                        int index) {
  if (cell == &m_autoImportScript && !m_script.isNull()) {
    m_autoImportScript.accessory()->setState(m_script.autoImportation());
  }
}

StackViewController *ScriptParameterController::stackViewController() {
  return static_cast<StackViewController *>(parentResponder());
}

}  // namespace Code
