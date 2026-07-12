#pragma once
#include "main.h"

void impulse_encoder_setup(TIM_HandleTypeDef *);
void impulse_encoder_update(TIM_HandleTypeDef *);
void impulse_encoder_tick(TIM_HandleTypeDef *);

uint16_t impulse_encoder_get_pos(void);
void impulse_encoder_zero(void);
