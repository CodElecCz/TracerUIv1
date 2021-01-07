#include "stdafx.h"
#include "ModBusC.h"
#include "ModBusLib.h"

using namespace std;
using namespace Utilities::ModBus;

static INodeException m_Exception = INodeException();

int ModBusNode_CreateNode(struct _SModBusNode* me, const char*id, enum EModBusNodeType type, unsigned int offset)
{
	if (Utilities::Node::INode* p = reinterpret_cast<Utilities::Node::INode*>(me->_h))
	{
		try
		{
			NodeType ntype = (NodeType)type;
			p->CreateNode(id, ntype, offset);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBusNode_CreateNode() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBusNode_Read(struct _SModBusNode* me, PUINT16 val)
{
	if (Utilities::Node::INode* p = reinterpret_cast<Utilities::Node::INode*>(me->_h))
	{
		try
		{
			switch(p->Type())
			{
			case Utilities::Node::NodeType_Bit:
				{
					bool bval;
					p->Read(&bval);
					if(val) *val = (bval)? 1:0;
				}
				break;
			case Utilities::Node::NodeType_Byte:
				{
					UINT8 bval;
					p->Read(&bval);
					if(val) *val = bval;
				}
				break;
			case Utilities::Node::NodeType_Word:
				p->Read(val);
				break;
			}
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBusNode_Read() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBusNode_Write(struct _SModBusNode* me, UINT16 val)
{
	if (Utilities::Node::INode* p = reinterpret_cast<Utilities::Node::INode*>(me->_h))
	{
		try
		{
			switch(p->Type())
			{
			case Utilities::Node::NodeType_Bit:
				{
					bool bval = (val!=0);
					p->Write(val);
				}
				break;
			case Utilities::Node::NodeType_Byte:
				{
					UINT8 bval = (UINT8)val;
					p->Write(val);
				}
				break;
			case Utilities::Node::NodeType_Word:
				p->Write(val);
				break;
			}
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBusNode_Write() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBusNode_OnValueChanged(struct _SModBusNode* me, OnValueChanged_t EventFncPtr)
{
	if (Utilities::Node::INode* p = reinterpret_cast<Utilities::Node::INode*>(me->_h))
	{
		try
		{
			p->OnValueChanged += EventFncPtr;
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

SModBusNodePtr ModBusNode_GetNode(struct _SModBusNode* me, const char*id)
{
	if (Utilities::Node::INode* p = reinterpret_cast<Utilities::Node::INode*>(me->_h))
	{
		try
		{
			INode* node = p->GetNodeInterface(id);
			SModBusNodePtr pnode = new SModBusNode();
			pnode->_h = reinterpret_cast<HANDLE>(node);
			pnode->CreateNode = ModBusNode_CreateNode;
			pnode->GetNode = ModBusNode_GetNode;
			pnode->OnValueChanged = ModBusNode_OnValueChanged;
			pnode->Write = ModBusNode_Write;
			pnode->Read = ModBusNode_Read;

			return pnode;
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return NULL;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBusNode_GetNode() Unhandled Exception");
			return NULL;
		}
	}

	return NULL;
}

int ModBusTask_CreateRegisterArea(struct _SModBusTask* me, const char* id, UINT16 StartingAddress, UINT16 RequestedWordCount)
{
	if (Utilities::ModBus::ModBusTask* p = reinterpret_cast<Utilities::ModBus::ModBusTask*>(me->_h))
	{
		try
		{
			p->CreateRegisterArea(id, StartingAddress, RequestedWordCount);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBusTask_CreateRegisterArea() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBusTask_CreateCoilArea(struct _SModBusTask* me, const char*id, UINT16 StartingAddress, UINT16 RequestedBitCount)
{
	if (Utilities::ModBus::ModBusTask* p = reinterpret_cast<Utilities::ModBus::ModBusTask*>(me->_h))
	{
		try
		{
			p->CreateCoilArea(id, StartingAddress, RequestedBitCount);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBusTask_CreateCoilArea() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

SModBusNodePtr ModBusTask_GetNode(struct _SModBusTask* me, const char*id)
{
	if (ModBusTask* p = reinterpret_cast<ModBusTask*>(me->_h))
	{
		try
		{
			INode* node = p->GetNodeInterface(id);
			SModBusNodePtr pnode = new SModBusNode();
			pnode->_h = reinterpret_cast<HANDLE>(node);
			pnode->CreateNode = ModBusNode_CreateNode;
			pnode->GetNode = ModBusNode_GetNode;
			pnode->OnValueChanged = ModBusNode_OnValueChanged;
			pnode->Write= ModBusNode_Write;
			pnode->Read = ModBusNode_Read;

			return pnode;
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return NULL;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBusTask_GetNode() Unhandled Exception");
			return NULL;
		}
	}

	return NULL;
}

int ModBus_CreateTask(struct _SModBus* me, const char* id, const char* IPaddress)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			p->CreateTask(id, IPaddress);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_CreateTask() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_CreateTaskSpecific(struct _SModBus* me, const char* id, const char* IPaddress, UINT16 LoopTimeMS)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			p->CreateTask(id, IPaddress, LoopTimeMS);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_CreateNodeSpecific() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_Start(struct _SModBus* me, const char* id)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			p->Start(id);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_Start() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_Stop(struct _SModBus* me, const char* id)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			p->Stop(id);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_Stop() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_Clear(struct _SModBus* me, const char* id)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			p->Clear(id);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_Clear() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_Read(struct _SModBusNode* me, const char* path, PUINT16 val)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			INode* node = p->GetNodeInterfaceFromPath(path);

			switch(node->Type())
			{
			case Utilities::Node::NodeType_Bit:
				{
					bool bval;
					node->Read(&bval);
					if(val) *val = (bval)? 1:0;
				}
				break;
			case Utilities::Node::NodeType_Byte:
				{
					UINT8 bval;
					node->Read(&bval);
					if(val) *val = bval;
				}
				break;
			case Utilities::Node::NodeType_Word:
				node->Read(val);
				break;
			}
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_Read() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_Write(struct _SModBusNode* me, const char* path, UINT16 val)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			INode* node = p->GetNodeInterfaceFromPath(path);

			switch(node->Type())
			{
			case Utilities::Node::NodeType_Bit:
				{
					bool bval = (val!=0);
					node->Write(val);
				}
				break;
			case Utilities::Node::NodeType_Byte:
				{
					UINT8 bval = (UINT8)val;
					node->Write(val);
				}
				break;
			case Utilities::Node::NodeType_Word:
				node->Write(val);
				break;
			}
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_Write() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_ReadMsg(struct _SModBusNode* me, const char* Path, UINT8 ReadData[], PUINT32 ReadDataSize)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			p->Read(Path, ReadData, ReadDataSize);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_ReadMsg() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

int ModBus_ReadMemory(struct _SModBusNode* me, const char* Path, UINT32 Address, UINT32 ReadRequest, UINT8 ReadData[], PUINT32 ReadDataSize)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			p->Read(Path, Address, ReadRequest, ReadData, ReadDataSize);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_ReadMemory() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}


int ModBus_WriteMemory(struct _SModBusNode* me, const char* Path, UINT32 Address, UINT8 WriteData[], UINT32 WriteDataSize)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			p->Write(Path, Address, WriteData, WriteDataSize);
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return -1;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_WriteMemory() Unhandled Exception");
			return -2;
		}
	}

	return 0;
}

SModBusTaskPtr ModBus_GetTask(struct _SModBus* me, const char* id)
{
	if (ModBusLib* p = reinterpret_cast<ModBusLib*>(me->_h))
	{
		try
		{
			ModBusTask* task = p->GetTask(id);

			SModBusTaskPtr ptask = new SModBusTask();
			ptask->_h = reinterpret_cast<HANDLE>(task);
			ptask->CreateCoilArea = ModBusTask_CreateCoilArea;
			ptask->CreateRegisterArea = ModBusTask_CreateRegisterArea;
			ptask->GetNode = ModBusTask_GetNode;

			return ptask;
		}
		catch(INodeException &e)
		{
			m_Exception = e;
			return NULL;
		}
		catch(...)
		{
			m_Exception = INodeException("ModBus_GetTask() Unhandled Exception");
			return NULL;
		}
	}

	return NULL;
}

void modbusnode_delete(SModBusNodePtr* pModBusNode)
{
	SModBusNodePtr pnode = *pModBusNode;
	if(pnode!=NULL)
	{
		delete pnode;
		*pModBusNode = NULL;
	}
}

void modbustask_delete(SModBusTaskPtr* pModBusTask)
{
	SModBusTaskPtr ptask = *pModBusTask;
	if(ptask!=NULL)
	{
		delete ptask;
		*pModBusTask = NULL;
	}
}

void modbus_new(SModBusPtr* pModBus)
{
	if (ModBusLib* p = new ModBusLib())
	{
		SModBusPtr pmodbus = new SModBus();
		pmodbus->_h = reinterpret_cast<HANDLE>(p);
		pmodbus->CreateTask = ModBus_CreateTask;
		pmodbus->CreateTaskSpecific = ModBus_CreateTaskSpecific;
		pmodbus->GetTask = ModBus_GetTask;
		pmodbus->Start = ModBus_Start;
		pmodbus->Stop = ModBus_Stop;
		pmodbus->Clear = ModBus_Clear;
		pmodbus->Read = ModBus_Read;
		pmodbus->Write = ModBus_Write;
		pmodbus->ReadMsg = ModBus_ReadMsg;
		pmodbus->ReadMemory = ModBus_ReadMemory;
		pmodbus->WriteMemory = ModBus_WriteMemory;

		*pModBus = pmodbus;
	}
}

void modbus_delete(SModBusPtr* pModBus)
{
	SModBusPtr pmodbus = *pModBus;
	if(pmodbus!=NULL)
	{
		if (ModBusLib* p = reinterpret_cast<ModBusLib*>(pmodbus->_h))
		{
			delete p;
		}
		delete pmodbus;
		*pModBus = NULL;
	}
}

void ModBusGetLastError(char* msg, unsigned int msgSize)
{
	if(!m_Exception.empty())
	{
		sprintf_s(msg, msgSize, "%s; id:%s; func:%s; file:%s; line:%d", m_Exception.what(), m_Exception.id(), m_Exception.fnc(), m_Exception.file(), m_Exception.line());
	}
	else
	{
		sprintf_s(msg, msgSize, "");
	}
}