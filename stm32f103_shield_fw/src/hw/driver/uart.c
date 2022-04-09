/*
 * uart.c
 *
 *  Created on: Apr 9, 2022
 *      Author: baram
 */


#include "uart.h"
#include "qbuffer.h"



#define UART_RX_BUF_MAX     64


enum UartRxMode
{
  UART_MODE_POLLING,
  UART_MODE_INTERRUPT,
  UART_MODE_DMA,
  UART_MODE_VCP,
};



typedef struct
{
  bool     is_open;
  uint32_t baud;

  uint8_t rx_mode;
  uint8_t tx_mode;

  qbuffer_t rx_q;
  uint8_t   rx_buf[UART_RX_BUF_MAX];
  uint8_t   rx_data;
  UART_HandleTypeDef *p_uart;
} uart_tbl_t;


static bool is_init = false;

static uart_tbl_t uart_tbl[UART_MAX_CH];


static DMA_HandleTypeDef hdma_usart2_rx;

static UART_HandleTypeDef huart2;



bool uartInit(void)
{

  for (int i=0; i<UART_MAX_CH; i++)
  {
    uart_tbl[i].is_open = false;
    uart_tbl[i].baud = 115200;
    uart_tbl[i].p_uart = NULL;
    uart_tbl[i].rx_mode = UART_MODE_POLLING;
    uart_tbl[i].tx_mode = UART_MODE_POLLING;

    uartOpen(i, 115200);
  }

  is_init = true;

  return true;
}

bool uartOpen(uint8_t ch, uint32_t baud)
{
  bool ret = false;


  switch(ch)
  {
    case _DEF_UART1:
      if (uart_tbl[ch].is_open == true)
      {
        HAL_UART_DeInit(&huart2);
      }
      uart_tbl[ch].p_uart = &huart2;
      uart_tbl[ch].baud = baud;
      uart_tbl[ch].rx_mode = UART_MODE_DMA;

      uart_tbl[ch].p_uart->Instance           = USART2;
      uart_tbl[ch].p_uart->Init.BaudRate      = baud;
      uart_tbl[ch].p_uart->Init.WordLength    = UART_WORDLENGTH_8B;
      uart_tbl[ch].p_uart->Init.StopBits      = UART_STOPBITS_1;
      uart_tbl[ch].p_uart->Init.Parity        = UART_PARITY_NONE;
      uart_tbl[ch].p_uart->Init.Mode          = UART_MODE_TX_RX;
      uart_tbl[ch].p_uart->Init.HwFlowCtl     = UART_HWCONTROL_NONE;
      uart_tbl[ch].p_uart->Init.OverSampling  = UART_OVERSAMPLING_16;


      qbufferCreate(&uart_tbl[ch].rx_q, uart_tbl[ch].rx_buf, sizeof(uart_tbl[ch].rx_buf));

      __HAL_RCC_DMA1_CLK_ENABLE();

      if (HAL_UART_Init(uart_tbl[ch].p_uart) != HAL_OK)
      {
        Error_Handler();
      }
      uart_tbl[ch].is_open = true;

      HAL_UART_Receive_DMA(uart_tbl[ch].p_uart, (uint8_t *)&uart_tbl[ch].rx_buf[0], sizeof(uart_tbl[ch].rx_buf));

      uart_tbl[ch].rx_q.in  = uart_tbl[ch].rx_q.len - uart_tbl[ch].p_uart->hdmarx->Instance->CNDTR;
      uart_tbl[ch].rx_q.out = uart_tbl[ch].rx_q.in;

      ret = true;
      break;
  }

  return ret;
}

bool uartClose(uint8_t ch)
{
  uart_tbl[ch].is_open = false;
  return true;
}

uint32_t uartAvailable(uint8_t ch)
{
  uint32_t ret = 0;

  switch(ch)
  {
    case _DEF_UART1:
      uart_tbl[ch].rx_q.in = uart_tbl[ch].rx_q.len - (uart_tbl[ch].p_uart->hdmarx->Instance)->CNDTR;
      ret = qbufferAvailable(&uart_tbl[ch].rx_q);
      break;
  }

  return ret;
}

bool uartFlush(uint8_t ch)
{
  uint32_t pre_time;

  if (ch >= UART_MAX_CH) return false;

  pre_time = millis();
  while(uartAvailable(ch))
  {
    if (millis()-pre_time >= 10)
    {
      break;
    }
    uartRead(ch);
  }

  return true;
}

uint8_t uartRead(uint8_t ch)
{
  uint8_t ret = 0;


  switch(ch)
  {
    case _DEF_UART1:
      qbufferRead(&uart_tbl[ch].rx_q, &ret, 1);
      break;
  }

  return ret;
}

uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t length)
{
  uint32_t ret = 0;

  switch(ch)
  {
    case _DEF_UART1:
      if (HAL_UART_Transmit(uart_tbl[ch].p_uart, p_data, length, 100) == HAL_OK)
      {
        ret = length;
      }
      break;
  }

  return ret;
}

uint32_t uartPrintf(uint8_t ch, char *fmt, ...)
{
  int32_t ret = 0;
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];

  if (ch >= UART_MAX_CH) return 0;

  len = vsnprintf(print_buffer, 256, fmt, arg);
  va_end (arg);

  ret = uartWrite(ch, (uint8_t *)print_buffer, len);

  return ret;
}

uint32_t uartGetBaud(uint8_t ch)
{
  if (ch >= UART_MAX_CH) return 0;

  return uart_tbl[ch].baud;
}




void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == uart_tbl[_DEF_UART1].p_uart)
  {
    qbufferWrite(&uart_tbl[_DEF_UART1].rx_q, &uart_tbl[_DEF_UART1].rx_data, 1);

    __HAL_UNLOCK(uart_tbl[_DEF_UART1].p_uart);
    HAL_UART_Receive_IT(uart_tbl[_DEF_UART1].p_uart, &uart_tbl[_DEF_UART1].rx_data, 1);
  }
}


void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Channel6;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);


    /* USART2 interrupt Init */
    //HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    //HAL_NVIC_EnableIRQ(USART2_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  }
}
