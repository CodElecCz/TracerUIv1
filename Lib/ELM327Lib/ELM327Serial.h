#pragma once

#include "stdafx.h"
#include "SerialProtocol.h"

namespace Utilities
{
	namespace ELM327
	{
		class ELM327Serial : public SerialProtocol
		{
		public:
			ELM327Serial(void) : SerialProtocol() 
			{}

			ELM327Serial(const char* COMPort, SerialProtocol::BaudRate Baudrate = BR_9600, SerialProtocol::DataBits Databits = DB_8, SerialProtocol::Parity Parity = P_NONE, SerialProtocol::StopBits Stopbit = SB_ONE)
				: SerialProtocol(COMPort, Baudrate, Databits, Parity, Stopbit) 
			{}

			~ELM327Serial(void) 
			{}

		public:
			void InitCmd(UINT8 ReadData[], PUINT32 ReadCount);
			void Query(UINT8 WriteData[], UINT32 WriteCount, UINT8 ReadData[], PUINT32 ReadCount);
			void Read(UINT8 ReadData[], PUINT32 ReadCount);
			void Write(UINT8 WriteData[], UINT32 WriteCount);
			bool IsInitialized(void) { return SerialProtocol::IsInitialized(); }
			const char* GetCOMPort() { return SerialProtocol::GetCOMPort(); }
			int GetTimeout() { return SerialProtocol::GetTimeout(); }

		private:
			CLock m_Lock;
		};
	}
}

