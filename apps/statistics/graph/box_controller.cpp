#include "box_controller.h"
#include "../app.h"
#include <poincare/print.h>

using namespace Poincare;
using namespace Shared;
using namespace Escher;

namespace Statistics {

BoxController::BoxController(Responder * parentResponder, ButtonRowController * header, Responder * tabController, Escher::StackViewController * stackViewController, Escher::ViewController * typeViewController, Store * store) :
  MultipleDataViewController(parentResponder, tabController, header, stackViewController, typeViewController, store),
  m_view(store, &m_selectedIndex),
  m_boxParameterController(nullptr, store),
  m_parameterButton(this, I18n::Message::StatisticsGraphSettings, Invocation([](void * context, void * sender) {
    BoxController * boxController = static_cast<BoxController * >(context);
    boxController->stackController()->push(boxController->boxParameterController());
    return true;
  }, this), KDFont::SmallFont)
{
}

Button * BoxController::buttonAtIndex(int index, ButtonRowController::Position position) const {
  return index == 0 ? GraphButtonRowDelegate::buttonAtIndex(index, position) : const_cast<Button *>(&m_parameterButton);
}

bool BoxController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::Toolbox) {
    stackController()->push(boxParameterController());
    return true;
  }
  return MultipleDataViewController::handleEvent(event);
}

bool BoxController::moveSelectionHorizontally(int deltaIndex) {
  return m_view.moveSelectionHorizontally(selectedSeriesIndex(), deltaIndex);
}

bool BoxController::reloadBannerView() {
  int series = selectedSeriesIndex();
  if (series < 0) {
    return false;
  }
  KDCoordinate previousHeight = m_view.bannerView()->minimalSizeForOptimalDisplay().height();

  int precision = Preferences::sharedPreferences()->numberOfSignificantDigits();
  Poincare::Preferences::PrintFloatMode displayMode = Poincare::Preferences::sharedPreferences()->displayMode();
  // With 7 = KDFont::SmallFont->glyphSize().width(), should fit in one line
  constexpr static int k_bufferSize = 1 + Ion::Display::Width / 7 - sizeof("V1/N1");
  char buffer[k_bufferSize] = "";

  // Display series name
  StoreController::FillSeriesName(series, buffer, false);
  m_view.bannerView()->seriesName()->setText(buffer);

  // Display calculation
  int selectedBoxCalculation = m_view.dataViewAtIndex(series)->selectedBoxCalculation();
  double value = m_store->boxPlotCalculationAtIndex(series, selectedBoxCalculation);
  Poincare::Print::customPrintf(
    buffer,
    k_bufferSize,
    "%s%s%*.*ed",
    I18n::translate(m_store->boxPlotCalculationMessageAtIndex(series, selectedBoxCalculation)),
    I18n::translate(I18n::Message::StatisticsColonConvention),
    value, displayMode, precision
  );
  m_view.bannerView()->calculationValue()->setText(buffer);

  m_view.bannerView()->reload();
  return previousHeight != m_view.bannerView()->minimalSizeForOptimalDisplay().height();
}

}
