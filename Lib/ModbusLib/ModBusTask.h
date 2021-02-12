#pragma once

#include "stdafx.h"
#include "ModBusProtocol.h"

using namespace std;
using namespace Utilities::ModBus;
using namespace Utilities::Node;


namespace Utilities
{
	namespace ModBus
	{
		enum ModBusAreaType
		{
			ModBusAreaCoil = 0,
			ModBusAreaMultipleCoils,
			ModBusAreaDiscretInput,
			ModBusAreaRegister,
			ModBusAreaHoldingRegister
		};

		class LIB_API ModBusTask : public INode
		{
		public:
			ModBusTask(string Id);
			~ModBusTask(void);

		public:
			//create task connection for reading over TCP
			void Create(const char* IPaddress, UINT32 LoopTimeMS);

			//create task connection for reading over Serial
			void Create(const char* COMport, UINT32 LoopTimeMS, SerialProtocol::BaudRate Baudrate);
			
			//register task memory area for RW thread
			NodeWordArray* CreateRegisterArea(const char* Id, UINT16 StartingAddress, UINT16 RequestedWordCount, UINT32 LoopTimeMS = 0, NodeAccess Access = (NodeAccess)(NodeAccess_Read | NodeAccess_Write));

			//register task memory area for R thread
			NodeWordArray* CreateHoldingRegisterArea(const char* Id, UINT16 StartingAddress, UINT16 RequestedWordCount, UINT32 LoopTimeMS = 0);
			
			//register task memory area for RW thread
			NodeBitArray* CreateCoilArea(const char* Id, UINT16 StartingAddress, UINT16 RequestedBitCount, NodeAccess Access = (NodeAccess)(NodeAccess_Read | NodeAccess_Write), bool UseSingleCoil = true);

			//register task memory area for R thread
			NodeBitArray* CreateDiscretInputsArea(const char* Id, UINT16 StartingAddress, UINT16 RequestedBitCount);
			
			//nodes
			list<INode*> SearchNodeInterface(const char* Id);
			void Refresh(void) {}
			
			//timestamp
			SYSTEMTIME ReadTimeStamp() { CAutoLock al(m_Lock); return m_ReadTime; }
			SYSTEMTIME WriteTimeStamp() { CAutoLock al(m_Lock); return m_WriteTime; }

			//task
			void Start(bool WaitForTaskRunning = false);
			void Stop();
			void Clear();
			bool TaskIsRunning() { return m_TaskIsRunning; };
			void WaitForTaskRunning();
			int GetThreadTimeout();

			//memory Read("AreaId", ...)
			virtual void Read(const char* id, UINT8 ReadData[], PUINT32 ReadDataSize);
			//memory Read() not supported
			virtual void Read(const char* id, UINT16 Address, UINT32 ReadRequest, UINT8 ReadData[], PUINT32 ReadDataSize);
			//memory Read() not supported
			virtual void Write(const char* id, UINT16 Address, UINT8 WriteData[], UINT32 WriteDataSize);

		private:
			static DWORD WINAPI ThreadFunc(void* pArguments);
			void ThreadAreaRegisterRW(NodeWordArray* area, UINT16 Data[], UINT16 Flag[]);
			void ThreadAreaHoldingRegisterR(NodeWordArray* area, UINT16 Data[]);
			void ThreadAreaCoilRW(NodeBitArray* area, bool Data[], UINT16 Flag[], bool UseSingleCoil = true);
			void ThreadAreaDiscretInputsR(NodeBitArray* area, bool Data[]);
			void SetReadTimeStamp() { CAutoLock al(m_Lock); GetSystemTime(&m_ReadTime); }
			void SetWriteTimeStamp() { CAutoLock al(m_Lock); GetSystemTime(&m_WriteTime); }

		protected:
			ModBusProtocol* GetProtocol() { return m_ModBusProtocol.get(); }
			void ReadRegisterArea(const char* id, UINT16 ReadData[], PUINT32 ReadDataSize);

		public:
			typedef shared_ptr<ModBusTask> ModBusTaskPtr;

		private:
#pragma warning(push)
#pragma warning(disable:4251)
			ModBusProtocol::ModBusProtocolPtr	m_ModBusProtocol;
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

