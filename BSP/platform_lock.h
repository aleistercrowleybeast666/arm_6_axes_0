#ifndef __PLATFORM_LOCK_H
#define __PLATFORM_LOCK_H
/* Task-context only; nested critical sections are supported by the RTOS port. */
void Platform_EnterCritical(void);
void Platform_ExitCritical(void);
#endif
