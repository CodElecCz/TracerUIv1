#pragma once

#include "stdafx.h"
#include "ModBusTask.h"

using namespace std;
using namespace Utilities::ModBus;

namespace Utilities
{
	namespace ModBus
	{
		class LIB_API ModBusLib
		{
		public:
			ModBusLib(void);
			~ModBusLib(void);

		public:
			//add TCP task
			ModBusTask* CreateTask(const char* Id, const char* IPaddress, UINT32 LoopTimeMS = 20);			
			//add Serial task
			ModBusTask* CreateTask(const char* Id, const char* COMPort, SerialProtocol::BaudRate Baudrate, UINT32 LoopTimeMS = 20);
			
			ModBusTask* GetTask(const char* Id);
			ModBusTask* GetTaskByIndex(int Index);
			size_t GetTaskCount() { return m_Tasks.size(); }
			const char* GetTaskNameByIndex(int Index);

			ModBusTask* Start(const char* TaskId, bool WaitForTaskRunning = false);
			ModBusTask* Stop(const char* TaskId);
			void Clear(const char* TaskId);
			void Remove(const char* TaskId) {};

			void Start(bool WaitForTaskRunning = false);
			void Stop(void);
			void Clear(void);

			//check all tasks are running
			bool TasksRunning(void);
			bool TasksStopped(void);

			//check single task
			bool TaskIsRunning(const char* TaskId);

			//check single task with exception
			void CheckTaskIsRunning(const char* Path);

			//node BIT Read("TaskId.NodeId.SubNodeId", &val);
			void Read(const char* Path, bool* Val, bool WaitForNewDataAvailable = false);
			//node BYTE Read("TaskId.NodeId.SubNodeId", &val);
			void Read(const char* Path, PUINT8 Val, bool WaitForNewDataAvailable = false);
			//node WORD Read("TaskId.NodeId.SubNodeId", &val);
			void Read(const char* Path, PUINT16 Val, bool WaitForNewDataAvailable = false);
			//node INT16 Read("TaskId.NodeId.SubNodeId", &val);
			void Read(const char* Path, PINT16 Val, bool WaitForNewDataAvailable = false);
			//node BIT, BYTE, WORD Read("TaskId.NodeId.SubNodeId", &val);
			void Read(const char* Path, UINT32_UT *Val, bool WaitForNewDataAvailable = false);

			//node BIT Write("TaskId.NodeId.SubNodeId", val);
			void Write(const char* Path, bool Val, bool WaitForWriteDataAcknowledge = false);
			//node BYTE Write("TaskId.NodeId.SubNodeId", val);
			void Write(const char* Path, UINT8 Val, bool WaitForWriteDataAcknowledge = false);
			//node WORD Write("TaskId.NodeId.SubNodeId", val);
			void Write(const char* Path, UINT16 Val, bool WaitForWriteDataAcknowledge = false);
			//node INT16 Write("TaskId.NodeId.SubNodeId", val);
			void Write(const char* Path, INT16 Val, bool WaitForWriteDataAcknowledge = false);
			//node BIT, BYTE, WORD Write("TaskId.NodeId.SubNodeId", val);
			void Write(const char* Path, UINT32_UT Val, bool WaitForWriteDataAcknowledge = false);

			//memory Read("TaskId.AreaId", ...)
			void Read(const char* Path, UINT8 ReadData[], PUINT32 ReadDataSize);
			//not supported
			void Read(const char* Path, UINT16 Address, UINT32 ReadRequest, UINT8 ReadData[], PUINT32 ReadDataSize);
			//not supported
			void Write(const char* Path, UINT16 Address, UINT8 WriteData[], UINT32 WriteDataSize);

			INode* GetNodeInterface(const char* id);
			INode* GetNodeInterfaceFromPath(const char* path);
			//SearchNodeInterface("TaskId", "NodeId.SubNodeId")
			list<INode*> SearchNodeInterface(const char* TaskId, const char* NodeId);

		private:
#pragma warning(push)
#pragma warning(disable:4251)
			map<string, ModBusTask::ModBusTaskPtr> m_Tasks;
#pragma warning(pop)
		};
	}
}