/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : STM32TouchController.cpp
  ******************************************************************************
  * This file was created by TouchGFX Generator 4.26.0. This file is only
  * generated once! Delete this file from your project and re-generate code
  * using STM32CubeMX or change this file manually to update it.
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

/* USER CODE BEGIN STM32TouchController */

#include "ft5336.h"
#include <STM32TouchController.hpp>
#include <stm32746g_discovery_ts.h>

static TS_DrvTypeDef *tsDriver;
extern I2C_HandleTypeDef hi2c3;
static TS_StateTypeDef TS_State;

void STM32TouchController::init()
{
    /**
     * Initialize touch controller and driver
     *
     */
    BSP_TS_Init(480, 272);
    tsDriver = &ft5336_ts_drv;
}

bool STM32TouchController::sampleTouch(int32_t& x, int32_t& y)
{
    /**
     * By default sampleTouch returns false,
     * return true if a touch has been detected, otherwise false.
     *
     * Coordinates are passed to the caller by reference by x and y.
     *
     * This function is called by the TouchGFX framework.
     * By default sampleTouch is called every tick, this can be adjusted by HAL::setTouchSampleRate(int8_t);
     *
     */
    if (tsDriver)
    {
        BSP_TS_GetState(&TS_State);
        if (TS_State.touchDetected == 1)
        {
            /* Get each touch coordinates */
            x = TS_State.touchX[0];
            y = TS_State.touchY[0];
            return true;
        }
    }
    return false;
}

/* USER CODE END STM32TouchController */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
