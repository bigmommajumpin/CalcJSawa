#include "suspend_timer.h"
#include "apps_container.h"

SuspendTimer::SuspendTimer(AppsContainer * container) :
  Timer(k_idleBeforeSuspendDuration/Timer::TickDuration),
  m_container(container)
{
}

void SuspendTimer::fire() {
  m_container->suspend();
}
