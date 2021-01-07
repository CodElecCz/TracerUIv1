#ifndef LINEEDITDELEGATE_H
#define LINEEDITDELEGATE_H

//#include <QItemDelegate>
#include <QStyledItemDelegate>

class LineEditDelegate : public QStyledItemDelegate//QItemDelegate
{
Q_OBJECT

public:
    LineEditDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                   const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // LINEEDITDELEGATE_H
