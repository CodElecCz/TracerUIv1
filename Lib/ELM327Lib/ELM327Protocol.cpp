#include "stdafx.h"
#include "ELM327Protocol.h"

#define ELM327_READDATA_BUFFER_SIZE 1024

namespace Utilities
{
	namespace ELM327
	{
		bool ELM327Protocol::IsInitialized(void)
		{
			switch (m_Type)
			{
			
			case ELM327ProtocolSerial:
				return ELM327Serial::IsInitialized();
			default:
				return false;
			}
		}

		const char* ELM327Protocol::GetDeviceId()
		{
			switch (m_Type)
			{
			
			case ELM327ProtocolSerial:
				return ELM327Serial::GetCOMPort();
			default:
				return "";
			}
		}

		int ELM327Protocol::GetTimeout()
		{
			switch (m_Type)
			{
			
			case ELM327ProtocolSerial:
				return ELM327Serial::GetTimeout();
			default:
				return -1;
			}
		}

		void ELM327Protocol::Enter()
		{
			UINT8 rdata[8 * ELM327_READDATA_BUFFER_SIZE];
			unsigned int rdataSize = sizeof(rdata);
			memset(rdata, 0, sizeof(rdata));

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::InitCmd(rdata, &rdataSize);
				break;
			}
		}

		void ELM327Protocol::Reset()
		{
			UINT8 wdata[16];
			int i = 0;
			wdata[i++] = 'Z';
			UINT16 wdataSize = i;

			UINT8 rdata[ELM327_READDATA_BUFFER_SIZE];
			unsigned int rdataSize = sizeof(rdata);
			memset(rdata, 0, sizeof(rdata));

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Query(wdata, wdataSize, rdata, &rdataSize);
				break;
			}
		}

		void ELM327Protocol::ResponseEcho(bool val)
		{
			UINT8 wdata[16];
			int i = 0;
			wdata[i++] = 'E';
			wdata[i++] = (val) ? '1' : '0';
			UINT16 wdataSize = i;

			UINT8 rdata[ELM327_READDATA_BUFFER_SIZE];
			unsigned int rdataSize = sizeof(rdata);
			memset(rdata, 0, sizeof(rdata));

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Query(wdata, wdataSize, rdata, &rdataSize);
				break;
			}
		}

		void ELM327Protocol::CANProtocol(ELM327CommProtocolType protocol)
		{
			UINT8 wdata[16];
			int i = 0;
			wdata[i++] = 'S';
			wdata[i++] = 'P';
			wdata[i++] = protocol + 0x30;
			UINT16 wdataSize = i;

			UINT8 rdata[ELM327_READDATA_BUFFER_SIZE];
			unsigned int rdataSize = sizeof(rdata);
			memset(rdata, 0, sizeof(rdata));

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Query(wdata, wdataSize, rdata, &rdataSize);
				break;
			}
		}

		void ELM327Protocol::SetLongMsg()
		{
			UINT8 wdata[16];
			int i = 0;
			wdata[i++] = 'A';
			wdata[i++] = 'L';
			UINT16 wdataSize = i;

			UINT8 rdata[ELM327_READDATA_BUFFER_SIZE];
			unsigned int rdataSize = sizeof(rdata);
			memset(rdata, 0, sizeof(rdata));

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Query(wdata, wdataSize, rdata, &rdataSize);
				break;
			}
		}

		void ELM327Protocol::DisplaySpace(bool val)
		{
			UINT8 wdata[16];
			int i = 0;
			wdata[i++] = 'S';
			wdata[i++] = (val) ? '1' : '0';
			UINT16 wdataSize = i;

			UINT8 rdata[ELM327_READDATA_BUFFER_SIZE];
			unsigned int rdataSize = sizeof(rdata);
			memset(rdata, 0, sizeof(rdata));

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Query(wdata, wdataSize, rdata, &rdataSize);
				break;
			}
		}

		void ELM327Protocol::DisplayHeader(bool val)
		{
			UINT8 wdata[16];
			int i = 0;
			wdata[i++] = 'H';
			wdata[i++] = (val) ? '1' : '0';
			UINT16 wdataSize = i;

			UINT8 rdata[ELM327_READDATA_BUFFER_SIZE];
			unsigned int rdataSize = sizeof(rdata);
			memset(rdata, 0, sizeof(rdata));

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Query(wdata, wdataSize, rdata, &rdataSize);
				break;
			}
		}

		void ELM327Protocol::DisplayDataLength(bool val)
		{
			UINT8 wdata[16];
			int i = 0;
			wdata[i++] = 'D';
			wdata[i++] = (val) ? '1' : '0';
			UINT16 wdataSize = i;

			UINT8 rdata[ELM327_READDATA_BUFFER_SIZE];
			unsigned int rdataSize = sizeof(rdata);
			memset(rdata, 0, sizeof(rdata));

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Query(wdata, wdataSize, rdata, &rdataSize);
				break;
			}
		}

		void ELM327Protocol::DisplayLineFeed(bool val)
		{
			UINT8 wdata[16];
			int i = 0;
			wdata[i++] = 'L';
			wdata[i++] = (val) ? '1' : '0';
			UINT16 wdataSize = i;

			UINT8 rdata[ELM327_READDATA_BUFFER_SIZE];
			unsigned int rdataSize = sizeof(rdata);
			memset(rdata, 0, sizeof(rdata));

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Query(wdata, wdataSize, rdata, &rdataSize);
				break;
			}
		}

		void ELM327Protocol::CANAutoFormating(bool val)
		{
			UINT8 wdata[16];
			int i = 0;
			wdata[i++] = 'C';
			wdata[i++] = 'A';
			wdata[i++] = 'F';
			wdata[i++] = (val)? '1':'0';
			UINT16 wdataSize = i;

			UINT8 rdata[ELM327_READDATA_BUFFER_SIZE];
			unsigned int rdataSize = sizeof(rdata);
			memset(rdata, 0, sizeof(rdata));

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Query(wdata, wdataSize, rdata, &rdataSize);
				break;
			}
		}

		void ELM327Protocol::StartMonitoring()
		{
			UINT8 wdata[16];
			int i = 0;
			wdata[0] = 'M';
			wdata[1] = 'A';
			UINT16 wdataSize = 2;

			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Write(wdata, wdataSize);
				break;
			}
		}

		void ELM327Protocol::TraceInit()
		{
			/*
			ATE0	//response echo OFF
			ATSP6	//ISO 15765-4 (CAN 11/500) and save it
			ATAL	//Set long messages ON
			ATS1	//Space ON [08F 00 00 00 00 00 00 00 00]
			ATH1	//Set header ON [08F 00 00 00 00 00 00 00 00]
			ATD1	//Display data lenght ON [08F 8 00 00 00 00 00 00 00 00]
			ATL0	//Line feed OFF
			ATCAF0	//CAN Auto Formatting OFF
			ATMA	//Start monitoring
			*/

			ResponseEcho(0);
			CANProtocol(ELM327CommProtocol_CAN_ISO_15765_11b_500k);
			SetLongMsg();
			DisplaySpace(true);
			DisplayHeader(true);
			DisplayDataLength(true);
			DisplayLineFeed(false);
			CANAutoFormating(false);
			StartMonitoring();
		}

		void ELM327Protocol::TraceRead(UINT8 ReadData[], PUINT32 ReadCount)
		{
//#define TRACE_SIMULATION
#ifdef TRACE_SIMULATION
			static UINT32 counter = 0;
			static UINT32 COUNTER_TIME = 50; //ms

#define TRACE_SIMULATION_MSG_CNT 10

			const char* const SimCANMessages[TRACE_SIMULATION_MSG_CNT] = {
				"0f1 8 01 02 03 04 05 06 07 %02X\r",
				"1f1 8 11 02 13 04 05 06 %02X 08\r",
				"2f1 8 21 02 23 04 05 %02X 27 08\r",
				"3f1 8 31 02 33 04 %02X 06 37 78\r",
				"4f1 8 41 02 43 %02X 05 06 47 68\r",
				"5f1 8 51 02 %02X 04 05 06 57 58\r",
				"6f1 8 61 %02X 63 04 05 06 67 48\r",
				"7f1 8 %02X 02 73 04 05 06 77 38\r",
				"8f1 %02d 81 02 83 04 05 06 87 28\r",
				"9f1 8 %02X 02 93 04 05 06 97 18\r"
			};

			const UINT32 SimCANMessagesTiming[TRACE_SIMULATION_MSG_CNT] = {
				100,
				100,
				300,
				400,
				600,
				700,
				900,
				1000,
				1000,
				1000
			};

			char simBuffer[2048] = "";

			char* pend = simBuffer;
			for (int i = 0; i < TRACE_SIMULATION_MSG_CNT; i++)
			{
				UINT32 rest = (counter*COUNTER_TIME) % SimCANMessagesTiming[i];
				if (rest == 0)
				{
					pend += sprintf(pend, SimCANMessages[i], (UINT8)counter);
				}
			}
			UINT32 dataCnt = (UINT32)(pend - simBuffer);
			memcpy(ReadData, simBuffer, dataCnt);
			ReadData[dataCnt] = 0;
			*ReadCount = dataCnt;

			counter++;
#else
			switch (m_Type)
			{
			case ELM327ProtocolSerial:
				ELM327Serial::Read(ReadData, ReadCount);
				break;
			}
#endif
		}
	}
}