#pragma once

#include "stdafx.h"
#include "TcpProtocol.h"

using namespace Utilities;

namespace Utilities
{
	namespace ModBus
	{
		class ModBusTcp : private TcpProtocol
		{
		public:
			ModBusTcp(void) : TcpProtocol(), m_Id(0) {};
			ModBusTcp(const char* IPaddress) : TcpProtocol(IPaddress), m_Id(0) {};
			~ModBusTcp(void) {};

		public:
			void Query(UINT8 WriteData[], UINT16 WriteCount, UINT8 ReadData[], PUINT32 ReadCount, UINT32 ReadCountRequest);
			bool IsInitialized(void) { return TcpProtocol::IsInitialized(); };
			const char* GetIPaddress() { return TcpProtocol::GetIPaddress(); };
			int GetTimeout() { return TcpProtocol::GetTimeout(); };

		private:
			unsigned int m_Id; //Transaction Identifier
			CLock m_ModBusTcpLock;
		};
	}
}

