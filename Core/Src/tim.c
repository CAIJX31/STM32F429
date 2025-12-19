/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "tim.h"

/* USER CODE BEGIN 0 */
#include <stddef.h>
#include <string.h>

#define WAVEFORM_PREVIEW_POINTS 100U
#define WAVEFORM_RING_SAMPLES (ADC_BUF_LEN * 8U)
#define MID_FILTER_SHIFT_FAST 10U
#define MID_FILTER_SHIFT_SLOW 18U
#define MID_FILTER_FAST_SAMPLES ADC_BUF_LEN
#define DEFAULT_HYSTERESIS_COUNTS 4U
#define HYSTERESIS_MIN_COUNTS 2U
#define HYSTERESIS_MAX_COUNTS 128U
#define FREQUENCY_STALE_SECONDS 2U
#define MAX_MEASURE_FREQUENCY_HZ 10000U
#define FREQ_SWITCH_HIGH_HZ 1100U
#define FREQ_SWITCH_LOW_HZ 900U
#define FAST_DISPLAY_SMOOTH_SHIFT 3U
#define FAST_DISPLAY_STEP_PCT 10U

static float s_last_vpp_mv = 0.0f;
static uint16_t s_preview_buffer[WAVEFORM_PREVIEW_POINTS];
static size_t s_preview_count = 0U;
static uint16_t s_wave_ring[WAVEFORM_RING_SAMPLES];
static volatile uint32_t s_wave_ring_pos = 0U;
static volatile uint32_t s_wave_ring_count = 0U;

extern uint16_t adc_buf[ADC_BUF_LEN];

static volatile uint32_t s_fast_frequency_hz = 0U;
static volatile uint32_t s_slow_frequency_hz = 0U;
static volatile uint32_t s_sample_rate_hz = 0U;
static volatile uint32_t s_sample_counter = 0U;
static volatile uint32_t s_fast_last_crossing_sample = 0U;
static volatile uint32_t s_slow_last_crossing_sample = 0U;
static volatile uint8_t s_fast_have_crossing = 0U;
static volatile uint8_t s_slow_have_crossing = 0U;

static uint32_t s_mid_q16 = 0U;
static uint32_t s_mid_warmup = 0U;
static uint8_t s_mid_initialized = 0U;
static uint8_t s_slow_state_high = 0U;
static uint16_t s_hysteresis_counts = DEFAULT_HYSTERESIS_COUNTS;
static uint8_t s_use_fast = 0U;
static uint64_t s_fast_last_crossing_q16 = 0U;
static uint64_t s_slow_last_crossing_q16 = 0U;
static uint64_t s_fast_period_samples_q16 = 0U;
static uint64_t s_slow_period_samples_q16 = 0U;
static uint32_t s_display_frequency_hz = 0U;
static uint16_t s_prev_sample = 0U;
static uint8_t s_prev_valid = 0U;

static uint32_t GetAdcSampleRateHz(void)
{
  /* TIM2 drives ADC1 external trigger: sample rate equals TIM2 update rate. */
  RCC_ClkInitTypeDef clk_config;
  uint32_t flash_latency;
  HAL_RCC_GetClockConfig(&clk_config, &flash_latency);

  uint32_t tim_clk = HAL_RCC_GetPCLK1Freq();
  if (clk_config.APB1CLKDivider != RCC_HCLK_DIV1)
  {
    tim_clk *= 2U; /* Timers on APB1 multiply clock when prescaled */
  }

  const uint32_t div = (htim2.Init.Prescaler + 1U) * (htim2.Init.Period + 1U);
  if (div == 0U)
  {
    return 0U;
  }
  return tim_clk / div;
}

void TIM1_ProcessAdcSamples(const uint16_t *samples, size_t count)
{
  if ((samples == NULL) || (count == 0U))
  {
    return;
  }

  uint32_t sample_rate_hz = s_sample_rate_hz;
  if (sample_rate_hz == 0U)
  {
    sample_rate_hz = GetAdcSampleRateHz();
    if (sample_rate_hz != 0U)
    {
      s_sample_rate_hz = sample_rate_hz;
    }
  }

  uint32_t min_period_samples = 0U;
  if (sample_rate_hz > 0U)
  {
    min_period_samples = sample_rate_hz / MAX_MEASURE_FREQUENCY_HZ;
    if (min_period_samples < 2U)
    {
      min_period_samples = 2U;
    }
  }

  const uint16_t hysteresis = (s_hysteresis_counts > 0U) ? s_hysteresis_counts : DEFAULT_HYSTERESIS_COUNTS;
  const uint64_t min_period_q16 = ((uint64_t)min_period_samples) << 16;
  uint64_t fast_min_period_q16 = min_period_q16;
  if (s_fast_period_samples_q16 > 0U)
  {
    fast_min_period_q16 = (s_fast_period_samples_q16 * 3U) / 4U;
  }
  else if (min_period_q16 > 0U)
  {
    fast_min_period_q16 = (min_period_q16 * 4U) / 5U;
  }
  uint32_t sample_counter = s_sample_counter;
  uint16_t prev_sample = s_prev_sample;
  uint8_t prev_valid = s_prev_valid;
  uint32_t ring_pos = s_wave_ring_pos;
  uint32_t ring_count = s_wave_ring_count;

  for (size_t i = 0; i < count; ++i)
  {
    const uint16_t sample = samples[i];
    s_wave_ring[ring_pos] = sample;
    ring_pos++;
    if (ring_pos >= WAVEFORM_RING_SAMPLES)
    {
      ring_pos = 0U;
    }
    if (ring_count < WAVEFORM_RING_SAMPLES)
    {
      ring_count++;
    }

    if (!s_mid_initialized)
    {
      s_mid_q16 = ((uint32_t)sample) << 16;
      s_mid_initialized = 1U;
      s_mid_warmup = 0U;
    }
    else
    {
      const uint32_t shift = (s_mid_warmup < MID_FILTER_FAST_SAMPLES) ? MID_FILTER_SHIFT_FAST : MID_FILTER_SHIFT_SLOW;
      int32_t mid_q16 = (int32_t)s_mid_q16;
      const int32_t diff = ((int32_t)sample << 16) - mid_q16;
      mid_q16 += diff >> shift;
      s_mid_q16 = (uint32_t)mid_q16;
      if (s_mid_warmup < MID_FILTER_FAST_SAMPLES)
      {
        s_mid_warmup++;
      }
    }

    if (!prev_valid)
    {
      prev_sample = sample;
      prev_valid = 1U;
      sample_counter++;
      continue;
    }

    const uint16_t mid = (uint16_t)(s_mid_q16 >> 16);
    if ((prev_sample < mid) && (sample >= mid))
    {
      uint32_t fraction_q16 = 0U;
      if (sample > prev_sample)
      {
        const uint32_t num = (uint32_t)(mid - prev_sample);
        const uint32_t den = (uint32_t)(sample - prev_sample);
        if (den > 0U)
        {
          fraction_q16 = (num << 16) / den;
          if (fraction_q16 > 0xFFFFU)
          {
            fraction_q16 = 0xFFFFU;
          }
        }
      }

      const uint32_t sample_index = (sample_counter > 0U) ? (sample_counter - 1U) : 0U;
      const uint64_t crossing_q16 = (((uint64_t)sample_index) << 16) + (uint64_t)fraction_q16;
      if (s_fast_have_crossing)
      {
        const uint64_t delta_q16 = crossing_q16 - s_fast_last_crossing_q16;
        uint8_t accept_fast = 0U;
        if ((fast_min_period_q16 == 0U) || (delta_q16 >= fast_min_period_q16))
        {
          accept_fast = 1U;
        }
        else if ((min_period_q16 > 0U) && (delta_q16 >= min_period_q16))
        {
          /* Allow fast period shrink on step-up in frequency. */
          s_fast_period_samples_q16 = 0U;
          accept_fast = 1U;
        }

        if (accept_fast)
        {
          if (s_fast_period_samples_q16 == 0U)
          {
            s_fast_period_samples_q16 = delta_q16;
          }
          else
          {
            s_fast_period_samples_q16 = (s_fast_period_samples_q16 * 3U + delta_q16) / 4U;
          }

          if ((s_fast_period_samples_q16 > 0U) && (sample_rate_hz > 0U))
          {
            const uint64_t numerator = (((uint64_t)sample_rate_hz) << 16) + (s_fast_period_samples_q16 / 2U);
            s_fast_frequency_hz = (uint32_t)(numerator / s_fast_period_samples_q16);
          }
          s_fast_last_crossing_q16 = crossing_q16;
          s_fast_last_crossing_sample = sample_counter;
        }
      }
      else
      {
        s_fast_last_crossing_q16 = crossing_q16;
        s_fast_last_crossing_sample = sample_counter;
        s_fast_have_crossing = 1U;
      }
    }

    uint16_t high = (uint16_t)(mid + hysteresis);
    if (high < mid)
    {
      high = 0xFFFFU;
    }
    const uint16_t low = (mid > hysteresis) ? (uint16_t)(mid - hysteresis) : 0U;

    if (!s_slow_state_high)
    {
      if (sample >= high)
      {
        uint32_t fraction_q16 = 0U;
        if ((sample > prev_sample) && (prev_sample < high))
        {
          const uint32_t num = (uint32_t)(high - prev_sample);
          const uint32_t den = (uint32_t)(sample - prev_sample);
          if (den > 0U)
          {
            fraction_q16 = (num << 16) / den;
            if (fraction_q16 > 0xFFFFU)
            {
              fraction_q16 = 0xFFFFU;
            }
          }
        }

        const uint32_t sample_index = (sample_counter > 0U) ? (sample_counter - 1U) : 0U;
        const uint64_t crossing_q16 = (((uint64_t)sample_index) << 16) + (uint64_t)fraction_q16;
        if (s_slow_have_crossing)
        {
          const uint64_t delta_q16 = crossing_q16 - s_slow_last_crossing_q16;
          if ((min_period_q16 == 0U) || (delta_q16 >= min_period_q16))
          {
            if (s_slow_period_samples_q16 == 0U)
            {
              s_slow_period_samples_q16 = delta_q16;
            }
            else
            {
              s_slow_period_samples_q16 = (s_slow_period_samples_q16 * 3U + delta_q16) / 4U;
            }

            if ((s_slow_period_samples_q16 > 0U) && (sample_rate_hz > 0U))
            {
              const uint64_t numerator = (((uint64_t)sample_rate_hz) << 16) + (s_slow_period_samples_q16 / 2U);
              s_slow_frequency_hz = (uint32_t)(numerator / s_slow_period_samples_q16);
            }
            s_slow_last_crossing_q16 = crossing_q16;
            s_slow_last_crossing_sample = sample_counter;
          }
        }
        else
        {
          s_slow_last_crossing_q16 = crossing_q16;
          s_slow_last_crossing_sample = sample_counter;
          s_slow_have_crossing = 1U;
        }
        s_slow_state_high = 1U;
      }
    }
    else
    {
      if (sample <= low)
      {
        s_slow_state_high = 0U;
      }
    }

    prev_sample = sample;
    sample_counter++;
  }

  s_sample_counter = sample_counter;
  s_prev_sample = prev_sample;
  s_prev_valid = prev_valid;
  s_wave_ring_pos = ring_pos;
  s_wave_ring_count = ring_count;
}
/* USER CODE END 0 */

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* TIM2 init function */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 839;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}
/* TIM3 init function */
void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 90;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspInit 0 */

  /* USER CODE END TIM3_MspInit 0 */
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();
  /* USER CODE BEGIN TIM3_MspInit 1 */

  /* USER CODE END TIM3_MspInit 1 */
  }
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspPostInit 0 */

  /* USER CODE END TIM3_MspPostInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM3 GPIO Configuration
    PA6     ------> TIM3_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM3_MspPostInit 1 */

  /* USER CODE END TIM3_MspPostInit 1 */
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspDeInit 0 */

  /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();
  /* USER CODE BEGIN TIM3_MspDeInit 1 */

  /* USER CODE END TIM3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */


uint32_t TIM1_GetFrequencyHz(void)
{
  uint16_t min_val = 0xFFFFU;
  uint16_t max_val = 0U;
  for (size_t i = 0; i < ADC_BUF_LEN; ++i)
  {
    const uint16_t v = adc_buf[i];
    if (v < min_val)
    {
      min_val = v;
    }
    if (v > max_val)
    {
      max_val = v;
    }
  }

  const uint32_t vpp_counts = (uint32_t)(max_val - min_val);
  s_last_vpp_mv = (float)vpp_counts * 3300.0f / 4095.0f;

  uint32_t hysteresis = vpp_counts / 32U;
  if (hysteresis < HYSTERESIS_MIN_COUNTS)
  {
    hysteresis = HYSTERESIS_MIN_COUNTS;
  }
  if (hysteresis > HYSTERESIS_MAX_COUNTS)
  {
    hysteresis = HYSTERESIS_MAX_COUNTS;
  }
  s_hysteresis_counts = (uint16_t)hysteresis;

  uint32_t sample_rate_hz = s_sample_rate_hz;
  if (sample_rate_hz == 0U)
  {
    sample_rate_hz = GetAdcSampleRateHz();
    if (sample_rate_hz != 0U)
    {
      s_sample_rate_hz = sample_rate_hz;
    }
  }

  uint32_t frequency_fast = s_fast_frequency_hz;
  uint32_t frequency_slow = s_slow_frequency_hz;

  if ((sample_rate_hz == 0U) || (!s_fast_have_crossing))
  {
    frequency_fast = 0U;
  }
  else
  {
    const uint32_t samples_since_cross = s_sample_counter - s_fast_last_crossing_sample;
    const uint32_t stale_limit = sample_rate_hz * FREQUENCY_STALE_SECONDS;
    if (samples_since_cross > stale_limit)
    {
      frequency_fast = 0U;
      s_fast_frequency_hz = 0U;
      s_fast_period_samples_q16 = 0U;
    }
  }

  if ((sample_rate_hz == 0U) || (!s_slow_have_crossing))
  {
    frequency_slow = 0U;
  }
  else
  {
    const uint32_t samples_since_cross = s_sample_counter - s_slow_last_crossing_sample;
    const uint32_t stale_limit = sample_rate_hz * FREQUENCY_STALE_SECONDS;
    if (samples_since_cross > stale_limit)
    {
      frequency_slow = 0U;
      s_slow_frequency_hz = 0U;
    }
  }

  if (s_use_fast)
  {
    if ((frequency_fast == 0U) || (frequency_fast < FREQ_SWITCH_LOW_HZ))
    {
      s_use_fast = 0U;
    }
  }
  else
  {
    if (frequency_fast >= FREQ_SWITCH_HIGH_HZ)
    {
      s_use_fast = 1U;
    }
  }

  uint32_t frequency_hz = s_use_fast ? frequency_fast : frequency_slow;
  uint32_t display_frequency_hz = frequency_hz;

  if (frequency_hz == 0U)
  {
    s_display_frequency_hz = 0U;
  }
  else if (s_use_fast)
  {
    if (s_display_frequency_hz == 0U)
    {
      s_display_frequency_hz = frequency_hz;
    }
    else
    {
      const uint32_t diff = (frequency_hz > s_display_frequency_hz) ? (frequency_hz - s_display_frequency_hz)
                                                                     : (s_display_frequency_hz - frequency_hz);
      const uint32_t step_threshold = frequency_hz / FAST_DISPLAY_STEP_PCT;
      if (diff > step_threshold)
      {
        s_display_frequency_hz = frequency_hz;
      }
      else
      {
        s_display_frequency_hz = (s_display_frequency_hz * ((1U << FAST_DISPLAY_SMOOTH_SHIFT) - 1U) + frequency_hz)
                                 >> FAST_DISPLAY_SMOOTH_SHIFT;
      }
    }
    display_frequency_hz = s_display_frequency_hz;
  }
  else
  {
    s_display_frequency_hz = frequency_hz;
  }

  float samples_per_period = 0.0f;
  if ((frequency_hz > 0U) && (sample_rate_hz > 0U))
  {
    samples_per_period = (float)sample_rate_hz / (float)frequency_hz;
  }

  size_t available_samples = (size_t)s_wave_ring_count;
  if (available_samples > WAVEFORM_RING_SAMPLES)
  {
    available_samples = WAVEFORM_RING_SAMPLES;
  }

  size_t preview_window = available_samples;
  if ((samples_per_period > 0.0f) && (preview_window > 0U))
  {
    const float desired_samples = samples_per_period * 6.0f; /* show ~6 periods */
    if (desired_samples < (float)preview_window)
    {
      preview_window = (size_t)desired_samples;
    }
  }
  if (preview_window < WAVEFORM_PREVIEW_POINTS)
  {
    preview_window = (available_samples < WAVEFORM_PREVIEW_POINTS) ? available_samples : WAVEFORM_PREVIEW_POINTS;
  }

  const size_t step = (preview_window / WAVEFORM_PREVIEW_POINTS) ? (preview_window / WAVEFORM_PREVIEW_POINTS) : 1U;
  size_t preview_index = 0U;
  if (preview_window > 0U)
  {
    uint32_t ring_pos = s_wave_ring_pos;
    uint32_t start_index = (ring_pos + WAVEFORM_RING_SAMPLES - (uint32_t)preview_window) % WAVEFORM_RING_SAMPLES;
    for (size_t i = 0; (i < preview_window) && (preview_index < WAVEFORM_PREVIEW_POINTS); i += step)
    {
      const uint32_t ring_index = (start_index + (uint32_t)i) % WAVEFORM_RING_SAMPLES;
      s_preview_buffer[preview_index++] = s_wave_ring[ring_index];
    }
  }
  s_preview_count = preview_index;

  return display_frequency_hz;
}

float TIM1_GetPeakToPeakmV(void)
{
  return s_last_vpp_mv;
}

size_t TIM1_GetWaveformPreview(uint16_t *dest, size_t max_samples)
{
  if ((dest == NULL) || (max_samples == 0U))
  {
    return 0U;
  }

  const size_t to_copy = (s_preview_count < max_samples) ? s_preview_count : max_samples;
  memcpy(dest, s_preview_buffer, to_copy * sizeof(uint16_t));
  return to_copy;
}

/* USER CODE END 1 */
