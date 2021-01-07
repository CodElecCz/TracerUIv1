#include "stdafx.h"
#include "ModBusProtocol.h"

namespace Utilities
{
	namespace ModBus
	{
		typedef struct SModBusExcepetion{
			int code;
			const char* desc;
		} ModBusExcepetion;

		static ModBusExcepetion m_ModBusExceptions[] = 
		{
			{1, "ILLEGAL FUNCTION"},
			{2, "ILLEGAL DATA ADDRESS"},
			{3, "ILLEGAL DATA VALUE"},
			{4, "SLAVE DEVICE FAILURE"},
			{5, "ACKNOWLEDGE"},
			{6, "SLAVE DEVICE BUSY"},
			{8, "MEMORY PARITY ERROR"},
			{0xA, "GATEWAY PATH UNAVAILABLE"},
			{0xB, "GATEWAY TARGET DEVICE FAILED TO RESPOND"}
		};

		const char* ModBusExceptionsExplain(UINT8 code)
		{
			const char* msg = "Unknown modbus expection";
			int size = sizeof(m_ModBusExceptions)/sizeof(m_ModBusExceptions[0]);

			for(int i = 0; i<size; i++)
			{
				if(code==m_ModBusExceptions[i].code)
				{
					msg = m_ModBusExceptions[i].desc;
					break;
				}
			}

			return msg;
		}

		void ModBusProtocol::ModBusCheckResponse(UINT8 function, UINT8 returnCode, UINT8 excCode)
		{
			//check function
			switch(returnCode)
			{
			case 0x81:
			case 0x82:
			case 0x83:
			case 0x84:
			case 0x85:
			case 0x86:
			case 0x8f:
			case 0x90:
				{
				char msg[256];
				const char* modbusErr = ModBusExceptionsExplain(excCode);
				sprintf_s(msg, sizeof(msg), "ModBus response, return code 0x%X, exception code: %d (%s)", returnCode, excCode, modbusErr);
				throw Exc(GetDeviceId(), msg); 
				}
			default:
				if(function!=returnCode)
				{
					char msg[256];
					sprintf_s(msg, sizeof(msg), "ModBus reponse, unknown return function code: 0x%X", returnCode);
					throw Exc(GetDeviceId(), msg);
				}
			}
		}

		void FromByte(UINT8 c, bool b[], int bitSizeLimit)
		{
			int max = (bitSizeLimit<8)? bitSizeLimit:8;
			for (int i=0; i < max; ++i)
				b[i] = (c & (1<<i)) != 0;
		}

		void ToByte(bool b[], PUINT8 c, int bitSizeLimit)
		{
			int max = (bitSizeLimit<8)? bitSizeLimit:8;
			UINT8 res = 0;
			for (int i=0; i < max; ++i)
				res += b[i] << i;

			if(c) *c = res;
		}

		void ModBusProtocol::ReadCoils(UINT16 StartingAddress, UINT16 RequestedCount, bool ReadData[], PUINT32 ReadDataSize)
		{
			int ReadDataSizeAlloc = *ReadDataSize;
			*ReadDataSize = 0;

			int RequestedByteCount = (RequestedCount/8) + ((RequestedCount%8>0)? 1:0);				
			unsigned int rdataSize = RequestedByteCount;
			vector<UINT8> rdata(RequestedByteCount);

			ReadCoils(StartingAddress, RequestedCount, rdata.data(), &rdataSize);

			//copy result
			for(unsigned int i = 0; i<rdataSize; i++)
			{
				int bitSize = RequestedCount - i*8;
				FromByte(rdata[i], &ReadData[i*8], bitSize);
			}
			*ReadDataSize = RequestedCount;
		}

		void ModBusProtocol::ReadDiscreteInputs(UINT16 StartingAddress, UINT16 RequestedCount, bool ReadData[], PUINT32 ReadDataSize)
		{
			int ReadDataSizeAlloc = *ReadDataSize;
			*ReadDataSize = 0;

			int RequestedByteCount = (RequestedCount/8) + ((RequestedCount%8>0)? 1:0);
			
			vector<UINT8> rdata(RequestedByteCount);
			unsigned int rdataSize = RequestedByteCount;

			ReadDiscreteInputs(StartingAddress, RequestedCount, rdata.data(), &rdataSize);

			//copy result
			for(unsigned int i = 0; i<rdataSize; i++)
			{
				int bitSize = RequestedCount - i*8;
				FromByte(rdata[i], &ReadData[i*8], bitSize);
			}
			*ReadDataSize = RequestedCount;
		}

		void ModBusProtocol::ReadCoils(UINT16 StartingAddress, UINT16 RequestedCount, UINT8 ReadData[], PUINT32 ReadDataSize)
		{
			ReadCoil(0x1, StartingAddress, RequestedCount, ReadData, ReadDataSize);
		}

		void ModBusProtocol::ReadDiscreteInputs(UINT16 StartingAddress, UINT16 RequestedCount, UINT8 ReadData[], PUINT32 ReadDataSize)
		{
			ReadCoil(0x2, StartingAddress, RequestedCount, ReadData, ReadDataSize);
		}

		void ModBusProtocol::ReadCoil(UINT8 FncId, UINT16 StartingAddress, UINT16 RequestedCount, UINT8 ReadData[], PUINT32 ReadDataSize)
		{
			unsigned int ReadDataSizeAlloc = *ReadDataSize;
			*ReadDataSize = 0;

			UINT8 wdata[5];
			wdata[0] = FncId;
			wdata[1] = (unsigned char)(StartingAddress>>8);
			wdata[2] = (unsigned char)(StartingAddress&0xff);
			wdata[3] = (unsigned char)(RequestedCount>>8);
			wdata[4] = (unsigned char)(RequestedCount&0xff);

			//TODO RequestedCount limit: 0x0001 to 0x07D0 (2000)

			int RequestedByteCount = (RequestedCount/8) + ((RequestedCount%8>0)? 1:0);
			int rdataSizeReq = 2 + RequestedByteCount; //function + count + bytes
			vector<UINT8> rdata(rdataSizeReq);
			unsigned int rdataSize = rdataSizeReq;

			switch(m_Type)
			{
			case ModBusProtocolTCP:
				ModBusTcp::Query(wdata, sizeof(wdata), rdata.data(), &rdataSize, rdataSizeReq);
				break;
			case ModBusProtocolSerial:
				ModBusSerial::Query(wdata, sizeof(wdata), rdata.data(), &rdataSize, rdataSizeReq, m_SerialDevId);
				break;
			}

			if(rdataSize!=rdataSizeReq && rdataSize!=2)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect return length: %d, expected: %d", rdataSize, rdataSizeReq);
				throw Exc(GetDeviceId(), msg);
			}
			
			ModBusCheckResponse(wdata[0], rdata[0], rdata[1]);

			//check response length
			if(rdata[1]!=RequestedByteCount)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect read data size: %d, expected: %d", rdata[1], RequestedByteCount);
				throw Exc(GetDeviceId(), msg);
			}

			//copy result
			unsigned int sizeCopyTo = ((rdataSize - 2)<ReadDataSizeAlloc)? (rdataSize - 2):ReadDataSizeAlloc;
			memcpy(ReadData, &rdata[2], sizeCopyTo);
			*ReadDataSize = rdataSize-2;
		}

		void ModBusProtocol::ReadHoldingRegister(UINT16 StartingAddress, UINT16 RequestedCount, UINT16 ReadData[], PUINT ReadDataSize)
		{
			ReadRegister(0x3, StartingAddress, RequestedCount, ReadData, ReadDataSize);
		}

		void ModBusProtocol::ReadInputRegister(UINT16 StartingAddress, UINT16 RequestedCount, UINT16 ReadData[], PUINT ReadDataSize)
		{
			ReadRegister(0x4, StartingAddress, RequestedCount, ReadData, ReadDataSize);
		}

		void ModBusProtocol::ReadRegister(UINT8 FncId, UINT16 StartingAddress, UINT16 RequestedCount, UINT16 ReadData[], PUINT32 ReadDataSize)
		{
			unsigned int ReadDataSizeAlloc = *ReadDataSize;
			*ReadDataSize = 0;

			UINT8 wdata[5];
			wdata[0] = FncId;
			wdata[1] = (unsigned char)(StartingAddress>>8);
			wdata[2] = (unsigned char)(StartingAddress&0xff);
			wdata[3] = (unsigned char)(RequestedCount>>8);
			wdata[4] = (unsigned char)(RequestedCount&0xff);

			//TODO RequestedCount limit: 0x0001 to 0x007D

			int rdataSizeReq = 2+2*RequestedCount; //function + count + 2*requestCount
			vector<UINT8> rdata(rdataSizeReq);
			unsigned int rdataSize = rdataSizeReq;

			switch(m_Type)
			{
			case ModBusProtocolTCP:
				ModBusTcp::Query(wdata, sizeof(wdata), rdata.data(), &rdataSize, rdataSizeReq);
				break;
			case ModBusProtocolSerial:
				ModBusSerial::Query(wdata, sizeof(wdata), rdata.data(), &rdataSize, rdataSizeReq, m_SerialDevId);
				break;
			}

			if(rdataSize!=rdataSizeReq && rdataSize!=2)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect return length: %d, expected: %d", rdataSize, rdataSizeReq);
				throw Exc(GetDeviceId(), msg);
			}

			ModBusCheckResponse(wdata[0], rdata[0], rdata[1]);

			//check response length
			if(rdata[1]!=2*RequestedCount)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect read data size: %d, expected: %d", rdata[1], RequestedCount);
				throw Exc(GetDeviceId(), msg);
			}

			//copy result
			unsigned int sizeCopyTo = ((rdataSize - 2)<(ReadDataSizeAlloc*2))? (rdataSize - 2):(ReadDataSizeAlloc*2);
			memcpy(ReadData, &rdata[2], sizeCopyTo);
			//byte swap
			for(unsigned int i = 0; i<sizeCopyTo/2; i++)
			{
				ReadData[i] = _byteswap_ushort(ReadData[i]);
			}
			*ReadDataSize = (rdataSize - 2)/2;
		}

		void ModBusProtocol::WriteSingleCoil(UINT16 StartingAddress, bool WriteData)
		{
			UINT8 wdata[5];
			wdata[0] = 0x5; //WriteSingleCoil
			wdata[1] = (unsigned char)(StartingAddress>>8);
			wdata[2] = (unsigned char)(StartingAddress&0xff);
			wdata[3] = (WriteData)? 0xff:0x00;
			wdata[4] = 0;

			int rdataSizeReq = 1 + 2 + 2; //function  + address + value
			unsigned char rdata[5];
			unsigned int rdataSize = sizeof(rdata);

			switch(m_Type)
			{
			case ModBusProtocolTCP:
				ModBusTcp::Query(wdata, sizeof(wdata), rdata, &rdataSize, rdataSizeReq);
				break;
			case ModBusProtocolSerial:
				ModBusSerial::Query(wdata, sizeof(wdata), rdata, &rdataSize, rdataSizeReq, m_SerialDevId);
				break;
			}
			
			if(rdataSize!=rdataSizeReq && rdataSize!=2)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect return length: %d, expected: %d", rdataSize, rdataSizeReq);
				throw Exc(GetDeviceId(), msg);
			}

			ModBusCheckResponse(wdata[0], rdata[0], rdata[1]);

			//check response address
			unsigned int startingAddress = (unsigned int)(rdata[1]<<8) + (unsigned int)rdata[2];
			if(startingAddress!=StartingAddress)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect return address: %d, expected: %d", startingAddress, StartingAddress);
				throw Exc(GetDeviceId(), msg);
			}

			//check response value
			if(rdata[3]!=wdata[3])
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect write data value: %d, expected: %d", rdata[3], wdata[3]);
				throw Exc(GetDeviceId(), msg);
			}	
		}

		void ModBusProtocol::WriteSingleRegister(UINT16 StartingAddress, UINT16 WriteData)
		{
			UINT8 wdata[5];
			wdata[0] = 0x6; //WriteSingleRegister
			wdata[1] = (unsigned char)(StartingAddress>>8);
			wdata[2] = (unsigned char)(StartingAddress&0xff);
			wdata[3] = (unsigned char)(WriteData>>8);
			wdata[4] = (unsigned char)(WriteData&0xff);

			int rdataSizeReq = 1 + 2 + 2; //function  + address + value
			unsigned char rdata[5];
			unsigned int rdataSize = sizeof(rdata);

			switch(m_Type)
			{
			case ModBusProtocolTCP:
				ModBusTcp::Query(wdata, sizeof(wdata), rdata, &rdataSize, rdataSizeReq);
				break;
			case ModBusProtocolSerial:
				ModBusSerial::Query(wdata, sizeof(wdata), rdata, &rdataSize, rdataSizeReq, m_SerialDevId);
				break;
			}

			if(rdataSize!=rdataSizeReq && rdataSize!=2)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus reponse, uncorrect return length: %d, expected: %d", rdataSize, rdataSizeReq);
				throw Exc(GetDeviceId(), msg);
			}

			ModBusCheckResponse(wdata[0], rdata[0], rdata[1]);

			//check response address
			unsigned int startingAddress = (unsigned int)(rdata[1]<<8) + (unsigned int)rdata[2];
			if(startingAddress!=StartingAddress)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect return address: %d, expected: %d", startingAddress, StartingAddress);
				throw Exc(GetDeviceId(), msg);
			}

			//check response value
			unsigned short writeData = (unsigned int)(rdata[3]<<8) + (unsigned int)rdata[4];
			if(writeData != WriteData)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect write value: %d, expected: %d", rdata[3], WriteData);
				throw Exc(GetDeviceId(), msg);
			}	
		}

		void ModBusProtocol::WriteMultipleCoils(UINT16 StartingAddress, bool WriteData[], UINT32 WriteDataSize)
		{
			int wdataSize = WriteDataSize/8+1;
			vector<UINT8> wdata(wdataSize);

			//copy data
			for(unsigned int i = 0; i<(unsigned int)wdataSize; i++)
			{
				int bitSize = WriteDataSize - i*8;
				ToByte(&WriteData[i*8], &wdata[i], bitSize);
			}

			WriteMultipleCoils(StartingAddress, wdata.data(), wdataSize, WriteDataSize); 
		}

		void ModBusProtocol::WriteMultipleCoils(UINT16 StartingAddress, UINT8 WriteData[], UINT32 WriteDataSize, UINT32 WriteRequestBitCount)
		{
			unsigned int wdataSize = 6 + WriteDataSize;
			vector<UINT8> wdata(wdataSize);
			wdata[0] = 0xf; //WriteMultipleCoils
			wdata[1] = (unsigned char)(StartingAddress>>8);
			wdata[2] = (unsigned char)(StartingAddress&0xff);			
			wdata[3] = (unsigned char)(WriteRequestBitCount>>8);
			wdata[4] = (unsigned char)(WriteRequestBitCount&0xff);
			wdata[5] = (unsigned char)(WriteDataSize);
			
			for(unsigned int i = 0; i<WriteDataSize; i++)
			{
				wdata[6+i] = WriteData[i];
			}

			unsigned int rdataSizeReq = 1 + 2 + 2; //function  + address + quantity
			unsigned char rdata[5];
			unsigned int rdataSize = sizeof(rdata);

			switch(m_Type)
			{
			case ModBusProtocolTCP:
				ModBusTcp::Query(wdata.data(), wdataSize, rdata, &rdataSize, rdataSizeReq);
				break;
			case ModBusProtocolSerial:
				ModBusSerial::Query(wdata.data(), wdataSize, rdata, &rdataSize, rdataSizeReq, m_SerialDevId);
				break;
			}

			if(rdataSize!=rdataSizeReq && rdataSize!=2)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect return length: %d, expected: %d", rdataSize, rdataSizeReq);
				throw Exc(GetDeviceId(), msg);
			}

			ModBusCheckResponse(wdata[0], rdata[0], rdata[1]);

			//check response address
			unsigned int startingAddress = (unsigned int)(rdata[1]<<8) + (unsigned int)rdata[2];
			if(startingAddress!=StartingAddress)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect address: %d, expected: %d", startingAddress, StartingAddress);
				throw Exc(GetDeviceId(), msg);
			}

			//check response value
			unsigned short writeDataSizeRes = (unsigned int)(rdata[3]<<8) + (unsigned int)rdata[4];
			if(WriteDataSize != writeDataSizeRes)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect write data quantity: %d, expected: %d", writeDataSizeRes, WriteDataSize);
				throw Exc(GetDeviceId(), msg);
			}
		}

		void ModBusProtocol::WriteMultipleRegisters(UINT16 StartingAddress, UINT16 WriteData[], UINT32 WriteDataSize)
		{
			unsigned int wdataSize = 6+WriteDataSize*2;
			vector<UINT8> wdata(wdataSize);
			wdata[0] = 0x10; //WriteMultipleRegisters
			wdata[1] = (unsigned char)(StartingAddress>>8);
			wdata[2] = (unsigned char)(StartingAddress&0xff);			
			wdata[3] = (unsigned char)(WriteDataSize>>8);
			wdata[4] = (unsigned char)(WriteDataSize&0xff);
			wdata[5] = (unsigned char)(WriteDataSize*2);
			
			for(unsigned int i = 0; i<WriteDataSize; i++)
			{
				wdata[6+2*i] = (unsigned char)(WriteData[i]>>8);
				wdata[6+2*i+1] = (unsigned char)(WriteData[i]&0xff);
			}

			unsigned int rdataSizeReq = 1 + 2 + 2; //function  + address + quantity
			unsigned char rdata[5];
			unsigned int rdataSize = sizeof(rdata);

			switch(m_Type)
			{
			case ModBusProtocolTCP:
				ModBusTcp::Query(wdata.data(), wdataSize, rdata, &rdataSize, rdataSizeReq);
				break;
			case ModBusProtocolSerial:
				ModBusSerial::Query(wdata.data(), wdataSize, rdata, &rdataSize, rdataSizeReq, m_SerialDevId);
				break;
			}			

			if(rdataSize!=rdataSizeReq && rdataSize!=2)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect return length: %d, expected: %d", rdataSize, rdataSizeReq);
				throw Exc(GetDeviceId(), (const char*)msg);
			}

			ModBusCheckResponse(wdata[0], rdata[0], rdata[1]);

			//check response address
			unsigned int startingAddress = (unsigned int)(rdata[1]<<8) + (unsigned int)rdata[2];
			if(startingAddress!=StartingAddress)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect address: %d, expected: %d", startingAddress, StartingAddress);
				throw Exc(GetDeviceId(), (const char*)msg);
			}

			//check response value
			unsigned short writeDataSizeRes = (unsigned int)(rdata[3]<<8) + (unsigned int)rdata[4];
			if(WriteDataSize != writeDataSizeRes)
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "ModBus response, uncorrect write data quantity: %d, expected: %d", writeDataSizeRes, WriteDataSize);
				throw Exc(GetDeviceId(), (const char*)msg);
			}
		}
	}
}