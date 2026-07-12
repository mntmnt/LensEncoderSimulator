#pragma once

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
///MODIFIED_TAG

typedef enum HidReportTypeEnum {
	FeatureRequest_RX0  = 0xE0,
	DataRequest_DataOUT = 0xDA0
} HidReportType;
typedef int HidReportTypeCode;

void myhid_data_arrived(const uint8_t *, HidReportTypeCode);
void myhid_process_request(void);
