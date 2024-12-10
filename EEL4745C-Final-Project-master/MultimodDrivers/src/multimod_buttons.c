// multimod_buttons.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for button functions

/************************************Includes***************************************/

#include "../multimod_buttons.h"

#include "../multimod_i2c.h"

#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_i2c.h>

/********************************Public Functions***********************************/

// Buttons_Init
// Initializes buttons on the multimod by configuring the I2C module and
// relevant interrupt pin.
// Return: void
void MultimodButtons_Init() {
    // Initialize this function & the relevant interrupt pin
	I2C_Init(I2C_A_BASE);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIODirModeSet(BUTTONS_INT_GPIO_BASE,  BUTTONS_INT_PIN, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(BUTTONS_INT_GPIO_BASE,BUTTONS_INT_PIN,GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOIntDisable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
	GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
	GPIOIntTypeSet(BUTTONS_INT_GPIO_BASE,BUTTONS_INT_PIN,GPIO_FALLING_EDGE);
	GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
}

// MultimodButtons_Get
// Gets the input to GPIO bank 1, [0..7].
// Return: uint8_t 
uint8_t MultimodButtons_Get() {
    // complete this function
	I2C_WriteSingle(I2C_A_BASE, BUTTONS_PCA9555_GPIO_ADDR, 0x00);
	return I2C_ReadSingle(I2C_A_BASE, BUTTONS_PCA9555_GPIO_ADDR);
}

