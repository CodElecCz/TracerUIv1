#include "stdafx.h"
#include "TurckRfidTask.h"

namespace Utilities
{
	namespace ModBus
	{
		
#define READ_OFFSET			0x4000	//words
#define WRITE_OFFSET		0x4400	//words
#define TIMEOUT				2000	//ms

#define DEFAULT_READ_AREA		"@IN"
#define DEFAULT_WRITE_AREA		"@OUT"

		TurckRfidTask::TurckRfidTask(string Id) : ModBusTask(Id)
		{
			m_TaskType = ModBusTaskTurckRfid;
		}

		TurckRfidTask::~TurckRfidTask(void)
		{
		}

		void TurckRfidTask::Create(const char* IPaddress, UINT32 LoopTimeMS)
		{
			ModBusTask::Create(IPaddress, LoopTimeMS);

			//read area
			NodeWordArray* nodearray1 = AddRegisterArea(DEFAULT_READ_AREA, READ_OFFSET, NB_OF_MODULES*MODULE_OFFSET, NodeAccess_Read);

			nodearray1->AddNode("ID0", NodeType_Byte, MODULE_OFFSET*0);
			nodearray1->AddNode("ID1", NodeType_Byte, MODULE_OFFSET*2);
			nodearray1->AddNode("ID2", NodeType_Byte, MODULE_OFFSET*4);
			nodearray1->AddNode("ID3", NodeType_Byte, MODULE_OFFSET*6);

			NodeInterface* node = nodearray1->GetNodeInterface("ID0");
			for(int i = 0; i<8; i++)
			{
				node->AddNode(CTurckRfidRStatusByte[i], NodeType_Bit, i);
			}
			node = nodearray1->GetNodeInterface("ID1");
			for(int i = 0; i<8; i++)
			{
				node->AddNode(CTurckRfidRStatusByte[i], NodeType_Bit, i);
			}
			node = nodearray1->GetNodeInterface("ID2");
			for(int i = 0; i<8; i++)
			{
				node->AddNode(CTurckRfidRStatusByte[i], NodeType_Bit, i);
			}
			node = nodearray1->GetNodeInterface("ID3");
			for(int i = 0; i<8; i++)
			{
				node->AddNode(CTurckRfidRStatusByte[i], NodeType_Bit, i);
			}

			//write area
			NodeWordArray* nodearray2 = AddRegisterArea(DEFAULT_WRITE_AREA, WRITE_OFFSET, NB_OF_MODULES*MODULE_OFFSET, NodeAccess_Write);

			nodearray2->AddNode("ID0_STATUS", NodeType_Byte, MODULE_OFFSET*0);
			nodearray2->AddNode("ID1_STATUS", NodeType_Byte, MODULE_OFFSET*2);
			nodearray2->AddNode("ID2_STATUS", NodeType_Byte, MODULE_OFFSET*4);
			nodearray2->AddNode("ID3_STATUS", NodeType_Byte, MODULE_OFFSET*6);

			node = nodearray2->GetNodeInterface("ID0_STATUS");
			for(int i = 0; i<8; i++)
			{
				node->AddNode(CTurckRfidWStatusByte[i], NodeType_Bit, i);
			}
			node = nodearray2->GetNodeInterface("ID1_STATUS");
			for(int i = 0; i<8; i++)
			{
				node->AddNode(CTurckRfidWStatusByte[i], NodeType_Bit, i);
			}
			node = nodearray2->GetNodeInterface("ID2_STATUS");
			for(int i = 0; i<8; i++)
			{
				node->AddNode(CTurckRfidWStatusByte[i], NodeType_Bit, i);
			}
			node = nodearray2->GetNodeInterface("ID3_STATUS");
			for(int i = 0; i<8; i++)
			{
				node->AddNode(CTurckRfidWStatusByte[i], NodeType_Bit, i);
			}

			nodearray2->AddNode("ID0_SIZE", NodeType_Byte, MODULE_OFFSET*0 + 1);
			nodearray2->AddNode("ID1_SIZE", NodeType_Byte, MODULE_OFFSET*2 + 1);
			nodearray2->AddNode("ID2_SIZE", NodeType_Byte, MODULE_OFFSET*4 + 1);
			nodearray2->AddNode("ID3_SIZE", NodeType_Byte, MODULE_OFFSET*6 + 1);

			nodearray2->AddNode("ID0_ADDRESS", NodeType_Byte, MODULE_OFFSET*0 + 1);
			nodearray2->AddNode("ID1_ADDRESS", NodeType_Byte, MODULE_OFFSET*1 + 1);
			nodearray2->AddNode("ID2_ADDRESS", NodeType_Byte, MODULE_OFFSET*2 + 1);
			nodearray2->AddNode("ID3_ADDRESS", NodeType_Byte, MODULE_OFFSET*3 + 1);
		}

		TurckRfidRData TurckRfidTask::ReadModule(const char* Id)
		{
			TurckRfidRDataModules rmod = {0};
			UINT32 rmodSize = NB_OF_MODULES*MODULE_OFFSET;
			ReadRegisterArea(DEFAULT_READ_AREA, rmod.word, &rmodSize);

			if(0==strcmp(Id, "ID0"))
			{
				return rmod.data.ID0;
			}
			else if(0==strcmp(Id, "ID1"))
			{
				return rmod.data.ID1;
			}
			else if(0==strcmp(Id, "ID2"))
			{
				return rmod.data.ID2;
			}
			else if(0==strcmp(Id, "ID3"))
			{
				return rmod.data.ID3;
			}
			else
			{
				throw Exception("ReadModule() error: unknown module id");
			}
		}

		void TurckRfidTask::Write(const char* id, STurckRfidWStatusByte status)
		{
			TurckRfidRUStatusByte s;
			s.data = status;

			char id_status[32];
			sprintf_s(id_status, sizeof(id_status), "%s_STATUS", id);
			NodeInterface* area =  GetNodeInterface(DEFAULT_WRITE_AREA);
			NodeInterface* node = area->GetNodeInterface(id_status);
			node->Write(s.byte);
		}

		void TurckRfidTask::Write(const char* id, UINT16 address)
		{
			char id_address[32];
			sprintf_s(id_address, sizeof(id_address), "%s_ADDRESS", id);
			NodeInterface* area =  GetNodeInterface(DEFAULT_WRITE_AREA);
			NodeInterface* node = area->GetNodeInterface(id_address);
			node->Write(address);
		}

		void TurckRfidTask::Write(const char* id, UINT8 size)
		{
			char id_size[32];
			sprintf_s(id_size, sizeof(id_size), "%s_SIZE", id);
			NodeInterface* area = GetNodeInterface(DEFAULT_WRITE_AREA);
			NodeInterface* node = area->GetNodeInterface(id_size);
			node->Write(size);
		}

		void TurckRfidTask::Write(const char* Id, UINT8 Data[], UINT8 DataSize)
		{
			UINT32 offset = 0;
			NodeInterface* nodearea =  GetNodeInterface(DEFAULT_WRITE_AREA);
			if(0==strcmp(Id, "ID0"))
			{
				offset = MODULE_OFFSET*0 + 2;
			}
			else if(0==strcmp(Id, "ID1"))
			{
				offset = MODULE_OFFSET*1 + 2;
			}
			else if(0==strcmp(Id, "ID2"))
			{
				offset = MODULE_OFFSET*2 + 2;
			}
			else if(0==strcmp(Id, "ID3"))
			{
				offset = MODULE_OFFSET*3 + 2;
			}
			else
			{
				throw Exception("ReadModule() error: unknown module id");
			}

			NodeWordArray* area = dynamic_cast<NodeWordArray*>(nodearea);

			UINT16 data[4];
			memset(data, 0, 4*sizeof(UINT16));
			memcpy(data, Data, DataSize*sizeof(UINT8));
			//area->Write(offset, data, 4);
		}

		void TurckRfidTask::Read(const char* Id, bool* Present)
		{
			if(Present)
				*Present = ReadModule(Id).data.Status.data.TAG_PRESENT;
		}

		void TurckRfidTask::Read(const char* Id, UINT8 ReadData[], PUINT32 ReadDataSize)
		{
			UINT32 ReadDataSizeAlloc = *ReadDataSize;
			*ReadDataSize = 0;

			if(ReadModule(Id).data.Status.data.TAG_PRESENT)
			{				
				STurckRfidWStatusByte status = {0};
				status.XCVR = true;
				status.TAGID = true;
				Write(Id, status);

				clock_t t = clock();
				while(ReadModule(Id).data.Status.data.DONE)
				{
					UINT32 elapsed = (UINT32)(clock() - t);
					if(elapsed > TIMEOUT)
						throw Exc(Id, "Read() error: not DONE timeout");

					Sleep(5);
				}

				status.TAGID = false;
				Write(Id, status);
				t = clock();
				while(!ReadModule(Id).data.Status.data.DONE)
				{
					UINT32 elapsed = (UINT32)(clock() - t);
					if(elapsed > TIMEOUT)
						throw Exc(Id, "Read() error: DONE timeout");

					Sleep(5);
				}

				TurckRfidRData rdata = ReadModule(Id);
				memcpy_s(ReadData, ReadDataSizeAlloc, rdata.data.Data, 8);
				*ReadDataSize = 8;
			}
		}

		void TurckRfidTask::Read(const char* Id, UINT16 Address, UINT8 ReadRequest, UINT8 ReadData[], PUINT32 ReadDataSize)
		{
			UINT32 ReadDataSizeAlloc = *ReadDataSize;
			*ReadDataSize = 0;

			if(ReadModule(Id).data.Status.data.TAG_PRESENT)
			{	
				Write(Id, Address);
				Write(Id, ReadRequest);

				STurckRfidWStatusByte status = {0};
				status.XCVR = true;
				status.READ = true;
				Write(Id, status);

				clock_t t = clock();
				while(ReadModule(Id).data.Status.data.DONE)
				{
					UINT32 elapsed = (UINT32)(clock() - t);
					if(elapsed > TIMEOUT)
						throw Exc(Id, "Read() error: not DONE timeout");

					Sleep(5);
				}

				status.READ = false;
				Write(Id, status);

				t = clock();
				while(!ReadModule(Id).data.Status.data.DONE)
				{
					UINT32 elapsed = (UINT32)(clock() - t);
					if(elapsed > TIMEOUT)
						throw Exc(Id, "Read() error: DONE timeout");

					Sleep(5);
				}

				TurckRfidRData rdata = ReadModule(Id);
				memcpy_s(ReadData, ReadDataSizeAlloc, rdata.data.Data, 8);
				*ReadDataSize = 8;
			}
		}
		
		void TurckRfidTask::Read(const char* id, UINT16 Address, UINT32 ReadRequest, UINT8 ReadData[], PUINT32 ReadDataSize)
		{
			UINT32 ReadDataSizeAlloc = *ReadDataSize;
			*ReadDataSize = 0;

			UINT32 readCount = ReadRequest/8 + (((ReadRequest%8)>0)? 1:0);

			for(UINT32 i = 0; i < readCount; i++)
			{
				UINT8 rdata[8];
				UINT32 rdataSize = sizeof(rdata);

				UINT8 dataToRead = ((i*8+8) > ReadRequest)? ReadRequest%8 : 8;
				Read(id, Address + (i*8), dataToRead, rdata, &rdataSize);

				if(ReadDataSizeAlloc>=i*8+dataToRead)
				{
					memcpy(ReadData + i*8, rdata, dataToRead);
					*ReadDataSize += dataToRead;
				}
			}
		}

		void TurckRfidTask::Write(const char* Id, UINT16 Address, UINT8 WriteData[], UINT8 WriteDataSize)
		{
			if(ReadModule(Id).data.Status.data.TAG_PRESENT)
			{	
				Write(Id, Address);
				Write(Id, WriteDataSize);
				Write(Id, WriteData, WriteDataSize);

				STurckRfidWStatusByte status = {0};
				status.XCVR = true;
				status.WRITE = true;
				Write(Id, status);

				clock_t t = clock();
				while(ReadModule(Id).data.Status.data.DONE)
				{
					UINT32 elapsed = (UINT32)(clock() - t);
					if(elapsed > TIMEOUT)
						throw Exc(Id, "Read() error: not DONE timeout");

					Sleep(5);
				}

				status.WRITE = false;
				Write(Id, status);

				t = clock();
				while(!ReadModule(Id).data.Status.data.DONE)
				{
					UINT32 elapsed = (UINT32)(clock() - t);
					if(elapsed > TIMEOUT)
						throw Exc(Id, "Read() error: DONE timeout");

					Sleep(5);
				}
			}
		}

		void TurckRfidTask::Write(const char* id, UINT16 Address, UINT8 WriteData[], UINT32 WriteDataSize)
		{
			UINT32 writeCount = WriteDataSize/8 + (((WriteDataSize%8)>0)? 1:0);

			for(UINT32 i = 0; i < writeCount; i++)
			{
				UINT8 wdata[8];
				UINT8 wdataSize = ((i*8+8) > WriteDataSize)? WriteDataSize%8 : 8;
				
				memcpy(wdata, WriteData + i*8, wdataSize);
				Write(id, Address + (i*8), wdata, wdataSize);
			}
		}

		void TurckRfidTask::TryXcvrOn(const char* Id)
		{
			STurckRfidWStatusByte status = {0};
			status.XCVR = true;
			Write(Id, status);
			
			clock_t t = clock();
			while(!ReadModule(Id).data.Status.data.XCVR_ON)
			{
				UINT32 elapsed = (UINT32)(clock() - t);
				if(elapsed > TIMEOUT)
					throw Exc(Id, "TryXcvrOn() error: timeout");

				Sleep(5);
			}
		}
	}
}