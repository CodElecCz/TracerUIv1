// ModBusWrapper.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ModBusWrapper.h"
#include "tinyxml2.h"
#include "..\ModbusLib\ModBusLib.h"

using namespace tinyxml2;

extern char gModuleName[];

namespace Utilities
{
	ModBusWrapperLib::ModBusWrapperLib(void)
	{
		m_modbus = new ModBusLib();
	}

	ModBusWrapperLib::~ModBusWrapperLib(void)
	{
		delete m_modbus;
	}

	void ModBusWrapperLib::AppendNode(HANDLE xml_parent, HANDLE node_parent)
	{
		XMLElement* xml_node = reinterpret_cast<XMLElement*>(xml_parent)->FirstChildElement("node");
		while (xml_node != NULL)
		{
			const char* sid = xml_node->Attribute("id");
			const char* stype = xml_node->Attribute("type");
			const char* soffset = xml_node->Attribute("offset");

			if (0 == strncmp(stype, "WORD", sizeof("WORD") - 1))
			{
				reinterpret_cast<INode*>(node_parent)->CreateNode(sid, NodeType_Word, (UINT16)strtoul(soffset, NULL, 10));
				INode* node = reinterpret_cast<INode*>(node_parent)->GetNodeInterface(sid);

				AppendNode(xml_node, node);
			}
			if (0 == strncmp(stype, "INT16", sizeof("INT16") - 1))
			{
				reinterpret_cast<INode*>(node_parent)->CreateNode(sid, NodeType_Int16, (UINT16)strtoul(soffset, NULL, 10));
				INode* node = reinterpret_cast<INode*>(node_parent)->GetNodeInterface(sid);

				AppendNode(xml_node, node);
			}
			else if (0 == strncmp(stype, "BYTE", sizeof("BYTE") - 1))
			{
				reinterpret_cast<INode*>(node_parent)->CreateNode(sid, NodeType_Byte, (UINT16)strtoul(soffset, NULL, 10));
				INode* node = reinterpret_cast<INode*>(node_parent)->GetNodeInterface(sid);

				AppendNode(xml_node, node);
			}
			else if (0 == strncmp(stype, "BIT", sizeof("BIT") - 1) ||
				0 == strncmp(stype, "BOOL", sizeof("BOOL") - 1))

			{
				reinterpret_cast<INode*>(node_parent)->CreateNode(sid, NodeType_Bit, (UINT16)strtoul(soffset, NULL, 10));
				INode* node = reinterpret_cast<INode*>(node_parent)->GetNodeInterface(sid);

				AppendNode(xml_node, node);
			}

			xml_node = xml_node->NextSiblingElement("node");
		}
	}

	void ModBusWrapperLib::Initialize()
	{
		try
		{
			ModBusLib *modbus = reinterpret_cast<ModBusLib*>(m_modbus);
			XMLDocument doc;
			XMLError err;

			string spath(gModuleName);
			spath.replace(spath.find(".dll"), sizeof(".xml") - 1, ".xml");

			err = doc.LoadFile(spath.data());
			if (err == XML_SUCCESS)
			{
				XMLElement* xml_task = doc.FirstChildElement("modbus")->FirstChildElement("task");
				while (xml_task != NULL)
				{
					const char* sid = xml_task->Attribute("id");
					const char* sip = xml_task->Attribute("ip");
					const char* stime = xml_task->Attribute("loop_time");

					ModBusTask* task = modbus->CreateTask(sid, sip, strtoul(stime, NULL, 10));

					XMLElement* xml_area = xml_task->FirstChildElement("area");
					while (xml_area != NULL)
					{
						const char* stype = xml_area->Attribute("type");
						const char* saccess = xml_area->Attribute("access");

						if (0==strncmp(stype, "Register", sizeof("Register")-1))
						{
							const char* sid2 = xml_area->Attribute("id");
							const char* saddress = xml_area->Attribute("address");
							const char* scount = xml_area->Attribute("count");
							
							bool isR = true;
							bool isW = true;
							if (saccess!=NULL && strlen(saccess)==1)
							{
								isR = (saccess[0] == 'R');
								isW = (saccess[0] == 'W');
							}
							NodeAccess node_access = (NodeAccess)((isR ? NodeAccess_Read : 0) | (isW ? NodeAccess_Write : 0));
							
							NodeWordArray* area = task->CreateRegisterArea(sid2, (UINT16)strtoul(saddress, NULL, 10), (UINT16)strtoul(scount, NULL, 10), node_access);
							INode* node = task->GetNodeInterface(sid2);

							AppendNode(xml_area, node);
						}

						xml_area = xml_area->NextSiblingElement("area");
					}

					//next node
					xml_task = xml_task->NextSiblingElement("task");
				}
			}
			else
			{
				char errMsg[1024];
				sprintf_s(errMsg, sizeof(errMsg)-1, "Xml file not load: '%s'", spath.data());
				throw ModBusWrapperExc("ModBusWrapper", errMsg);
			}

		}
		catch (INodeException& e)
		{
			const char* serrror = e.what();

			char errText[1024];
			e.str_ext(errText, sizeof(errText) - 1);
			
			throw ModBusWrapperExc("ModBusWrapper", errText);
		}
	}

	void ModBusWrapperLib::Start()
	{
		try
		{
			reinterpret_cast<ModBusLib*>(m_modbus)->Start(true);
		}
		catch (INodeException& e)
		{
			const char* serrror = e.what();

			char errText[1024];
			e.str_ext(errText, sizeof(errText) - 1);

			throw ModBusWrapperExc("ModBusWrapper", errText);
		}
	}

	void ModBusWrapperLib::Stop()
	{
		try
		{
			reinterpret_cast<ModBusLib*>(m_modbus)->Stop();
		}
		catch (INodeException& e)
		{
			const char* serrror = e.what();

			char errText[1024];
			e.str_ext(errText, sizeof(errText) - 1);

			throw ModBusWrapperExc("ModBusWrapper", errText);
		}
	}

	
	void ModBusWrapperLib::Read(const char* Path, PUINT16 val, bool WaitForNewDataAvailable)
	{
		try
		{
			INode* node = reinterpret_cast<ModBusLib*>(m_modbus)->GetNodeInterfaceFromPath(Path);

			switch (node->Type())
			{
			case Utilities::Node::NodeType_Bit:
				{
					bool bval;
					//node->Read(&bval);
					reinterpret_cast<ModBusLib*>(m_modbus)->Read(Path, &bval, WaitForNewDataAvailable);
					if (val) *val = (bval) ? 1 : 0;
				}
				break;
			case Utilities::Node::NodeType_Byte:
				{
					UINT8 bval;
					//node->Read(&bval);
					reinterpret_cast<ModBusLib*>(m_modbus)->Read(Path, &bval, WaitForNewDataAvailable);
					if (val) *val = static_cast<UINT16>(bval);
				}
				break;
			case Utilities::Node::NodeType_Int16:
				{
					INT16 bval;
					reinterpret_cast<ModBusLib*>(m_modbus)->Read(Path, &bval, WaitForNewDataAvailable);
					if (val) *val = static_cast<UINT16>(bval);
				}
				break;
			case Utilities::Node::NodeType_Word:
				//node->Read(val);
				reinterpret_cast<ModBusLib*>(m_modbus)->Read(Path, val, WaitForNewDataAvailable);
				break;
			default:
				throw ModBusWrapperExc("ModBusWrapper", "Not existing node type.");
			}
		}
		catch (INodeException& e)
		{
			const char* serrror = e.what();

			char errText[1024];
			e.str_ext(errText, sizeof(errText) - 1);

			throw ModBusWrapperExc("ModBusWrapper", errText);
		}
	}

	void ModBusWrapperLib::Write(const char* Path, UINT16 val, bool WaitForWriteDataAcknowledge)
	{
		try
		{
			INode* node = reinterpret_cast<ModBusLib*>(m_modbus)->GetNodeInterfaceFromPath(Path);

			switch (node->Type())
			{
			case Utilities::Node::NodeType_Bit:
				{
					bool bval = (val != 0);
					//node->Write(val);
					reinterpret_cast<ModBusLib*>(m_modbus)->Write(Path, bval, WaitForWriteDataAcknowledge);
				}
				break;
			case Utilities::Node::NodeType_Byte:
				{
					UINT8 bval = static_cast<UINT8>(val);
					//node->Write(val);
					reinterpret_cast<ModBusLib*>(m_modbus)->Write(Path, bval, WaitForWriteDataAcknowledge);
				}
				break;
			case Utilities::Node::NodeType_Int16:
				{
					INT16 bval = static_cast<INT16>(val);
					//node->Write(val);
					reinterpret_cast<ModBusLib*>(m_modbus)->Write(Path, bval, WaitForWriteDataAcknowledge);
				}
				break;
			case Utilities::Node::NodeType_Word:
				//node->Write(val);
				reinterpret_cast<ModBusLib*>(m_modbus)->Write(Path, val, WaitForWriteDataAcknowledge);
				break;
			default:
				throw ModBusWrapperExc("ModBusWrapper", "Not existing node type.");
			}
		}
		catch (INodeException& e)
		{
			const char* serrror = e.what();

			char errText[1024];
			e.str_ext(errText, sizeof(errText) - 1);

			throw ModBusWrapperExc("ModBusWrapper", errText);
		}
	}

	HANDLE ModBusWrapperLib::GetNode(const char* path)
	{
		try
		{
			INode* node = reinterpret_cast<ModBusLib*>(m_modbus)->GetNodeInterfaceFromPath(path);
			return reinterpret_cast<HANDLE>(node);
		}
		catch (INodeException& e)
		{
			const char* serrror = e.what();

			char errText[1024];
			e.str_ext(errText, sizeof(errText) - 1);

			throw ModBusWrapperExc("ModBusWrapper", errText);
		}
	}
}




