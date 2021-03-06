/**
  ******************************************************************************
  * @file    I2C/I2C_OLED/main.c 
  * @author  IOP Team
  * @version V1.0.0
  * @date    15-JUN-2015
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, WIZnet SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2015 WIZnet Co.,Ltd.</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "W7500x_uart.h"
#include "W7500x_i2c.h"
#include "OLED_I2C.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define OLED_ADDRESS	0x78
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
I2C_ConfigStruct conf;
///                   memaddress,data0,data1 data2,data3,data4,data5,data6,data7
/* Private function prototypes -----------------------------------------------*/
void delay_us(int us);
void delay_ms(int count);
/* Private functions ---------------------------------------------------------*/

int main()
{
    UART_InitTypeDef UART_InitStructure;

	SystemInit();
//    *(volatile uint32_t *)(0x41001014) = 0x0060100; //clock setting 48MHz
    
    /* CLK OUT Set */
//    PAD_AFConfig(PAD_PA,GPIO_Pin_2, PAD_AF2); // PAD Config - CLKOUT used 3nd Function
     /* UART0 and UART1 configuration*/
    UART_StructInit(&UART_InitStructure);
    /* Configure UART1 */
    UART_Init(UART1,&UART_InitStructure);
    
    conf.scl = I2C_PA_9;
    conf.sda = I2C_PA_10;

    I2C_Init(&conf);
    OLED_Init();
	
	
	OLED_Fill(0xFF);
	delay_ms(20);
	OLED_Fill(0x00);
	delay_ms(10);
    
	OLED_ShowStr(0,3,"Welcome to",1);
	OLED_ShowStr(0,4,"WIznet Academy^^",2);	

  } 

