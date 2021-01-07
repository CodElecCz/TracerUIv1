#include "stdafx.h"
#include "ELM327Task.h"

namespace Utilities
{
	namespace ELM327
	{
		ELM327Task::ELM327Task(string Id) : INode(Id, NodeType_Task)
		{
			m_hThread = 0;
			m_threadID = 0;
			m_StopEvent = 0;
			m_TaskIsRunning = false;
			memset(&m_ReadTime, 0, sizeof(SYSTEMTIME));
			memset(&m_WriteTime, 0, sizeof(SYSTEMTIME));
		}

		ELM327Task::~ELM327Task(void)
		{
			Clear();
		}

		void ELM327Task::Create(const char* COMport, UINT32 LoopTimeMS, SerialProtocol::BaudRate Baudrate)
		{
			Clear();
			
			m_LoopTimeMS = LoopTimeMS;
			m_ELM327Protocol = ELM327Protocol::ELM327ProtocolPtr(new ELM327Protocol(COMport, Baudrate));
		}

		NodeUInt64Array* ELM327Task::AddArea(const char* Id, UINT16 RequestedUInt64Count, NodeAccess Access)
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

			NodeUInt64Array::NodeUInt64ArrayPtr area(new NodeUInt64Array(Id, NodeType_UInt64Array, Access, 0, RequestedUInt64Count));
			m_Nodes[id] = area;

			return area.get();
		}

		void ELM327Task::Start(bool WaitForTaskRunning)
		{
			if(m_Nodes.size()==0)
				throw Exc(this->Id(), "No area was defined");

			if(m_hThread == 0)
			{
				m_StopEvent	= CreateEvent( NULL,   // No security attributes
					TRUE,  // Manual-reset event
					FALSE,   // Initial state is not signaled
					NULL);  // Object not named

				m_hThread = CreateThread(NULL, 0, &ELM327Task::ThreadFunc, this, 0, &m_threadID);

				if(WaitForTaskRunning)
				{
					this->WaitForTaskRunning();
				}
			}
		}

		void ELM327Task::Stop()
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

		int ELM327Task::GetThreadTimeout()
		{
			int timeout = 0;

			if(m_Nodes.size())
			{
				size_t nbOfAreas = m_Nodes.size();
				timeout = (int)nbOfAreas * (this->m_ELM327Protocol.get()->GetTimeout() + 200);
			}

			return timeout;
		}

		void ELM327Task::Clear()
		{
			Stop(); 

			m_Nodes.clear();
		}

		list<INode*> ELM327Task::SearchNodeInterface(const char* Id)
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

		void ELM327Task::ReadArea(const char* Id, PUINT32 ReadData)
		{
			UINT32 readData[1];
			UINT32 dataSize = 1;
			GetNodeInterface(Id)->Read(readData, &dataSize);

			if (ReadData) *ReadData = readData[0];
		}

		void ELM327Task::Read(const char* id, UINT8 ReadData[], PUINT32 ReadDataSize)
		{
			UINT32 ReadDataSizeAlloc = *ReadDataSize;
			*ReadDataSize = 0;

			NodeUInt64Array* area = dynamic_cast<NodeUInt64Array*>(GetNodeInterface(id));

			UINT64 data[1];
			UINT32 dataSize = 1;
			area->Read(data, &dataSize);
			dataSize = sizeof(data);

			memcpy_s(ReadData, ReadDataSizeAlloc, &data, dataSize*2);
			*ReadDataSize = dataSize*2;
		}

		void ELM327Task::WaitForTaskRunning()
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

		void ELM327Task::ThreadAreaR(NodeUInt64Array* area, UINT64 Data[], UINT32 Offset)
		{
			if(area->Access()&NodeAccess_Read)
			{									
				//timestamp
				SetReadTimeStamp();
				
				//copy read data
				area->SetReadArea(Data, 1, Offset);	
				//init write area
				if(!this->m_TaskIsRunning)
					area->SetWriteArea(Data, 1, Offset);
			}
		}

		DWORD WINAPI ELM327Task::ThreadFunc(void* pArguments)
		{
			ELM327Task*	me = (ELM327Task*)pArguments;
			UINT32 wait = -1;
			clock_t t;
			UINT32 WAIT = me->m_LoopTimeMS; //ms default 20ms

			UINT8 data[8192];
			string s("");
			
			do
			{
				t = clock();
				
				map<string, INode::INodePtr>::iterator it = me->m_Nodes.begin();
			
				if (it != me->m_Nodes.end())
				{
					NodeUInt64Array* area = dynamic_cast<NodeUInt64Array*>(it->second.get());

					try
					{
						if (!me->m_TaskIsRunning)
						{
							me->GetProtocol()->Enter();
							me->GetProtocol()->TraceInit();
						}
						else
						{
#if 1
							UINT32 data_Size = sizeof(data);
							memset(data, 0, data_Size);
							me->GetProtocol()->TraceRead(data, &data_Size);
#if 1
							s.append((char*)data, data_Size+1);
							
							size_t offset = s.find('\r', 0);
							size_t offsetLast = 0;
							string msg;
							while (offset != std::string::npos)
							{
								msg.clear();
								msg.append(&s.at(offsetLast), &s.at(offset - 1));

								//message parse
								size_t msgSpc = msg.find(' ', 0);
								if (msgSpc == 3)
								{
									UINT32 can32[8] = { 0,0,0,0,0,0,0,0 };
									UINT32 canSize = 0;
									char* endPtr;
									UINT32 canId = strtoul(msg.c_str(), &endPtr, 16);

									if(endPtr!=NULL)
										canSize = strtoul(endPtr, &endPtr, 10);

									if (endPtr != NULL)
										sscanf(endPtr, " %02X %02X %02X %02X %02X %02X %02X %02X",
															&can32[0],
															&can32[1],
															&can32[2],
															&can32[3],
															&can32[4],
															&can32[5],
															&can32[6],
															&can32[7]);

									char canIdStr[8];
									sprintf_s(canIdStr, "0x%03X", canId);

									INode* node;
									UINT32 offset;
									if (area->NodeExist(canIdStr))
									{
										node = area->GetNodeInterface(canIdStr);
										offset = node->Offset();
									}
									else
									{
										offset = area->NodesCount();
										node = area->CreateNode(canIdStr, NodeType_UInt64, offset);
									}

									UINT64 can64[1];
									can64[0] = ((UINT64)(can32[0] & 0xff) << 0)
										+ ((UINT64)(can32[1] & 0xff) << 8)
										+ ((UINT64)(can32[2] & 0xff) << 16)
										+ ((UINT64)(can32[3] & 0xff) << 24)
										+ ((UINT64)(can32[4] & 0xff) << 32)
										+ ((UINT64)(can32[5] & 0xff) << 40)
										+ ((UINT64)(can32[6] & 0xff) << 48)
										+ ((UINT64)(can32[7] & 0xff) << 56);

									me->ThreadAreaR(area, can64, offset);
								}
								else
								{
									//wrong formated
									printf("Wrong format: '%s'\r\n", msg.c_str());
								}

								offsetLast = offset+1;

								if ((offset + 1) < data_Size)
									offset = s.find('\r', offset + 1);
								else
									break;
							}
							
							if ((offsetLast + 1) < data_Size)
							{								
								msg.clear();
								msg.append(&s.at(offsetLast), &s.at(s.length()-1));
								s.clear();
								s.append(msg);

								//printf("Append: '%s'\r\n", msg.c_str());
							}
							else
								s.clear();

#else
							/* TODO: Parse Received data to Nodes with CAN Ids */
							char *next_token = NULL;
							char *token = strtok_s((char*)data, "\r", &next_token);
							while (token != NULL)
							{
								UINT32 can32[8] = { 0,0,0,0,0,0,0,0 };
								UINT32 canId = 0;
								UINT32 canSize = 0;
								sscanf(token, "%03X %d %02X %02X %02X %02X %02X %02X %02X %02X", 
													&canId, 
													&canSize, 
													&can32[0], 
													&can32[1],
													&can32[2],
													&can32[3],
													&can32[4],
													&can32[5],
													&can32[6],
													&can32[7]);

								char canIdStr[8];
								sprintf_s(canIdStr, "0x%03X", canId);

								INode* node;
								UINT32 offset;
								if (area->NodeExist(canIdStr))
								{
									node = area->GetNodeInterface(canIdStr);
									offset = node->Offset();				
								}
								else
								{
									offset = area->NodesCount();
									node = area->CreateNode(canIdStr, NodeType_UInt64, offset);
								}
								
								UINT64 can64[1];
								can64[0] = ((UINT64)(can32[0] & 0xff) << 0)
									+ ((UINT64)(can32[1] & 0xff) << 8)
									+ ((UINT64)(can32[2] & 0xff) << 16)
									+ ((UINT64)(can32[3] & 0xff) << 24)
									+ ((UINT64)(can32[4] & 0xff) << 32)
									+ ((UINT64)(can32[5] & 0xff) << 40)
									+ ((UINT64)(can32[6] & 0xff) << 48)
									+ ((UINT64)(can32[7] & 0xff) << 56);

								me->ThreadAreaR(area, can64, offset);

								token = strtok_s(NULL, "\r", &next_token);
							}
#endif
#endif
							me->ClearException();
						}
					}
					catch (INodeException &e)
					{
						char msg[512];
						e.str(msg, sizeof(msg));
						printf("%s\n", msg);
						me->SetException(e);
					}

					if (!me->m_TaskIsRunning)
						me->m_TaskIsRunning = true;

					//calculate next wait
					UINT32 elapsed = (UINT32)(clock() - t);
					if (WAIT > elapsed)
						wait = WAIT - elapsed;
					else
					{
#ifdef _DEBUG
						if (elapsed > (WAIT*1.5))
							printf("Task timeout: %d [ms]; default: %d [ms]\n", elapsed, WAIT);
#endif
						wait = 1;
					}
				}
			} while(WaitForSingleObject( me->m_StopEvent, wait) != WAIT_OBJECT_0);

			/* Cleanup */
			try
			{
				me->GetProtocol()->Enter();
//				me->GetProtocol()->Reset();
			}
			catch (INodeException &e)
			{
				char msg[512];
				e.str(msg, sizeof(msg));
				printf("%s\n", msg);
			}

			me->m_TaskIsRunning = false;

			return 0;
		}
	}
}
