#ifndef MODBUSWRAPPERC_H
#define MODBUSWRAPPERC_H

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef void(*OnValueChanged_t)(const char*, UINT64);

	typedef struct _SModBusWrapper
	{
		int(*Initialize)(struct _SModBusWrapper* me);
		int(*Start)(struct _SModBusWrapper* me);
		int(*Stop)(struct _SModBusWrapper* me);
	
		int(*Read)(struct _SModBusWrapper* me, const char* path, PUINT16 val);
		int(*Write)(struct _SModBusWrapper* me, const char* path, UINT16 val);

		int(*OnValueChanged)(struct _SModBusWrapper* me, const char* path, OnValueChanged_t EventFncPtr);

		HANDLE _h;
	}SModBus, *SModBusPtr;

	LIB_API void modbus_new(SModBusPtr* pModBus);
	LIB_API void modbus_delete(SModBusPtr* pModBus);
	LIB_API void ModBusGetLastError(char* msg, unsigned int msgSize);

#ifdef __cplusplus 
}
#endif

#endif