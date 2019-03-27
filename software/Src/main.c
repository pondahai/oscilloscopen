
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include "ssd1306_tests.h"
#include "ssd1306.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
int adc_finish=0;
int trigger_threshold = 1950;
int volt_scale = 500;
int time_scale_position = 5;
float time_scale_step = 1;
long adc_freq =  14000000L;
int now_key = 0;
int old_key = 0;
int drawline_switch = 1;
int graph_x = 0;
int time_scale_over_5000_div_counter = 0;

//#define ADC_DATA_ARRAY_SIZE 975
#define ADC_DATA_ARRAY_SIZE 3000
#define MIDDLE_POINT ADC_DATA_ARRAY_SIZE
uint16_t find_x=MIDDLE_POINT;

int slow_adc_div_counter = 0;
//int graph_point_sampled = 0;
int key_func_group = 1;
int key_debounce_timer = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM4_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void init() {
//    ssd1306_TestAll();
    ssd1306_Init();
//    ssd1306_TestFPS();

}

void loop() {
	HAL_Delay(100);
}

uint32_t adc_datas[ADC_DATA_ARRAY_SIZE];

uint16_t get_adc_data(int index)
{
	uint16_t tmp;
	if(index % 2)
	{
		// index % 2 != 0
		tmp = ((adc_datas[index/2])&0xffff);
	}
	else
	{
		// index % 2 == 0
		tmp = ((adc_datas[index/2]>>16)&0xffff);
	}
//#define OFFSET 90
#define OFFSET 0
	if(tmp < (4096-OFFSET)) tmp += OFFSET;
	return tmp;
}
//
// scale : Volts/DIV (unit:mV)
//
uint16_t convert_adc_scale(uint16_t data,int scale)
{
	int tmp=0;
	if(scale == 100)
	{
//		tmp = data/(0.001/(3.3/4096));
//		tmp -= (4096/(0.001/(3.3/4096)) - 64) / 2;
	}
	else if(scale == 200)
	{
//		tmp = data/(0.002/(3.3/4096));
//		tmp -= (4096/(0.002/(3.3/4096)) - 64) / 2;
	}
	else if(scale == 500)
	{
		// 1/10
		// 1-(1/10) = 9/10
		// 9/10 /2 = 4.5/10
		tmp = data - (4.5*4096/10); // 4.5/10=0.45
		tmp /= (4096/10/64); // ratio = 10
	}
	else if(scale == 1000)
	{
		// 1/5
		// 1-(1/5) = 4/5
		// 4/5 /2 = 2/5
		tmp = data - (2*4096/5); // 2/5=0.4
		tmp /= (4096/5/64); // 10/2=5
	}
	else if(scale == 2000)
	{
		// 1/2.5
		// 1-(1/2.5) = 1.5/2.5
		// 1.5/2.5 /2 = 0.75/2.5 = 7.5/25
		tmp = data - (7.5*4096/25); // 7.5/25=0.3
		tmp /= (4096/2.5/64); // 10/4=2.5
	}
	else if(scale == 5000)
	{
		// 1/1
		// 1-(1/1) = 0
		// 0 /2 = 0
		tmp = data - (0);
		tmp /= (4096/1/64); // 10/10=1
	}
	else if(scale == 10000)
	{
//		tmp = data/(1.0/(3.3/4096));
//		tmp -= (4096/(1.0/(3.3/4096)) - 64) / 2;
	}
	if(tmp < 0) tmp = 0;
	return tmp;
}
void i2cClock_Reconfig(void)
{
	  // i2c clock re-config
	  hi2c1.Init.ClockSpeed = 8 * 100000;
	  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

}
//void sysTick_Reconfig(void)
//{
//    /**Configure the Systick interrupt time
//    */
//	  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/2000U);
//	    /**Configure the Systick
//	    */
//	  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
//
//	  /* SysTick_IRQn interrupt configuration */
//	  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

//	  HAL_SetTickFreq(0.5);
//}
void ADCClock_ReConfig(int div)
{

  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;


    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  if(div==14000000L)
  {
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  }
  if(div==1750000L)
  {
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV8;
  }
  if(div==437500L)
  {
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV16;
  }


  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  if(div==14000000L || div==1750000L)
  {
	  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  }
  if(div==437500L)
  {
	  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
  }

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void drawline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
//	uint16_t dx, dy, p, x, y;
	if(y0 == 0xffff) return;
	if(y1 == 0xffff) return;
	if((y0 > 64-1)) y0 = 63;
	if((y1 > 64-1)) y1 = 63;

	  int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
	  int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
	  int err = dx + dy, e2; /* error value e_xy */

	  for (;;){  /* loop */
	    //setPixel (x0,y0);
	    ssd1306_DrawPixel(x0, y0, Black);
	    if (x0 == x1 && y0 == y1) break;
	    e2 = 2 * err;
	    if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
	    if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
	  }




}

void draw_frame(void )
{
	  char str[16];
	  char str2[16];
	//
	// draw frame
	//
			  		  for(int i=0 ;i<128;i++)
			  		  {
			  			  if((i % 2) == 0)
			  				  ssd1306_DrawPixel(i, 64/2, Black);

			  			  if(i%10 == 0)
			  			  {
			  				  ssd1306_DrawPixel(i, (64/2)-60, Black);
			  				  ssd1306_DrawPixel(i, (64/2)-50, Black);
			  				  ssd1306_DrawPixel(i, (64/2)-40, Black);
			  				  ssd1306_DrawPixel(i, (64/2)-30, Black);
			  				  ssd1306_DrawPixel(i, (64/2)-20, Black);
			  				  ssd1306_DrawPixel(i, (64/2)-10, Black);
			  //				  ssd1306_DrawPixel(i, (64/2)-0, Black);
			  				  ssd1306_DrawPixel(i, (64/2)+10, Black);
			  				  ssd1306_DrawPixel(i, (64/2)+20, Black);
			  				  ssd1306_DrawPixel(i, (64/2)+30, Black);
			  				  ssd1306_DrawPixel(i, (64/2)+40, Black);
			  				  ssd1306_DrawPixel(i, (64/2)+50, Black);
			  				  ssd1306_DrawPixel(i, (64/2)+60, Black);
			  			  }
			  		  }
			  		  for(int i=0 ;i<64;i++)
			  		  {
			  			  if((i % 2) == 0)
			  			  {
			  				  ssd1306_DrawPixel(50, i, Black);
			  				  ssd1306_DrawPixel(100, i, Black);
			  			  }
			  		  }
			  		  for(int i=0 ;i<128;i++)
			  		  {
			  			  if((i % 3))
			  				  ssd1306_DrawPixel(i, convert_adc_scale(trigger_threshold,volt_scale), Black);
			  		  }
			  		  //
//			  		  drawline(127-17-1,0+1,127-1,0+1);
//			  		  drawline(127-17-1,10*6+1,127-1,10*6+1);
//			  		  drawline(127-17-1,0+1,127-17-1,10*6+1);
//			  		  drawline(127-1,0+1,127-1,10*6+1);
//			  		  drawline(127-17-1,0+1+10,127-1,0+1+10);
//			  		  drawline(127-17-1,0+1+20,127-1,0+1+20);
//			  		  drawline(127-17-1,0+1+30,127-1,0+1+30);
//			  		  drawline(127-17-1,0+1+40,127-1,0+1+40);
//			  		  drawline(127-17-1,0+1+50,127-1,0+1+50);
			  		  for(int y=0;y<0+21;y++)
//			  			  for(int x=120;x<120+28;x++)
			  			  {
			  				drawline(100,y,120+27,y);
			  			  }
			  		  for(int y=64-19;y<64;y++)
//			  			  for(int x=120;x<120+28;x++)
			  			  {
			  				drawline(100,y,120+27,y);
			  			  }
			  		  //
			  	  	  sprintf(str," %c",FONT8X9_DAHAI);
			  	  	  ssd1306_SetCursor(127-17, 0);
			  	  	  ssd1306_WriteString(str, Font_8x9, White);

			  		  switch (volt_scale)
			  		  {
			  			  case 100:
//			  				  sprintf(str,"%c",FONT5X8_0p5V);
			  				  break;
			  			  case 200:
//			  				  sprintf(str,"0.2V");
			  				  break;
			  			  case 500:
			  				  sprintf(str,"%c",FONT8X9_0p5V);
			  				  break;
			  			  case 1000:
			  				  sprintf(str,"%c",FONT8X9_1V);
			  				  break;
			  			  case 2000:
			  				  sprintf(str,"%c",FONT8X9_2V);
			  				  break;
			  			  case 5000:
			  				  sprintf(str,"%c",FONT8X9_5V);
			  				  break;
			  			  default:
			  				  break;
			  		  }
			  	  	  ssd1306_SetCursor(127-17-1, 0+11);
			  	  	  ssd1306_WriteString(str, Font_8x9, White);
			  	  	  ssd1306_SetCursor(127-8,0+11);
			  	  	  sprintf(str,"%c",FONT8X9_DIV);
			  	  	  ssd1306_WriteString(str, Font_8x9, White);

			  		  switch (time_scale_position)
			  		  {
			  			  case 5:
			  				  sprintf(str2,"%cM",FONT8X9_2);
			  				  sprintf(str,"%c%c",FONT8X9_5,FONT8X9_uS);
			  				  break;
			  			  case 10:
			  				  sprintf(str2,"%cM",FONT8X9_2);
			  				  sprintf(str,"%c%c",FONT8X9_10,FONT8X9_uS);
			  				  break;
			  			  case 20:
			  				  sprintf(str2,"%cM",FONT8X9_2);
			  				  sprintf(str,"%c%c",FONT8X9_20,FONT8X9_uS);
			  				  break;
			  			  case 50:
			  				  sprintf(str2,"%cM",FONT8X9_2);
			  				  sprintf(str,"%c%c",FONT8X9_50,FONT8X9_uS);
			  				  break;
			  			  case 100:
			  				  sprintf(str2,"%ck",FONT8X9_200);
			  				  sprintf(str,"%c%c",FONT8X9_100,FONT8X9_uS);
			  				  break;
			  			  case 200:
			  				  sprintf(str2,"%ck",FONT8X9_200);
			  				  sprintf(str,"%c%c",FONT8X9_200,FONT8X9_uS);
			  				  break;
			  			  case 500:
			  				  sprintf(str2,"%ck",FONT8X9_200);
			  				  sprintf(str,"%c%c",FONT8X9_500,FONT8X9_uS);
			  				  break;
			  			  case 1000:
			  				  sprintf(str2,"%ck",FONT8X9_20);
			  				  sprintf(str,"%c%c",FONT8X9_1,FONT8X9_mS);
			  				  break;
			  			  case 2000:
			  				  sprintf(str2,"%ck",FONT8X9_20);
			  				  sprintf(str,"%c%c",FONT8X9_2,FONT8X9_mS);
			  				  break;
			  			  case 5000:
			  				  sprintf(str2,"%ck",FONT8X9_20);
			  				  sprintf(str,"%c%c",FONT8X9_5,FONT8X9_mS);
			  				  break;
			  			  case 10000:
			  				  sprintf(str2,"%ck",FONT8X9_2);
			  				  sprintf(str,"%c%c",FONT8X9_10,FONT8X9_mS);
			  				  break;
			  			  case 20000:
			  				  sprintf(str2,"%ck",FONT8X9_2);
			  				  sprintf(str,"%c%c",FONT8X9_20,FONT8X9_mS);
			  				  break;
			  			  case 50000:
			  				  sprintf(str2,"%ck",FONT8X9_2);
			  				  sprintf(str,"%c%c",FONT8X9_50,FONT8X9_mS);
			  				  break;
			  			  case 100000:
			  				  sprintf(str2,"    ");
			  				  sprintf(str,"%c%c",FONT8X9_0p1,FONT8X9_S);
			  				  break;
			  			  case 200000:
			  				  sprintf(str2,"    ");
			  				  sprintf(str,"%c%c",FONT8X9_0p2,FONT8X9_S);
			  				  break;
			  			  case 500000:
			  				  sprintf(str2,"    ");
			  				  sprintf(str,"%c%c",FONT8X9_0p5,FONT8X9_S);
			  				  break;
			  			  default:
			  				  break;
			  		  }
			  	  	  ssd1306_SetCursor(128-25, 64-19);
			  	  	  ssd1306_WriteString(str, Font_8x9, White);
			  	  	  sprintf(str,"%c",FONT8X9_DIV);
			  	  	  ssd1306_SetCursor(128-9, 64-19);
			  	  	  ssd1306_WriteString(str, Font_8x9, White);

			  	  	  ssd1306_SetCursor(128-25, 64-10);
			  	  	  ssd1306_WriteString(str2, Font_8x9, White);
			  	  	  sprintf(str2,"%c",FONT8X9_SPS);
			  	  	  ssd1306_SetCursor(128-9, 64-10);
			  	  	  ssd1306_WriteString(str2, Font_8x9, White);
	switch(key_func_group)
	{
	case 1:
		ssd1306_SetCursor(93,44);
		ssd1306_WriteString(">", Font_8x9, White);
		break;
	case 2:
		ssd1306_SetCursor(93,9);
		ssd1306_WriteString(">", Font_8x9, White);
		break;
	case 3:
		ssd1306_SetCursor(0, convert_adc_scale(trigger_threshold,volt_scale)-4);
		ssd1306_WriteString(">", Font_8x9, White);
		break;
	default:
		break;
	}
}
void process_key(void)
{
	if(key_debounce_timer > 0)		return;

//	if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13) == GPIO_PIN_RESET) now_key = 1;
//	else if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_14) == GPIO_PIN_RESET) now_key = 2;
	 if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4) == GPIO_PIN_RESET) now_key = 1;
	else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5) == GPIO_PIN_RESET) now_key = 2;
	else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6) == GPIO_PIN_RESET) now_key = 3;
	else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7) == GPIO_PIN_RESET) now_key = 4;
	else now_key = 0;

	if(old_key != now_key)
	{
	  old_key = now_key;

	  if(now_key == 3)
	  {
		switch(key_func_group)
		{
		case 1:
			key_func_group = 2;
			break;
		case 2:
			key_func_group = 3;
			break;
		case 3:
			key_func_group = 1;
			break;
		default:
			break;
		}
	  }
	  if(now_key == 4)
	  {
		switch(key_func_group)
		{
		  case 3:
			  key_func_group = 2;
			  break;
		  case 2:
			  key_func_group = 1;
			  break;
		  case 1:
			  key_func_group = 3;
			  break;
		  default:
			  break;
		}
	  }

	  if(key_func_group == 1 && now_key == 1)
	  {
		  switch (time_scale_position)
		  {
			  case 5:
				  // change to 10us/DIV = 1us/div
				  // 14M clk = 2M SPS = 0.5us
				  adc_freq =  14000000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 10;
				  time_scale_step = 2;
				  break;
			  case 10:
//					  adc_freq =  14000000L;
//					  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 20;
				  time_scale_step = 4;
				  break;
			  case 20:
//					  adc_freq =  14000000L;
//					  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 50;
				  time_scale_step = 10;
				  break;
			  case 50:
				  // change to 100us/DIV = 10us/div
				  // 1.75M clk = 250k SPS = 4us
				  //
				  // 10us / 4us = 2.5
				  adc_freq =  1750000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 100;
				  time_scale_step = (1.25 * 2);
				  break;
			  case 100:
//					  adc_freq =  1750000L;
//					  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 200;
				  time_scale_step = (1.25 * 4);
				  break;
			  case 200:
//					  adc_freq =  1750000L;
//					  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 500;
				  time_scale_step = (1.25 * 10);
				  break;
			  case 500:
				  adc_freq = 437500L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 1000;
				  time_scale_step = (3.125 * 2);
				  break;
			  case 1000:
//					  adc_freq = 437500L;
//					  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 2000;
				  time_scale_step = (3.125 * 4);
				  break;
			  case 2000:
//					  adc_freq = 437500L;
//					  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 5000;
				  time_scale_step = (3.125 * 10);
				  break;
			  case 5000:
				  adc_freq =  14000000L;
				  time_scale_position = 10000;
				  time_scale_step = 2;
				  break;
			  case 10000:
				  time_scale_position = 20000;
				  time_scale_step = 4;
				  break;
			  case 20000:
				  time_scale_position = 50000;
				  time_scale_step = 10;
				  break;
			  case 50000:
				  time_scale_position = 100000;
				  time_scale_step = 1;
				  break;
			  case 100000:
				  time_scale_position = 200000;
				  time_scale_step = 1;
				  break;
			  case 200000:
				  time_scale_position = 500000;
				  time_scale_step = 1;
				  break;
			  case 500000:
				  adc_freq =  14000000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 5;
				  time_scale_step = 1;
				  break;
			  default:
				  break;
  		  }
		  HAL_ADC_Stop(&hadc1);
		  HAL_ADC_Stop(&hadc2);
		  HAL_ADCEx_Calibration_Start(&hadc1);
		  HAL_ADCEx_Calibration_Start(&hadc2);
//		  HAL_ADC_Start(&hadc2);
	  }
	  if(key_func_group == 1 && now_key == 2)
	  {
		  switch (time_scale_position)
		  {
			  case 5:
				  adc_freq = 14000000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 500000;
				  time_scale_step = 1;
				  break;
			  case 10:
				  adc_freq =  14000000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 5;
				  time_scale_step = 1;
				  break;
			  case 20:
				  adc_freq =  14000000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 10;
				  time_scale_step = 2;
				  break;
			  case 50:
				  adc_freq =  14000000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 20;
				  time_scale_step = 4;
				  break;
			  case 100:
				  adc_freq =  14000000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 50;
				  time_scale_step = 10;
				  break;
			  case 200:
				  adc_freq =  1750000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 100;
				  time_scale_step = (1.25 * 2);
				  break;
			  case 500:
				  adc_freq =  1750000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 200;
				  time_scale_step = (1.25 * 4);
				  break;
			  case 1000:
				  adc_freq =  1750000L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 500;
				  time_scale_step = (1.25 * 10);
				  break;
			  case 2000:
				  adc_freq = 437500L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 1000;
				  time_scale_step = (3.125 * 2);
				  break;
			  case 5000:
				  adc_freq = 437500L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 2000;
				  time_scale_step = (3.125 * 4);
				  break;
			  case 10000:
				  adc_freq = 437500L;
				  ADCClock_ReConfig(adc_freq);
				  time_scale_position = 5000;
				  time_scale_step = (3.125 * 10);
				  break;
			  case 20000:
				  time_scale_position = 10000;
				  time_scale_step = 2;
				  break;
			  case 50000:
				  time_scale_position = 20000;
				  time_scale_step = 4;
				  break;
			  case 100000:
				  time_scale_position = 50000;
				  time_scale_step = 10;
				  break;
			  case 200000:
				  time_scale_position = 100000;
				  time_scale_step = 1;
				  break;
			  case 500000:
				  time_scale_position = 200000;
				  time_scale_step = 1;
				  break;
			  default:
				  break;
  		  }
		  HAL_ADC_Stop(&hadc1);
		  HAL_ADC_Stop(&hadc2);
		  HAL_ADCEx_Calibration_Start(&hadc1);
		  HAL_ADCEx_Calibration_Start(&hadc2);
//		  HAL_ADC_Start(&hadc2);
	  }
	  if(key_func_group == 2 && now_key == 1)
	  {
		  switch(volt_scale)
		  {
			  case 500:
				  volt_scale = 1000;
				  break;
			  case 1000:
				  volt_scale = 2000;
				  break;
			  case 2000:
				  volt_scale = 5000;
				  break;
			  case 5000:
				  volt_scale = 500;
				  break;
			  default:
				  break;
		  }
	  }
	  if(key_func_group == 2 && now_key == 2)
	  {
		  switch(volt_scale)
		  {
			  case 500:
				  volt_scale = 5000;
				  break;
			  case 1000:
				  volt_scale = 500;
				  break;
			  case 2000:
				  volt_scale = 1000;
				  break;
			  case 5000:
				  volt_scale = 2000;
				  break;
			  default:
				  break;
		  }
	  }
	  if(key_func_group == 3 && now_key == 1)
	  {
		  if(trigger_threshold > ((volt_scale/10)/7.3)) trigger_threshold-=((volt_scale/10)/7.3);
	  }
	  if(key_func_group == 3 && now_key == 2)
	  {
		  if(trigger_threshold < (4096 - ((volt_scale/10)/7.3))) trigger_threshold+=((volt_scale/10)/7.3);
	  }
	  // debounce
	  key_debounce_timer = 40;
//	  for(long i=0;i<300000;i++);
	  // adc re-start
	  HAL_ADC_Start(&hadc2);
	  HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc_datas, 1);
	}// old_key != new_key

}
void process_buffer(void)
{
	  uint16_t x=0;
	  uint16_t new_y=0;
	  volatile uint16_t old_y=0;
	  uint16_t adc_data;
//		  uint16_t middle_y;
	  // first half screen after x=50
	  x=50;
	  for(float i=find_x+(1*time_scale_step); i<find_x+(1*time_scale_step)+((128-50)*time_scale_step); i+=time_scale_step)
	  {
		  adc_data = get_adc_data(i);
		  if(adc_data < 4096)
			  new_y = convert_adc_scale(adc_data,volt_scale);
		  else
			  new_y = adc_data;

		  if(drawline_switch & (x!=50))
		  {
			  drawline(x-1,old_y,x,new_y);

			  x++;
		  }
		  else
		  {
			  // first point
			  ssd1306_DrawPixel(x, new_y, Black);
			  x++;
		  }
		  old_y = new_y;

	  }
	  // the other half screen before x=50
	  x=50;
	  for(float i=find_x+(1*time_scale_step); i>find_x+(1*time_scale_step)-(50*time_scale_step); i-=time_scale_step)
	  {
		  adc_data = get_adc_data(i);
		  if(adc_data < 4096)
			  new_y = convert_adc_scale(adc_data,volt_scale);
		  else
			  new_y = adc_data;

		  if(drawline_switch & (x!=50))
		  {
			  drawline(x+1,old_y,x,new_y);
			  x--;
		  }
		  else
		  {
			  // first point
			  ssd1306_DrawPixel(x, new_y, Black);
			  x--;
		  }
		  old_y = new_y;

	  }
//	  ssd1306_DrawPixel(x & 0x7f, y & 0x3f, Black);

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  //
  i2cClock_Reconfig();
//  sysTick_Reconfig();

  ADCClock_ReConfig(adc_freq);

// timer1
//  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_Base_Start_IT(&htim4);
//  HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_1);
//  HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_2);

// adc first start
  HAL_ADC_Start(&hadc2);
  HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc_datas, ADC_DATA_ARRAY_SIZE);
  for(volatile long i=0;i<1000000L;i++);
// adc calibration
  HAL_ADC_Stop(&hadc1);
  HAL_ADC_Stop(&hadc2);
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADCEx_Calibration_Start(&hadc2);
// adc re-start
//  HAL_ADC_Start(&hadc1);
  HAL_ADC_Start(&hadc2);
//  ADC_Enable(&hadc1);
//  ADC_Enable(&hadc2);
  HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc_datas, ADC_DATA_ARRAY_SIZE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  init();
//  int rand = 65537;
//  int x,y;
//  for(int i; i<100; i++)
//	 loop();
//  loop();
//  loop();
//  ssd1306_Fill(Black);
//  int trigger_threshold = 1920;

  while (1)
  {
//	  for(y=0; y<60; y+=10)
//	  for(x=0; x<128; x+=8)
//	  {
////	    loop();
//
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
//	  init();
//	  for(int i; i<100; i++)
//		 loop();

//	    ssd1306_SetCursor(x,y);
//	    ssd1306_WriteChar(*(unsigned char *)(&rand), Font_7x10, White);
//	    ssd1306_UpdateScreen();
//	    rand = rand * 3 + 1;
//	  }

//	  ssd1306_DrawPixel(rand() % 128, rand() % 64, White);
//	  ssd1306_UpdateScreen();

	  //	  x = *(unsigned char *)(&rand);
//	  rand = rand * 3 + 1;
//	  y = *(unsigned char *)(&rand);
//	  rand = rand * 3 + 1;

//	  ssd1306_SetCursor(0, 0);
//	  sprintf(str,"%d    ",adc_datas[0]);
//	  ssd1306_WriteString(str, Font_7x10, White);

	  //
	process_key();
//
	  if(adc_finish)
	  {
		  adc_finish = 0;

	  	  // waveform or graph
		  if(time_scale_position < 10000)
		  {
// find cross-trigger point
		  // find cross-trigger point from middle of data array
//		  uint16_t tmp1;
		  uint16_t tmp2;
		  uint16_t tmp3;
//		  uint16_t tmp4;
		  find_x = MIDDLE_POINT-time_scale_step;
		  while(find_x+(4*time_scale_step) < (ADC_DATA_ARRAY_SIZE * 2))
		  {
//			  tmp1 = get_adc_data(find_x+(1*time_scale_step));
			  tmp2 = get_adc_data(find_x+(2*time_scale_step));
			  tmp3 = get_adc_data(find_x+(3*time_scale_step));
//			  tmp4 = get_adc_data(find_x+(4*time_scale_step));

			  if(tmp2 > trigger_threshold && tmp3 < trigger_threshold)
			  {
//				  if(tmp1 > tmp2 )
//					  if (tmp3 > tmp4)
//					  {
							  break;
//					  }
			  }
			  find_x+=time_scale_step;
		  }

//
		  	  if(find_x+(78*time_scale_step) >= (ADC_DATA_ARRAY_SIZE * 2)) find_x=MIDDLE_POINT;
		  	//
  			  ssd1306_Fill(White);
			  process_buffer();
			  draw_frame();
			  ssd1306_UpdateScreen();
			  //
			  // adc re-start
			  HAL_ADC_Start(&hadc2);
			  HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc_datas, ADC_DATA_ARRAY_SIZE);
		  }
		  if(time_scale_position == 10000 || time_scale_position == 20000 || time_scale_position == 50000)
		  {

			  // find cross-trigger point
			  // find cross-trigger point from middle of data array

			  uint16_t tmp2;
			  uint16_t tmp3;

			  find_x = MIDDLE_POINT-time_scale_step;
			  while(find_x+(4*time_scale_step) < (ADC_DATA_ARRAY_SIZE * 2))
			  {

				  tmp2 = get_adc_data(find_x+(2*time_scale_step));
				  tmp3 = get_adc_data(find_x+(3*time_scale_step));


				  if(tmp2 > trigger_threshold && tmp3 < trigger_threshold)
				  {

								  break;

				  }
				  find_x+=time_scale_step;
			  }

			  //
			  if(find_x+(78*time_scale_step) >= (ADC_DATA_ARRAY_SIZE * 2)) find_x=MIDDLE_POINT-time_scale_step;
			//
			  ssd1306_Fill(White);
			  process_buffer();
			  draw_frame();
			  ssd1306_UpdateScreen();







//			  //			  while(graph_point_sampled == 0);
//
//			  //				  graph_point_sampled = 0;
//
//				  adc_datas[(int)(MIDDLE_POINT - (50*time_scale_step) + (graph_x*time_scale_step))/2] = adc_datas[0];
//
//				  if(graph_x < 128) graph_x++;
//				  else
//				  {
//					  graph_x = 0;
//
//					  find_x=MIDDLE_POINT;
//					  //
//			  		  ssd1306_Fill(White);
//					  process_buffer();
//					  draw_frame();
//					  ssd1306_UpdateScreen();
//				  }
//
//				  // time_scale_position > 5000
//



		  }
		  if(time_scale_position == 100000 || time_scale_position == 200000 || time_scale_position == 500000)
		  {
			  find_x=MIDDLE_POINT-time_scale_step;
			  ssd1306_Fill(White);
			  process_buffer();
			  draw_frame();
			  ssd1306_UpdateScreen();
		  }


		  } // if adc_finish





  } // while
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_MultiModeTypeDef multimode;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the ADC multi-mode 
    */
  multimode.Mode = ADC_DUALMODE_INTERLFAST;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* ADC2 init function */
static void MX_ADC2_Init(void)
{

    /**Common config 
    */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = ENABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM4 init function */
static void MX_TIM4_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 99;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 279;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA4 PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
int slow_adc_data_count = 2;
int slow_adc_halfdata_flag = 0;
uint32_t adc_data_temp;
int old_time_scale_position = 0;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	//
	if(old_time_scale_position != time_scale_position)
	{
		slow_adc_halfdata_flag = 0;
		slow_adc_data_count = 2;
		graph_x = ((MIDDLE_POINT-time_scale_step)/2) - (50/2);
		adc_finish = 1;
	}
	old_time_scale_position = time_scale_position;
	//
	if(time_scale_position < 10000)
	{
		adc_finish = 1;
	}
	if(time_scale_position == 10000 || time_scale_position == 20000 || time_scale_position == 50000)
	{
		if(slow_adc_halfdata_flag == 0)
		{
			adc_data_temp = adc_datas[0];
			slow_adc_halfdata_flag = ~slow_adc_halfdata_flag;
		}else{
			adc_data_temp &= 0xffff0000;
			adc_data_temp |= (adc_datas[0]&0x0000ffff);
			adc_datas[slow_adc_data_count] = adc_data_temp;
			slow_adc_data_count++;
			slow_adc_halfdata_flag = ~slow_adc_halfdata_flag;
		}

		if(slow_adc_data_count == ADC_DATA_ARRAY_SIZE)
		{
			slow_adc_data_count = 2;
			adc_finish = 1;
		}
	}
	if(time_scale_position == 100000 || time_scale_position == 200000 || time_scale_position == 500000)
	{
		//				  adc_datas[(int)(MIDDLE_POINT - (50*time_scale_step) + (graph_x*time_scale_step))/2] = adc_datas[0];
		//
		//				  if(graph_x < 128) graph_x++;
		//				  else
		//				  {
		//					  graph_x = 0;
		if(slow_adc_halfdata_flag == 0)
		{
			adc_datas[graph_x] = adc_datas[graph_x] & 0x0000ffff;
			adc_datas[graph_x] = adc_datas[graph_x] | (adc_datas[0] & 0xffff0000);
			adc_datas[graph_x] = adc_datas[graph_x] | 0x0000ffff;

			slow_adc_halfdata_flag = ~slow_adc_halfdata_flag;
		}else{
			adc_datas[graph_x] = adc_datas[graph_x] & 0xffff0000;
			adc_datas[graph_x] = adc_datas[graph_x] | (adc_datas[0]>>16 & 0x0000ffff);
			adc_datas[graph_x+1] = adc_datas[graph_x+1] | 0xffff0000;
			//
			slow_adc_halfdata_flag = ~slow_adc_halfdata_flag;
			//
			if(graph_x < (((MIDDLE_POINT-time_scale_step)/2) - (50/2) + (128/2))) graph_x++;
			else graph_x = ((MIDDLE_POINT-time_scale_step)/2) - (50/2);
		}
		//
		adc_finish = 1;
	}
}
int pin_toogle=0;
void HAL_SYSTICK_Callback(void)
{
}
// TIM4 freq. = 2kHz
void TIM4_UP_Callback(void)
{
	//
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, pin_toogle);
	  pin_toogle=~pin_toogle;
	//
	  if(key_debounce_timer > 0) key_debounce_timer--;

	  // 10ms/DIV, 20ms/DIV, 50ms/DIV
		if(time_scale_position == 10000 || time_scale_position == 20000 || time_scale_position == 50000)
		{



			  // adc re-start
			  HAL_ADC_Start(&hadc2);
			  HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc_datas, 1);



		}
		// 100ms/DIV, 200ms/DIV, 500ms/DIV
		// 2kHz/10=0.2kHz => 5ms
		//  100ms/DIV, 200ms/DIV, 500ms/DIV
		// =10ms/dot, 20ms/dot, 50ms/dot
		//  @2kHz, div=
		//  20,       40,       ,100
		if(time_scale_position == 100000 || time_scale_position == 200000 || time_scale_position == 500000)
		{
			if(slow_adc_div_counter < (time_scale_position/5000)) slow_adc_div_counter++;
			else
			{
				slow_adc_div_counter = 0;
				  // adc re-start
				  HAL_ADC_Start(&hadc2);
				  HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc_datas, 1);

			}
		}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
