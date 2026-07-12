#include "impulse-encoder.h"
#include "main.h"

#include <stdio.h>

enum EncoderTick { EncoderTick_Unchanged, EncoderTick_Changed };
enum Constants {
	ENCODER_PULSES_PER_STEP = 4
};

static volatile enum EncoderTick gs_encoderTick = EncoderTick_Unchanged;
static uint32_t gs_currentEncoderValue = 0;
static TIM_HandleTypeDef * gs_encoderTimer = NULL;

static void process_encoder_impulses() {
    assert_param( gs_encoderTimer != NULL );

    const uint32_t current_ticks = __HAL_TIM_GET_COUNTER(gs_encoderTimer);
    gs_currentEncoderValue = current_ticks / ENCODER_PULSES_PER_STEP;
    printf("Lens Encoder: %d\r\n", (int)gs_currentEncoderValue);
}


void impulse_encoder_update(TIM_HandleTypeDef * htim) {
    assert_param( gs_encoderTimer != NULL );
    assert_param( htim == gs_encoderTimer );

    if ( gs_encoderTick == EncoderTick_Changed ) {
		process_encoder_impulses();
		gs_encoderTick = EncoderTick_Unchanged;
	}
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef * htim) {
	assert_param( gs_encoderTimer != NULL );
	assert_param( htim == gs_encoderTimer );

	if ( htim == gs_encoderTimer ) {
		gs_encoderTick = EncoderTick_Changed;
	}
}


void impulse_encoder_setup(TIM_HandleTypeDef * htim) {
	gs_encoderTimer = htim;
	HAL_TIM_Encoder_Start_IT(gs_encoderTimer, TIM_CHANNEL_ALL);
}


uint16_t impulse_encoder_get_pos(void) {
	return gs_currentEncoderValue & 0xFFFF;
}


void impulse_encoder_zero() {
	assert_param( gs_encoderTimer != NULL );
	__HAL_TIM_SET_COUNTER(gs_encoderTimer, 0);
}
