#pragma once	

#include "ModBusTCP.h"

namespace Utilities
{
	namespace ModBus
	{

#define MODULE_OFFSET		6		//words
#define NB_OF_MODULES		4

		typedef struct STurckRfidRStatusByte {
				unsigned RESERVED:1, TAG_FREAD:1, TAG_PRESENT:1, XCVR_ON:1, XCVR_CONNECTED:1, ERR:1, BUSY:1, DONE:1;
		} STurckRfidRStatusByte;

		const char* const CTurckRfidRStatusByte[] = {"RESERVED", "TAG_FREAD", "TAG_PRESENT", "XCVR_ON", "XCVR_CONNECTED", "ERR", "BUSY", "DONE"};

		typedef struct STurckRfidWStatusByte {
				unsigned RESET:1, XCVR_INFO:1, TAG_INFO:1, WRITE:1, READ:1, TAGID:1, NEXT:1, XCVR:1;
		} STurckRfidWStatusByte;

		const char* const CTurckRfidWStatusByte[] = {"RESET", "XCVR_INFO", "TAG_INFO", "WRITE", "READ", "TAGID", "NEXT", "XCVR"};

		union TurckRfidWUStatusByte 
		{
			STurckRfidRStatusByte data;
			UINT8 byte;
		};

		union TurckRfidRUStatusByte {
			STurckRfidWStatusByte data;
			UINT8 byte;
		};

		typedef struct _STurckRfidRData
		{
			TurckRfidWUStatusByte Status;
			UINT8 ERROR1;
			UINT8 RES;
			UINT8 ERROR2;
			UINT8 Data[8];
		}STurckRfidRData;

		typedef struct _STurckRfidWData
		{
			TurckRfidRUStatusByte Status;
			UINT8 Size;
			UINT16 Address;
			UINT8 Data[8];
		}STurckRfidWData;

		union TurckRfidRData
		{
			STurckRfidRData data;
			UINT8 byte[MODULE_OFFSET*2];
			UINT16 word[MODULE_OFFSET];
		};

		union TurckRfidWData
		{
			STurckRfidWData data;
			UINT8 byte[MODULE_OFFSET*2];
			UINT16 word[MODULE_OFFSET];
		};

		typedef struct _STurckRfidRDataModules
		{
			TurckRfidRData ID0;
			TurckRfidRData ID1;
			TurckRfidRData ID2;
			TurckRfidRData ID3;
		} STurckRfidRDataModules;

		typedef struct _STurckRfidWDataModules
		{
			TurckRfidWData ID0;
			TurckRfidWData ID1;
			TurckRfidWData ID2;
			TurckRfidWData ID3;
		} STurckRfidWDataModules;

		union TurckRfidRDataModules
		{
			STurckRfidRDataModules data;
			UINT8 byte[NB_OF_MODULES*MODULE_OFFSET*2];
			UINT16 word[NB_OF_MODULES*MODULE_OFFSET];
		};

		union TurckRfidWDataModules
		{
			STurckRfidWDataModules data;
			UINT8 byte[NB_OF_MODULES*MODULE_OFFSET*2];
			UINT16 word[NB_OF_MODULES*MODULE_OFFSET];
		};
	}
}