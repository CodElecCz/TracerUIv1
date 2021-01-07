#ifndef NODEDIALOG_H
#define NODEDIALOG_H

#include <QDialog>
#include <QString>
#include "TreeView.h"
#include "TreeItem.h"

namespace Ui {
class NodeDialog;
}

typedef struct SNodeDialogData
{
    EModelType  modelType;
    NodeType    type;
    QString     id;
    uint16_t    offset;
} NodeDialogData;

class NodeDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit NodeDialog(EModelType type = EModelType_ModBus, QWidget *parent = nullptr);
    ~NodeDialog();
    
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

public:
    NodeDialogData nodeData;

private:
    Ui::NodeDialog *ui;
};

#endif // NODEDIALOG_H
