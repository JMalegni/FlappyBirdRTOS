
/************************************Includes***************************************/

#include "G8RTOS/G8RTOS.h"
#include "./MultimodDrivers/multimod.h"
#include <driverlib/timer.h>

#include "./threads.h"

/************************************Includes***************************************/
/***********************************Semaphores**************************************/

semaphore_t sem_UART;

/***********************************Semaphores**************************************/

/********************************Public Functions***********************************/

extern uint32_t SystemTime;
/********************************Public Functions***********************************/


/************************************MAIN*******************************************/
int main(void) {
    // Sets clock speed to 80 MHz. You'll need it!
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    G8RTOS_Init();
    multimod_init();

    G8RTOS_InitSemaphore(&sem_PCA9555_Debounce, 0);
    G8RTOS_InitSemaphore(&sem_Joystick_Debounce, 0);
    G8RTOS_InitSemaphore(&sem_SPIA, 1);

    G8RTOS_AddThread(Play_StartScreen, 0, "");
    G8RTOS_AddThread(Check_ButtonPress, 0, "");
    G8RTOS_AddThread(Check_JoystickPress, 0, "");

    G8RTOS_Add_APeriodicEvent(GPIOE_Handler, 0, INT_GPIOE);
    G8RTOS_Add_APeriodicEvent(GPIOD_Handler, 0, INT_GPIOD);

    G8RTOS_Launch();
    while (1);
}

/************************************MAIN*******************************************/
