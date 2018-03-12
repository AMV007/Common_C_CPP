#ifndef __USB_DEFS_H__
#define __USB_DEFS_H__

#include <stdint.h>

#define LUSB_OK											0
#define LUSB_ERROR_INVALID_PARAMETERS					-1001
#define LUSB_ERROR_OPEN_DEVICE							-1002
#define LUSB_ERROR_CLOSE_DEVICE							-1003
#define LUSB_ERROR_DEVICE_CLOSED						-1004
#define LUSB_ERROR_CANCEL_IO							-1005
#define LUSB_ERROR_TIMEOUT_EXPIRED						-1006
#define LUSB_ERROR_ASYNC_ERROR							-1007
#define LUSB_ERROR_ASYNC_DATA_TRNSFER_LENGTH			-1008
#define LUSB_ERROR_ASYNC_DATA_TRNSFER					-1009
#define LUSB_ERROR_READ_FIFO_THREAD_DEAD				-1010
#define LUSB_ERROR_WRITE_FIFO_THREAD_DEAD				-1011
#define LUSB_ERROR_WAIT_MUTEX							-1012
#define LUSB_WARNING_ASYNC_DATA_TRNSFER_LENGTH			-1013

#define MAX_CONTROL_TRANSFER_SIZE						4096

#define BULK_ENDPOINT_SIZE								512

#pragma pack(1)
typedef struct 
{
	uint16_t RequestDirectionIN; // 1 - получить данные 0 - только осослать
	uint16_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
}_ControlRequest;
#pragma pack()

#endif