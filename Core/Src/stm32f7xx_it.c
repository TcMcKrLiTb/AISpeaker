/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f7xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern ETH_HandleTypeDef heth;
extern DMA2D_HandleTypeDef hdma2d;
extern SDRAM_HandleTypeDef hsdram1;
extern I2C_HandleTypeDef hi2c3;
extern LTDC_HandleTypeDef hltdc;
extern DMA_HandleTypeDef hdma_sai2_a;
extern DMA_HandleTypeDef hdma_sai2_b;
extern DMA_HandleTypeDef hdma_sdmmc1_rx;
extern DMA_HandleTypeDef hdma_sdmmc1_tx;
extern SD_HandleTypeDef hsd1;
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim6;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M7 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

    uint32_t lr_val;
    uint32_t msp, psp;
    uint32_t strLen = 0;
    char printStr[100];
    __asm volatile ("mov %0, lr" : "=r" (lr_val) );
    __asm volatile ("mrs %0, msp" : "=r" (msp) );
    __asm volatile ("mrs %0, psp" : "=r" (psp) );

    /* Choose the correct stack frame pointer (stacked registers) */
    uint32_t *stacked = ( (lr_val & 4) == 0 ) ? (uint32_t *)msp : (uint32_t *)psp;

    /* Extract stacked registers */
    uint32_t r0   = stacked[0];
    uint32_t r1   = stacked[1];
    uint32_t r2   = stacked[2];
    uint32_t r3   = stacked[3];
    uint32_t r12  = stacked[4];
    uint32_t lr   = stacked[5];
    uint32_t pc   = stacked[6];
    uint32_t xpsr = stacked[7];

    /* Read SCB fault registers directly (addresses from Cortex-M spec) */
    volatile uint32_t *CFSR = (volatile uint32_t *)0xE000ED28UL;
    volatile uint32_t *HFSR = (volatile uint32_t *)0xE000ED2CUL;
    volatile uint32_t *MMFAR = (volatile uint32_t *)0xE000ED34UL;
    volatile uint32_t *BFAR  = (volatile uint32_t *)0xE000ED38UL;

    uint32_t cfsr = *CFSR;
    uint32_t hfsr = *HFSR;
    uint32_t mmfar = *MMFAR;
    uint32_t bfar  = *BFAR;

    /* Print everything in one function */
    strLen = snprintf(printStr, 100, "\r\n=== HARDFAULT ===\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "Stack used: %s (LR=0x%08lx)\r\n", ((lr_val & 4) == 0) ? "MSP" : "PSP",
                      (unsigned long)lr_val);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "R0  = 0x%08lx\r\n", (unsigned long)r0);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "R1  = 0x%08lx\r\n", (unsigned long)r1);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "R2  = 0x%08lx\r\n", (unsigned long)r2);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "R3  = 0x%08lx\r\n", (unsigned long)r3);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "R12 = 0x%08lx\r\n", (unsigned long)r12);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "LR  = 0x%08lx\r\n", (unsigned long)lr);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "PC  = 0x%08lx\r\n", (unsigned long)pc);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "xPSR= 0x%08lx\r\n", (unsigned long)xpsr);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);

    strLen = snprintf(printStr, 100, "CFSR=0x%08lx HFSR=0x%08lx\r\n", (unsigned long)cfsr, (unsigned long)hfsr);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "BFAR=0x%08lx MMFAR=0x%08lx\r\n", (unsigned long)bfar, (unsigned long)mmfar);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);

    /* Optional: decode a couple of common bits (brief) */
    if (cfsr & (1 << 7)) { /* BFARVALID */
        strLen = snprintf(printStr, 100, "BusFault: BFARVALID set. Fault address = 0x%08lx\r\n", (unsigned long)bfar);
        HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    }
    if (cfsr & (1 << 1)) { /* PRECISERR */
        strLen = snprintf(printStr, 100, "BusFault: PRECISERR (precise data bus error) set.\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    }
    if (hfsr & (1UL << 30)) {
        strLen = snprintf(printStr, 100, "HFSR: FORCED bit set (escalated fault -> HardFault).\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    }
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

    uint32_t lr_val;
    uint32_t msp, psp;
    uint32_t strLen = 0;
    char printStr[100];
    __asm volatile ("mov %0, lr" : "=r" (lr_val) );
    __asm volatile ("mrs %0, msp" : "=r" (msp) );
    __asm volatile ("mrs %0, psp" : "=r" (psp) );

    /* Choose the correct stack frame pointer (stacked registers) */
    uint32_t *stacked = ( (lr_val & 4) == 0 ) ? (uint32_t *)msp : (uint32_t *)psp;

    /* Extract stacked registers */
    uint32_t r0   = stacked[0];
    uint32_t r1   = stacked[1];
    uint32_t r2   = stacked[2];
    uint32_t r3   = stacked[3];
    uint32_t r12  = stacked[4];
    uint32_t lr   = stacked[5];
    uint32_t pc   = stacked[6];
    uint32_t xpsr = stacked[7];

    /* Read SCB fault registers directly (addresses from Cortex-M spec) */
    volatile uint32_t *CFSR = (volatile uint32_t *)0xE000ED28UL;
    volatile uint32_t *HFSR = (volatile uint32_t *)0xE000ED2CUL;
    volatile uint32_t *MMFAR = (volatile uint32_t *)0xE000ED34UL;
    volatile uint32_t *BFAR  = (volatile uint32_t *)0xE000ED38UL;

    uint32_t cfsr = *CFSR;
    uint32_t hfsr = *HFSR;
    uint32_t mmfar = *MMFAR;
    uint32_t bfar  = *BFAR;

    /* Print everything in one function */
    strLen = snprintf(printStr, 100, "\r\n=== MemManage_Handler ===\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "Stack used: %s (LR=0x%08lx)\r\n", ((lr_val & 4) == 0) ? "MSP" : "PSP",
                      (unsigned long)lr_val);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "R0  = 0x%08lx\r\n", (unsigned long)r0);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "R1  = 0x%08lx\r\n", (unsigned long)r1);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "R2  = 0x%08lx\r\n", (unsigned long)r2);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "R3  = 0x%08lx\r\n", (unsigned long)r3);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "R12 = 0x%08lx\r\n", (unsigned long)r12);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "LR  = 0x%08lx\r\n", (unsigned long)lr);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "PC  = 0x%08lx\r\n", (unsigned long)pc);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "xPSR= 0x%08lx\r\n", (unsigned long)xpsr);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);

    strLen = snprintf(printStr, 100, "CFSR=0x%08lx HFSR=0x%08lx\r\n", (unsigned long)cfsr, (unsigned long)hfsr);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    strLen = snprintf(printStr, 100, "BFAR=0x%08lx MMFAR=0x%08lx\r\n", (unsigned long)bfar, (unsigned long)mmfar);
    HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);

    /* Optional: decode a couple of common bits (brief) */
    if (cfsr & (1 << 7)) { /* BFARVALID */
        strLen = snprintf(printStr, 100, "BusFault: BFARVALID set. Fault address = 0x%08lx\r\n", (unsigned long)bfar);
        HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    }
    if (cfsr & (1 << 1)) { /* PRECISERR */
        strLen = snprintf(printStr, 100, "BusFault: PRECISERR (precise data bus error) set.\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    }
    if (hfsr & (1UL << 30)) {
        strLen = snprintf(printStr, 100, "HFSR: FORCED bit set (escalated fault -> HardFault).\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)printStr, strLen, HAL_MAX_DELAY);
    }
  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles FMC global interrupt.
  */
void FMC_IRQHandler(void)
{
  /* USER CODE BEGIN FMC_IRQn 0 */

  /* USER CODE END FMC_IRQn 0 */
  HAL_SDRAM_IRQHandler(&hsdram1);
  /* USER CODE BEGIN FMC_IRQn 1 */

  /* USER CODE END FMC_IRQn 1 */
}

/**
  * @brief This function handles SDMMC1 global interrupt.
  */
void SDMMC1_IRQHandler(void)
{
  /* USER CODE BEGIN SDMMC1_IRQn 0 */

  /* USER CODE END SDMMC1_IRQn 0 */
  HAL_SD_IRQHandler(&hsd1);
  /* USER CODE BEGIN SDMMC1_IRQn 1 */

  /* USER CODE END SDMMC1_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */

  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream3 global interrupt.
  */
void DMA2_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream3_IRQn 0 */

  /* USER CODE END DMA2_Stream3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_sdmmc1_rx);
  /* USER CODE BEGIN DMA2_Stream3_IRQn 1 */

  /* USER CODE END DMA2_Stream3_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream4 global interrupt.
  */
void DMA2_Stream4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream4_IRQn 0 */

  /* USER CODE END DMA2_Stream4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_sai2_a);
  /* USER CODE BEGIN DMA2_Stream4_IRQn 1 */

  /* USER CODE END DMA2_Stream4_IRQn 1 */
}

/**
  * @brief This function handles Ethernet global interrupt.
  */
void ETH_IRQHandler(void)
{
  /* USER CODE BEGIN ETH_IRQn 0 */

  /* USER CODE END ETH_IRQn 0 */
  HAL_ETH_IRQHandler(&heth);
  /* USER CODE BEGIN ETH_IRQn 1 */

  /* USER CODE END ETH_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream6 global interrupt.
  */
void DMA2_Stream6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream6_IRQn 0 */

  /* USER CODE END DMA2_Stream6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_sdmmc1_tx);
  /* USER CODE BEGIN DMA2_Stream6_IRQn 1 */

  /* USER CODE END DMA2_Stream6_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream7 global interrupt.
  */
void DMA2_Stream7_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream7_IRQn 0 */

  /* USER CODE END DMA2_Stream7_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_sai2_b);
  /* USER CODE BEGIN DMA2_Stream7_IRQn 1 */

  /* USER CODE END DMA2_Stream7_IRQn 1 */
}

/**
  * @brief This function handles I2C3 event interrupt.
  */
void I2C3_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C3_EV_IRQn 0 */

  /* USER CODE END I2C3_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c3);
  /* USER CODE BEGIN I2C3_EV_IRQn 1 */

  /* USER CODE END I2C3_EV_IRQn 1 */
}

/**
  * @brief This function handles LTDC global interrupt.
  */
void LTDC_IRQHandler(void)
{
  /* USER CODE BEGIN LTDC_IRQn 0 */

  /* USER CODE END LTDC_IRQn 0 */
  HAL_LTDC_IRQHandler(&hltdc);
  /* USER CODE BEGIN LTDC_IRQn 1 */

  /* USER CODE END LTDC_IRQn 1 */
}

/**
  * @brief This function handles DMA2D global interrupt.
  */
void DMA2D_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2D_IRQn 0 */

  /* USER CODE END DMA2D_IRQn 0 */
  HAL_DMA2D_IRQHandler(&hdma2d);
  /* USER CODE BEGIN DMA2D_IRQn 1 */

  /* USER CODE END DMA2D_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
