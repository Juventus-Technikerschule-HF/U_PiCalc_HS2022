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

uint8_t mode;

void controllerTask(void* pvParameters);
void leibniztask(void* pvParameters);
void Kelallur(void* pvParameters);
void vUITask(void *pvParameters);
float Pi_Calc = 0; 

TaskHandle_t leibniz;
TaskHandle_t Kelullar;

//EventGroup for ButtonEvents.
EventGroupHandle_t egButtonEvents = NULL;
#define BUTTON1_SHORT	0x01 //Startet Algorithmus
#define BUTTON2_SHORT	0x02 //Stoppt Algorithmus
#define BUTTON3_SHORT	0x04 //Zurücksetzen des Algorithmus
#define BUTTON4_SHORT	0x08 //Für Zustand und wechsel von Algorithmus

int main(void)
{
	vInitClock();
	vInitDisplay();
	
	xTaskCreate( controllerTask, (const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate( leibniztask, (const char *) "leibniz_task", configMINIMAL_STACK_SIZE+150, NULL, 1, &leibniz);
	xTaskCreate( Kelallur, (const char *) "Algorithmus", configMINIMAL_STACK_SIZE+150, NULL, 1, &Kelullar);
	xTaskCreate( vUITask, (const char *) "uitask", configMINIMAL_STACK_SIZE, NULL, 2, NULL); //Init UITask. Lowest Priority. Least time critical.

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"PI-Calc HS2022");
	
	vTaskStartScheduler();
	return 0;
}


//Modes for Finite State Machine
#define MODE_IDLE 0
#define MODE_Leibniz 1
#define MODE_Kelallur 2
#define MODE_Base 3
#define MODE_Time 4

void vUITask(void *pvParameters) {
	char Pi_Value[12] = ""; //Variable for Pi_Calc
	//uint16_t counter = 0;
	mode = MODE_IDLE;
		vTaskDelay(10/portTICK_RATE_MS);
		//uint8_t mode = MODE_Base;
	
	for(;;) {
		switch(mode) {
			case MODE_IDLE: 
				vDisplayClear(); //Clear Display before rewriting it
				vDisplayWriteStringAtPos(0,0,"PI-Calc HS2022"); //Draw Title
				vDisplayWriteStringAtPos(1,0, "Trying to fix this...");
				if(xEventGroupGetBits(egButtonEvents) & BUTTON1_SHORT) { //If Button1 is pressed short -> Goto MODE_Leibnitz
					mode = MODE_Leibniz;
				}
				if(xEventGroupGetBits(egButtonEvents) & BUTTON2_SHORT) { //If Button2 is pressed short -> Goto MODE_Kelallur
					mode = MODE_Kelallur;
				}
				if(xEventGroupGetBits(egButtonEvents) & BUTTON3_SHORT) { //If Button3 is pressed short -> Goto MODE_Base
					mode = MODE_Base;
				}
				
			//	if (counter == 200 || 400 || 600){ //Für die Zählung der Zeit
			//		mode = MODE_Time
			//		if (counter >= 600){
			//			counter = 0;
			//		}
			//	}
				
				if(xEventGroupGetBits(egButtonEvents) & BUTTON4_SHORT) {
					eTaskState state = eTaskGetState(leibniz);
					if(state == eSuspended){
						vTaskSuspend(Kelullar);
						vTaskResume(leibniz);
					}	else {
						vTaskSuspend(leibniz);
						vTaskResume(Kelullar);
					}
							

			break;
			
			case MODE_Leibniz:
			
				sprintf(Pi_Value,"%.7f",Pi_Calc);
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0,"Leibniz");
				vDisplayWriteStringAtPos(1,0,"Value: %s", Pi_Value);
				vDisplayWriteStringAtPos(2,0, "Time Calc:"); 
				vDisplayWriteStringAtPos(3,0, "Start|Stop|Reset|ChgMode");
			break;
			case MODE_Kelallur:
			
				sprintf(Pi_Value,"%.7f",Pi_Calc);
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0,"Kelallur");
				vDisplayWriteStringAtPos(1,0,"Value: %s", Pi_Value);
				vDisplayWriteStringAtPos(2,0, "Time Calc:"); 
				vDisplayWriteStringAtPos(3,0, "Start|Stop|Reset|ChgMode");
			break;
			case MODE_Base: //Homescreen bzw. Menü Auswahl
			 
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0, "Calculate Pi");
				vDisplayWriteStringAtPos(1,0, "Start by pressing S1");
				vDisplayWriteStringAtPos(2,0, "Time:");
				vDisplayWriteStringAtPos(3,0, "Start|Stop|Reset|ChgMode");
			
				break;
			}
		}	
		vTaskDelay(500);
	}
}
	
void leibniztask(void* pvParameters) {
	vTaskSuspend(leibniz); 
	float pi4 = 1;
	//float pi = 0;
	uint32_t n = 3;
	for(;;){
		pi4 = pi4 -1.0/n + 1.0/(n+2);
		n = n+4;
		Pi_Calc = pi4 * 4;
		xEventGroupSetBits(egButtonEvents, BUTTON1_SHORT);
		vTaskDelay(100);
	}
}

void Kelallur(void* pvParameters) {
	float pi5  = 3;
	uint32_t n = 3;
	for(;;){
		pi5 = pi5  + (4/(pow(n, 3) - n)) - (4/(pow((n+2), 3) - (n+2)));
		n = n+4;
		Pi_Calc = pi5;
		xEventGroupSetBits(egButtonEvents, BUTTON2_SHORT);
		vTaskDelay(100);
	}
}
		
void controllerTask(void* pvParameters) {
	egButtonEvents = xEventGroupCreate();
	initButtons();
	for(;;) {
		updateButtons();
		if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
		}
		updateButtons();
		
		if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON1_SHORT);
			//mode = MODE_Leibniz;
		}
		if(getButtonPress(BUTTON2) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON2_SHORT);
			//mode = MODE_Kelallur;
		}
		if(getButtonPress(BUTTON3) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON3_SHORT);
			//mode = MODE_Base;
		}
		if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON4_SHORT);
		}
		vTaskDelay(10);
	}
}