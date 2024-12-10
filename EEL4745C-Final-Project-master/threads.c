
/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"

#include "./sprites/bird.h"
#include "./sprites/floor.h"
#include "./sprites/background.h"
#include "./sprites/play.h"
#include "./sprites/gameover.h"
#include "./sprites/ground_pipe.h"
#include "./sprites/ceiling_pipe.h"
#include "./sprites/zero.h"
#include "./sprites/one.h"
#include "./sprites/two.h"
#include "./sprites/pause.h"

#include <stdio.h>
#include <stdlib.h>

// Change this to change the number of points that make up each line of a cube.
// Note that if you set this too high you will have a stack overflow!
#define Num_Interpolated_Points 10

// sizeof(float) * num_lines * (Num_Interpolated_Points + 2) = ?

#define MAX_NUM_CUBES           (MAX_THREADS - 3)


/*********************************Global Variables**********************************/
struct bird player = {140, 0};
struct pipe botPipe;
struct pipe topPipe;
uint16_t score = 0;
uint16_t round = 0;

uint8_t playing = 0;
uint8_t gameover = 0;
uint8_t pause = 0;
/*********************************Global Variables**********************************/

/*************************************Threads***************************************/

void Update_Player(void) {
	while(1) {

		if (pause) continue;

		if (player.birdPosY > 70) {
			G8RTOS_WaitSemaphore(&sem_SPIA);
			ST7789_DrawRectangle(70, player.birdPosY, 17*2, 15*2, 0xCE0A);
			G8RTOS_SignalSemaphore(&sem_SPIA);

			player.birdPosY -= player.birdSpeed;
			player.birdSpeed += 1;

			if (botPipe.posX > 40 && botPipe.posX < 110) {
				if (player.birdPosY < botPipe.posY + 130 || player.birdPosY > topPipe.posY - 15) {
					G8RTOS_WaitSemaphore(&sem_SPIA);
					ST7789_drawSprite(50, 170, 140, 34, gameover_sprite, 1, 0, 0);
					G8RTOS_SignalSemaphore(&sem_SPIA);
					playing = 0;
					gameover = 1;
				}
			}

		} else {
			G8RTOS_WaitSemaphore(&sem_SPIA);
			ST7789_drawSprite(50, 170, 140, 34, gameover_sprite, 1, 0, 0);
			G8RTOS_SignalSemaphore(&sem_SPIA);
			playing = 0;
			gameover = 1;
		}

		G8RTOS_WaitSemaphore(&sem_SPIA);
		ST7789_drawSprite(70, player.birdPosY, 17, 12, bird_sprite, 2, 0, 0);
		G8RTOS_SignalSemaphore(&sem_SPIA);

		sleep(60);

		if (!playing) {
			G8RTOS_KillSelf();
		}
	}
}

void Update_Pipes(void) {
	while(1) {

		if (pause) continue;

//		UARTprintf("PipX: %d\n", botPipe.posX);

		if (botPipe.posX > -45) {
			G8RTOS_WaitSemaphore(&sem_SPIA);
			ST7789_DrawRectangle(botPipe.posX + 36, 70, 9, botPipe.posY + 130 - 70, 0xCE0A);
			G8RTOS_SignalSemaphore(&sem_SPIA);

			G8RTOS_WaitSemaphore(&sem_SPIA);
			ST7789_DrawRectangle(topPipe.posX + 36, topPipe.posY, 9, Y_MAX - topPipe.posY, 0xCE0A);
			G8RTOS_SignalSemaphore(&sem_SPIA);

			botPipe.posX -= 10;
			topPipe.posX -= 10;

			//UARTprintf("PipY: %d\n", topPipe.posY);

//PIPE VERTICAL MOVEMENT TESTING

//			if (topPipe.posY > 130) {
//				topPipe.direction = 1;
//			}
/*
			if (topPipe.direction) {
				topPipe.posY += 4;
				botPipe.posY += 4;
				if (topPipe.posY > 250) topPipe.direction = 0;
			} else {
				topPipe.posY -= 4;
				botPipe.posY -= 4;
				if (topPipe.posY < 200) topPipe.direction = 1;
			}
*/
//			} else {
//				topPipe.posY -= 4;
//				botPipe.posY -= 4;
//			}

			if (topPipe.posX < 70 && !round) {
				score++;
				round++;
//				PCA9556b_SetLED(score, 0xFF, 0xFF);
			}

			G8RTOS_WaitSemaphore(&sem_SPIA);
			ST7789_drawSprite(botPipe.posX, botPipe.posY, 46, 132, ground_pipe_sprite, 1, 0, 70);
			G8RTOS_SignalSemaphore(&sem_SPIA);

			G8RTOS_WaitSemaphore(&sem_SPIA);
			ST7789_drawSprite(topPipe.posX, topPipe.posY, 46, 132, ceiling_pipe_sprite, 1, 0, 0);
			G8RTOS_SignalSemaphore(&sem_SPIA);

//TODO: FIX SCORE INCREASING
/*			if (score == 1) {
				G8RTOS_WaitSemaphore(&sem_SPIA);
				ST7789_drawSprite(107, 230, 12, 19, zero_sprite, 2, 0, 0);
				G8RTOS_SignalSemaphore(&sem_SPIA);
			} else if (score == 2) {
				G8RTOS_WaitSemaphore(&sem_SPIA);
				ST7789_drawSprite(107, 230, 12, 19, one_sprite, 2, 0, 0);
				G8RTOS_SignalSemaphore(&sem_SPIA);
			} else if (score == 3) {
				G8RTOS_WaitSemaphore(&sem_SPIA);
				ST7789_drawSprite(107, 230, 12, 19, two_sprite, 2, 0, 0);
				G8RTOS_SignalSemaphore(&sem_SPIA);
			}
*/
		} else {
			botPipe.posX = 240;
			botPipe.posY = rand() % 60;
			topPipe.posX = botPipe.posX;
			topPipe.posY = botPipe.posY + 130 + 60;
			topPipe.direction = 0;
			round = 0;
		}

		sleep(400);

		if (!playing) {
			G8RTOS_KillSelf();
		}
	}
}

void Play_StartScreen(void) {
	G8RTOS_WaitSemaphore(&sem_SPIA);
	ST7789_DrawRectangle(0, 150, 240, 170, 0xCE0A);
	ST7789_drawSprite(0, 0, 480, 70, floor_sprite, 1, 0, 0);
	ST7789_drawSprite(0, 69, 240, 88, background_sprite, 1, 0, 0);
	ST7789_drawSprite(70, 100, 100, 100, play_sprite, 1, 0, 0);
	ST7789_drawSprite(70, player.birdPosY, 17, 12, bird_sprite, 2, 0, 0);
	G8RTOS_SignalSemaphore(&sem_SPIA);
	PCA9956b_SetAllOff();
	score = 1;
	round = 0;

	uint16_t scroll = 0;

	while(1) {
		scroll += 8;

		G8RTOS_WaitSemaphore(&sem_SPIA);
		ST7789_drawSprite(0, 0, 480, 70, floor_sprite, 1, scroll % 240, 0);
		G8RTOS_SignalSemaphore(&sem_SPIA);

		if (playing) {
			G8RTOS_KillSelf();
		}
	}
}

void Check_ButtonPress(void) {
	while (1) {
		G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);

		sleep(10);

		uint8_t buttons = MultimodButtons_Get();
		if (!(buttons & SW4)) {
			if (playing) {
				player.birdSpeed = -5;
			} else {
				playing = 1;
				G8RTOS_WaitSemaphore(&sem_SPIA);
				ST7789_DrawRectangle(0, 71, 240, 200, 0xCE0A);
				G8RTOS_SignalSemaphore(&sem_SPIA);

				botPipe.posX = 280;
				botPipe.posY = rand() % 60;
				topPipe.posX = botPipe.posX;
				topPipe.posY = botPipe.posY + 130 + 60;
				topPipe.direction = 0;

			    G8RTOS_AddThread(Update_Player, 0, "");
			    G8RTOS_AddThread(Update_Pipes, 0, "");

			    player.birdSpeed = -5;
			}
		}
		if (!(buttons & SW1)) {
			if (gameover) {
				playing = 0;
				gameover = 0;
				player.birdPosY = 140;
				player.birdSpeed = 0;
				G8RTOS_AddThread(Play_StartScreen, 0, "");
			} else if (playing) {
				pause = !pause;
				if (pause) {
					G8RTOS_WaitSemaphore(&sem_SPIA);
					ST7789_drawSprite(15, 240, 15, 16, pause_sprite, 2, 0, 0);
					G8RTOS_SignalSemaphore(&sem_SPIA);
				} else {
					G8RTOS_WaitSemaphore(&sem_SPIA);
					ST7789_DrawRectangle(15, 240, 15*2, 18*2, 0xCE0A);
					G8RTOS_SignalSemaphore(&sem_SPIA);
				}
//				score++;
//				PCA9556b_SetLED(score, 0xFF, 0xFF);
			}
		}
		if (!(buttons & SW2)) {  // Add SW2 handler
		    if (playing || gameover || pause) {  // Reset game if playing or game over
                playing = 0;
                gameover = 0;
                pause = 0;
                player.birdPosY = 140;
                player.birdSpeed = 0;
                G8RTOS_AddThread(Play_StartScreen, 0, "");
            }
        }

		GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
		GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
	}
}

void Check_JoystickPress() {
    while(1) {
        G8RTOS_WaitSemaphore(&sem_Joystick_Debounce);
        sleep(10);
        if (JOYSTICK_GetPress()) {
            if (playing) {
                player.birdSpeed = -7;
            } else {
                playing = 1;
                G8RTOS_WaitSemaphore(&sem_SPIA);
                ST7789_DrawRectangle(0, 71, 240, 200, 0xCE0A);
                G8RTOS_SignalSemaphore(&sem_SPIA);

                botPipe.posX = 280;
                botPipe.posY = rand() % 60;
                topPipe.posX = botPipe.posX;
                topPipe.posY = botPipe.posY + 130 + 60;
                topPipe.direction = 0;

                G8RTOS_AddThread(Update_Player, 0, "");
                G8RTOS_AddThread(Update_Pipes, 0, "");

                player.birdSpeed = -7;
            }
        }

        GPIOIntClear(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
        GPIOIntEnable(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
    }
}

/********************************Periodic Threads***********************************/

/*******************************Aperiodic Threads***********************************/
void GPIOE_Handler() {
	GPIOIntDisable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
	G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);
}

void GPIOD_Handler() {
	GPIOIntDisable(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
	G8RTOS_SignalSemaphore(&sem_Joystick_Debounce);
}
