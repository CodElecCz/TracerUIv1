#pragma once

#include "stdafx.h"
#include "ModBusTCP.h"
#include "ModBusSerial.h"

using namespace Utilities::ModBus;

namespace Utilities
{
	namespace ModBus
	{
#pragma warning(push)
#pragma warning(disable:4275)

		enum ModBusProtocolType
		{
			ModBusProtocolTCP,
			ModBusProtocolSerial
		};

		class LIB_API ModBusProtocol : public ModBusTcp, ModBusSerial
		{
		public:			
			ModBusProtocol(const char* IPaddress) 
				: ModBusTcp(IPaddress), m_Type(ModBusProtocolTCP) {};
			ModBusProtocol(const char* COMPort, SerialProtocol::BaudRate Baudrate, SerialProtocol::DataBits Databits = SerialProtocol::DB_8, SerialProtocol::Parity Parity = SerialProtocol::P_NONE, SerialProtocol::StopBits Stopbit = SerialProtocol::SB_ONE)
				: ModBusSerial(COMPort, Baudrate, Databits, Parity, Stopbit), m_Type(ModBusProtocolSerial), m_SerialDevId(49) {};
			~ModBusProtocol(void) {};
		
			//01 (0x01) Read Coils
			void ReadCoils(UINT16 StartingAddress, UINT16 RequestedCount, bool ReadData[], PUINT32 ReadDataSize);
			void ReadCoils(UINT16 StartingAddress, UINT16 RequestedBitCount, UINT8 ReadData[], PUINT32 ReadDataSize);

			//02 (0x02) Read Discrete Inputs
			void ReadDiscreteInputs(UINT16 StartingAddress, UINT16 ReadRequestedCount, bool ReadData[], PUINT32 ReadDataSize);
			void ReadDiscreteInputs(UINT16 StartingAddress, UINT16 ReadRequestedBitCount, UINT8 ReadData[], PUINT32 ReadDataSize);

			//03 (0x03) Read Holding Registers
			void ReadHoldingRegister(UINT16 StartingAddress, UINT16 ReadRequestedCount, UINT16 ReadData[], PUINT32 ReadDataSize);

			//04 (0x04) Read Input Registers
			void ReadInputRegister(UINT16 StartingAddress, UINT16 ReadRequestedCount, UINT16 ReadData[], PUINT32 ReadDataSize);

			//05 (0x05) Write Single Coil
			void WriteSingleCoil(UINT16 StartingAddress, bool WriteData);

			//06 (0x06) Write Single Register
			void WriteSingleRegister(UINT16 StartingAddress, UINT16 WriteData);

			//07 (0x07) Read Exception Status (Serial Line only)
			void ReadExceptionStatus(PUINT8 Status);

			//08 (0x08) Diagnostics (Serial Line only)
			/*  hx   dc Subfunction
				0x00 00 Return Query Data
				0x01 01 Restart Communications Option
				0x02 02 Return Diagnostic Register
				0x03 03 Change ASCII Input Delimiter
				0x04 04 Force Listen Only Mode
				0x0A 10 Clear Counters and Diagnostic Register
				0x0B 11 Return Bus Message Count
				0x0C 12 Return Bus Communication Error Count
				0x0D 13 Return Bus Exception Error Count
				0x0E 14 Return Slave Message Count
				0x0F 15 Return Slave No Response Count
				0x10 16 Return Slave NAK Count
				0x11 17 Return Slave Busy Count
				0x12 18 Return Bus Character Overrun Count
			*/
			void Diagnostics(UINT16 Subfunction, UINT16 WriteData[], UINT32 WriteDataSize, UINT32 ReadData[], PUINT32 ReadDataSize);

			//11 (0x0B) Get Comm Event Counter (Serial Line only)
			void GetCommEventCounter(PUINT16 Status, PUINT16 EventCount);

			//12 (0x0C) Get Comm Event Log (Serial Line only)
			void GetCommEventLog(PUINT16 Status, PUINT16 EventCount, PUINT16 MessageCount, UINT8 Events[], PUINT32 EventsCount);

			//15 (0x0F) Write Multiple Coils
			void WriteMultipleCoils(UINT16 StartingAddress, bool WriteData[], UINT32 WriteDataSize);
			void WriteMultipleCoils(UINT16 StartingAddress, UINT8 WriteData[], UINT32 WriteDataSize, UINT32 WriteRequestBitCount);

			//16 (0x10) Write Multiple Registers
			void WriteMultipleRegisters(UINT16 StartingAddress, UINT16 WriteData[], UINT32 WriteDataSize);

			//17 (0x11) Report Slave ID (Serial Line only)
			void ReportSlaveID(UINT8 Data[], PUINT32 DataSize);

			//20 (0x14) Read File Record
			//21 (0x15) Write File Record

			//22 (0x16) Mask Write Register
			void MaskWriteRegister(UINT16 StartingAddress, UINT16 WriteMask, UINT16 WriteData);

			//23 (0x17) Read/Write Multiple registers
			void ReadWriteMultipleRegisters(UINT16 StartingAddress, UINT16 ReadRequestedCount, UINT16 ReadData[], PUINT ReadDataSize, UINT16 WStartingAddress, UINT16 WriteData[], UINT32 WriteDataSize);
		
			//24 (0x18) Read FIFO Queue.
			//43 ( 0x2B) Encapsulated Interface Transport
			//43 / 13 (0x2B / 0x0D) CANopen General Reference Request and Response PDU
			//43 / 14 (0x2B / 0x0E) Read Device Identification

			bool IsInitialized(void) { return (m_Type==ModBusProtocolTCP)? ModBusTcp::IsInitialized() : ModBusSerial::IsInitialized(); };
			const char* GetDeviceId(void) { return (m_Type==ModBusProtocolTCP)? ModBusTcp::GetIPaddress() : ModBusSerial::GetCOMPort();  };
			int GetTimeout(void) { return (m_Type==ModBusProtocolTCP)? ModBusTcp::GetTimeout() : ModBusSerial::GetTimeout(); };
			void SetSerialDevId(UINT8 DevId) { m_SerialDevId = DevId; }
			ModBusProtocolType GetProtocolType() { return m_Type; }

		private:
			//01 (0x01) Read Coils
			//02 (0x02) Read Discrete Inputs
			void ReadCoil(UINT8 FncId, UINT16 StartingAddress, UINT16 RequestedBitCount, UINT8 ReadData[], PUINT32 ReadDataSize);

			//03 (0x03) Read Holding Registers
			//04 (0x04) Read Input Registers
			void ReadRegister(UINT8 FncId, UINT16 StartingAddress, UINT16 ReadRequestedCount, UINT16 ReadData[], PUINT32 ReadDataSize);

			void ModBusCheckResponse(UINT8 function, UINT8 returnCode, UINT8 excCode);

		public:
			typedef shared_ptr<ModBusProtocol> ModBusProtocolPtr;

		private:
			UINT8 m_SerialDevId;
			ModBusProtocolType m_Type;
		};
#pragma warning(pop)
	}
}


