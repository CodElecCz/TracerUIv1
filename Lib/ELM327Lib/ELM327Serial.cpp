#include "stdafx.h"
#include "ELM327Serial.h"

#include <algorithm>

namespace Utilities
{
	namespace ELM327
	{

#define SERIAL_LOG
#define SERIAL_TIMEOUT_DEFAULT	1000

		void Log(const char* COMPort, const char* id, UINT8 data[], UINT32 dataSize, SYSTEMTIME t)
		{
#ifdef _DEBUG
#ifdef SERIAL_LOG
			char header[128];
			sprintf_s(header, sizeof(header), "[%02d:%02d:%02d.%03d] %s[%s] 0x%x ", (int)t.wHour, (int)t.wMinute, (int)t.wSecond, (int)t.wMilliseconds, id, COMPort, GetCurrentThreadId());
			
			string sbuffer(header);
			sbuffer.append((char*)data, dataSize);

			std::replace(sbuffer.begin(), sbuffer.end(), 0x0d, 0x2e);

			const char eol[] = "\n";
			sbuffer.append(eol);

			printf(sbuffer.c_str());
#endif
#endif
		}

		void ELM327Serial::InitCmd(UINT8 ReadData[], PUINT32 ReadCount)
		{
			CAutoLock al(m_Lock);

			unsigned int ReadDataAlloc = *ReadCount;
			*ReadCount = 0;

			//Header
			vector<UINT8> wdata(2);
			unsigned int wdataSize = 1;
			wdata[0] = 0x0D; //<CR>
			wdata[1] = 0;
			
			vector<UINT8> rdata(ReadDataAlloc);
			unsigned int rdataSize = ReadDataAlloc;

			SYSTEMTIME wt;
			GetSystemTime(&wt);
			Log(GetCOMPort(), "W", wdata.data(), wdataSize, wt);

			try
			{
				//SerialProtocol::SetTimeout(SERIAL_TIMEOUT_DEFAULT);
				//SerialProtocol::SetTermChar(true, 0x3e); //0x0D
				//SerialProtocol::Query(wdata.data(), wdataSize, rdata.data(), &rdataSize);
				
				SerialProtocol::Write(wdata.data(), wdataSize);
				SerialProtocol::Clear();
			}
			catch (char* &e)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "%s", (const char*)e);
				throw Exc(GetCOMPort(), msg);
			}
#if 0
			SYSTEMTIME rt;
			GetSystemTime(&rt);
			Log(GetCOMPort(), "R", rdata.data(), rdataSize, rt);

			if (rdataSize > 0 && rdata[rdataSize-1] != 0x3e)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ELM327 response missing '>'");
				throw Exc(GetCOMPort(), msg);
			}

			memcpy(ReadData, &rdata[0], rdataSize - 0);
			*ReadCount = rdataSize - 0;
#endif
		}

		void ELM327Serial::Query(UINT8 WriteData[], UINT32 WriteCount, UINT8 ReadData[], PUINT32 ReadCount)
		{
			CAutoLock al(m_Lock);

			unsigned int ReadDataAlloc = *ReadCount;
			*ReadCount = 0;

			//Header
			vector<UINT8> wdata(WriteCount + 4);
			unsigned int wdataSize = 0;

			wdata[0] = 'A';
			wdata[1] = 'T';

			int i;
			for (i = 0; i < WriteCount; i++)
			{
				wdata[i + 2] = WriteData[i];
			}
			i += 2;
			wdata[i++] = 0x0D; //<CR>
			wdata[i] = 0;
			wdataSize = i;

			if (wdataSize > 2048)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Message too big >2048bytes, actual size: %d bytes", wdataSize);
				throw Exc(GetCOMPort(), msg);
			}

			vector<UINT8> rdata(ReadDataAlloc);
			unsigned int rdataSize = ReadDataAlloc;

			SYSTEMTIME wt;
			GetSystemTime(&wt);
			Log(GetCOMPort(), "W", wdata.data(), wdataSize, wt);

			try
			{
				SerialProtocol::SetTermChar(true, 0x3e); //0x0D
				SerialProtocol::SetTimeout(SERIAL_TIMEOUT_DEFAULT);
				SerialProtocol::Query(wdata.data(), wdataSize, rdata.data(), &rdataSize);
			}
			catch (char* &e)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "%s", (const char*)e);
				throw Exc(GetCOMPort(), msg);
			}

			SYSTEMTIME rt;
			GetSystemTime(&rt);
			Log(GetCOMPort(), "R", rdata.data(), rdataSize, rt);

			if (rdataSize > 0 && rdata[rdataSize - 1] != 0x3e)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ELM327 response missing '>'");
				throw Exc(GetCOMPort(), msg);
			}

			memcpy(ReadData, &rdata[0], rdataSize - 0);
			*ReadCount = rdataSize - 0;
		}

		void ELM327Serial::Read(UINT8 ReadData[], PUINT32 ReadCount)
		{
			CAutoLock al(m_Lock);

			unsigned int ReadDataAlloc = *ReadCount;
			*ReadCount = 0;

			vector<UINT8> rdata(ReadDataAlloc);
			unsigned int rdataSize = ReadDataAlloc;

			try
			{
				SerialProtocol::SetTermChar(false, 0);
				SerialProtocol::SetTimeout(100);
				SerialProtocol::Read(rdata.data(), &rdataSize);
			}
			catch (char* &e)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "%s", (const char*)e);
				throw Exc(GetCOMPort(), msg);
			}

			SYSTEMTIME rt;
			GetSystemTime(&rt);
			Log(GetCOMPort(), "R", rdata.data(), rdataSize, rt);

			if (rdataSize > 0 && rdata[rdataSize - 1] == 0x3e)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ELM327 log stopped, new command required '>'");
				throw Exc(GetCOMPort(), msg);
			}

			memcpy(ReadData, &rdata[0], rdataSize - 0);
			*ReadCount = rdataSize - 0;
		}

		void ELM327Serial::Write(UINT8 WriteData[], UINT32 WriteCount)
		{
			CAutoLock al(m_Lock);

			//Header
			vector<UINT8> wdata(WriteCount + 4);
			unsigned int wdataSize = 0;

			wdata[0] = 'A';
			wdata[1] = 'T';

			int i;
			for (i = 0; i<WriteCount; i++)
			{
				wdata[i + 2] = WriteData[i];
			}
			i += 2;
			wdata[i++] = 0x0D; //<CR>
			wdata[i] = 0;
			wdataSize = i;

			if (wdataSize>2048)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Message too big >2048bytes, actual size: %d bytes", wdataSize);
				throw Exc(GetCOMPort(), msg);
			}

			SYSTEMTIME wt;
			GetSystemTime(&wt);
			Log(GetCOMPort(), "W", wdata.data(), wdataSize, wt);

			try
			{
				SerialProtocol::Write(wdata.data(), wdataSize);
			}
			catch (char* &e)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "%s", (const char*)e);
				throw Exc(GetCOMPort(), msg);
			}
		}
	}
}