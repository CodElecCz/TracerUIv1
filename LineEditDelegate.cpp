#include <QLineEdit>
#include <QPainter>
#include "LineEditDelegate.h"

LineEditDelegate::LineEditDelegate(QObject *parent) :
   QStyledItemDelegate(parent)// QItemDelegate(parent)
{
}

QWidget *LineEditDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &/* option */,
     const QModelIndex &/* index */) const
{
    QLineEdit *editor = new QLineEdit(parent);

    QRegExp reg("[0-9A-Fa-f.x-]{1,16}");
    QRegExpValidator* validator = new QRegExpValidator(reg, editor);
    editor->setValidator(validator);

    return editor;
}

void LineEditDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    QString val = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(val);
}

void LineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    QString value = lineEdit->text();

    model->setData(index, value, Qt::EditRole);
}

void LineEditDelegate::updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
     editor->setGeometry(option.rect);
}
