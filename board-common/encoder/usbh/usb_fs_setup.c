#include "../../encoder/usbh/usb_fs_setup.h"
#include "../../encoder/encoder/impulse-encoder.h"

#include "main.h"

#include "usbd_conf.h"
#include <math.h>
#include <string.h>

#include "usb_device.h"

int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *, uint16_t);

enum Constants {
	MaxTickCount       = 1843, //0xFFFF,
	IgnorableByte      = 0xFF,
	StatusBytePosition = 1,
	RotationPosition   = 2,
	FlashLightBit      = 0x80,
	ZeroPosBit         = 0x40,
	BlackoutBit        = 0x20,

	ReportInterval_ms  = 100,
	CommonReportSize   = (USBD_CUSTOMHID_OUTREPORT_BUF_SIZE),
};

static uint8_t gs_hidreport[CommonReportSize] = { 0x00 };

static volatile bool gs_hidrequest_arrived = false;
static uint8_t gs_hidrequest_buffer[CommonReportSize] = { 0x00 };

static bool gs_FlashLightOn = true;
static bool gs_BlackoutOn = false;
static bool gs_Zerobit = true;

static uint32_t gs_PreviousCall = 0;
static bool gs_PreviousCallSet = false;


static void process_feature_request(const uint8_t * income) {
	switch ( income[0] ) {
	case 2:
		///[[pass]] led_onLight
		break;
	case 3:
		///[[pass]] led_offLight
		break;
	}
}


static void process_data_out_request(const uint8_t * income) {
	memcpy(gs_hidrequest_buffer, income, CommonReportSize);
	gs_hidrequest_arrived = true;
}


static void compose_status(void) {
	gs_hidreport[0] = 0x00;
	gs_hidreport[StatusBytePosition] = (gs_FlashLightOn ? FlashLightBit : 0x00) |
					  (gs_Zerobit ? ZeroPosBit : 0x00) |
					  (gs_BlackoutOn   ? BlackoutBit : 0x00);

	const uint16_t rotation = impulse_encoder_get_pos();
	gs_hidreport[RotationPosition]     =  (rotation >> 8) & 0xFFu;
	gs_hidreport[RotationPosition + 1] =  (rotation) & 0xFFu;
}


static void report_status(void) {
	compose_status();
	USBD_CUSTOM_HID_SendReport_FS((uint8_t*)gs_hidreport, sizeof(gs_hidreport));
}


static bool is_report_timeout(void) {
	uint32_t currentTick = HAL_GetTick();

	if (! gs_PreviousCallSet ) {
		gs_PreviousCall = currentTick;
		gs_PreviousCallSet = true;
	}

	if ( (currentTick - gs_PreviousCall) >= ReportInterval_ms ) {
		gs_PreviousCall = currentTick;
		return true;
	}
	return false;
}


static void process_request(void) {
	if ( gs_hidrequest_buffer[StatusBytePosition] != IgnorableByte ) {
		gs_FlashLightOn = (gs_hidrequest_buffer[StatusBytePosition] & FlashLightBit) == FlashLightBit;
		gs_BlackoutOn   = (gs_hidrequest_buffer[StatusBytePosition] & BlackoutBit)   == BlackoutBit;
		if ( (gs_hidrequest_buffer[StatusBytePosition] & ZeroPosBit) == ZeroPosBit ) {
			impulse_encoder_zero();
			gs_Zerobit = true;
		}
	}
}


void myhid_process_request() {
	if ( gs_hidrequest_arrived ) {
		gs_hidrequest_arrived = false;

		process_request();
	} else {
		if ( is_report_timeout() ) {
			report_status();
		}
	}
}


void myhid_data_arrived(const uint8_t * income, HidReportTypeCode code) {
	// Probably Feature report
	switch ( code ) {
	case FeatureRequest_RX0:  // Feature request
		process_feature_request(income);
		break;
	case DataRequest_DataOUT: // DataOut (hid_write)
		process_data_out_request(income);
		break;
	default:
		assert_param(false);
	}
}
