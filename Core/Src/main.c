/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "syn6288.h"
#include "max7219.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t cmdStop[] = {0xfd, 0x00, 0x02, 0x02, 0xfd};
uint8_t cmdSuspend[] = {0XFD, 0X00, 0X02, 0X03, 0XFC};
uint8_t cmdResume[] = {0XFD, 0X00, 0X02, 0X04, 0XFB};
uint8_t cmdStatus[] = {0XFD, 0X00, 0X02, 0X21, 0XDE};
uint8_t cmdPowerdown[] = {0XFD, 0X00, 0X02, 0X88, 0X77};

//matrix
uint8_t YunTech_Logo[6][32] = {
	//雲
	{
		0x00,0x00,0x00,0x20,0x1F,0xF0,0x41,0x04,
		0x7F,0xFE,0x41,0x04,0x99,0x30,0x05,0x40,
		0x19,0x30,0x00,0x00,0x1F,0xF0,0x00,0x04,
		0xFF,0xFE,0x04,0x20,0x3F,0xF8,0x10,0x08
	},
	//林
	{
		/*
		0x00, 0x00 > top left, top right
		0x36, 0x70 > bottom left, bottom right
		*/
		//top
		0x00,0x00,0x18,0x30,0x10,0x20,0x14,0x24,
		0xFE,0xFE,0x10,0x20,0x30,0x60,0x38,0x70,
		//bottom
		0x36,0x70,0x52,0xA8,0x50,0xAC,0x91,0x26,
		0x12,0x24,0x10,0x20,0x10,0x20,0x10,0x20
	},
	//科
	{
		0x00,0x00,0x1C,0x18,0xF1,0x10,0x10,0xD0,
		0x14,0x50,0xFE,0x10,0x11,0x10,0x30,0xD0,
		0x38,0x50,0x54,0x14,0x50,0x1E,0x93,0xF0,
		0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10
	},
	//技
	{
		0x00,0x00,0x30,0x30,0x20,0x20,0x20,0x24,
		0xFB,0xFE,0x20,0x20,0x20,0x28,0x29,0xFC,
		0x30,0x88,0xE0,0x88,0xA0,0x50,0x20,0x50,
		0x20,0x20,0x20,0x50,0xE0,0x88,0x43,0x06
	},
	//大
	{
		0x00,0x00,0x01,0x80,0x01,0x00,0x01,0x00,
		0x01,0x04,0xFF,0xFE,0x01,0x00,0x01,0x00,
		0x03,0x00,0x02,0x80,0x04,0x40,0x04,0x40,
		0x08,0x20,0x10,0x10,0x20,0x0E,0xC0,0x04
	},
	//學
	{
		0x00,0x00,0x2A,0x88,0x31,0x3C,0x22,0x88,
		0x38,0x38,0x22,0x88,0x39,0x38,0x22,0x88,
		0x7F,0xFE,0x40,0x04,0x8F,0xE8,0x00,0x40,
		0x7F,0xFC,0x00,0x80,0x03,0x80,0x01,0x00
	}
};

//encode -> big5
uint8_t syn6288_data[] = {0x5b,0x76,0x36,0x5d,0x5b,0x6d,0x30,0x5d,0x5b,0x74,0x35,0x5d,0xb6,0xb3,0xaa,0x4c,0xac,0xec,0xa7,0xde,0xa4,0x6a,0xbe,0xc7};

uint8_t Max7219_data[4];
/*
task handle define	
*/	
TaskHandle_t handleYuntech;
TaskHandle_t handleTTS;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//matrix display
void taskYuntech(void *pvParm){
	while(1){
		for(int l = 0; l < 6; l++)//total 6 words
		{
			for(int i = 0; i < 8; i++)//one step one line > so run 8 times
			{
				Max7219_data[0] = YunTech_Logo[l][i*2];//top left 
				Max7219_data[1] = YunTech_Logo[l][i*2 + 1];//top right
				Max7219_data[2] = YunTech_Logo[l][i*2 + 16];//bottom left 
				Max7219_data[3] = YunTech_Logo[l][i*2 + 17];//bottom right
				Write_Matrix(i + 1, Max7219_data);
			}
			vTaskDelay(1000);
		}
	}
}

void taskTTS(void *pvParm){
	while(1){
		SYN_FrameInfo(0, syn6288_data);
		vTaskDelay(8000);
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_SPI1_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
	//must place after spi init
	Init_MAX7219_8x8(); 
	
	//create task
	xTaskCreate(taskYuntech, "Yuntech Logo", 512, NULL, 1, &handleYuntech);
	xTaskCreate(taskTTS, "TTS", 512, NULL, 1, &handleTTS);
	
	//start scheduler
	vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Init scheduler */

  /* Start scheduler */

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
