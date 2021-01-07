#ifndef OMC8000MODEL_H
#define OMC8000MODEL_H

#include <QAbstractItemModel>
#include <QtXml/QDomDocument>
#include <QTimer>
#include "TreeItem.h"
#include "OMC8000Lib/OMC8000Lib.h"

class Omc8000Model : public QAbstractItemModel
{
    Q_OBJECT

public:
    Omc8000Model(const QString xmlFile, QObject *parent = nullptr);
    ~Omc8000Model();

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
    void setupDefaultNode(TreeItem* parentItem, INode *parentNode);
    void setupNode(QDomElement parentElem, TreeItem* parentItem, INode* parentNode);
    TreeItem *getItem(const QModelIndex &index) const;
    void OnValueChanged(INode* node, UINT64 val);
    void updateNode(QString itemName, QModelIndex parentIndex);

private:
    TreeItem    *rootItem;
    OMC8000Lib  *omc8000Lib;
};

#endif // OMC8000MODEL_H
