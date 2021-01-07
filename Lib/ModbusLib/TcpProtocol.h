#pragma once

#include <WinSock2.h>
#include "stdafx.h"

using namespace std;

namespace Utilities
{
	namespace ModBus
	{
		class LIB_API TcpProtocol
		{
		public:
			TcpProtocol(void);
			TcpProtocol(const char* IPadress, UINT16 port = 502);
			~TcpProtocol(void);

		public:
			bool IsInitialized(void) { return m_WSAStartup; };

			void Connect(const char* IPaddress, UINT16 port = 502);
			bool IsConnected() { return m_Socket != INVALID_SOCKET; };
			void Disconnect();

			void Send(UINT8 WriteData[], UINT32 WriteCount);
			void Receive(UINT8 ReadData[], PUINT32 ReadCount, UINT32 ReadCountRequest); /* Receive until ReadCountRequest or Timeout */
			void Query(UINT8 WriteData[], UINT32 WriteCount, UINT8 ReadData[], PUINT32 ReadCount, UINT32 ReadCountRequest = 0); /* ReadCountRequest==0 read any; ReadCountRequest!=0 read requested */

			const char* GetIPaddress() { return m_IPaddress.c_str(); };
			int GetTimeout() { return m_Timeout; };

		private:
			void socket_send(UINT8 WriteData[], UINT32 WriteCount);
			void socket_receive(UINT8 ReadData[], PUINT32 ReadCount, UINT32 ReadCountRequest);

		private:
			SOCKET m_Socket;
			bool m_WSAStartup;
			UINT16 m_Port;
			int m_Timeout;
#pragma warning(push)
#pragma warning(disable:4251)
			string m_IPaddress;
			CLock m_SocketLock;
#pragma warning(pop)
		};
	}
}
