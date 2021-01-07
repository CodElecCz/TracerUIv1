#include "stdafx.h"
#include "ModBusTask.h"

namespace Utilities
{
	namespace ModBus
	{
		ModBusTask::ModBusTask(string Id) : INode(Id, NodeType_Task)
		{
			m_hThread = 0;
			m_threadID = 0;
			m_StopEvent = 0;
			m_TaskIsRunning = false;
			memset(&m_ReadTime, 0, sizeof(SYSTEMTIME));
			memset(&m_WriteTime, 0, sizeof(SYSTEMTIME));
		}

		ModBusTask::~ModBusTask(void)
		{
			Clear();
		}

		void ModBusTask::Create(const char* IPaddress, UINT32 LoopTimeMS)
		{
			Clear();
			
			m_LoopTimeMS = LoopTimeMS;
			m_ModBusProtocol = ModBusProtocol::ModBusProtocolPtr(new ModBusProtocol(IPaddress));
		}

		void ModBusTask::Create(const char* COMport, UINT32 LoopTimeMS, SerialProtocol::BaudRate Baudrate)
		{
			Clear();
			
			m_LoopTimeMS = LoopTimeMS;
			m_ModBusProtocol = ModBusProtocol::ModBusProtocolPtr(new ModBusProtocol(COMport, Baudrate));
		}

		NodeWordArray* ModBusTask::CreateRegisterArea(const char* Id, UINT16 StartingAddress, UINT16 RequestedWordCount, NodeAccess Access)
		{
			string id(Id);

			//check if allready exist
			std::map<string, INode::INodePtr>::iterator it = m_Nodes.find(id);
			if(it != m_Nodes.end())
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Duplicate area id: '%s'", Id);
				throw Exc(this->Id(), msg);
			}

			NodeWordArray::NodeWordArrayPtr area(new NodeWordArray(Id, NodeType_WordArray, Access, StartingAddress, RequestedWordCount, ModBusAreaRegister));
			m_Nodes[id] = area;

			return area.get();
		}

		NodeWordArray* ModBusTask::CreateHoldingRegisterArea(const char* Id, UINT16 StartingAddress, UINT16 RequestedWordCount)
		{
			string id(Id);

			//check if allready exist
			std::map<string, INode::INodePtr>::iterator it = m_Nodes.find(id);
			if(it != m_Nodes.end())
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Duplicate area id: '%s'", Id);
				throw Exc(this->Id(), msg);
			}

			NodeWordArray::NodeWordArrayPtr area(new NodeWordArray(Id, NodeType_WordArray, (NodeAccess)NodeAccess_Read, StartingAddress, RequestedWordCount, ModBusAreaHoldingRegister));
			m_Nodes[id] = area;

			return area.get();
		}
		
		NodeBitArray* ModBusTask::CreateCoilArea(const char* Id, UINT16 StartingAddress, UINT16 RequestedBitCount, NodeAccess Access, bool UseSingleCoil)
		{
			string id(Id);

			//check if allready exist
			std::map<string, INode::INodePtr>::iterator it = m_Nodes.find(id);
			if(it != m_Nodes.end())
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Duplicate area id: '%s'", Id);
				throw Exc(this->Id(), msg);
			}

			NodeBitArray::NodeBitArrayPtr area(new NodeBitArray(Id, NodeType_BitArray, Access, StartingAddress, RequestedBitCount, (UseSingleCoil)? ModBusAreaCoil : ModBusAreaMultipleCoils));
			m_Nodes[id] = area;

			return area.get();
		}

		NodeBitArray* ModBusTask::CreateDiscretInputsArea(const char* Id, UINT16 StartingAddress, UINT16 RequestedBitCount)
		{
			string id(Id);

			//check if allready exist
			std::map<string, INode::INodePtr>::iterator it = m_Nodes.find(id);
			if(it != m_Nodes.end())
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Duplicate area id: '%s'", Id);
				throw Exc(this->Id(), msg);
			}

			NodeBitArray::NodeBitArrayPtr area(new NodeBitArray(Id, NodeType_BitArray, (NodeAccess)NodeAccess_Read, StartingAddress, RequestedBitCount, ModBusAreaDiscretInput));
			m_Nodes[id] = area;

			return area.get();
		}

		void ModBusTask::Start(bool WaitForTaskRunning)
		{
			if(m_Nodes.size()==0)
				throw Exc(this->Id(), "No area was defined");

			if(m_hThread == 0)
			{
				m_StopEvent	= CreateEvent( NULL,   // No security attributes
					TRUE,  // Manual-reset event
					FALSE,   // Initial state is not signaled
					NULL);  // Object not named

				m_hThread = CreateThread(NULL, 0, &ModBusTask::ThreadFunc, this, 0, &m_threadID);

				if(WaitForTaskRunning)
				{
					this->WaitForTaskRunning();
				}
			}
		}

		void ModBusTask::Stop()
		{
			if(m_StopEvent)
			{
				SetEvent( m_StopEvent);

				int timeout = GetThreadTimeout();

				if(WaitForSingleObject(m_hThread, timeout))
				{
					//timeout
					if ( m_TaskIsRunning )
					{
						TerminateThread(m_hThread, 0);
						m_TaskIsRunning = false;
					}
				}
				m_threadID = 0;
				CloseHandle(m_StopEvent);
				m_StopEvent = 0;
				CloseHandle(m_hThread);
				m_hThread = 0;
			}
		}

		int ModBusTask::GetThreadTimeout()
		{
			int timeout = 0;

			if(m_Nodes.size())
			{
				size_t nbOfAreas = m_Nodes.size();
				timeout = (int)nbOfAreas * (this->m_ModBusProtocol.get()->GetTimeout() + 200);
			}

			return timeout;
		}

		void ModBusTask::Clear()
		{
			Stop(); 

			m_Nodes.clear();
		}

		list<INode*> ModBusTask::SearchNodeInterface(const char* Id)
		{
			string id(Id);
			list<INode*> nodelist;

			map<string, INode::INodePtr>::iterator it = m_Nodes.find(id);
			if(it!=m_Nodes.end())
			{
				nodelist.push_back(it->second.get());
			}
			else
			{
				it = m_Nodes.begin();
				while(it!=m_Nodes.end())
				{
					if(it->second->NodesCount()>0)
					{
						list<INode*> sublist = it->second->SearchNodeInterface(Id);
						nodelist.insert(nodelist.end(), sublist.begin(), sublist.end());
					}
					it++;
				}
			}
		
			return nodelist;
		}

		void ModBusTask::ReadRegisterArea(const char* Id, UINT16 ReadData[], PUINT32 ReadDataSize)
		{
			GetNodeInterface(Id)->Read(ReadData, ReadDataSize);
		}

		void ModBusTask::Read(const char* id, UINT8 ReadData[], PUINT32 ReadDataSize)
		{
			UINT32 ReadDataSizeAlloc = *ReadDataSize;
			*ReadDataSize = 0;

			NodeWordArray* area = dynamic_cast<NodeWordArray*>(GetNodeInterface(id));

			vector<UINT16> data(area->Count());
			
			UINT32 dataSize = area->Count();
			area->Read(data.data(), &dataSize);

			memcpy_s(ReadData, ReadDataSizeAlloc, data.data(), dataSize*2);
			*ReadDataSize = dataSize*2;
		}

		void ModBusTask::Read(const char* id, UINT16 Address, UINT32 ReadRequest, UINT8 ReadData[], PUINT32 ReadDataSize)
		{
			throw Exc(this->Id(), "Memory read not supported");
		}

		void ModBusTask::Write(const char* id, UINT16 Address, UINT8 WriteData[], UINT32 WriteDataSize)
		{
			throw Exc(this->Id(), "Memory write not supported");
		}

		void ModBusTask::WaitForTaskRunning()
		{
			int timeout = GetThreadTimeout();
			clock_t t = clock();
			UINT32 wait = ((m_LoopTimeMS/2)>2)? (m_LoopTimeMS/2):2;
			while(!m_TaskIsRunning)
			{
				UINT32 elapsed = (UINT32)(clock() - t);
				if(elapsed > (UINT32)timeout)
					throw Exc(Id(), "WaitForTaskRunning() error: timeout");

				Sleep(wait);
			}
		}

		void ModBusTask::ThreadAreaRegisterRW(NodeWordArray* area, UINT16 Data[], UINT16 Flag[])
		{
			if(area->Access()&NodeAccess_Write)
			{			
				//write
				UINT32 dataSize = area->Count();
				area->GetWriteArea(Data, Flag, &dataSize);
				for(UINT32 offset = 0; offset<dataSize; offset++)
				{
					if(Flag[offset])
					{				
						UINT32 offsetNext;
											
						//check next flags
						for(offsetNext = offset+1; offsetNext<dataSize; offsetNext++)
						{
							if(!Flag[offsetNext])											
								break;												
						}

						UINT32 writeAreaSize = offsetNext-offset;
						switch(writeAreaSize)
						{
						case 1:
							this->m_ModBusProtocol->WriteSingleRegister(area->Address() + offset, Data[offset]);
							break;
						default:
							this->m_ModBusProtocol->WriteMultipleRegisters(area->Address() + offset, &Data[offset], writeAreaSize);
							offset += writeAreaSize-1;
							break;
						}

						//write data acknoledge
						area->SetWriteAreaAcknoledge(offset, writeAreaSize);
						SetWriteTimeStamp();
					}
				}										
			}

			if(area->Access()&NodeAccess_Read)
			{									
				//read
				UINT32 dataSize = area->Count();
				this->m_ModBusProtocol->ReadInputRegister(area->Address(), area->Count(), Data, &dataSize);
				
				//set timestamp
				SetReadTimeStamp();
				
				//copy read data
				area->SetReadArea(Data, dataSize);	
				
				//init write area
				if(!this->m_TaskIsRunning)
					area->SetWriteArea(Data, dataSize);
			}
		}

		void ModBusTask::ThreadAreaHoldingRegisterR(NodeWordArray* area, UINT16 Data[])
		{
			if(area->Access()&NodeAccess_Read)
			{									
				//read
				UINT32 dataSize = area->Count();
				this->m_ModBusProtocol->ReadHoldingRegister(area->Address(), area->Count(), Data, &dataSize);
				
				//set timestamp
				SetReadTimeStamp();
				
				//copy read data
				area->SetReadArea(Data, dataSize);					
			}
		}

		void ModBusTask::ThreadAreaCoilRW(NodeBitArray* area, bool Data[], UINT16 Flag[], bool UseSingleCoil)
		{
			//set serial dev id for serial communication
			this->m_ModBusProtocol->SetSerialDevId(area->GetSerialDevId());

			if(area->Access()&NodeAccess_Write)
			{							
				//Write
				UINT32 dataSize = area->Count();
				area->GetWriteArea(Data, Flag, &dataSize);
				for(UINT32 offset = 0; offset<dataSize; offset++)
				{
					if(Flag[offset])
					{	
						UINT32 offsetNext;
											
						//check next flags
						for(offsetNext = offset+1; offsetNext<dataSize; offsetNext++)
						{
							if(!Flag[offsetNext])											
								break;												
						}

						UINT32 writeAreaSize = offsetNext-offset;
						switch(writeAreaSize)
						{
						case 1:	
							if(UseSingleCoil)
							{
								this->m_ModBusProtocol->WriteSingleCoil(area->Address() + offset, Data[offset]);
							}
							else
							{
								this->m_ModBusProtocol->WriteMultipleCoils(area->Address() + offset, &Data[offset], writeAreaSize);
								offset += writeAreaSize-1;
							}
							break;
						default:
							this->m_ModBusProtocol->WriteMultipleCoils(area->Address() + offset, &Data[offset], writeAreaSize);
							offset += writeAreaSize-1;
							break;
						}
						
						//write data acknoledge
						area->SetWriteAreaAcknoledge(offset, writeAreaSize);
						SetWriteTimeStamp();						
					}
				}								
			}

			if(area->Access()&NodeAccess_Read)
			{									
				//Read
				UINT32 dataSize = area->Count();
				this->m_ModBusProtocol->ReadCoils(area->Address(), area->Count(), Data, &dataSize);
				
				//set timestamp
				SetReadTimeStamp();
				
				//copy read data
				area->SetReadArea(Data, dataSize);
				
				//init write area
				if(!this->m_TaskIsRunning)
					area->SetWriteArea(Data, dataSize);
			}
		}

		void ModBusTask::ThreadAreaDiscretInputsR(NodeBitArray* area, bool Data[])
		{								
			//set serial dev id for serial communication
			this->m_ModBusProtocol->SetSerialDevId(area->GetSerialDevId());

			//Read
			UINT32 dataSize = area->Count();
			this->m_ModBusProtocol->ReadDiscreteInputs(area->Address(), area->Count(), Data, &dataSize);
			
			//set timestamp
			SetReadTimeStamp();
			
			//copy read data
			area->SetReadArea(Data, dataSize);					
		}

		DWORD WINAPI ModBusTask::ThreadFunc(void* pArguments)
		{
			ModBusTask*	me = (ModBusTask*)pArguments;
			UINT32 wait = -1;
			clock_t t;
			UINT32 WAIT = me->m_LoopTimeMS; //ms default 20ms

			UINT32 dataAlloc = 128;
			vector<UINT16> data(dataAlloc);
			vector<UINT16> flag(dataAlloc);

			UINT32 coilAlloc = 128;
			bool* coil = new bool[dataAlloc];
			vector<UINT16> coilflag(dataAlloc);

			do
			{
				t = clock();

				map<string, INode::INodePtr>::iterator it = me->m_Nodes.begin();
				while (it != me->m_Nodes.end())
				{
					try
					{
						switch (it->second->Type())
						{
						case NodeType_WordArray:
						{
							NodeWordArray* area = dynamic_cast<NodeWordArray*>(it->second.get());

							switch (area->CustomType())
							{
							case ModBusAreaRegister: //input register
							{
								UINT32 dataSize = area->Count();

								//check allocation
								if (dataAlloc < dataSize)
								{
									data.resize(dataSize);
									flag.resize(dataSize);
									dataAlloc = dataSize;
								}

								me->ThreadAreaRegisterRW(area, data.data(), flag.data());
							}
							break;
							case ModBusAreaHoldingRegister: //holding register
							{
								UINT32 dataSize = area->Count();

								//check allocation
								if (dataAlloc < dataSize)
								{
									data.resize(dataSize);
									dataAlloc = dataSize;
								}

								me->ThreadAreaHoldingRegisterR(area, data.data());
							}
							break;
							}
							area->ClearExceptionForAllNodes();
							break;
						}
						case NodeType_BitArray:
						{
							NodeBitArray* area = dynamic_cast<NodeBitArray*>(it->second.get());

							switch (area->CustomType())
							{
							case ModBusAreaCoil: //coil
							case ModBusAreaMultipleCoils:
							{
								UINT32 dataSize = area->Count();

								//check allocation
								if (coilAlloc < dataSize)
								{
									coil = (bool*)std::realloc(coil, dataSize * sizeof(bool));
									coilflag.resize(dataSize);
									coilAlloc = dataSize;
								}
								me->ThreadAreaCoilRW(area, coil, coilflag.data(), (area->CustomType() == ModBusAreaCoil));
							}
							break;
							case ModBusAreaDiscretInput: //discret inputs
							{
								UINT32 dataSize = area->Count();

								//check allocation
								if (coilAlloc < dataSize)
								{
									coil = (bool*)std::realloc(coil, dataSize * sizeof(bool));
									coilAlloc = dataSize;
								}

								me->ThreadAreaDiscretInputsR(area, coil);
							}
							break;
							}
							area->ClearExceptionForAllNodes();
							break;
						}
						}
						me->ClearException();
					}
					catch(INodeException &e)
					{
						char msg[512];
						e.str(msg, sizeof(msg));
						printf("%s\n", msg);

						INode* area = it->second.get();
						area->SetExceptionForAllNodes(e);
						area->OnErrorChanged();
						
						me->SetException(e);
					}
					it++;
				}
				
				if(!me->m_TaskIsRunning)
					me->m_TaskIsRunning = true;

				//calculate next wait
				UINT32 elapsed = (UINT32)(clock() - t);
				if(WAIT > elapsed)
					wait = WAIT - elapsed;
				else
				{
#ifdef _DEBUG
					if(elapsed>(WAIT*1.5))
						printf("Task timeout: %d [ms]; default: %d [ms]\n", elapsed, WAIT);
#endif
					wait = 1;
				}
			} while(WaitForSingleObject( me->m_StopEvent, wait) != WAIT_OBJECT_0);

			me->m_TaskIsRunning = false;

			delete[] coil;
			data.clear();
			flag.clear();
			coilflag.clear();

			return 0;
		}
	}
}
