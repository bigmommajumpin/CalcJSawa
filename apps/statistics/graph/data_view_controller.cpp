#include "data_view_controller.h"
#include <escher/container.h>
#include <assert.h>

namespace Statistics {

DataViewController::DataViewController(Escher::Responder * parentResponder, Escher::Responder * tabController, Escher::ButtonRowController * header, Escher::StackViewController * stackViewController, Escher::ViewController * typeViewController, Store * store) :
  ViewController(parentResponder),
  GraphButtonRowDelegate(header, stackViewController, this, typeViewController),
  m_store(store),
  m_selectedSeries(-1),
  m_selectedIndex(DataView::k_defaultSelectedIndex),
  m_tabController(tabController)
{
  assert(numberOfButtons(Escher::ButtonRowController::Position::Top) > 0);
}

void DataViewController::viewWillAppear() {
  ViewController::viewWillAppear();
  sanitizeSeriesIndex();

  /* Call children's viewWillAppear implementation after sanitizing selected
   * series index and before reloading banner and the dataView */
  viewWillAppearBeforeReload();

  header()->setSelectedButton(-1);
  reloadBannerView();
  dataView()->reload();
}

bool DataViewController::handleEvent(Ion::Events::Event event) {
  int selectedButton = header()->selectedButton();
  if (selectedButton >= 0) {
    if (event == Ion::Events::Up || event == Ion::Events::Back) {
      header()->setSelectedButton(-1);
      Escher::Container::activeApp()->setFirstResponder(m_tabController);
      return true;
    }
    if (event == Ion::Events::Down && hasValidSeries()) {
      header()->setSelectedButton(-1);
      Escher::Container::activeApp()->setFirstResponder(this);
      dataView()->setDisplayBanner(true);
      dataView()->selectViewForSeries(m_selectedSeries);
      highlightSelection();
      reloadBannerView();
      return true;
    }
    return buttonAtIndex(selectedButton, Escher::ButtonRowController::Position::Top)->handleEvent(event);
  }
  assert(m_selectedSeries >= 0 && hasValidSeries());
  bool isVerticalEvent = (event == Ion::Events::Down || event == Ion::Events::Up);
  if ((isVerticalEvent || event == Ion::Events::Left || event == Ion::Events::Right)) {
    int direction = (event == Ion::Events::Up || event == Ion::Events::Left) ? -1 : 1;
    if (isVerticalEvent ? moveSelectionVertically(direction) : moveSelectionHorizontally(direction)) {
      if (reloadBannerView()) {
        dataView()->reload();
      }
      return true;
    }
  }
  return false;
}

void DataViewController::didEnterResponderChain(Responder * firstResponder) {
  if (!hasValidSeries() || !dataView()->curveViewForSeries(m_selectedSeries)->isMainViewSelected()) {
    header()->setSelectedButton(0);
  } else {
    assert(seriesIsValid(m_selectedSeries));
    dataView()->setDisplayBanner(true);
    dataView()->selectViewForSeries(m_selectedSeries);
    highlightSelection();
  }
}

void DataViewController::willExitResponderChain(Responder * nextFirstResponder) {
  if (nextFirstResponder == m_tabController) {
    assert(m_tabController != nullptr);
    if (header()->selectedButton() >= 0) {
      header()->setSelectedButton(-1);
    } else if (hasValidSeries()) {
      assert(m_selectedSeries >= 0);
      dataView()->deselectViewForSeries(m_selectedSeries);
      dataView()->setDisplayBanner(false);
    }
  }
}

int DataViewController::numberOfValidSeries() const {
  int nonEmptySeriesCount = 0;
  for (int i = 0; i < Store::k_numberOfSeries; i++) {
    nonEmptySeriesCount += seriesIsValid(i);
  }
  return nonEmptySeriesCount;
}

int DataViewController::validSeriesIndex(int series) const {
  assert(seriesIsValid(series));
  int index = 0;
  for (int i = 0; i < series; i++) {
    index += seriesIsValid(i);
  }
  return index;
}

int DataViewController::indexOfKthValidSeries(int k) const {
  assert(k >= 0 && k < numberOfValidSeries());
  int validSeriesCount = 0;
  for (int i = 0; i < Store::k_numberOfSeries; i++) {
    if (seriesIsValid(i)) {
      if (validSeriesCount == k) {
        return i;
      }
      validSeriesCount++;
    }
  }
  assert(false);
  return 0;
}

void DataViewController::sanitizeSeriesIndex() {
  // Sanitize m_selectedSeries
  if (m_selectedSeries < 0 || !seriesIsValid(m_selectedSeries)) {
    for (int series = 0; series < Store::k_numberOfSeries; series++) {
      if (seriesIsValid(series)) {
        m_selectedSeries = series;
        return;
      }
    }
    assert(false);
  }
}

bool DataViewController::moveSelectionVertically(int direction) {
  int nextSelectedSubview = validSeriesIndex(m_selectedSeries) + direction;
  if (nextSelectedSubview >= numberOfValidSeries()) {
    return false;
  }
  dataView()->deselectViewForSeries(m_selectedSeries);
  if (nextSelectedSubview < 0) {
    dataView()->setDisplayBanner(false);
    header()->setSelectedButton(0);
  } else {
    m_selectedSeries = indexOfKthValidSeries(nextSelectedSubview);
    m_selectedIndex = DataView::k_defaultSelectedIndex;
    dataView()->selectViewForSeries(m_selectedSeries);
    highlightSelection();
  }
  return true;
}

}
