/**
  ******************************************************************************
  * @file    shell.c
  * @brief   This file provides code for shell interface
  ******************************************************************************
  *  Created on: Nov 7, 2022
  *      Author: nicolas
  *
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "usart.h"
#include "MCC.h"


const uint8_t prompt[]="user@Nucleo-STM32G474>>";
const uint8_t initialisation[]="Motor initialization\r\n";
const uint8_t started[]=
		"\r\n*-----------------------------*"
		"\r\n| Welcome on Nucleo-STM32G474 |"
		"\r\n*-----------------------------*"
		"\r\n";
const uint8_t newline[]="\r\n";
const uint8_t help[]="\r\nYou can use the following functions :"
		"\r\n pinout : To show the list of the pin."
		"\r\n init : To initialize the motor."
		"\r\n start : To start the motor."
		"\r\n stop : To stop the motor."
		"\r\n speed = alpha : To set the motor speed to alpha."
		"\r\n       - alpha = 0 : maximum speed in one direction of rotation."
		"\r\n       - alpha = 1024 : maximum speed in the other direction of rotation."
		"\r\n       - alpha = 512 : initial speed, the motor is stopped."
		"\r\n /!| Too great a difference in speed will cause the motor to stop."
		"\r\n get ADC : To show the current."
		"\r\n get pos : To show the speed."
		"\r\n";
const uint8_t pinout[]="\r\nList of the pins :"
		"\r\n*-----------------------------------------------------------*"
		"\r\n|20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39|"
		"\r\n|1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 x |"
		"\r\n*-----------------------------------------------------------*"
		"\r\n 13 : Red Phase Top -> CH1 (PA8)"
		"\r\n 12 : Yellow Phase Top -> CH2 (PA9)"
		"\r\n 31 : Red Phase Bottom -> CH1N (PA11)"
		"\r\n 30 : Yellow Phase Bottom -> CH2N (PA12)"
		"\r\n 33 : Fault Reset Command -> ISO_RESET (PC3)"
		"\r\n 16 : Yellow Phase Hall Current -> ADC (PC2)"
		"\r\n B  : Encodeur -> TIM3_CH2 (PA4)"
		"\r\n A  : Encodeur -> TIM3_CH1 (PA6)"
		"\r\n";

const uint8_t powerOn[]="Power ON\r\n";			// Fait demarrer le moteur
const uint8_t powerOff[]="Power OFF\r\n";		// Fait arrêter le moteur
const uint8_t cmdNotFound[]="Command not found\r\n";

char cmdBuffer[CMD_BUFFER_SIZE];
extern uint8_t 	uartRxBuffer[UART_RX_BUFFER_SIZE];
uint8_t	idxCmd;
char* argv[MAX_ARGS];
uint8_t	argc;
extern uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];
extern uint8_t stringSize;

//FLAG
extern int adcFlag;
extern int printAdcFlag;
extern int encoderFlag;
extern float consigneCurrent;


/**
  * @brief  Send a stating message
  * @retval None
  */
void shellInit(void){
	HAL_UART_Transmit(&huart2, started, sizeof(started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
}

/**
  * @brief  Send the prompt
  * @retval None
  */
void shellPrompt(void){
	HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
}

/**
  * @brief  Send the default message if the command is not found
  * @retval None
  */
void shellCmdNotFound(void){
	HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
}

/**
  * @brief  Function called for saving the new character and call and setup argc and argv variable if ENTER is pressed
  * @retval 1 if a new command is available, 0 if not.
  */
uint8_t shellGetChar(void){
	uint8_t newCmdReady = 0;
	char* token;

	switch(uartRxBuffer[0]){
		// If Enter, update argc and argv
	case ASCII_CR:
		HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
		cmdBuffer[idxCmd] = '\0';
		argc = 0;
		token = (char*)strtok(cmdBuffer, " ");
		while(token!=NULL){
			argv[argc++] = token;
			token = (char*)strtok(NULL, " ");
		}

		idxCmd = 0;
		newCmdReady = 1;
		break;
		// Delete last character if "return" is pressed
	case ASCII_BS:
		cmdBuffer[idxCmd--] = '\0';
		HAL_UART_Transmit(&huart2, uartRxBuffer, 1, HAL_MAX_DELAY);
		break;
		// Default state : add new character to the command buffer
	default:
		cmdBuffer[idxCmd++] = uartRxBuffer[0];
		HAL_UART_Transmit(&huart2, uartRxBuffer, 1, HAL_MAX_DELAY);
	}

	return newCmdReady;
}

/**
  * @brief  Call function depends of the value of argc and argv
  * @retval None
  */
void shellExec(void){
	// Affiche la liste des commandes utilisables
	if(strcmp(argv[0],"help")==0){
	  HAL_UART_Transmit(&huart2, help, sizeof(help), HAL_MAX_DELAY);
	}
  // Affiche la liste des pins configurés
	else if(strcmp(argv[0],"pinout")==0){
	  HAL_UART_Transmit(&huart2, pinout, sizeof(pinout), HAL_MAX_DELAY);
	}
  // Démarre le moteur
	else if(strcmp(argv[0],"start")==0){
	  HAL_UART_Transmit(&huart2, powerOn, sizeof(powerOn), HAL_MAX_DELAY);
	  startMotor();
	}
  // Stop le moteur
	else if(strcmp(argv[0],"stop")==0){
	  HAL_UART_Transmit(&huart2, powerOff, sizeof(powerOff), HAL_MAX_DELAY);
	  stopMotor();
	}
  // Initialise le moteur
	else if(strcmp(argv[0],"init")==0){
	  HAL_UART_Transmit(&huart2, initialisation, sizeof(initialisation), HAL_MAX_DELAY);
	  init();
	}
  // Fonction cachée de débugage du shell
	else if(strcmp(argv[0],"set")==0){
	  if(strcmp(argv[1],"PA5")==0){
		  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, atoi(argv[2]));
		  sprintf((char *)uartTxBuffer,"Switch on/off led : %d\r\n",atoi(argv[2]));
		  HAL_UART_Transmit(&huart2, uartTxBuffer, 64, HAL_MAX_DELAY);
	  }
	  else{
		  HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
	  }
	}
  // Afficher les valeurs de l'ADC
	else if(strcmp(argv[0],"get")==0){
	  if(strcmp(argv[1],"ADC")==0){
		  printAdcFlag = 1;
	  }
	  else if(strcmp(argv[1],"pos")==0){
		  encoderFlag = 1;
	  }
	  else{
		  HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
	  }
	}
	else if(strcmp(argv[0],"quit")==0){
	  printAdcFlag = 0;
	  encoderFlag = 0;
	}
  // Choisir la valeur de la vitesse de la façon "speed = XXXX" avec une valeur bornée entre 0 et 1023.
  	else if(strcmp(argv[0],"speed")==0){
	  if(strcmp(argv[1],"=")==0){
		  if(atoi(argv[2]) >= 1024){sprintf((char *)uartTxBuffer,"The motor speed is : %d\r\n",1023);}
		  else if(atoi(argv[2]) <= 0){sprintf((char *)uartTxBuffer,"The motor speed is : %d\r\n",0);}
		  else{sprintf((char *)uartTxBuffer,"The motor speed is : %d\r\n",atoi(argv[2]));}
		  HAL_UART_Transmit(&huart2, uartTxBuffer, 64, HAL_MAX_DELAY);
		  speed(atoi(argv[2]));
	  }
	  else{
		  HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
	  }
	}

  	else if(strcmp(argv[0],"current")==0){
  		  if(strcmp(argv[1],"=")==0){
  			  sprintf((char *)uartTxBuffer,"The motor speed is : %1.2f\r\n",(float)atof(argv[2]));
  			  HAL_UART_Transmit(&huart2, uartTxBuffer, 64, HAL_MAX_DELAY);
  			  consigneCurrent = (float)atof(argv[2]);
  	  	  }
  		  else{
  			  HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
  		  }
	}
	else{
		shellCmdNotFound();
	}
}
