/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *export PATH=$PATH:/Users/shenzi/Library/Python/3.9/bin
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "RGBMatrix_device.h"
#include "GUI_Paint.h"
#include <stdio.h>
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

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
  
  uint16_t width = 64; // 单屏宽度（像素）
  uint8_t address_size = 5; // 地址位数，例如 1/32 扫描为 5 位地址
  uint8_t bitDepth=4; // 位深设置，最大 4 位
  
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

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
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  // 微秒级延时初始化
  DWT_Init();

  // HUB75 显示屏初始化，设置宽度、地址位数、位深
  HUB75_Init(width,address_size,bitDepth);
  
  // 为显示屏分配黑色背景缓冲区内存
  if((RGB_Matrix.BlackImage = (UWORD *)malloc(RGB_Matrix.all_width * RGB_Matrix.height)) == NULL) 
  {
    printf("Failed to apply for black memory...\r\n"); // 内存分配失败，打印错误并死循环
    while(1);
  }

  // 创建新图像缓冲区，设置为黑色
  Paint_NewImage(RGB_Matrix.BlackImage,RGB_Matrix.all_width, RGB_Matrix.height, 0, BLACK);

  // 清屏，全部填充为黑色
  Paint_Clear(BLACK);
  
  // 彩虹色竖线扫描测试，防止某一列灯不亮，仅取 RGB565 高四位
  for(uint16_t x=0;x < width + 1;x++ ){
    if((Wheel(x) & 0xFE1F)  == 0) // 仅取高四位，避免某列不亮
    {
      Paint_DrawLine(x, 0, x, 64, Wheel(x-1) , DOT_PIXEL_1X1, LINE_STYLE_SOLID); // 画竖线
    }
    else
      Paint_DrawLine(x, 0, x, 64, Wheel(x), DOT_PIXEL_1X1, LINE_STYLE_SOLID);
  }
  HUB75_show(); // 刷新显示屏
  HAL_Delay(1000); // 延时 1 秒

  // 清屏后绘制十字和圆点
  Paint_Clear(BLACK);
  Paint_DrawLine(20, 11, 20, 31, Wheel(1), DOT_PIXEL_1X1, LINE_STYLE_SOLID); // 竖线
  Paint_DrawLine(10, 21, 30, 21, Wheel(31), DOT_PIXEL_1X1, LINE_STYLE_SOLID); // 横线
  Paint_DrawCircle(20, 21, 10, Wheel(14), DOT_PIXEL_1X1, DRAW_FILL_EMPTY);    // 空心圆
  Paint_DrawPoint(20,21, Wheel(1) | Wheel(31), DOT_PIXEL_1X1,  DOT_FILL_RIGHTUP); // 圆心点

  Paint_DrawLine(44, 11, 44, 31, Wheel(16), DOT_PIXEL_1X1, LINE_STYLE_SOLID); // 右侧竖线
  Paint_DrawLine(34, 21, 54, 21, Wheel(63), DOT_PIXEL_1X1, LINE_STYLE_SOLID); // 右侧横线
  Paint_DrawCircle(44, 21, 10, Wheel(54), DOT_PIXEL_1X1, DRAW_FILL_EMPTY);    // 右侧空心圆
  Paint_DrawPoint(44,21, Wheel(16) | Wheel(63), DOT_PIXEL_1X1,  DOT_FILL_RIGHTUP); // 右侧圆心点

  Paint_DrawRectangle(1, 1, 64, 64, Wheel(6), DOT_PIXEL_1X1, DRAW_FILL_EMPTY); // 画边框矩形

  printf("Display string.\n"); // 串口输出提示
  Paint_DrawString_EN(5, 32, "Hello", &Font16, BLACK, YELLOW); // 英文字符串
  Paint_DrawString_EN(5, 47, "world", &Font12, MAGENTA, BLACK); // 英文字符串
  HAL_Delay(1000); // 延时 1 秒

  // 清屏后显示数字和中文
  Paint_Clear(BLACK);
  Paint_DrawNum(0, 25, 23.456789, &Font16, 2, Wheel(90), BLACK); // 显示浮点数字
  Paint_DrawString_CN(1, 1, "汉字abc", &Font12CN, Wheel(35), BLACK); // 显示中文字符串
  Paint_DrawCircle(20, 21, 10, Wheel(14), DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  Paint_DrawPoint(20,21, Wheel(1) | Wheel(31), DOT_PIXEL_1X1,  DOT_FILL_RIGHTUP);
   
  Paint_DrawLine(44, 11, 44, 31, Wheel(16), DOT_PIXEL_1X1, LINE_STYLE_SOLID);
  Paint_DrawLine(34, 21, 54, 21, Wheel(63), DOT_PIXEL_1X1, LINE_STYLE_SOLID);
  Paint_DrawCircle(44, 21, 10, Wheel(54), DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  Paint_DrawPoint(44,21, Wheel(16) | Wheel(63), DOT_PIXEL_1X1,  DOT_FILL_RIGHTUP);
  
  Paint_DrawRectangle(1, 1, 64, 64, Wheel(6), DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
  
  printf("Display string.\n");
  Paint_DrawString_EN(5, 32, "Hello", &Font16, BLACK, YELLOW);
  Paint_DrawString_EN(5, 47, "world", &Font12, MAGENTA, BLACK);
  HAL_Delay(1000);

  Paint_Clear(BLACK);
  Paint_DrawNum(0, 25, 23.456789, &Font16, 2, Wheel(90), BLACK);
  Paint_DrawString_CN(1, 1, "���abc", &Font12CN, Wheel(35), BLACK);

  /* USER CODE END 2 */

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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 34;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 3072;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

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
#ifdef USE_FULL_ASSERT
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
