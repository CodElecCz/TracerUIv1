#ifndef MODELMODBUS_H
#define MODELMODBUS_H

#include <QAbstractItemModel>
#include <QtXml/QDomDocument>
#include <QTimer>
#include "TreeItem.h"
#include "ModBusLib/ModBusLib.h"

class ModBusModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ModBusModel(const QString xmlFile, QObject *parent = nullptr);
    ~ModBusModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    //editable
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                      int role = Qt::EditRole);
    bool insertRows(int position, int rows,
                 const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows,
                 const QModelIndex &parent = QModelIndex());

    void loadModelData(const QString xmlFile);

public slots:
    void StartTask();
    void StopTask();

private slots:
    void update(QString itemName);
    void update();

private:
    void setupModelData(const QString xmlFile, TreeItem *parent);
    void setupDefaultNode(TreeItem* parentItem, INode* parentNode);
    void setupNode(QDomElement parentElem, TreeItem* parentItem, INode* parentNode);
    TreeItem *getItem(const QModelIndex &index) const;
    void updateNode(QString itemName, QModelIndex parentIndex);

    void OnValueChanged(INode* node, UINT64 val);
    void OnErrorChanged(INode* node);

private:
    TreeItem    *rootItem;
    ModBusLib   *modBusLib;
};

#endif // MODELMODBUS_H
