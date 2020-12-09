/**
  ******************************************************************************
  * @file    USART/Printf/main.c 
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    04/16/2010
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32_eval.h"
#include <stdio.h>

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup USART_Printf
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define RCC_GPIO_LED               RCC_APB2Periph_GPIOD
#define GPIO_LED_PORT              GPIOD    
#define GPIO_LED1                  GPIO_Pin_2    
#define GPIO_LED2                  GPIO_Pin_3    
#define GPIO_LED3                  GPIO_Pin_4    
#define GPIO_LED4                  GPIO_Pin_7
#define GPIO_LED_ALL               GPIO_LED1 |GPIO_LED2 |GPIO_LED3 |GPIO_LED4 

/* Private variables ---------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure;

/* Private function prototypes -----------------------------------------------*/

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
  
/* Private functions ---------------------------------------------------------*/
static void Delay_ARMJISHU(__IO uint32_t nCount)
{
  for (; nCount != 0; nCount--);
}

void LED_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  /* Enable GPIOB, GPIOC and AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_GPIO_LED | RCC_APB2Periph_AFIO , ENABLE);  //RCC_APB2Periph_AFIO
  
  /* LEDs pins configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_LED_ALL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIO_LED_PORT, &GPIO_InitStructure);
}

void Led_Turn_on_all(void)
{
	/* Turn On All LEDs */
    GPIO_ResetBits(GPIO_LED_PORT, GPIO_LED_ALL);
}

void Led_Turn_off_all(void)
{
	/* Turn Off All LEDs */
    GPIO_SetBits(GPIO_LED_PORT, GPIO_LED_ALL);
}

void LED_Spark(void)
{
  static __IO uint32_t TimingDelayLocal = 0;
  
  if(TimingDelayLocal == 0)  // 熄灭LED指示灯
  {
    Led_Turn_off_all();
    TimingDelayLocal = 1;
  }
  else                       // 点亮LED指示灯
  {
    Led_Turn_on_all();
    TimingDelayLocal = 0;
  }
}


static void TIM5_Init(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  /* TIM5 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  
  /* ---------------------------------------------------------------
    TIM4 Configuration: Output Compare Timing Mode:
    TIM2CLK = 36 MHz, Prescaler = 7200, TIM2 counter clock = 7.2 MHz
  --------------------------------------------------------------- */

  /* Time base configuration */
  //这个就是自动装载的计数值，由于计数是从0开始的，计数10000次后为9999
  TIM_TimeBaseStructure.TIM_Period = (10000 - 1);
  // 这个就是预分频系数，当由于为0时表示不分频所以要减1
  TIM_TimeBaseStructure.TIM_Prescaler = (7200 - 1);
  // 高级应用本次不涉及。定义在定时器时钟(CK_INT)频率与数字滤波器(ETR,TIx)
  // 使用的采样频率之间的分频比例
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  //向上计数
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  //初始化定时器5
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

  /* Clear TIM5 update pending flag[清除TIM5溢出中断标志] */
  TIM_ClearITPendingBit(TIM5, TIM_IT_Update);

  /* TIM IT enable */ //打开溢出中断
  TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);

  /* TIM5 enable counter */
  TIM_Cmd(TIM5, ENABLE);  //计数器使能，开始工作
}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures the used IRQ Channels and sets their priority.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void NVIC_Configuration(void)
{ 
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Set the Vector Table base address at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);

  /* Enable the TIM5 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure);
  
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file */    
  /* USARTx configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled   */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  STM_EVAL_COMInit(COM1, &USART_InitStructure);

  /* Output a message on Hyperterminal using printf function */
  printf("\n\rUSART Printf Example: retarget the C library printf function to the USART\n\r");
  printf("\r\n\n\n WWW.ARMJISHU.COM  %s configured....", EVAL_COM1_STR);
  printf("\n\r ############ WWW.ARMJISHU.COM! ############ ("__DATE__ " - " __TIME__ ")");

  /* 初始化LED的GPIO */
  LED_config();
  /* 初始状态点亮全部LED */
  Led_Turn_on_all();

  /* 定时器5参数配置 */
  TIM5_Init();
  /* 中断参数配置 */
  NVIC_Configuration();
  
  while (1)
  {
   ; // 等待定时器中断
  }
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(EVAL_COM1, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)
  {}

  return ch;
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
