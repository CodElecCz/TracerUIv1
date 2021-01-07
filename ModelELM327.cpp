#include <QFile>
#include <QIcon>
#include <QSpinBox>
#include <QTimer>
#include <QThread>
#include <QMessageBox>
#include <QApplication>
#include "ModelELM327.h"

#ifdef _DEBUG
#define DISABLE_UPDATE_EVENTS
#endif

#define NODES_APPEND_DYNAMICALY

Elm327Model::Elm327Model(const QString xmlFile, QObject *parent) :
    QAbstractItemModel(parent)
{
    rootItem = new TreeItem();

    elm327Lib = new ELM327Lib();

    if(!xmlFile.isNull() && !xmlFile.isEmpty())
    {
        setupModelData(xmlFile, rootItem);
    }
}

Elm327Model::~Elm327Model()
{
    delete elm327Lib;
    delete rootItem;
}

void Elm327Model::loadModelData(const QString xmlFile)
{
    if(!xmlFile.isNull() && !xmlFile.isEmpty())
    {
        setupModelData(xmlFile, rootItem);
    }
}

void Elm327Model::updateNode(QString itemName, QModelIndex parentIndex)
{
    int varCount = this->rowCount(parentIndex);
    for(int i = 0; i<varCount; i++)
    {
         QModelIndex varIndex = parentIndex.child(i, TreeUIColumn_Id);
         QString varName = varIndex.data().toString();
         if(varName == itemName)
         {
             QModelIndex index = parentIndex.child(i, TreeUIColumn_Read);
             emit dataChanged(index, index);
             break;
         }
         else
         {
             updateNode(itemName, varIndex);
         }
    }
}

void Elm327Model::update(QString itemName)
{
    int taskCount = this->rowCount(QModelIndex());
    for(int i = 0; i<taskCount; i++)
    {
        QModelIndex taskIndex = this->index(i, TreeUIColumn_Id, QModelIndex());

        int areaCount = this->rowCount(taskIndex);
        for(int j = 0; j<areaCount; j++)
        {
            QModelIndex areaIndex = taskIndex.child(j, TreeUIColumn_Id);
            QString areaName = areaIndex.data().toString();
            if(areaName == itemName)
            {
                //Check if new nodes not available
                int nodeCount = this->rowCount(areaIndex);
#ifdef NODES_APPEND_DYNAMICALY
                ELM327Task* task = elm327Lib->GetTask(taskIndex.data().toString().toLatin1().data());
                INode *area = task->GetNodeInterface(areaName.toLatin1().data());

                int nodeCountLib = area->NodesCount();

                if(nodeCount<nodeCountLib)
                {
                    QStringList list;
                    int k;
                    for(k = 0; k<nodeCount; k++)
                    {
                        QModelIndex nodeIndex = areaIndex.child(k, TreeUIColumn_Id);
                        QString nodeName = nodeIndex.data().toString();
                        list.append(nodeName);
                    }

                    //append missing nodes from Lib
                    TreeItem *parentItem = this->getItem(areaIndex);

                    vector<INode*> nodeList = area->GetNodes();
                    for(k = 0; k<nodeList.size(); k++)
                    {
                        INode* node  = nodeList[k];

                        QString nodeId(node->Id());
                        bool exist = list.contains(nodeId);
                        if(!exist)
                        {
                            TreeItem *nodeItem = new TreeItem(node, parentItem);
                            parentItem->appendChild(nodeItem);
                        }
                    }
                    //TODO update treeview
                    //QModelIndex index0 = areaIndex.child(0, TreeUIColumn_Id);
                    //QModelIndex index1 = areaIndex.child(k-1, TreeUIColumnSize-1);
                    //emit dataChanged(index0, index1);
                }
                else
#endif
                {
                    //data change
                    QModelIndex index0 = taskIndex.child(j, TreeUIColumn_Type);
                    QModelIndex index1 = taskIndex.child(j, TreeUIColumnSize-1);
                    emit dataChanged(index0, index1);
                }
                break;
            }
            else
            {
                //updateNode(itemName, areaIndex);
            }
        }
    }
}

void Elm327Model::update()
{
    int taskCount = this->rowCount(QModelIndex());
    for(int i = 0; i<taskCount; i++)
    {
        QModelIndex startIndex = this->index(i, TreeUIColumn_Type, QModelIndex());
        QModelIndex stopIndex = this->index(i, TreeUIColumnSize-1, QModelIndex());

        emit dataChanged(startIndex, stopIndex);
    }
}

void Elm327Model::OnValueChanged(INode* node, UINT64 val)
{
    if (thread()!=QThread::currentThread())
    {
        QString *nodeId = new QString(node->Id());
        //qDebug()<<"PlcViewModel::OnValueChanged() [" + *nodeId + "] threadId:0x" + QString::number((int)thread()->currentThreadId(), 16);

        QGenericArgument arg("QString", nodeId);
        QMetaObject::invokeMethod(this, "update", arg);
    }
    else
        emit this->update(node->Id());
}

QVariant Elm327Model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if(role == Qt::DisplayRole)
    {
        return item->data(index.column());
    }
    else if(role == Qt::EditRole)
    {
        return item->data(index.column());
    }
    else if(role == Qt::DecorationRole)
    {
        if(index.column()==TreeUIColumn_Id)
            return item->icon();
        else if(index.column()==TreeUIColumn_Read)
        {
            return item->iconState();
        }
    }

    return QVariant();
}

int Elm327Model::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

     if (!parent.isValid())
         parentItem = rootItem;
     else
         parentItem = static_cast<TreeItem*>(parent.internalPointer());

     return parentItem->childCount();
}

int Elm327Model::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QModelIndex Elm327Model::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

     TreeItem *parentItem;

     if (!parent.isValid())
         parentItem = rootItem;
     else
         parentItem = static_cast<TreeItem*>(parent.internalPointer());

     TreeItem *childItem = parentItem->child(row);
     if (childItem)
         return createIndex(row, column, childItem);
     else
         return QModelIndex();
}

QModelIndex Elm327Model::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(child.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

Qt::ItemFlags Elm327Model::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (!index.isValid())
        return 0;

    if(index.column()==TreeUIColumn_Write)
        flag |= Qt::ItemIsEditable;

    return flag;
}

QVariant Elm327Model::headerData(int section, Qt::Orientation orientation,
                                int role) const
 {
     if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
         return rootItem->data(section);

     return QVariant();
 }

TreeItem *Elm327Model::getItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

bool Elm327Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool result = false;

    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    switch(value.type())
    {
    case QVariant::String:
    case QVariant::Int:
        result = item->setData(index.column(), value);
        break;
    default:
        result = item->setData(value);
        break;
    }

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool Elm327Model::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}

bool Elm327Model::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

void Elm327Model::setupDefaultNode(TreeItem* parentItem, INode* parentNode)
{
    TreeItem* nodeItem;
    vector<INode*> nodeList = parentNode->GetNodes();
    for(UINT32 i = 0; i<nodeList.size(); i++)
    {
        INode* node  = nodeList[i];

        node->OnValueChanged += event_handler(this, &Elm327Model::OnValueChanged);
        nodeItem = new TreeItem(node, parentItem);

        //search for subnodes
        setupDefaultNode(nodeItem, node);

        parentItem->appendChild(nodeItem);
    }
}

TreeItemStateColors Elm327ItemColor(QString scolor)
{
    TreeItemStateColors color;
    if(!scolor.isNull() && !scolor.isEmpty())
    {
        if(scolor.contains("Black", Qt::CaseInsensitive))
            color = TreeItemStateColor_Black;
        else if(scolor.contains("Blue", Qt::CaseInsensitive))
            color = TreeItemStateColor_Blue;
        else if(scolor.contains("DarkGreen", Qt::CaseInsensitive))
            color = TreeItemStateColor_DarkGreen;
        else if(scolor.contains("Gold", Qt::CaseInsensitive))
            color = TreeItemStateColor_Gold;
        else if(scolor.contains("Geay", Qt::CaseInsensitive))
            color = TreeItemStateColor_Gray;
        else if(scolor.contains("LightGreen", Qt::CaseInsensitive) || scolor.contains("Green", Qt::CaseInsensitive))
            color = TreeItemStateColor_LightGreen;
        else if(scolor.contains("Orange", Qt::CaseInsensitive))
            color = TreeItemStateColor_Orange;
        else if(scolor.contains("Pink", Qt::CaseInsensitive))
            color = TreeItemStateColor_Pink;
        else if(scolor.contains("Red", Qt::CaseInsensitive))
            color = TreeItemStateColor_Red;
        else if(scolor.contains("Violet", Qt::CaseInsensitive))
            color = TreeItemStateColor_Violet;
        else
            color = TreeItemStateColor_Gray;
    }

    return color;
}

void Elm327Model::setupNode(QDomElement parentElem, TreeItem *parentItem, INode* parentNode)
{
    INode *node;
    TreeItem* nodeItem;

    QDomNode nodeNode = parentElem.firstChild();
    while(!nodeNode.isNull())
    {
        //node
        QDomElement nodeElem = nodeNode.toElement();
        if(!nodeElem.isNull())
        {
            QString sid = nodeElem.attribute("id");
            QString stype = nodeElem.attribute("type");
            QString offset = nodeElem.attribute("offset");
            QString access = nodeElem.attribute("access");
            QString sdesc = nodeElem.attribute("desc");

            //default RW
            bool isR = true;
            bool isW = true;
            if(!access.isNull() && !access.isEmpty())
            {
                isR = (access.indexOf(QChar('R'))>=0);
                isW = (access.indexOf(QChar('W'))>=0);
            }
            NodeAccess node_access = (NodeAccess)((isR? NodeAccess_Read:0)|(isW? NodeAccess_Write:0));

            if(stype.contains("REAL64", Qt::CaseInsensitive))
            {
                parentNode->CreateNode(sid.toLatin1().constData(), NodeType_Real64, offset.toUShort(), node_access);
                node = parentNode->GetNodeInterface(sid.toLatin1().constData());
                node->Description(sdesc.toUtf8().constData());
                //node->OnNodeValueChanged += event_handler(this, &Omc8000Model::OnValueChanged);
                nodeItem = new TreeItem(node, parentItem);

                //nodes
                setupNode(nodeElem, nodeItem, node);

                parentItem->appendChild(nodeItem);
            }
            else if(stype.contains("UINT64", Qt::CaseInsensitive))
            {
                parentNode->CreateNode(sid.toLatin1().constData(), NodeType_UInt64, offset.toUShort(), node_access);
                node = parentNode->GetNodeInterface(sid.toLatin1().constData());
                node->Description(sdesc.toUtf8().constData());
                //node->OnNodeValueChanged += event_handler(this, &Omc8000Model::OnValueChanged);
                nodeItem = new TreeItem(node, parentItem);

                //nodes
                setupNode(nodeElem, nodeItem, node);

                parentItem->appendChild(nodeItem);
            }
            else if(stype.contains("REAL", Qt::CaseInsensitive))
            {
                parentNode->CreateNode(sid.toLatin1().constData(), NodeType_Real32, offset.toUShort(), node_access);
                node = parentNode->GetNodeInterface(sid.toLatin1().constData());
                node->Description(sdesc.toUtf8().constData());
                //node->OnNodeValueChanged += event_handler(this, &Omc8000Model::OnValueChanged);
                nodeItem = new TreeItem(node, parentItem);

                //nodes
                setupNode(nodeElem, nodeItem, node);

                parentItem->appendChild(nodeItem);
            }
            else if(stype.contains("DWORD", Qt::CaseInsensitive))
            {
                parentNode->CreateNode(sid.toLatin1().constData(), NodeType_DWord, offset.toUShort(), node_access);
                node = parentNode->GetNodeInterface(sid.toLatin1().constData());
                node->Description(sdesc.toUtf8().constData());
                //node->OnNodeValueChanged += event_handler(this, &Omc8000Model::OnValueChanged);
                nodeItem = new TreeItem(node, parentItem);

                //nodes
                setupNode(nodeElem, nodeItem, node);

                parentItem->appendChild(nodeItem);
            }
            else if(stype.contains("WORD", Qt::CaseInsensitive))
            {
                parentNode->CreateNode(sid.toLatin1().constData(), NodeType_Word, offset.toUShort(), node_access);
                node = parentNode->GetNodeInterface(sid.toLatin1().constData());
                node->Description(sdesc.toUtf8().constData());
                //node->OnNodeValueChanged += event_handler(this, &Omc8000Model::OnValueChanged);
                nodeItem = new TreeItem(node, parentItem);

                //nodes
                setupNode(nodeElem, nodeItem, node);

                parentItem->appendChild(nodeItem);
            }
            else if(stype.contains("BYTE", Qt::CaseInsensitive))
            {
                parentNode->CreateNode(sid.toLatin1().constData(), NodeType_Byte, offset.toUShort(), node_access);
                node = parentNode->GetNodeInterface(sid.toLatin1().constData());
                node->Description(sdesc.toUtf8().constData());
                //node->OnNodeValueChanged += event_handler(this, &Omc8000Model::OnValueChanged);
                nodeItem = new TreeItem(node, parentItem);

                //nodes
                setupNode(nodeElem, nodeItem, node);

                parentItem->appendChild(nodeItem);
            }
            if(stype.contains("BIT", Qt::CaseInsensitive) || stype.contains("BOOL", Qt::CaseInsensitive))
            {
                parentNode->CreateNode(sid.toLatin1().constData(), NodeType_Bit, offset.toUShort(), node_access);
                node = parentNode->GetNodeInterface(sid.toLatin1().constData());
                node->Description(sdesc.toUtf8().constData());
                //node->OnNodeValueChanged += event_handler(this, &Omc8000Model::OnValueChanged);
                nodeItem = new TreeItem(node, parentItem);

                //color
                QString scolor_on = nodeElem.attribute("color_on");
                if(!scolor_on.isNull() && !scolor_on.isEmpty())
                {
                    nodeItem->setcolorOn(Elm327ItemColor(scolor_on));
                }
                QString scolor_off = nodeElem.attribute("color_off");
                if(!scolor_off.isNull() && !scolor_off.isEmpty())
                {
                    nodeItem->setcolorOff(Elm327ItemColor(scolor_off));
                }

                //delay
                QString sdelayon = nodeElem.attribute("delay_on");
                QString sdelayoff = nodeElem.attribute("delay_off");
                if(!sdelayon.isNull() && !sdelayon.isEmpty() && !sdelayoff.isNull() && !sdelayoff.isEmpty())
                {
                    node->DelayTimer(sdelayon.toULong(), sdelayoff.toULong());
                }

                //nodes
                setupNode(nodeElem, nodeItem, node);

                parentItem->appendChild(nodeItem);
            }
        }
        nodeNode = nodeNode.nextSibling();
    }
}

void Elm327Model::setupModelData(const QString xmlFile, TreeItem *parent)
{
    ELM327Task *task;
    INode *taskInterface;
    INode *area;
    TreeItem* taskItem;
    TreeItem* areaItem;

    QDomDocument doc(xmlFile);
    QFile file(xmlFile);
    if (!file.open(QIODevice::ReadOnly))
        return;
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement docElem = doc.documentElement();
    QDomNode taskNode = docElem.firstChild();
    //tasks
    while(!taskNode.isNull())
    {
        //task
        QDomElement taskElem = taskNode.toElement();
        if(!taskElem.isNull())
        {
            QString sid = taskElem.attribute("id");
            QString sip = taskElem.attribute("ip");
            QString st = taskElem.attribute("loop_time");
            QString sdesc = taskElem.attribute("desc");

            bool ip_task = (sip!=nullptr && !sip.isEmpty());
            if(ip_task)
            {
                //elm327Lib->AddTask(sid.toLatin1().data(), sip.toLatin1().constData(), st.toUInt());
            }
            else //serial
            {
                 QString scom = taskElem.attribute("com");
                 QString sbaudrate = taskElem.attribute("baud_rate");
                 elm327Lib->AddTask(sid.toLatin1().data(), scom.toLatin1().data(), (SerialProtocol::BaudRate)sbaudrate.toUInt(0, 10) , st.toUInt());
            }
            task = elm327Lib->GetTask(sid.toLatin1().constData());
            taskInterface = static_cast<INode*>(task);
            if(!sdesc.isNull() && !sdesc.isEmpty())
                taskInterface->Description(sdesc.toUtf8().data());
            taskItem = new TreeItem(taskInterface, parent);

            //check default areas
            vector<INode*> areaList = task->GetNodes();
            for(UINT32 i = 0; i<areaList.size(); i++)
            {
                area = areaList[i];
                area->OnValueChanged += event_handler(this, &Elm327Model::OnValueChanged);
                areaItem = new TreeItem(area, taskItem);

                //search for subnodes
                setupDefaultNode(areaItem, area);

                taskItem->appendChild(areaItem);
            }

            //areas
            QDomNode areaNode = taskElem.firstChild();
            while(!areaNode.isNull())
            {
                //area
                QDomElement areaElem = areaNode.toElement();
                if(!areaElem.isNull())
                {
                    QString sid = areaElem.attribute("id");
                    QString scount = areaElem.attribute("count");
                    QString access = areaElem.attribute("access");
                    QString sdevid = areaElem.attribute("com_dev");
                    QString sdesc = areaElem.attribute("desc");

                    //default RW
                    bool isR = true;
                    bool isW = true;
                    if(!access.isNull() && !access.isEmpty())
                    {
                        isR = (access.indexOf(QChar('R'))>=0) || (access.indexOf(QChar('r'))>=0);
                        isW = (access.indexOf(QChar('W'))>=0) || (access.indexOf(QChar('w'))>=0);
                    }
                    NodeAccess node_access = (NodeAccess)((isR? NodeAccess_Read:0)|(isW? NodeAccess_Write:0));

                    //serial dev id
                    UINT8 devid = 255;
                    if(!sdevid.isNull() && !sdevid.isEmpty())
                    {
                        devid = sdevid.toUInt();
                    }

                    {
                        task->AddArea(sid.toLatin1().constData(), scount.toUShort(), node_access);
                        area = task->GetNodeInterface(sid.toLatin1().data());
                        area->Description(sdesc.toUtf8().constData());
                        area->OnValueChanged += event_handler(this, &Elm327Model::OnValueChanged);
                        areaItem = new TreeItem(area, taskItem);

                        //nodes
                        setupNode(areaElem, areaItem, area);

                        taskItem->appendChild(areaItem);
                    }
                }

                areaNode = areaNode.nextSibling();
            }

            parent->appendChild(taskItem);
        }
        taskNode = taskNode.nextSibling();
    }
}

void Elm327Model::StartTask()
{
    try
    {
        elm327Lib->Start(true);
    }
    catch(Utilities::INodeException& e)
    {
        QMessageBox::warning(QApplication::allWidgets().at(0), "Warning", QString(e.what()));
    }
}

void Elm327Model::StopTask()
{
    try
    {
        elm327Lib->Stop();
    }
    catch(Utilities::INodeException& e)
    {
        QMessageBox::warning(QApplication::allWidgets().at(0), "Warning", QString(e.what()));
    }
}
