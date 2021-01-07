#pragma once

#include "stdafx.h"
#include "ELM327Serial.h"

namespace Utilities
{
	namespace ELM327
	{
#pragma warning(push)
#pragma warning(disable:4275)

		enum ELM327ProtocolType
		{
			ELM327ProtocolSimulate = 0,
			ELM327ProtocolTCP,
			ELM327ProtocolSerial
		};

		enum ELM327CommProtocolType
		{
			ELM327CommProtocol_None = 0,
			ELM327CommProtocol_SAE_J1850_PWM,
			ELM327CommProtocol_SAE_J1850_VPW,
			ELM327CommProtocol_ISO_9141,
			ELM327CommProtocol_ISO_14230_1,
			ELM327CommProtocol_ISO_14230_2,
			ELM327CommProtocol_CAN_ISO_15765_11b_500k,
			ELM327CommProtocol_CAN_ISO_15765_29b_500k,
			ELM327CommProtocol_CAN_ISO_15765_11b_250k,
			ELM327CommProtocol_CAN_ISO_15765_29b_250k,
			ELM327CommProtocol_CAN_SAE_J1939,
			ELM327CommProtocol_CAN_USER1,
			ELM327CommProtocol_CAN_USER2
		};

		class LIB_API ELM327Protocol : public ELM327Serial
		{
		public:
			ELM327Protocol(const char* COMPort, SerialProtocol::BaudRate Baudrate, SerialProtocol::DataBits Databits = SerialProtocol::DB_8, SerialProtocol::Parity Parity = SerialProtocol::P_NONE, SerialProtocol::StopBits Stopbit = SerialProtocol::SB_ONE)
				: ELM327Serial(COMPort, Baudrate, Databits, Parity, Stopbit), 
				m_Type(ELM327ProtocolSerial) 
			{}

			~ELM327Protocol(void) 
			{}

		public:
			
			void Enter();

			/*
			>atz


			ELM327 v1.4

			>
			*/
			void Reset();

			/*
			>ate0
			ate0
			OK

			>
			*/
			void ResponseEcho(bool val);
			void CANProtocol(ELM327CommProtocolType protocol);
			void SetLongMsg();
			void DisplaySpace(bool val);
			void DisplayHeader(bool val);
			void DisplayDataLength(bool val);
			void DisplayLineFeed(bool val);
			void CANAutoFormating(bool val);
			void StartMonitoring();
		
			void TraceInit();
			void TraceRead(UINT8 ReadData[], PUINT32 ReadCount);

			bool IsInitialized(void);
			const char* GetDeviceId();
			int GetTimeout();
			void SetSerialDevId(UINT8 DevId) { m_SerialDevId = DevId; }
			ELM327ProtocolType GetProtocolType() { return m_Type; }

		public:
			typedef shared_ptr<ELM327Protocol> ELM327ProtocolPtr;


		private:
			UINT8 m_SerialDevId;
			ELM327ProtocolType m_Type;
		};
#pragma warning(pop)
	}
}


