// ModBusWrapper.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "ModBusWrapperC.h"

#include "ModBusWrapper.h"
#include "ModBusWrapperException.h"

#include "..\ModbusLib\ModBusLib.h"

using namespace std;
using namespace Utilities;

static ModBusWrapperException m_Exception = ModBusWrapperException();

int ModBus_Start(struct _SModBusWrapper* me)
{
	if (ModBusWrapperLib* p = reinterpret_cast<ModBusWrapperLib*>(me->_h))
	{
		try
		{
			p->Start();
		}
		catch (ModBusWrapperException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch (...)
		{
			m_Exception = ModBusWrapperException("ModBus_Start() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_Stop(struct _SModBusWrapper* me)
{
	if (ModBusWrapperLib* p = reinterpret_cast<ModBusWrapperLib*>(me->_h))
	{
		try
		{
			p->Stop();
		}
		catch (ModBusWrapperException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch (...)
		{
			m_Exception = ModBusWrapperException("ModBus_Stop() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_Initialize(struct _SModBusWrapper* me)
{
	if (ModBusWrapperLib* p = reinterpret_cast<ModBusWrapperLib*>(me->_h))
	{
		try
		{
			p->Initialize();
		}
		catch (ModBusWrapperException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch (...)
		{
			m_Exception = ModBusWrapperException("ModBus_Stop() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_Read(struct _SModBusWrapper* me, const char* path, PUINT16 val)
{
	if (ModBusWrapperLib* p = reinterpret_cast<ModBusWrapperLib*>(me->_h))
	{
		try
		{
			p->Read(path, val);
		}
		catch (ModBusWrapperException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch (...)
		{
			m_Exception = ModBusWrapperException("ModBus_Stop() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_Write(struct _SModBusWrapper* me, const char* path, UINT16 val)
{
	if (ModBusWrapperLib* p = reinterpret_cast<ModBusWrapperLib*>(me->_h))
	{
		try
		{
			p->Write(path, val);
		}
		catch (ModBusWrapperException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch (...)
		{
			m_Exception = ModBusWrapperException("ModBus_Stop() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_OnValueChanged(struct _SModBusWrapper* me, const char* path, OnValueChanged_t EventFncPtr)
{
	if (ModBusWrapperLib* p = reinterpret_cast<ModBusWrapperLib*>(me->_h))
	{
		try
		{
			INode* node = reinterpret_cast<INode*>(p->GetNode(path));

			node->OnValueChanged += EventFncPtr;
		}
		catch (ModBusWrapperException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch (...)
		{
			m_Exception = ModBusWrapperException("ModBus_Stop() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

void modbus_new(SModBusPtr* pModBus)
{
	if (ModBusWrapperLib* p = new ModBusWrapperLib())
	{
		SModBusPtr pmodbus = new SModBus();
		pmodbus->_h = reinterpret_cast<HANDLE>(p);
		
		pmodbus->Start = ModBus_Start;
		pmodbus->Stop = ModBus_Stop;
		pmodbus->Initialize = ModBus_Initialize;
		pmodbus->Read = ModBus_Read;
		pmodbus->Write = ModBus_Write;
		pmodbus->OnValueChanged = ModBus_OnValueChanged;
		*pModBus = pmodbus;
	}
}

void modbus_delete(SModBusPtr* pModBus)
{
	SModBusPtr pmodbus = *pModBus;
	if (pmodbus != NULL)
	{
		if (ModBusWrapperLib* p = reinterpret_cast<ModBusWrapperLib*>(pmodbus->_h))
		{
			p->Stop();
			delete p;
		}
		delete pmodbus;
		*pModBus = NULL;
	}
}

void ModBusGetLastError(char* msg, unsigned int msgSize)
{
	if (!m_Exception.empty())
	{
		sprintf_s(msg, msgSize, "%s; id:%s; func:%s; file:%s; line:%d", m_Exception.what(), m_Exception.id(), m_Exception.fnc(), m_Exception.file(), m_Exception.line());
	}
	else
	{
		sprintf_s(msg, msgSize, "");
	}
}

