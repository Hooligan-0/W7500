/**
  ******************************************************************************
  * @file    /DMA/MemoryToGpio/main.c 
  * @brief   Main program body
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "W7500x_dma.h"
#include "W7500x_uart.h"
#include "W7500x_crg.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int cnum;
uint32_t rx_flag;
int result;

uint32_t sysclock;

static __IO uint32_t TimingDelay;

volatile extern int dma_done_irq_occurred;
volatile extern int dma_done_irq_expected;
volatile extern int dma_error_irq_occurred;
volatile extern int dma_error_irq_expected;

volatile uint16_t source_data_array[1024];  /* Data array for memory DMA test */

void dma_gpio_copy (uint32_t chnl_num, unsigned int src, unsigned int dest, unsigned int size, unsigned int num);

UART_InitTypeDef UART_InitStructure;

/* Private function prototypes -----------------------------------------------*/
void delay(__IO uint32_t milliseconds);
void TimingDelay_Decrement(void);

int  dma_simple_test(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int main (void)
{
    cnum    = 1;
    rx_flag = 0;
    result  = 0;
    
    /* Set Systme init */
    SystemInit();
    *(volatile uint32_t *)(0x41001014) = 0x0060100; //clock setting 48MHz
    
    /* Get System Clock */
    sysclock = GetSystemClock();     

    /* SysTick_Config */
    SysTick_Config((sysclock/1000));

    /* External Clock */
    CRG_PLL_InputFrequencySelect(CRG_OCLK);
	
    /* UART0 Init */
    UART_StructInit(&UART_InitStructure);
    UART_Init(UART1,&UART_InitStructure);
    
    /* Set all GPIO-A to output */
    *(volatile unsigned long *)(0x42000000 + 0x10) = 0xFFFF;
    *(volatile unsigned long *)(0x41002000 + 0x30) = 1; /* PA_12 : AF1 = GPIO */
    *(volatile unsigned long *)(0x41002000 + 0x34) = 1; /* PA_13 : AF1 = GPIO */
    *(volatile unsigned long *)(0x41002000 + 0x38) = 1; /* PA_14 : AF1 = GPIO */
    *(volatile unsigned long *)(0x41002000 + 0x3C) = 1; /* PA_15 : AF1 = GPIO */
    
    /* wait for test */ 
    delay(100);

    dma_done_irq_expected = 0;
    dma_done_irq_occurred = 0;
    dma_error_irq_expected = 0;
    dma_error_irq_occurred = 0;
    dma_data_struct_init();
    dma_init();

    result += dma_simple_test();
    if (result==0) {
        printf ("\n** TEST PASSED **\n");
    } else {
        printf ("\n** TEST FAILED **, Error code = (0x%x)\n", result);
    }
    return 0;
}

/* --------------------------------------------------------------- */
/*  Simple software DMA test                                       */
/* --------------------------------------------------------------- */
int dma_simple_test(void)
{
  int return_val=0;
  int err_code=0;
  int i;
  unsigned int current_state;
  uint32_t dest_gpio;

  printf("uDMA simple test");
  DMA->CHNL_ENABLE_SET = (1<<cnum); /* Enable channel 0 */

  /* setup data for DMA */
  for (i=0;i<1024;i++) {
    source_data_array[i] = 0x5555;
    i++;
    source_data_array[i] = 0xAAAA;
  }
  
  dest_gpio = 0x42000004;
  
  while(1)
  {
  dma_gpio_copy (cnum, (unsigned int) &source_data_array[0],(unsigned int)dest_gpio, 2, 1024);
  do { /* Wait until PL230 DMA controller return to idle state */
    current_state = (DMA->DMA_STATUS >> 4)  & 0xF;
  } while (current_state!=0);
  }
  
  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : simple DMA failed (0x%x)\n", err_code);
    return_val=1;
  } else {
    printf ("-Passed");
  }

  return(return_val);
}

extern dma_data_structure *dma_data;

void dma_gpio_copy (uint32_t chnl_num, unsigned int src, unsigned int dest, unsigned int size, unsigned int num)
{
  unsigned long src_end_pointer =  src + ((1<<size)*(num-1));
  unsigned long dst_end_pointer = dest;
  unsigned long control         = (0x3  << 30) |  /* dst_inc : no increment */
                                  (size << 28) |  /* dst_size */
                                  (size << 26) |  /* src_inc */
                                  (size << 24) |  /* src_size */
                                  (size << 21) |  /* dst_prot_ctrl - HPROT[3:1] */
                                  (size << 18) |  /* src_prot_ctrl - HPROT[3:1] */
                                  (0    << 14) |  /* R_power */
                                  ((num-1)<< 4) | /* n_minus_1 */
                                  (0    <<  3) |  /* next_useburst */
                                  (2    <<  0) ;  /* cycle_ctrl - auto */
  
  dma_data->Primary[chnl_num].SrcEndPointer  = (EXPECTED_BE) ? __REV(src_end_pointer) : (src_end_pointer);
  dma_data->Primary[chnl_num].DestEndPointer = (EXPECTED_BE) ? __REV(dst_end_pointer) : (dst_end_pointer);
  dma_data->Primary[chnl_num].Control        = (EXPECTED_BE) ? __REV(control        ) : (control        );
  /* Debugging printfs: */
  printf ("SrcEndPointer  = %x\n", dma_data->Primary[chnl_num].SrcEndPointer);
  printf ("DestEndPointer = %x\n", dma_data->Primary[chnl_num].DestEndPointer);

  DMA->CHNL_ENABLE_SET = (1<<chnl_num); /* Enable channel */
  DMA->CHNL_SW_REQUEST = (1<<chnl_num); /* request channel DMA */

  return;
}


/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay(__IO uint32_t milliseconds)
{
  TimingDelay = milliseconds;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}





