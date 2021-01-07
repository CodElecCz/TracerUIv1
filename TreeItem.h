#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QString>
#include "SharedNodesLib/INode.h"

using namespace Utilities::Node;

enum TreeUIColumn
{
    TreeUIColumn_Id = 0,
    TreeUIColumn_Type,
    TreeUIColumn_Read,
    TreeUIColumn_Write,
    TreeUIColumn_Changes,
    TreeUIColumn_Counter,
    TreeUIColumn_UpdateRate,
    TreeUIColumn_ReadTimeStamp,
    TreeUIColumn_WriteTimeStamp,
    TreeUIColumn_Description,
    TreeUIColumn_Exception,
    /* size */
    TreeUIColumnSize
};

const char* const TreeUIColumnStr[TreeUIColumnSize] =
{
    "Tree",
    "Node",
    "R",
    "W",
    "Changes",
    "Counter",
    "Rate",
    "Read Time",
    "Write Time",
    "Description",
    "Exception"
};

enum TreeItemStateColors
{
    TreeItemStateColor_Black = 0,
    TreeItemStateColor_Blue,
    TreeItemStateColor_DarkGreen,
    TreeItemStateColor_Gold,
    TreeItemStateColor_Gray,
    TreeItemStateColor_LightGreen,
    TreeItemStateColor_Orange,
    TreeItemStateColor_Pink,
    TreeItemStateColor_Red,
    TreeItemStateColor_Violet
};

template <class T> class VPtr
{
public:
    static T* asPtr(QVariant v)
    {
    return  (T *) v.value<void *>();
    }

    static QVariant asQVariant(T* ptr)
    {
    return qVariantFromValue((void *) ptr);
    }
};

class TreeItem
{
public:
    TreeItem(INode *data = 0, TreeItem *parent = nullptr);
    ~TreeItem();

    void appendChild(TreeItem *child);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parent();

    //editable functions
    bool insertChildren(int position, int count, int columns);
    bool removeChildren(int position, int count);
    int childNumber() const;
    bool setData(int column, const QVariant &value);
    bool setData(const QVariant &value);

    QVariant icon() const;
    QVariant iconState() const;
    void setcolorOn(TreeItemStateColors stateColorOn) { this->stateColorOn = stateColorOn;}
    void setcolorOff(TreeItemStateColors stateColorOff) { this->stateColorOff = stateColorOff;}

    INode* node() const { return treeNode; }

 private:
    QString GetNodeValue() const;
    QString GetNodeWriteValue() const;
    void SetNodeValue(UINT64 val);
    void SetNodeValue(double val);

 private:
     QList<TreeItem*>   childItems;     //child nodes
     QList<QVariant>    itemData;       //column
     TreeItem           *parentItem;    //parent
     INode              *treeNode;      //child param

     TreeItemStateColors stateColorOn;
     TreeItemStateColors stateColorOff;

     UINT64 lastCounter;
     UINT64 lastCounterTick;
     double lastCounterVal;
};

#endif // TREEITEM_H
