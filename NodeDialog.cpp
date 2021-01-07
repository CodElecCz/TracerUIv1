#include "NodeDialog.h"
#include "Lib/ModbusLib/ModBusLib.h"
#include "Lib/OMC8000Lib/OMC8000Lib.h"

#include "ui_NodeDialog.h"

NodeDialog::NodeDialog(EModelType type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeDialog)
{
    ui->setupUi(this);

    switch(type)
    {
    case EModelType_ModBus:
        ui->type->addItem(QString("BOOL"));
        ui->type->addItem(QString("BYTE"));
        ui->type->addItem(QString("WORD"));
        break;
    case EModelType_OMC8000:
        ui->type->addItem(QString("BOOL"));
        ui->type->addItem(QString("BYTE"));
        ui->type->addItem(QString("WORD"));
        ui->type->addItem(QString("DWORD"));
        ui->type->addItem(QString("REAL32"));
        break;
    }
}

NodeDialog::~NodeDialog()
{
    delete ui;
}

void NodeDialog::on_buttonBox_accepted()
{
    nodeData.id.clear();
    nodeData.id.append(ui->id->text());
    switch(ui->type->currentIndex())
    {
    case 0:
        nodeData.type = NodeType_Bit;
        break;
    case 1:
        nodeData.type = NodeType_Byte;
        break;
    case 2:
        nodeData.type = NodeType_Word;
        break;
    case 3:
        nodeData.type = NodeType_DWord;
        break;
    case 4:
        nodeData.type = NodeType_Real32;
        break;
    }
    nodeData.offset = ui->offset->value();

    this->setResult(1);
}

void NodeDialog::on_buttonBox_rejected()
{
    this->setResult(0);
}
