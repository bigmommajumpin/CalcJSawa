#include "app.h"
#include "../apps_container.h"
#include "code_icon.h"
#include "../i18n.h"

namespace Code {

I18n::Message App::Descriptor::name() {
  return I18n::Message::CodeApp;
}

I18n::Message App::Descriptor::upperName() {
  return I18n::Message::CodeAppCapital;
}

const Image * App::Descriptor::icon() {
  return ImageStore::CodeIcon;
}

App::Snapshot::Snapshot() {
  m_programStore.addDefaultProgram();
}

App * App::Snapshot::unpack(Container * container) {
  return new App(container, this);
}

void App::Snapshot::reset() {
  m_programStore.deleteAll();
}

App::Descriptor * App::Snapshot::descriptor() {
  static Descriptor descriptor;
  return &descriptor;
}

ProgramStore * App::Snapshot::programStore() {
  return &m_programStore;
}

App::App(Container * container, Snapshot * snapshot) :
  ::App(container, snapshot, &m_codeStackViewController, I18n::Message::Warning),
  m_listFooter(&m_codeStackViewController, &m_menuController, &m_menuController, ButtonRowController::Position::Bottom, ButtonRowController::Style::EmbossedGrey),
  m_menuController(&m_listFooter, snapshot->programStore(), &m_listFooter),
  m_codeStackViewController(&m_modalViewController, &m_listFooter)
{
}

}
