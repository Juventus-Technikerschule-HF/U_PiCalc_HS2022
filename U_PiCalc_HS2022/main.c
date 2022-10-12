/*
 * U_PiCalc_HS2022.c
 *
 * Created: 20.03.2018 18:32:07
 * Author : Eric Lochner
 */ 

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "avr_compiler.h"
#include "pmic_driver.h"
#include "TC_driver.h"
#include "clksys_driver.h"
#include "sleepConfig.h"
#include "port_driver.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "stack_macros.h"

#include "mem_check.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"

#include "ButtonHandler.h"


void controllerTask(void* pvParameters);
void leibniztask(void* pvParameters);
//void zweiter Algo
void vUITask(void *pvParameters);
float piCalculatet;

//EventGroup for ButtonEvents.
EventGroupHandle_t egButtonEvents = NULL;
#define BUTTON1_SHORT	0x01 //Startet Algorithmus
#define BUTTON2_SHORT	0x02 //Stoppt Algorithmus
#define BUTTON3_SHORT	0x04 //Zur�cksetzen des Algorithmus
#define BUTTON4_SHORT	0x08 //F�r Zustand un wechselvom Algorithmus
#define BUTTON_ALL		0xFF //Reset 

//EventGroup for Alarm State.
EventGroupHandle_t egPiCalc = NULL;
#define ALARMSTATE_ENABLED	0x01



int main(void)
{
	vInitClock();
	vInitDisplay();
	
	xTaskCreate( controllerTask, (const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate( leibniztask, (const char *) "leibniz_task", configMINIMAL_STACK_SIZE+150, NULL, 1, NULL);
	//xTaskCreate( Algorithmus, (const char *) "Algorithmus", configMINIMAL_STACK_SIZE+150, NULL, 1, NULL);
	xTaskCreate( vUITask, (const char *) "uitask", configMINIMAL_STACK_SIZE, NULL, 1, NULL); //Init UITask. Lowest Priority. Least time critical.

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"PI-Calc HS2022");
	
	vTaskStartScheduler();
	return 0;
}


//Modes for Finite State Machine
#define MODE_IDLE 0
#define MODE_Leibnitz 1
#define MODE_SecondAlgorythm 2



void vUITask(void *pvParameters) {
	char timestring[20]; //Variable for temporary string
	uint8_t mode = MODE_IDLE;
	while(egButtonEvents == NULL) { //Wait for EventGroup to be initialized in other task
		vTaskDelay(10/portTICK_RATE_MS);
	}
	for(;;) {
		switch(mode) {
			case MODE_IDLE: {
				vDisplayClear(); //Clear Display before rewriting it
				vDisplayWriteStringAtPos(0,0,"PI-Calc HS2022"); //Draw Title
				sprintf(timestring, "Hello"); //Writing Text into one string, or Pi Number
				//vDisplayWriteStringAtPos(1,0,"Time:       %s", &timestring[0]); //Writing Time string onto Display, now Pi Number	
				if(xEventGroupGetBits(egButtonEvents) & BUTTON1_SHORT) { //If Button1 is pressed long -> Goto MODE_Leibnitz
					mode = MODE_Leibnitz;
				}
				if(xEventGroupGetBits(egButtonEvents) & BUTTON2_SHORT) { //If Button2 is pressed long -> Goto MODE_Algorithmus
					mode = MODE_SecondAlgorythm;
				}
			}
			case MODE_Leibnitz:
			{
				vDisplayClear();
				//vDisplayWriteStringAtPos(0,0"Text 1");
				sprintf(timestring, "Hello"); 
				vDisplayWriteStringAtPos(1,0,"Hello", &timestring[0]); 
				vDisplayWriteStringAtPos(2,0, "Text"); 
			}
			case MODE_SecondAlgorythm:
			{
				vDisplayClear();
				//vDisplayWriteStringAtPos(0,0,"Set Time");
				sprintf(timestring, "Hello"); //Writing Time into one string
				//vDisplayWriteStringAtPos(1,0,"Text", &timestring[0]);
				//vDisplayWriteStringAtPos(2,0, "Text"); //Draw Button Info
			}
		}	
	
		
		
void controllerTask(void* pvParameters) {
	egButtonEvents = xEventGroupCreate();
	initButtons();
	for(;;) {
		updateButtons();
		if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
			char pistring[12];
			sprintf(&pistring[0], "PI: %.8f", piCalculatet);
			vDisplayWriteStringAtPos(1,0, "%s", pistring);
		}
		updateButtons();
		
		if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON1_SHORT);
		}
		if(getButtonPress(BUTTON2) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON2_SHORT);
		}
		if(getButtonPress(BUTTON3) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON3_SHORT);
		}
		if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON4_SHORT);
		}
		vTaskDelay(10/portTICK_RATE_MS);
	}
}

void leibniztask(void* pvParameters) {
	float pi4 = 1;
	float pi = 0;
	uint32_t n = 3;
	for(;;){
		pi4 = pi4 -1.0/n + 1.0/(n+2);
		n = n+4;
		piCalculatet = pi4 * 4;
		xEventGroupSetBits(egButtonEvents, BUTTON1_SHORT);
	}
}

//void Algorithmus...

//xEventGroupSetBits(egButtonEvents, BUTTON2_SHORT);