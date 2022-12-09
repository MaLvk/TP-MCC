/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_TX_BUFFER_SIZE 64
#define UART_RX_BUFFER_SIZE 1
#define CMD_BUFFER_SIZE 64
#define MAX_ARGS 9
// LF = line feed, saut de ligne
#define ASCII_LF 0x0A
// CR = carriage return, retour chariot
#define ASCII_CR 0x0D
// DEL = delete
#define ASCII_DEL 0x7F

#define MAX_CCR_VALUE 1023
#define ADC_BUFFER_SIZE 64

#define ENCODER_INIT 65534/2
#define KP_CURRENT 0.29
#define KI_CURRENT 1124.88
#define KD_CURRENT 1.037
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
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
uint32_t uartRxReceived;
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];
uint32_t adcBuffer[ADC_BUFFER_SIZE];
int adcDmaFlag;
int adcValue;
int timFlag;
int encodeurFlag;
int encoder_speed;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
void init(void);
void startMotor(void);
void stopMotor(void);
void speed(int);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	char	 	cmdBuffer[CMD_BUFFER_SIZE];
	int 		idx_cmd;
	char* 		argv[MAX_ARGS];
	int		 	argc = 0;
	char*		token;
	int 		newCmdReady = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  memset(argv,0,MAX_ARGS*sizeof(char*));
  memset(cmdBuffer,0,CMD_BUFFER_SIZE*sizeof(char));
  memset(uartRxBuffer,0,UART_RX_BUFFER_SIZE*sizeof(char));
  memset(uartTxBuffer,0,UART_TX_BUFFER_SIZE*sizeof(char));

  HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
  HAL_Delay(10);
  HAL_UART_Transmit(&huart2, started, sizeof(started), HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc1, adcBuffer, 1);
  HAL_TIM_Base_Start(&htim1);

  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_Base_Start_IT(&htim2);

  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  HAL_TIM_Base_Start_IT(&htim4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // uartRxReceived is set to 1 when a new character is received on uart
	  if(uartRxReceived){
		  switch(uartRxBuffer[0]){
		  // Nouvelle ligne, instruction à traiter
		  case ASCII_CR:
			  HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
			  cmdBuffer[idx_cmd] = '\0';
			  argc = 0;
			  token = strtok(cmdBuffer, " ");
			  while(token!=NULL){
				  argv[argc++] = token;
				  token = strtok(NULL, " ");
			  }

			  idx_cmd = 0;
			  newCmdReady = 1;
			  break;
		  // Suppression du dernier caractère
		  case '\b':
			  cmdBuffer[idx_cmd--] = '\0';
			  HAL_UART_Transmit(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
			  break;
		  // Nouveau caractère
		  default:
			  cmdBuffer[idx_cmd++] = uartRxBuffer[0];
			  HAL_UART_Transmit(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
		  }
		  uartRxReceived = 0;
	  }

	  if(newCmdReady){
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
				  timFlag = 1;
			  }
			  else if(strcmp(argv[1],"pos")==0){
				  encodeurFlag = 1;
			  }
			  else{
				  HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
			  }
		  }
		  else if(strcmp(argv[0],"quit")==0){
			  timFlag = 0;
			  encodeurFlag = 0;
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
		  // Si la commande n'existe pas
		  else{
			  HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
		  }
			  HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
			  newCmdReady = 0;
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_TRGO;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 10-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1023;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 512;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 200;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 31999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 10;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 10;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 20000-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 799;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ISO_RESET_GPIO_Port, ISO_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ISO_RESET_Pin */
  GPIO_InitStruct.Pin = ISO_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ISO_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BLUE_BUTTON_Pin */
  GPIO_InitStruct.Pin = BLUE_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLUE_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback (UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}

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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	HAL_UART_Transmit(&huart2, powerOn, sizeof(powerOn), HAL_MAX_DELAY);
	startMotor();
	HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  adcDmaFlag = 1;
  adcValue = HAL_ADC_GetValue(&hadc1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim2){
		if (timFlag){
			if(adcDmaFlag){
			  sprintf((char *)uartTxBuffer, "%1.5f\r\n",((((float)adcBuffer[0])*3.3/4096)-2.53)*12);
			  HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer)*sizeof(char), HAL_MAX_DELAY);
			  adcDmaFlag = 0;
			}
		}

		if (encodeurFlag){
			sprintf((char *)uartTxBuffer, "%1.5f rad/s\r\n",(float)((encoder_speed * 10)*0.0016));
			HAL_UART_Transmit(&huart2, uartTxBuffer, strlen((char *)uartTxBuffer)*sizeof(char), HAL_MAX_DELAY);
		}
	}

	  if(htim == &htim4){
		  encoder_speed = TIM3->CNT - ENCODER_INIT;
		  TIM3->CNT = ENCODER_INIT;
	  }

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
