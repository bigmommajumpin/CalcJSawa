#ifndef APPS_CONTAINER_H
#define APPS_CONTAINER_H

#include "home/app.h"
#include "graph/app.h"
#include "probability/app.h"
#include "calculation/app.h"
#include "hardware_test/app.h"
#include "on_boarding/app.h"
#include "regression/app.h"
#include "sequence/app.h"
#include "settings/app.h"
#include "statistics/app.h"
#include "apps_window.h"
#include "empty_battery_window.h"
#include "math_toolbox.h"
#include "variable_box_controller.h"
#include "exam_pop_up_controller.h"
#include "led_timer.h"
#include "battery_timer.h"
#include "usb_timer.h"
#include "suspend_timer.h"
#include "backlight_dimming_timer.h"

#define USE_PIC_VIEW_APP 0
#if USE_PIC_VIEW_APP
#include "picview/picview_app.h"
#endif

class AppsContainer : public Container {
public:
  AppsContainer();
  ~AppsContainer();
  /* Rule of 5: the copy constructor, move constructor, copy assignment
   * operator and move assignment operator are deleted as their default
   * implementation does not create new apps. The new object thus become
   * obsolete as soon as the copy is deleted (because of our
   * implementation of deletion). To avoid any use of obsolete object,
   * we prevent to copy and assign. */
  AppsContainer(const AppsContainer& other) = delete;
  AppsContainer(AppsContainer&& other) = delete;
  AppsContainer& operator=(const AppsContainer& other) = delete;
  AppsContainer& operator=(AppsContainer&& other) = delete;
  /* */
  static bool poincareCircuitBreaker(const Poincare::Expression * e);
  int numberOfApps();
  App * appAtIndex(int index);
  App * hardwareTestApp();
  void reset();
  Poincare::Context * globalContext();
  MathToolbox * mathToolbox();
  VariableBoxController * variableBoxController();
  void suspend(bool checkIfPowerKeyReleased = false);
  virtual bool dispatchEvent(Ion::Events::Event event) override;
  void switchTo(App * app) override;
  void updateBatteryState();
  void refreshPreferences();
  void displayExamModePopUp(bool activate, bool forceRedrawWindow);
  void shutdownDueToLowBattery();
  void reloadTitleBar();
  void windowRedraw();
private:
  Window * window() override;
  int numberOfTimers() override;
  Timer * timerAtIndex(int i) override;
  static constexpr int k_numberOfApps = 8;
  AppsWindow m_window;
  EmptyBatteryWindow m_emptyBatteryWindow;
#if USE_PIC_VIEW_APP
  PicViewApp m_picViewApp;
#endif
  Poincare::GlobalContext m_globalContext;
  MathToolbox m_mathToolbox;
  VariableBoxController m_variableBoxController;
  ExamPopUpController m_examPopUpController;
  LedTimer m_ledTimer;
  BatteryTimer m_batteryTimer;
  USBTimer m_USBTimer;
  SuspendTimer m_suspendTimer;
  BacklightDimmingTimer m_backlightDimmingTimer;
  OnBoarding::App * m_onBoardingApp;
  Home::App * m_homeApp;
  Graph::App * m_graphApp;
  Probability::App * m_probabilityApp;
  Calculation::App * m_calculationApp;
  HardwareTest::App * m_hardwareTestApp;
  Regression::App * m_regressionApp;
  Sequence::App * m_sequenceApp;
  Settings::App * m_settingsApp;
  Statistics::App * m_statisticsApp;
};

#endif
