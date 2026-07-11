#include "../../encoder/usbh/usb_fs_setup.h"

#include "main.h"

#include "usbd_conf.h"
#include <math.h>
#include <string.h>

#include "usb_device.h"

int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *, uint16_t);
#define UDI_HID_REPORT_IN_SIZE (USBD_CUSTOMHID_OUTREPORT_BUF_SIZE)
uint8_t gUiHidReport[UDI_HID_REPORT_IN_SIZE] = { 0x00 };

static volatile bool usbhid_transfer_arrived = false;
static uint8_t usbhid_input_buffer[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE] = { 0x00 };


static void process_feature_request(const uint8_t * income) {
	switch ( income[0] ) {
	case 1:
		/// Some command. It is removed
		break;
	case 2:
		///[[pass]] led_onLight
		break;
	case 3:
		///[[pass]] led_offLight
		break;
	}
}


void udi_hid_generic_send_report_in(const uint8_t * buffer, uint16_t size) {
	UNUSED(size);
    USBD_CUSTOM_HID_SendReport_FS((uint8_t*)buffer, UDI_HID_REPORT_IN_SIZE);
}


static void process_data_out_request(const uint8_t * income) {
	memcpy(usbhid_input_buffer, income, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
	usbhid_transfer_arrived = true;
}


static bool gs_FlashLightOn = true;
static bool gs_BlackoutOn = false;
static uint16_t gs_Rotation = 0x00;
static bool gs_Zerobit = true;

static uint32_t gs_PreviosCall = 0;
static uint32_t gs_StartTick = 0;
static bool gs_PreviousCallSet = false;
const uint32_t ReportInterval_ms = 100;
enum Constants {
	MaxTickCount = 1843, //0xFFFF,
	IgnorableByte = 0xFF,
	StatusBytePosition = 1,
	FlashLightBit = 0x80,
	ZeroPosBit = 0x40,
	BlackoutBit = 0x20,
};

static void compose_status(void) {
	gUiHidReport[0] = 0x00;
	gUiHidReport[StatusBytePosition] = (gs_FlashLightOn ? FlashLightBit : 0x00) |
					  (gs_Zerobit ? ZeroPosBit : 0x00) |
					  (gs_BlackoutOn   ? BlackoutBit : 0x00);
	gUiHidReport[2] =  (gs_Rotation >> 8) & 0xFFu;
	gUiHidReport[3] =  (gs_Rotation) & 0xFFu;
}


static void report_status(void) {
	compose_status();
	udi_hid_generic_send_report_in(gUiHidReport, sizeof(gUiHidReport));
}


static bool report_timeout(void) {
	uint32_t currentTick = HAL_GetTick();

	if (! gs_PreviousCallSet ) {
		gs_PreviosCall = currentTick;
		gs_PreviousCallSet = true;
	}

	if ( (currentTick - gs_PreviosCall) >= ReportInterval_ms ) {
		gs_PreviosCall = currentTick;
		return true;
	}
	return false;
}


void fake_process_lens_simulation() {
	return;
	uint32_t currentTick = HAL_GetTick();
	if ( gs_StartTick == 0 || gs_StartTick > currentTick ) {
		gs_StartTick = currentTick;
	}
	uint32_t ticks = ((currentTick - gs_StartTick)) % (MaxTickCount);
	gs_Rotation = (uint16_t)( (1.0 + sin( (double)ticks / MaxTickCount * 3.1415)) / 2 * MaxTickCount );
}


void lens_simul_set_rotation(unsigned rotation, bool zeroBit) {
	if ( rotation > 0 ) {
		gs_Zerobit = false;
	}
	if ( rotation == 0 && zeroBit ) {
		gs_Zerobit = true;
	}
	gs_Rotation = rotation;
}


static void process_request(uint8_t * input, uint16_t size) {
	assert_param( size == 4 );
	if ( input[StatusBytePosition] != IgnorableByte ) {
		gs_FlashLightOn = (input[StatusBytePosition] & FlashLightBit) == FlashLightBit;
		gs_BlackoutOn   = (input[StatusBytePosition] & BlackoutBit)   == BlackoutBit;
		if ( (input[StatusBytePosition] & ZeroPosBit) == ZeroPosBit ) {
			gs_StartTick = 0;
			gs_Rotation  = 0;
			gs_Zerobit = true;
		}
	}
}


void myhid_process_request() {
	if ( usbhid_transfer_arrived ) {
		usbhid_transfer_arrived = false;

		process_request(usbhid_input_buffer, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
	} else {
		if ( report_timeout() ) {
			report_status();
		}
	}
}


void myhid_data_arrived(const uint8_t * income, HidReportTypeCode code) {
	// Probably Feature report
	switch ( code ) {
	case FeatureRequest_RX0: // probably, Feature request
		process_feature_request(income);
		break;
	case DataRequest_DataOUT: // probably, DataOut (hid_write)
		process_data_out_request(income);
		break;
	default:
		assert_param(false);
	}
}
