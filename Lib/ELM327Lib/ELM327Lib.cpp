#include "stdafx.h"
#include "ELM327Lib.h"

namespace Utilities
{
	namespace ELM327
	{
#define LIB_TIMEOUT_MS	2000
#define LIB_WAIT_MS		10

		ELM327Lib::ELM327Lib(void)
		{
			m_Tasks.empty();
		}

		ELM327Lib::~ELM327Lib(void)
		{
			Clear();
		}

		ELM327Task* ELM327Lib::AddTask(const char* Id, const char* COMPort, SerialProtocol::BaudRate Baudrate, UINT32 LoopTimeMS)
		{
			string id(Id);

			ELM327Task::ELM327TaskPtr modBusTask(new ELM327Task(id));
			modBusTask->Create(COMPort, LoopTimeMS, Baudrate);

			m_Tasks[id] = modBusTask;

			return modBusTask.get();
		}

		ELM327Task* ELM327Lib::GetTask(const char* Id)
		{
			string id(Id);
			map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.find(id);
			if (it != m_Tasks.end())
			{
				return it->second.get();
			}
			else
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Task id '%s' not created", Id);
				throw Exc(Id, msg);
			}
		}

		ELM327Task* ELM327Lib::GetTaskByIndex(int index)
		{
			if (index < 0 || index >= this->GetTaskCount())
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Task index %d outside of range", index);
				throw Exc("", msg);
			}
			else
			{
				map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.begin();

				for (int i = 0; i < index; i++, it++)
					;

				return it->second.get();
			}
		}

		const char* ELM327Lib::GetTaskNameByIndex(int index)
		{
			return GetTaskByIndex(index)->Id();
		}

		INode* ELM327Lib::GetNodeInterface(const char* Id)
		{
			return dynamic_cast<INode*>(GetTask(Id));
		}

		list<INode*> ELM327Lib::SearchNodeInterface(const char* TaskId, const char* NodeId)
		{
			string id(TaskId);
			map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.find(id);

			if (it != m_Tasks.end())
			{
				return it->second.get()->SearchNodeInterface(NodeId);
			}
			else
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Task id '%s' not exist", TaskId);
				throw Exc(TaskId, msg);
			}

			return list<INode*>();
		}

		INode* ELM327Lib::GetNodeInterfaceFromPath(const char* Path)
		{
			INode* ret = NULL;

			string path(Path);
			size_t pos = path.find('.');
			if (pos>0)
			{
				string TaskId = path.substr(0, pos);
				string NodeId = path.substr(pos + 1, path.length());

				list<INode*> nodeList = SearchNodeInterface(TaskId.c_str(), NodeId.c_str());
				if (nodeList.size() == 1)
				{
					ret = nodeList.front();
				}
				else if (nodeList.size()>1)
				{
					char msg[256];
					sprintf_s(msg, sizeof(msg), "Node id '%s' is duplicated", NodeId.c_str());
					throw Exc(Path, msg);
				}
				else
				{
					char msg[256];
					sprintf_s(msg, sizeof(msg), "Node id '%s' not found", NodeId.c_str());
					throw Exc(Path, msg);
				}
			}
			else
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Not correct path format '%s' ('TaskId.NodeId' or 'TaskId.NodeId.SubNodeId...')", Path);
				throw Exc(Path, msg);
			}

			return ret;
		}

		void ELM327Lib::CheckTaskIsRunning(const char* Path)
		{
			if (!TaskIsRunning(Path))
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Task '%s' not running", Path);
				throw Exc(Path, msg);
			}
		}

		bool  ELM327Lib::TasksRunning(void)
		{
			bool running = (m_Tasks.size()>0);

			map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.begin();
			while (it != m_Tasks.end())
			{
				if (!it->second->TaskIsRunning())
				{
					running = false;
					break;
				}
				it++;
			}

			return running;
		}

		bool  ELM327Lib::TasksStopped(void)
		{
			bool stopped = true;

			map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.begin();
			while (it != m_Tasks.end())
			{
				if (it->second->TaskIsRunning())
				{
					stopped = false;
					break;
				}
				it++;
			}

			return stopped;
		}

		bool ELM327Lib::TaskIsRunning(const char* Path)
		{
			bool running = false;

			string path(Path);
			size_t pos = path.find('.');
			if (pos>0)
			{
				string TaskId = path.substr(0, pos);
				string NodeId = path.substr(pos + 1, path.length());

				running = GetTask(TaskId.c_str())->TaskIsRunning();
			}
			else
			{
				running = GetTask(Path)->TaskIsRunning();
			}

			return running;
		}

		void CheckNewDataAvailable(INode* node)
		{
			INodeException* e = nullptr;
			clock_t t = clock();
			while (!node->ReadDataAvailable())
			{
				e = node->GetLastException();
				if (e != nullptr)
					throw INodeException(e);

				UINT32 elapsed = (UINT32)(clock() - t);
				if (elapsed > LIB_TIMEOUT_MS)
					throw Exc(node->Id(), "CheckNewDataAvailable() error: timeout");

				Sleep(LIB_WAIT_MS);
			}
		}

		void ELM327Lib::Read(const char* Path, bool* Val, bool WaitForNewDataAvailable)
		{
			CheckTaskIsRunning(Path);

			INode* node = GetNodeInterfaceFromPath(Path);

			if (WaitForNewDataAvailable)
				CheckNewDataAvailable(node);

			node->Read(Val);

			INodeException* e = node->GetLastException();
			if (e != nullptr)
				throw INodeException(e);
		}

		void ELM327Lib::Read(const char* Path, PUINT8 Val, bool WaitForNewDataAvailable)
		{
			CheckTaskIsRunning(Path);

			INode* node = GetNodeInterfaceFromPath(Path);

			if (WaitForNewDataAvailable)
				CheckNewDataAvailable(node);

			node->Read(Val);

			INodeException* e = node->GetLastException();
			if (e != nullptr)
				throw INodeException(e);
		}

		void ELM327Lib::Read(const char* Path, PUINT16 Val, bool WaitForNewDataAvailable)
		{
			CheckTaskIsRunning(Path);

			INode* node = GetNodeInterfaceFromPath(Path);

			if (WaitForNewDataAvailable)
				CheckNewDataAvailable(node);

			node->Read(Val);

			INodeException* e = node->GetLastException();
			if (e != nullptr)
				throw INodeException(e);
		}

		void ELM327Lib::Read(const char* Path, UINT32_UT *Val, bool WaitForNewDataAvailable)
		{
			UINT32_UT val;
			bool bitVal;
			UINT8 byteVal;
			UINT16 wordVal;

			val.dword = 0;
			if (Val) *Val = val;

			CheckTaskIsRunning(Path);

			INode* node = GetNodeInterfaceFromPath(Path);

			if (WaitForNewDataAvailable)
				CheckNewDataAvailable(node);

			switch (node->Type())
			{
			case NodeType_Bit:
				node->Read(&bitVal);
				val.bit.b0 = bitVal;
				break;
			case NodeType_Byte:
				node->Read(&byteVal);
				val.byte.byte0 = byteVal;
				break;
			case NodeType_Word:
				node->Read(&wordVal);
				val.word.word0 = wordVal;
				break;
			default:
				throw Exc(Path, "Unknown node type");
			}

			INodeException* e = node->GetLastException();
			if (e != nullptr)
				throw INodeException(e);

			if (Val) *Val = val;
		}

		void CheckWriteDataAcknowledge(INode* node)
		{
			INodeException* e = nullptr;
			clock_t t = clock();
			while (!node->WriteDataAcknowledge())
			{
				e = node->GetLastException();
				if (e != nullptr)
					throw INodeException(e);

				UINT32 elapsed = (UINT32)(clock() - t);
				if (elapsed > LIB_TIMEOUT_MS)
					throw Exc(node->Id(), "CheckWriteDataAcknowledge() error: timeout");

				Sleep(LIB_WAIT_MS);
			}
		}

		void ELM327Lib::Write(const char* path, bool val, bool WaitForWriteDataAcknowledge)
		{
			CheckTaskIsRunning(path);

			INode* node = GetNodeInterfaceFromPath(path);
			node->Write(val);

			if (WaitForWriteDataAcknowledge)
			{
				CheckWriteDataAcknowledge(node);
			}
			else
			{
				INodeException* e = node->GetLastException();
				if (e != nullptr)
					throw INodeException(e);
			}
		}

		void ELM327Lib::Write(const char* path, UINT8 val, bool WaitForWriteDataAcknowledge)
		{
			CheckTaskIsRunning(path);

			INode* node = GetNodeInterfaceFromPath(path);
			node->Write(val);

			if (WaitForWriteDataAcknowledge)
			{
				CheckWriteDataAcknowledge(node);
			}
			else
			{
				INodeException* e = node->GetLastException();
				if (e != nullptr)
					throw INodeException(e);
			}
		}

		void ELM327Lib::Write(const char* path, UINT16 val, bool WaitForWriteDataAcknowledge)
		{
			CheckTaskIsRunning(path);

			INode* node = GetNodeInterfaceFromPath(path);
			node->Write(val);

			if (WaitForWriteDataAcknowledge)
			{
				CheckWriteDataAcknowledge(node);
			}
			else
			{
				INodeException* e = node->GetLastException();
				if (e != nullptr)
					throw INodeException(e);
			}
		}

		void ELM327Lib::Write(const char* path, UINT32_UT Val, bool WaitForWriteDataAcknowledge)
		{
			CheckTaskIsRunning(path);

			INode* node = GetNodeInterfaceFromPath(path);

			switch (node->Type())
			{
			case NodeType_Bit:
				node->Write((bool)Val.bit.b0);
				break;
			case NodeType_Byte:
				node->Write(Val.byte.byte0);
				break;
			case NodeType_Word:
				node->Write(Val.word.word0);
				break;
			default:
				throw Exc(path, "Unknown node type");
			}

			if (WaitForWriteDataAcknowledge)
			{
				CheckWriteDataAcknowledge(node);
			}
			else
			{
				INodeException* e = node->GetLastException();
				if (e != nullptr)
					throw INodeException(e);
			}
		}

		void ELM327Lib::Read(const char* Path, UINT8 ReadData[], PUINT32 ReadDataSize)
		{
			string path(Path);
			size_t pos = path.find('.');
			if (pos>0)
			{
				string TaskId = path.substr(0, pos);
				string NodeId = path.substr(pos + 1, path.length());

				ELM327Task* task = GetTask(TaskId.c_str());
				task->Read(NodeId.c_str(), ReadData, ReadDataSize);
			}
			else
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Not correct path format '%s' ('TaskId.AreaId')", Path);
				throw Exc(Path, msg);
			}
		}

		void ELM327Lib::Start(bool WaitForTaskRunning)
		{
			int timeout = 0;

			std::map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.begin();
			while (it != m_Tasks.end())
			{
				Start(it->second->Id());
				if (timeout < it->second->GetThreadTimeout())
					timeout = it->second->GetThreadTimeout();
				it++;
			}

			if (WaitForTaskRunning)
			{
				clock_t t = clock();

				while (!this->TasksRunning())
				{
					UINT32 elapsed = (UINT32)(clock() - t);
					if (elapsed > (UINT32)timeout*2)
						throw Exc("", "TasksRunning() error: timeout");

					Sleep(LIB_WAIT_MS);
				}
			}

			it = m_Tasks.begin();
			while (it != m_Tasks.end())
			{
				
				INodeException* exc = it->second->GetLastException();
				if (exc)
					throw INodeException(exc);

				it++;
			}
		}

		ELM327Task* ELM327Lib::Start(const char* Id, bool WaitForTaskRunning)
		{
			string id(Id);
			std::map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.find(id);
			if (it != m_Tasks.end())
			{
				it->second->Start(WaitForTaskRunning);
				return it->second.get();
			}
			else
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Task id '%s' not created", Id);
				throw Exc(Id, msg);
			}
		}

		void ELM327Lib::Stop(bool WaitForTaskStopped)
		{
			int timeout = 0;

			std::map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.begin();
			while (it != m_Tasks.end())
			{
				Stop(it->second->Id());
				if (timeout < it->second->GetThreadTimeout())
					timeout = it->second->GetThreadTimeout();
				it++;
			}

			if (WaitForTaskStopped)
			{
				clock_t t = clock();

				while (!this->TasksStopped())
				{
					UINT32 elapsed = (UINT32)(clock() - t);
					if (elapsed > (UINT32)timeout*2)
						throw Exc("", "TasksStopped() error: timeout");

					Sleep(LIB_WAIT_MS);
				}
			}
		}

		ELM327Task* ELM327Lib::Stop(const char* Id)
		{
			string id(Id);
			std::map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.find(id);
			if (it != m_Tasks.end())
			{
				it->second->Stop();
				return it->second.get();
			}
			else
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Task id '%s' not created", Id);
				throw Exc(Id, msg);
			}
		}

		void ELM327Lib::Clear()
		{
			std::map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.begin();
			while (it != m_Tasks.end())
			{
				Clear(it->second->Id());
				it = m_Tasks.begin();
			}
		}

		void ELM327Lib::Clear(const char* Id)
		{
			string id(Id);
			std::map<string, ELM327Task::ELM327TaskPtr>::iterator it = m_Tasks.find(id);
			if (it != m_Tasks.end())
			{
				it->second->Clear();
				m_Tasks.erase(it);
			}
			else
			{
				char msg[256];
				sprintf_s(msg, sizeof(msg), "Task id '%s' not created", Id);
				throw Exc(Id, msg);
			}
		}

	}
}