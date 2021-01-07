#include <QIcon>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include "TreeItem.h"

TreeItem::TreeItem(INode *node, TreeItem *parent)
    : stateColorOn(TreeItemStateColor_LightGreen),
      stateColorOff(TreeItemStateColor_Gray),
      lastCounter(0),
      lastCounterTick(0),
      lastCounterVal(0.0)
{
   parentItem = parent;

   if(parent==nullptr)
   {
       //set header
       for(int i = 0; i<TreeUIColumnSize; i++)
       {
           itemData << TreeUIColumnStr[i];
       }
   }
   else
   {
       //set no data
       for(int i = 0; i<TreeUIColumnSize; i++)
       {
           itemData << QString("No Data");
       }
   }

   treeNode = node;
}

QString TreeItem::GetNodeValue(void) const
{
    switch(treeNode->Type())
    {
    case NodeType_Bit:
        {
        bool val;
        treeNode->Read(&val);
        return QString("%1").arg(val);
        }
        break;
    case NodeType_Byte:
        {
        UINT8 val;
        treeNode->Read(&val);
        return QString("0x%1").arg(val, 2, 16, QLatin1Char('0'));
        }
        break;
    case NodeType_Word:
        {
        UINT16 val;
        treeNode->Read(&val);
        return QString("0x%1").arg(val, 4, 16, QLatin1Char('0'));
        }
        break;
    case NodeType_Int16:
        {
        INT16 val;
        treeNode->Read(&val);
        return QString("%1").arg(val, 0, 10);
        }
        break;
    case NodeType_DWord:
        {
        UINT32 val;
        treeNode->Read(&val);        
        return QString("%1").arg(val, 8, 16, QLatin1Char('0'));
        }
        break;
    case NodeType_Real32:
        {
        float val;
        treeNode->Read(&val);
        return QString::number(val, 'f', 2);
        }
        break;
    case NodeType_UInt64:
        {
        UINT64 val;
        treeNode->Read(&val);        
        return QString("%1").arg(val, 16, 16, QLatin1Char('0'));
        }
        break;
    case NodeType_Real64:
        {
        double val;
        treeNode->Read(&val);
        return QString::number(val, 'f', 2);
        }
        break;
    default:
        break;
    }

    return "";
}

QString TreeItem::GetNodeWriteValue(void) const
{
    switch(treeNode->Type())
    {
    case NodeType_Bit:
        {
        bool val;
        treeNode->Write(&val);
        return QString("%1").arg(val);
        }
        break;
    case NodeType_Byte:
        {
        UINT8 val;
        treeNode->Write(&val);
        return QString("0x%1").arg(val, 2, 16, QLatin1Char('0'));
        }
        break;
    case NodeType_Word:
        {
        UINT16 val;
        treeNode->Write(&val);
        return QString("0x%1").arg(val, 4, 16, QLatin1Char('0'));
        }
        break;
    case NodeType_Int16:
        {
        INT16 val;
        treeNode->Write(&val);
        return QString("%1").arg(val, 0, 10);
        }
        break;
    case NodeType_DWord:
        {
        UINT32 val;
        treeNode->Write(&val);
        return QString("%1").arg(val, 8, 16, QLatin1Char('0'));
        }
        break;
    case NodeType_Real32:
        {
        float val;
        treeNode->Write(&val);
        return QString::number(val, 'f', 2);
        }
        break;
    case NodeType_UInt64:
        {
        UINT64 val;
        treeNode->Write(&val);
        return QString("%1").arg(val, 16, 16, QLatin1Char('0'));
        }
        break;
    case NodeType_Real64:
        {
        double val;
        treeNode->Write(&val);
        return QString::number(val, 'f', 2);
        }
        break;
    default:
        break;
    }

    return "";
}

void TreeItem::SetNodeValue(double val)
{
    switch(treeNode->Type())
    {
    case NodeType_Real32:
        {
        float v = (float)val;
        treeNode->Write(v);
        }
        break;
    case NodeType_Real64:
        treeNode->Write(val);
        break;
    }

}

void TreeItem::SetNodeValue(UINT64 val)
{
    UINT64_UT Val;
    Val.uint64 = val;

    switch(treeNode->Type())
    {
    case NodeType_Bit:
        {
        bool v = Val.bit.b0;
        treeNode->Write(v);
        }
        break;
    case NodeType_Byte:
        {
        UINT8 b = Val.byte.byte0;
        treeNode->Write(b);
        }
        break;
    case NodeType_Word:
        {
        UINT16 w = Val.word.word0;
        treeNode->Write(w);
        }
        break;
    case NodeType_Int16:
        {
        //convert from hex to dec
        UINT16 w = Val.word.word0;
        treeNode->Write(w);
        }
        break;
    case NodeType_DWord:
        {
        UINT32 w = Val.dword.dword0;
        treeNode->Write(w);
        }
        break;
    case NodeType_UInt64:
        {
        UINT64 w = Val.uint64;
        treeNode->Write(w);
        }
        break;
    default:
        break;
    }
}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem *child)
{
    childItems.append(child);
}

TreeItem *TreeItem::child(int row)
{
    return childItems.value(row);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::columnCount() const
{
    return itemData.count();
}

QVariant TreeItem::data(int column) const
{
    QString svar("");
    SYSTEMTIME t;

    try
    {

    if(treeNode!=nullptr)
    {
        switch(column)
        {
        case TreeUIColumn_Id:
            return QString(treeNode->Id());
        case TreeUIColumn_Type:
            return QString(treeNode->TypeStr());
        case TreeUIColumn_Read:
            if(treeNode->Access()&NodeAccess_Read)
            {
                switch(treeNode->Type())
                {
                case NodeType_Bit:
                    break;
                default:
                    return GetNodeValue();
                }
            }
            break;
        case TreeUIColumn_Write:
            if(treeNode->Access()&NodeAccess_Write)
            {
                return GetNodeWriteValue();
            }
            break;
        case TreeUIColumn_Changes:
            switch(treeNode->Type())
            {
            case NodeType_Bit:
            case NodeType_Byte:
            case NodeType_Word:
            case NodeType_DWord:
            case NodeType_UInt64:
            case NodeType_Real32:
            case NodeType_Real64:
                return QString("%1").arg(treeNode->GetChangesCnt());
            }
            break;
        case TreeUIColumn_Counter:
            switch(treeNode->Type())
            {
            //case NodeType_Bit:
            //case NodeType_Byte:
            case NodeType_Word:
            case NodeType_DWord:
            case NodeType_UInt64:
            //case NodeType_Real32:
            //case NodeType_Real64:
                return QString("%1").arg(treeNode->ReadCounter());
            }
            break;
        case TreeUIColumn_UpdateRate:
            switch(treeNode->Type())
            {
            //case NodeType_Bit:
            //case NodeType_Byte:
            case NodeType_Word:
            case NodeType_DWord:
            case NodeType_UInt64:
            //case NodeType_Real32:
            //case NodeType_Real64:                            
                return QString::number(treeNode->GetNodeReadRate(), 'f', 2);
            }
            break;
        case TreeUIColumn_ReadTimeStamp:
            if(treeNode->Access()&NodeAccess_Read)
            {
                switch(treeNode->Type())
                {
                case NodeType_BitArray:
                case NodeType_ByteArray:
                case NodeType_WordArray:
                case NodeType_DWordArray:
                case NodeType_UInt64Array:
                case NodeType_UInt64:
                    t = treeNode->ReadTimeStamp();                    
                    return QString("%1:%2:%3.%4").arg(t.wHour, 2, 10, QLatin1Char('0'))
                                                .arg(t.wMinute, 2, 10, QLatin1Char('0'))
                                                .arg(t.wSecond, 2, 10, QLatin1Char('0'))
                                                .arg(t.wMilliseconds, 2, 10, QLatin1Char('0'));
                }
            }
            break;
        case TreeUIColumn_WriteTimeStamp:
            if(treeNode->Access()&NodeAccess_Write)
            {
                switch(treeNode->Type())
                {
                case NodeType_Bit:
                case NodeType_Byte:
                case NodeType_Word:
                case NodeType_DWord:
                case NodeType_UInt64:
                case NodeType_Real32:
                case NodeType_Real64:
                    t = treeNode->WriteTimeStamp();
                    return QString("%1:%2:%3.%4").arg(t.wHour, 2, 10, QLatin1Char('0'))
                                                .arg(t.wMinute, 2, 10, QLatin1Char('0'))
                                                .arg(t.wSecond, 2, 10, QLatin1Char('0'))
                                                .arg(t.wMilliseconds, 2, 10, QLatin1Char('0'));
                }
            }
            break;
        case TreeUIColumn_Description:
            return QString(treeNode->Description());
            break;
        case TreeUIColumn_Exception:
            {
            INodeException* e = treeNode->GetLastException();
            if(e!=nullptr)
                return QString("[%1] %2; function: %3; file: %4; line: %5").arg(e->id(), e->what(), e->fnc(), e->file(), QString::number(e->line()));
            }
            break;
        default:
            break;
        }
    }
    else
        return itemData.value(column);

    }
    catch(Utilities::INodeException& e)
    {
        //QMessageBox::warning(QApplication::allWidgets().at(0), "Warning", QString(e.what()));
        qDebug() << "INodeException: " + QString(e.what());
    }

    return svar;
}

int TreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

TreeItem *TreeItem::parent()
{
    return parentItem;
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
             return false;

    for (int row = 0; row < count; ++row)
    {
         TreeItem *item = new TreeItem(nullptr, this);
         childItems.insert(position, item);
    }

    return true;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
    {
        const char* id = childItems.at(position)->treeNode->Id();
        delete childItems.takeAt(position);
        treeNode->RemoveNode(id);
    }

    return true;
}

int TreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
             return false;

    itemData[column] = value;

    if(treeNode!=nullptr)
    {
        switch(column)
        {
        case TreeUIColumn_Write:
            bool ok;
            QString sval = value.toString();

            switch(treeNode->Type())
            {
                case NodeType_Real32:
                case NodeType_Real64:
                    {
                        double val = sval.toDouble(&ok);
                        if(ok)
                            SetNodeValue(val);
                    }
                    break;
                case NodeType_Int16:
                case NodeType_Int32:
                case NodeType_Int64:
                    {
                        UINT64 val = static_cast<UINT64>(sval.toLongLong(&ok, 10));
                        if(ok)
                            SetNodeValue(val);
                    }
                    break;
                default:
                    {
                        UINT64 val = static_cast<UINT64>(sval.toLongLong(&ok, 16));
                        if(ok)
                            SetNodeValue(val);
                    }
                    break;
            }

            break;
        }
    }
    return true;
}

bool TreeItem::setData(const QVariant &value)
{
    treeNode = VPtr<INode>::asPtr(value);
    return true;
}

QVariant TreeItem::icon() const
{
    if(treeNode!=nullptr)
    {
        switch(treeNode->Type())
        {
        case NodeType_Bit:
            return QIcon(":/images/Letter-B-black.ico");
        case NodeType_Byte:
            return QIcon(":/images/Letter-B-black.ico");
        case NodeType_Word:
            return QIcon(":/images/Letter-W-black.ico");
        case NodeType_DWord:
        case NodeType_Real32:
            return QIcon(":/images/Letter-D-black.ico");
        case NodeType_UInt64:
        case NodeType_Real64:
            return QIcon(":/images/Letter-U-black.ico");
        case NodeType_Task:
            return QIcon(":/images/task.ico");
        case NodeType_WordArray:
        case NodeType_BitArray:
        case NodeType_DWordArray:
            //return QIcon(":/images/Letter-A-black.ico");
            return QIcon(":/images/list.png");
        default:
            ;
        }
    }

    return QVariant();
}

QVariant TreeItem::iconState() const
{
    if(treeNode!=nullptr)
    {
        switch(treeNode->Type())
        {
        case NodeType_Bit:
            bool bval;
            treeNode->Read(&bval);
            if(!bval)
            {
                switch(stateColorOff)
                {
                case TreeItemStateColor_Black:
                    return QIcon(":/images/O/black.ico");
                case TreeItemStateColor_Blue:
                    return QIcon(":/images/O/blue.ico");
                case TreeItemStateColor_DarkGreen:
                    return QIcon(":/images/O/dg.ico");
                case TreeItemStateColor_Gold:
                    return QIcon(":/images/O/gold.ico");
                case TreeItemStateColor_Gray:
                    return QIcon(":/images/O/grey.ico");
                case TreeItemStateColor_LightGreen:
                    return QIcon(":/images/O/lg.ico");
                case TreeItemStateColor_Orange:
                    return QIcon(":/images/O/orange.ico");
                case TreeItemStateColor_Pink:
                    return QIcon(":/images/O/pink.ico");
                case TreeItemStateColor_Red:
                    return QIcon(":/images/O/red.ico");
                case TreeItemStateColor_Violet:
                    return QIcon(":/images/O/violet.ico");
                default:
                    return QIcon(":/images/O/grey.ico");
                }
            }
            else
            {
                switch(stateColorOn)
                {
                case TreeItemStateColor_Black:
                    return QIcon(":/images/I/black.ico");
                case TreeItemStateColor_Blue:
                    return QIcon(":/images/I/blue.ico");
                case TreeItemStateColor_DarkGreen:
                    return QIcon(":/images/I/dg.ico");
                case TreeItemStateColor_Gold:
                    return QIcon(":/images/I/gold.ico");
                case TreeItemStateColor_Gray:
                    return QIcon(":/images/I/grey.ico");
                case TreeItemStateColor_LightGreen:
                    return QIcon(":/images/I/lg.ico");
                case TreeItemStateColor_Orange:
                    return QIcon(":/images/I/orange.ico");
                case TreeItemStateColor_Pink:
                    return QIcon(":/images/I/pink.ico");
                case TreeItemStateColor_Red:
                    return QIcon(":/images/I/red.ico");
                case TreeItemStateColor_Violet:
                    return QIcon(":/images/I/violet.ico");
                default:
                    return QIcon(":/images/I/grey.ico");
                }
            }
            break;
        default:
            break;
        }
    }

    return QVariant();
}
