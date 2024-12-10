// threads.h
// Date Created: 2023-07-26
// Date Updated: 2023-07-26
// Threads

#ifndef THREADS_H_
#define THREADS_H_

/************************************Includes***************************************/

#include "./G8RTOS/G8RTOS.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/
/*************************************Defines***************************************/

/***********************************Semaphores**************************************/
semaphore_t sem_PCA9555_Debounce;
semaphore_t sem_Joystick_Debounce;
semaphore_t sem_SPIA;

/***********************************Semaphores**************************************/

/***********************************Structures**************************************/
struct bird {
	uint16_t birdPosY;
	int16_t birdSpeed;
};

struct pipe {
	int16_t posX;
	int16_t posY;
	uint8_t direction;
};
/***********************************Structures**************************************/


/*******************************Background Threads**********************************/

void Update_Player(void);
void Update_Pipes(void);
void Check_ButtonPress(void);
void Check_JoystickPress(void);
void Play_StartScreen(void);


/*******************************Background Threads**********************************/

/********************************Periodic Threads***********************************/
/********************************Periodic Threads***********************************/

/*******************************Aperiodic Threads***********************************/
void GPIOE_Handler(void);
void GPIOD_Handler(void);
/*******************************Aperiodic Threads***********************************/


#endif /* THREADS_H_ */

