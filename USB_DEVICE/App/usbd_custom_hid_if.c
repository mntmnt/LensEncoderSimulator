/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.c
  * @version        : v2.0_Cube
  * @brief          : USB Device Custom HID interface file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_custom_hid_if.h"

/* USER CODE BEGIN INCLUDE */
#include "../../board-common/encoder/usbh/usb_fs_setup.h" ///MODIFIED_TAG
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @addtogroup USBD_CUSTOM_HID
  * @{
  */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions USBD_CUSTOM_HID_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Defines USBD_CUSTOM_HID_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Macros USBD_CUSTOM_HID_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Variables USBD_CUSTOM_HID_Private_Variables
  * @brief Private variables.
  * @{
  */

/** Usb HID report descriptor. */
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
  /* USER CODE BEGIN 0 */
		// COPIED FROM ATMEL PRJECT
		0x06, 0xFF, 0xFF,	// 04|2   , Usage Page (vendor defined?)
		0x09, 0x01,	// 08|1   , Usage      (vendor defined
		0xA1, 0x01,	// A0|1   , Collection (Application)
		// IN report
		0x09, 0x02,	// 08|1   , Usage      (vendor defined)
		0x09, 0x03,	// 08|1   , Usage      (vendor defined)
		0x15, 0x00,	// 14|1   , Logical Minimum(0 for signed byte?)
		0x26, 0xFF, 0x00,	// 24|1   , Logical Maximum(255 for signed byte?)
		0x75, 0x08,	// 74|1   , Report Size(8) = field size in bits = 1 byte
		// 94|1   , ReportCount(size) = repeat count of previous item
		0x95, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE /* sizeof(udi_hid_generic_report_in) */,
		0x81, 0x02,	// 80|1   , IN report (Data,Variable, Absolute)
		// OUT report
		0x09, 0x04,	// 08|1   , Usage      (vendor defined)
		0x09, 0x05,	// 08|1   , Usage      (vendor defined)
		0x15, 0x00,	// 14|1   , Logical Minimum(0 for signed byte?)
		0x26, 0xFF, 0x00,	// 24|1   , Logical Maximum(255 for signed byte?)
		0x75, 0x08,	// 74|1   , Report Size(8) = field size in bits = 1 byte
		// 94|1   , ReportCount(size) = repeat count of previous item
		0x95, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE /* sizeof(udi_hid_generic_report_out) */,
		0x91, 0x02,	// 90|1   , OUT report (Data,Variable, Absolute)
		// Feature report
		0x09, 0x06,	// 08|1   , Usage      (vendor defined)
		0x09, 0x07,	// 08|1   , Usage      (vendor defined)
		0x15, 0x00,	// 14|1   , LogicalMinimum(0 for signed byte)
		0x26, 0xFF, 0x00,	// 24|1   , Logical Maximum(255 for signed byte)
		0x75, 0x08,	// 74|1   , Report Size(8) =field size in bits = 1 byte
		0x95, 4 /* sizeof(udi_hid_generic_report_feature) */,	// 94|x   , ReportCount in byte
		0xB1, 0x02,	// B0|1   , Feature report
		/* 0xC0 */	// C0|0   , End Collection  **! Commented here because STM generates it by default
  /* USER CODE END 0 */
  0xC0    /*     END_COLLECTION	             */
};

/* USER CODE BEGIN PRIVATE_VARIABLES */
///MODIFIED_TAG
static_assert( (USBD_CUSTOMHID_OUTREPORT_BUF_SIZE) == 4, "Very important value. Should be the same as sizeof in report. If descriptor's report is less, no data receive */" );
static_assert( (USBD_CUSTOM_HID_REPORT_DESC_SIZE) == 53, "should be equal" );
static_assert( (CUSTOM_HID_EPIN_SIZE)  == 4,  "should have expected size" );
static_assert( (CUSTOM_HID_EPOUT_SIZE) == 4,  "should have expected size" );
static_assert( (CUSTOM_HID_FS_BINTERVAL) == 1, "bInterval should be as little as possible for our purposes. [1;0xFF]" );
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
  * @brief Public variables.
  * @{
  */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CUSTOM_HID_Init_FS(void);
static int8_t CUSTOM_HID_DeInit_FS(void);
static int8_t CUSTOM_HID_OutEvent_FS(const uint8_t * buf, int code /* E0 == RX0, 0xDA0 == DataOut*/); ///MODIFIED_TAG

/**
  * @}
  */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS
};

/** @defgroup USBD_CUSTOM_HID_Private_Functions USBD_CUSTOM_HID_Private_Functions
  * @brief Private functions.
  * @{
  */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_Init_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  DeInitializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_DeInit_FS(void)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Manage the CUSTOM HID class events
  * @param  event_idx: Event index
  * @param  state: Event state
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_OutEvent_FS(const uint8_t * buf, int code /* E0 == RX0, 0xDA0 == DataOut*/) ///MODIFIED_TAG
{
  /* USER CODE BEGIN 6 */
	myhid_data_arrived(buf, code); ///MODIFIED_TAG

  /* Start next USB packet transfer once data processing is completed */
  USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceFS);

  return (USBD_OK);
  /* USER CODE END 6 */
}

/* USER CODE BEGIN 7 */
/**
  * @brief  Send the report to the Host
  * @param  report: The report to be sent
  * @param  len: The report length
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */

///MODIFIED_TAG: usually it is commented
int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len)
{
  return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, len);
}
/**/
/* USER CODE END 7 */

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

