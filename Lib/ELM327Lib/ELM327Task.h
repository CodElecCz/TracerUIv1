#pragma once

#include "stdafx.h"
#include "ELM327Protocol.h"

using namespace std;
using namespace Utilities::Node;

namespace Utilities
{
	namespace ELM327
	{
		class LIB_API ELM327Task : public INode
		{
		public:
			ELM327Task(string Id);
			~ELM327Task(void);

		public:
			//create task connection for reading over Serial
			void Create(const char* COMport, UINT32 LoopTimeMS, SerialProtocol::BaudRate Baudrate);
			
			//area
			NodeUInt64Array* AddArea(const char* Id, UINT16 RequestedUInt64Count, NodeAccess Access = (NodeAccess)(NodeAccess_Read));

			//nodes
			list<INode*> SearchNodeInterface(const char* Id);
			void Refresh(void) {}

			//timestamp
			SYSTEMTIME ReadTimeStamp() { CAutoLock al(m_Lock); return m_ReadTime; }
			SYSTEMTIME WriteTimeStamp() { CAutoLock al(m_Lock); return m_WriteTime; }
			
			//thread
			void Start(bool WaitForTaskRunning = false);
			void Stop();
			void Clear();
			bool TaskIsRunning() { return m_TaskIsRunning; }
			void WaitForTaskRunning();
			int GetThreadTimeout();

			//memory Read("AreaId", ...)
			virtual void Read(const char* id, UINT8 ReadData[], PUINT32 ReadDataSize);
			
		private:
			static DWORD WINAPI ThreadFunc(void* pArguments);
			void ThreadAreaR(NodeUInt64Array* area, UINT64 Data[], UINT32 Offset);
			void SetReadTimeStamp() { CAutoLock al(m_Lock); GetSystemTime(&m_ReadTime); }
			void SetWriteTimeStamp() { CAutoLock al(m_Lock); GetSystemTime(&m_WriteTime); }
			
		protected:
			ELM327Protocol* GetProtocol() { return m_ELM327Protocol.get(); }
			void ReadArea(const char* id, PUINT32 ReadData);

		public:
			typedef shared_ptr<ELM327Task> ELM327TaskPtr;

		private:
#pragma warning(push)
#pragma warning(disable:4251)
			ELM327Protocol::ELM327ProtocolPtr	m_ELM327Protocol;
			CLock				m_Lock;
#pragma warning(pop)
			SYSTEMTIME			m_ReadTime;
			SYSTEMTIME			m_WriteTime;
			HANDLE				m_hThread;
			HANDLE				m_StopEvent;
			DWORD				m_threadID;
			UINT32				m_LoopTimeMS;
			bool				m_TaskIsRunning;
		};
	}
}

