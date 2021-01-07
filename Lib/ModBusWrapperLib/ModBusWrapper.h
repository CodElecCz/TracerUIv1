#pragma once

#include "stdafx.h"
#include "ModBusWrapperException.h"

using namespace std;

namespace Utilities
{
	
	class LIB_API ModBusWrapperLib
	{
	public:
		ModBusWrapperLib(void);
		~ModBusWrapperLib(void);

		//Singelton
		ModBusWrapperLib(ModBusWrapperLib const&) = delete;
		ModBusWrapperLib& operator=(ModBusWrapperLib const&) = delete;
		static std::shared_ptr<ModBusWrapperLib> Instance()
		{
			static std::shared_ptr<ModBusWrapperLib> s{ new ModBusWrapperLib };
			return s;
		}

		void Initialize();
		void Start();
		void Stop();

		void Read(const char* Path, PUINT16 Val, bool WaitForNewDataAvailable = false);
		void Write(const char* Path, UINT16 Val, bool WaitForWriteDataAcknowledge = false);
		HANDLE GetNode(const char* path);

	private:
		void AppendNode(HANDLE xml_node, HANDLE node);

	private:
		void* m_modbus;
	};
}