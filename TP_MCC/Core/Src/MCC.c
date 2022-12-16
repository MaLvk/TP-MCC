/**
  ******************************************************************************
  * @file    MCC.c
  * @brief   MCC program body
  ******************************************************************************
  *
  *      Author: Maelle and Felix
  *
  ******************************************************************************
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MCC.h"
#include "gpio.h"
#include "tim.h"

/**
  * @brief Fonction qui démarre le moteur en générant les quatres PWM.
  *
  * Le moteur reste à l'arrêt.
  * @param None
  * @retval None
  */
void startMotor(){
	  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
	  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
	  speed(512);
}

/**
  * @brief Fonction qui arrête le moteur en stoppant la génération de signaux PWM.
  * @param None
  * @retval None
  */
void stopMotor(){
	  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
	  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
	  HAL_Delay(5);
}

/**
  * @brief Fonction qui contrôle la vitesse de rotation du moteur.
  * @param alpha Nombre entier qui contrôle la vitesse.
  * 	- alpha = 0 : vitesse maximum dans un sens de rotation.
  * 	- alpha = 1024 : vitesse maximale dans l'autre sens de rotation.
  * 	- alpha = 512 : vitesse initiale, le moteur est à l'arrêt.
  * @retval None
  */
void speed(int alpha){
	if(alpha >= 1024){alpha=1023;}
	if(alpha <= 0){alpha = 0;}
	TIM1 -> CCR1 = alpha;
	TIM1 -> CCR2 = 1024-alpha;
}

/**
  * @brief Fonction qui permet d'initialiser la MCC.
  *
  * Cette fonction est à lancer avant toutes les autres.
  * Elle permet de réinitialiser le système en activant la ligne ISO_RESET à l'état actif.
  * Une fois réalisée, le système est prêt à être utilisé.
  * @param None
  * @retval None
  */
void init(){
	  HAL_GPIO_WritePin(ISO_RESET_GPIO_Port, ISO_RESET_Pin, SET);
	  HAL_Delay(10);
	  HAL_GPIO_WritePin(ISO_RESET_GPIO_Port, ISO_RESET_Pin, RESET);
}
