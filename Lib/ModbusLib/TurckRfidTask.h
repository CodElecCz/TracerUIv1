#pragma once

#include "stdafx.h"
#include "ModBusTask.h"
#include "TurckRfidTypes.h"

using namespace std;
using namespace Utilities::ModBus;

namespace Utilities
{
	namespace ModBus
	{
		class LIB_API TurckRfidTask : public ModBusTask
		{
		public:
			TurckRfidTask(string Id);
			~TurckRfidTask(void);

		public:
			void Create(const char* IPaddress, UINT32 LoopTimeMS = 20);

			//tag Read("ID0", &TagPresent)
			void Read(const char* Id, bool* TagPresent);
			//rfid Read("ID0", ...)
			void Read(const char* Id, UINT8 RfidId[], PUINT32 RfidIdSize);
			//memory Read("ID0", ...)
			void Read(const char* Id, UINT16 Address, UINT32 ReadRequest, UINT8 ReadData[], PUINT32 ReadDataSize);
			//memory Write("ID0", ...)
			void Write(const char* Id, UINT16 Address, UINT8 WriteData[], UINT32 WriteDataSize);

		public:
			typedef shared_ptr<TurckRfidTask> TurckRfidTaskPtr;

		private:
			TurckRfidRData ReadModule(const char* Id);

			void Write(const char* Id, STurckRfidWStatusByte status); //write status byte
			void Write(const char* Id, UINT16 Address); //write address
			void Write(const char* Id, UINT8 Size); //write size
			void Write(const char* Id, UINT8 Data[], UINT8 DataSize); //write data
			
			void TryXcvrOn(const char *Id);
			
			void Read(const char* Id, UINT16 Address, UINT8 ReadRequest, UINT8 ReadData[], PUINT32 ReadDataSize); //read memory
			void Write(const char* Id, UINT16 Address, UINT8 WriteData[], UINT8 WriteDataSize); //write memory
		};
	}
}
