#include "stdafx.h"
#include "ModBusTCP.h"

namespace Utilities
{
	namespace ModBus
	{
#ifdef TCP_LOG
		void Log(const char* IPaddress, const char* id, UINT8 data[], UINT32 dataSize, SYSTEMTIME t)
		{
			vector<char> sbuffer(dataSize*3 + 64);
			char* pend = sbuffer.data();

			pend += sprintf(pend, "[%02d:%02d:%02d.%03d] %s[%s] 0x%x ", (int)t.wHour, (int)t.wMinute, (int)t.wSecond, (int)t.wMilliseconds, id, IPaddress, GetCurrentThreadId());
			//header
			pend += sprintf(pend, "H[");
			for (int i = 0; i < 7; i++)
				pend += sprintf(pend, "%02x ", (unsigned int)data[i]);
			pend--;
			pend += sprintf(pend, "] ");
			//data
			pend += sprintf(pend, "D[");
			for (unsigned int i = 7; i < dataSize; i++)
				pend += sprintf(pend, "%02x ", (unsigned int)data[i]);
			pend--;
			pend += sprintf(pend, "]\n");
			printf(sbuffer.get());
		}
#endif
		void ModBusTcp::Query(UINT8 WriteData[], UINT16 WriteCount, UINT8 ReadData[], PUINT32 ReadCount, UINT32 ReadCountRequest)
		{
			CAutoLock al(m_ModBusTcpLock);
			
			unsigned int ReadDataAlloc = *ReadCount;
			*ReadCount = 0;

			//Header
			vector<UINT8> wdata(WriteCount+7);
			unsigned int wdataSize = 0;

			//Transaction Identifier - It is used for transaction pairing, the MODBUS server copies in the response the transaction identifier of the request
			m_Id++;
			if(m_Id>=0xffff) m_Id = 1;

			wdata[0] = (unsigned char)(m_Id>>8);
			wdata[1] = (unsigned char)(m_Id&0xff);

			//Protocol Identifier – It is used for intra-system multiplexing. The MODBUS protocol is identified by the value 0
			wdata[2] = 0;
			wdata[3] = 0;

			//Length - The length field is a byte count of the following fields, including the Unit Identifier and data fields
			wdata[4] = (unsigned char)((WriteCount+1)>>8);
			wdata[5] = (unsigned char)((WriteCount+1)&0xff);

			//Unit Identifier – This field is used for intra-system routing purpose. It is typically used to communicate to a MODBUS+ or a MODBUS serial line slave through a gateway between an Ethernet TCP-IP network and a MODBUS serial line. This field is set by the MODBUS Client in the request and must be returned with the same value in the response by the server.
			wdata[6] = 1;

			int i;
			for(i = 0; i<WriteCount; i++)
			{
				wdata[i+7] = WriteData[i];
			}
			wdataSize = i+7;

			//TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes
			if(wdataSize>260)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus message too big >260bytes, actual size: %d bytes", wdataSize);
				throw Exc(GetIPaddress(), msg);
			}

			unsigned int readCountRequest = 7 + ReadCountRequest; //header + data

			vector<UINT8> rdata(readCountRequest);
			unsigned int rdataSize = readCountRequest;
			
#ifdef TCP_LOG
			SYSTEMTIME wt;
			GetSystemTime(&wt);
			Log(Socket::GetIPaddress(), "W", wdata.get(), wdataSize, wt);
#endif
			try
			{
				TcpProtocol::Query(wdata.data(), wdataSize, rdata.data(), &rdataSize);
			}
			catch(char* &e)
			{
				throw Exc(GetIPaddress(), (const char*)e);
			}
			
#ifdef TCP_LOG
			SYSTEMTIME rt;
			GetSystemTime(&rt);
			Log(Socket::GetIPaddress(), "R", rdata.get(), rdataSize, rt);
#endif
			if(rdataSize<6)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus header, return size too small <6bytes");
				throw Exc(GetIPaddress(), msg);
			}

			//check header
			unsigned int id = (unsigned int)(rdata[0]<<8) + (unsigned int)rdata[1];
			if(id!=m_Id)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus header, Id fault; request: %d, response: %d", m_Id, id);
				throw Exc(GetIPaddress(), msg);
			}

			unsigned int readCount = (unsigned int)(rdata[4]<<8) + (unsigned int)rdata[5] - 1; //without unit id

			//negativ response
			if(readCount==2 && rdata[7]==(WriteData[0]+0x80))
			{
				//check it in higher level, for negativ response type
			}
			else if(readCount!=ReadCountRequest)
			{
				//TODO: Try to read rest of message from TCP
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus message, data length fault; request: %d, response: %d", ReadCountRequest, readCount);
				throw Exc(GetIPaddress(), msg);
			}

			unsigned char unitId = rdata[6]; 

			unsigned int sizeCopyTo = (readCount<ReadDataAlloc)? readCount:ReadDataAlloc;
			memcpy(ReadData, &rdata[7], sizeCopyTo);
			*ReadCount = readCount;
		}
	}
}