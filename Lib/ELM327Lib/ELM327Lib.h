#pragma once

#include "stdafx.h"
#include "ELM327Task.h"

using namespace std;
using namespace Utilities::ELM327;

namespace Utilities
{
	namespace ELM327
	{
        class LIB_API ELM327Lib
		{
		public:
			ELM327Lib(void);
			~ELM327Lib(void);

		public:
			//add Serial task
			ELM327Task* AddTask(const char* Id, const char* COMPort, SerialProtocol::BaudRate Baudrate, UINT32 LoopTimeMS = 20);

			ELM327Task* GetTask(const char* Id);
			ELM327Task* GetTaskByIndex(int Index);
			size_t GetTaskCount() { return m_Tasks.size(); }
			const char* GetTaskNameByIndex(int Index);

			ELM327Task* Start(const char* TaskId, bool WaitForTaskRunning = false);
			ELM327Task* Stop(const char* TaskId);
			void Clear(const char* TaskId);
			void Remove(const char* TaskId);

			void Start(bool WaitForTaskRunning = false);
			void Stop(bool WaitForTaskStopped = false);
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
			//node BIT, BYTE, WORD Read("TaskId.NodeId.SubNodeId", &val);
			void Read(const char* Path, UINT32_UT *Val, bool WaitForNewDataAvailable = false);

			//node BIT Write("TaskId.NodeId.SubNodeId", val);
			void Write(const char* Path, bool Val, bool WaitForWriteDataAcknowledge = false);
			//node BYTE Write("TaskId.NodeId.SubNodeId", val);
			void Write(const char* Path, UINT8 Val, bool WaitForWriteDataAcknowledge = false);
			//node WORD Write("TaskId.NodeId.SubNodeId", val);
			void Write(const char* Path, UINT16 Val, bool WaitForWriteDataAcknowledge = false);
			//node BIT, BYTE, WORD Write("TaskId.NodeId.SubNodeId", val);
			void Write(const char* Path, UINT32_UT Val, bool WaitForWriteDataAcknowledge = false);

			//OMC8000TaskDefault: memory Read("TaskId.AreaId", ...)
			void Read(const char* Path, UINT8 ReadData[], PUINT32 ReadDataSize);

			INode* GetNodeInterface(const char* id);
			INode* GetNodeInterfaceFromPath(const char* path);
			//SearchNodeInterface("TaskId", "NodeId.SubNodeId")
			list<INode*> SearchNodeInterface(const char* TaskId, const char* NodeId);

		private:
#pragma warning(push)
#pragma warning(disable:4251)
			map<string, ELM327Task::ELM327TaskPtr> m_Tasks;
#pragma warning(pop)

		};
	}
}
