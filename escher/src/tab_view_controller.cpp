extern "C" {
#include <assert.h>
}
#include <escher/tab_view_controller.h>
#include <escher/metric.h>
#include <escher/app.h>

TabViewController::ContentView::ContentView() :
  View(),
  m_activeView(nullptr)
{
};

void TabViewController::ContentView::setActiveView(View * view) {
  m_activeView = view;
  layoutSubviews();
  markRectAsDirty(bounds());
}

void TabViewController::ContentView::layoutSubviews() {
  KDRect tabViewFrame = KDRect(
      0, 0,
      m_frame.width(), Metric::TabHeight
      );
  m_tabView.setFrame(tabViewFrame);
  if (m_activeView) {
    KDRect activeViewFrame = KDRect(
        0, Metric::TabHeight,
        m_frame.width(),
        m_frame.height() - Metric::TabHeight
        );
    m_activeView->setFrame(activeViewFrame);
  }
}

int TabViewController::ContentView::numberOfSubviews() const {
  return 2;
}

View * TabViewController::ContentView::subviewAtIndex(int index) {
  if (index == 0) {
    return &m_tabView;
  } else {
    assert(index == 1);
    return m_activeView;
  }
}

#if ESCHER_VIEW_LOGGING
const char * TabViewController::ContentView::className() const {
  return "TabViewController::ContentView";
}
#endif

TabViewController::TabViewController(Responder * parentResponder, ViewController * one, ViewController * two, ViewController * three, ViewController * four) :
  ViewController(parentResponder),
  m_activeChildIndex(-1),
  m_selectedChildIndex(-1)
{
  assert(one != nullptr);
  assert(two != nullptr || (three == nullptr && four == nullptr));
  assert(three != nullptr || four == nullptr);
  m_children[0] = one;
  m_children[1] = two;
  m_children[2] = three;
  m_children[3] = four;

  m_numberOfChildren = 0;
  while (m_numberOfChildren < k_maxNumberOfChildren && m_children[m_numberOfChildren] != nullptr) {
    m_numberOfChildren++;
  }
}

int TabViewController::activeTab() const {
  return m_activeChildIndex;
}

bool TabViewController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Back) {
    if (app()->firstResponder() != this) {
      app()->setFirstResponder(this);
      return true;
    }
    return false;
  }
  if (app()->firstResponder() != this) {
    return false;
  }
  if (event == Ion::Events::Left) {
    if (m_selectedChildIndex > 0) {
      setSelectedTab(m_selectedChildIndex-1);
    }
    return true;
  }
  if (event == Ion::Events::Right) {
    if (m_selectedChildIndex < m_numberOfChildren-1) {
      setSelectedTab(m_selectedChildIndex+1);
    }
    return true;
  }
  if (event == Ion::Events::Down) {
    setActiveTab(m_activeChildIndex, false);
    return true;
  }
  if (event == Ion::Events::OK) {
    setActiveTab(m_selectedChildIndex, true);
    return true;
  }
  return false;
}

void TabViewController::setActiveTab(int8_t i, bool forceReactive) {
  assert(i >= 0 && i < m_numberOfChildren);
  ViewController * activeVC = m_children[i];
  if (i  != m_activeChildIndex || forceReactive) {
    if (i != m_activeChildIndex) {
      activeVC->loadView();
      m_view.setActiveView(activeVC->view());
    }
    m_view.m_tabView.setActiveIndex(i);
    if (m_activeChildIndex >= 0 && m_activeChildIndex != i) {
      m_children[m_activeChildIndex]->viewDidDisappear();
      m_children[m_activeChildIndex]->unloadView();
    }
    m_activeChildIndex = i;
    if (i >= 0) {
      m_children[i]->viewWillAppear();
    }
  }
  app()->setFirstResponder(activeVC);
}

void TabViewController::setSelectedTab(int8_t i) {
  if (i == m_selectedChildIndex) {
    return;
  }
  m_view.m_tabView.setSelectedIndex(i);
  m_selectedChildIndex = i;
}

void TabViewController::didEnterResponderChain(Responder * previousResponder) {
  app()->setFirstResponder(activeViewController());
}

void TabViewController::didBecomeFirstResponder() {
  setSelectedTab(m_activeChildIndex);
}

void TabViewController::willResignFirstResponder() {
  setSelectedTab(-1);
}


View * TabViewController::view() {
  return &m_view;
}

uint8_t TabViewController::numberOfTabs() {
  return m_numberOfChildren;
}

const char * TabViewController::tabName(uint8_t index) {
  return m_children[index]->title();
}

void TabViewController::viewWillAppear() {
  activeViewController()->viewWillAppear();
  m_view.m_tabView.setActiveIndex(m_activeChildIndex);
}

void TabViewController::viewDidDisappear() {
  activeViewController()->viewDidDisappear();
}

void TabViewController::loadView() {
  if (m_view.m_tabView.numberOfTabs() != m_numberOfChildren) {
    for (int i=0; i<m_numberOfChildren; i++) {
      m_view.m_tabView.addTab(m_children[i]);
    }
  }
  if (m_activeChildIndex < 0) {
    m_activeChildIndex = 0;
  }
  m_children[m_activeChildIndex]->loadView();
  m_view.setActiveView(m_children[m_activeChildIndex]->view());
}

void TabViewController::unloadView() {
  activeViewController()->unloadView();
}

ViewController * TabViewController::activeViewController() {
  assert(m_activeChildIndex >= 0 && m_activeChildIndex < m_numberOfChildren);
  return m_children[m_activeChildIndex];
}
