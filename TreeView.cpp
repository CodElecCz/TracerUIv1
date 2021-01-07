#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include "MainWindow.h"
#include "TreeView.h"
#include "ui_TreeView.h"
#include "NodeDialog.h"
#include "SpinboxDelegate.h"
#include "LineEditDelegate.h"

#include "ModelModbus.h"
#include "ModelOMC8000.h"
#include "ModelELM327.h"

TreeView::TreeView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TreeView)
{
    ui->setupUi(this);  
}

TreeView::~TreeView()
{
    StopTask();
    CloseConfig();

    delete ui;
}

void TreeView::writeSettings()
{
    QSettings settings("QT", "TracerUI");
    settings.beginGroup("TreeView");

    for(int i = 0; i<TreeUIColumnSize; i++)
    {
        settings.setValue(QString("Column%1").arg(i, 2, 10, QLatin1Char('0')), ui->tree->columnWidth(i));
        settings.setValue(QString("ColumnVisible%1").arg(i, 2, 10, QLatin1Char('0')), !ui->tree->isColumnHidden(i));
    }

    settings.endGroup();
}

void TreeView::readSettings()
{
    const int def[TreeUIColumnSize] = {200,100,50,50,80,80,150,400};    
    QSettings settings("QT", "TracerUI");

    settings.beginGroup("TreeView");

    for(int i = 0; i<TreeUIColumnSize; i++)
    {        
        int val = settings.value(QString("Column%1").arg(i, 2, 10, QLatin1Char('0')), def[i]).toInt();
        ui->tree->setColumnWidth(i, val);

        bool visible = settings.value(QString("ColumnVisible%1").arg(i, 2, 10, QLatin1Char('0')), true).toBool();
        ui->tree->setColumnHidden(i, !visible);
    }
    settings.endGroup();

    ui->tree->setItemDelegateForColumn(TreeUIColumn_Write, new LineEditDelegate());
}

EModelType TreeView::readConfig(QString fileName)
{
    EModelType model = EModelType_None;

    QDomDocument doc(fileName);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return EModelType_None;
    if (!doc.setContent(&file)) {
        file.close();
        return EModelType_None;
    }
    file.close();

    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement docElem = doc.documentElement();
    QString sname = docElem.nodeName();

    if(sname.contains("modbus", Qt::CaseInsensitive))
    {
        model = EModelType_ModBus;
    }
    else if(sname.contains("omc8000", Qt::CaseInsensitive))
    {
        model = EModelType_OMC8000;
    }
    else if(sname.contains("elm327", Qt::CaseInsensitive))
    {
        model = EModelType_ELM327;
    }

    return model;
}

void TreeView::LoadConfig(QString fileName)
{
    if(!fileName.isNull() && !fileName.isEmpty())
    {
        EModelType model = EModelType_None;

        CloseConfig();

        //read file to select model
        model = readConfig(fileName);

        //load model
        switch(model)
        {
            case EModelType_ModBus:
                {
                ModBusModel* model = new ModBusModel(fileName, this);
                ui->tree->setModel(model);
                connect(this, SIGNAL(startTask()), model, SLOT(StartTask()));
                connect(this, SIGNAL(stopTask()), model, SLOT(StopTask()));
                }
                break;
            case EModelType_OMC8000:
                {
                Omc8000Model* model = new Omc8000Model(fileName, this);
                ui->tree->setModel(model);
                connect(this, SIGNAL(startTask()), model, SLOT(StartTask()));
                connect(this, SIGNAL(stopTask()), model, SLOT(StopTask()));
                }
                break;
            case EModelType_ELM327:
                {
                Elm327Model* model = new Elm327Model(fileName, this);
                ui->tree->setModel(model);
                connect(this, SIGNAL(startTask()), model, SLOT(StartTask()));
                connect(this, SIGNAL(stopTask()), model, SLOT(StopTask()));
                }
                break;
            default:
                QMessageBox::warning(this, "Warning", QString("Configuration file root node should be type:\r\n") +
                                                      QString("modbus/omc8000/elm327..."));
                return;
        }

        ui->tree->collapseAll();

        readSettings();

        if(toolBar!=nullptr)
        {
            toolBar->actions().at(EToolBarItem_StartTask)->setEnabled(true);
            toolBar->actions().at(EToolBarItem_StopTask)->setEnabled(false);
            toolBar->actions().at(EToolBarItem_AddTask)->setEnabled(true);
        }
    }
}

void TreeView::CloseConfig()
{
    StopTask();

    QAbstractItemModel* model = ui->tree->model();
    if(model!=nullptr)
    {
        writeSettings();

        ui->tree->setModel(nullptr);
        delete model;

        ui->tree->reset();
    }
}

void TreeView::on_tree_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu(this);

    TreeItem *item = static_cast<TreeItem*>(ui->tree->indexAt(pos).internalPointer());
    if(item!=nullptr)
    {
        INode *node = item->node();
        if(node!=nullptr)
        {
            switch(node->Type())
            {
            case NodeType_Bit:
            case NodeType_Byte:
            case NodeType_Word:
            case NodeType_DWord:
            case NodeType_Real32:
            case NodeType_UInt64:
            case NodeType_Real64:
                menu.addAction(QString("Add Child Node"), this, SLOT(AddNode()));
                menu.addAction(QString("Remove Node"), this, SLOT(RemoveNode()));
                menu.exec(QCursor::pos());
                break;
            case NodeType_UInt64Array:
            case NodeType_DWordArray:
            case NodeType_WordArray:
            case NodeType_ByteArray:
            case NodeType_BitArray:
                menu.addAction(QString("Add Child Node"), this, SLOT(AddNode()));
                menu.addAction(QString("Remove Area"), this, SLOT(RemoveArea()));
                menu.exec(QCursor::pos());
                break;
            case NodeType_Task:
                menu.addAction(QString("Add Area"), this, SLOT(AddArea()));
                menu.addAction(QString("Remove Task"), this, SLOT(RemoveTask()));
                menu.exec(QCursor::pos());
                break;
            default:
                throw exception("TracerUI: Unknown node type.");
            }
        }
    }
}

void TreeView::on_tree_clicked(const QModelIndex &index)
{
    try
    {
        int column = index.column();
        switch(column)
        {
            case TreeUIColumn_Write:
                {
                    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
                    if(item!=nullptr)
                    {
                        INode *node = item->node();
                        if(node!=nullptr)
                        {
                            bool val;

                            switch(node->Type())
                            {
                                case NodeType_Bit:
                                    node->Write(&val);
                                    node->Write(!val);
                                    break;
                            }
                        }
                    }
                }
                break;
        }
    }
    catch(INodeException& e)
    {
        QMessageBox::warning(this, "Warning", QString(e.what()));
    }
}

void TreeView::on_tree_doubleClicked(const QModelIndex &index)
{

}

void TreeView::AddNode()
{
    try
    {
        NodeDialog dialog(EModelType_ModBus, this);
        int res = dialog.exec();
        if(res==QDialog::Accepted)
        {
            QModelIndex index = ui->tree->selectionModel()->currentIndex();
            QAbstractItemModel *model = ui->tree->model();

            TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
            if(item!=nullptr)
            {
                INode *node = item->node();
                if(node!=nullptr)
                {
                    INode* subnode;

                    node->CreateNode(dialog.nodeData.id.toLatin1().data(), (NodeType)dialog.nodeData.type, dialog.nodeData.offset);
                    subnode = node->GetNodeInterface(dialog.nodeData.id.toLatin1().data());

                    int row = 0;
                    if (!model->insertRow(row, index))
                        return;

                    QModelIndex child = model->index(row, 0, index);
                    model->setData(child, VPtr<INode>::asQVariant(subnode));
                }
            }
        }
    }
    catch(INodeException& e)
    {
        QMessageBox::warning(this, "Warning", QString(e.what()));
    }
}

void TreeView::RemoveNode()
{

}

void TreeView::AddArea()
{

}

void TreeView::RemoveArea()
{

}

void TreeView::AddTask()
{

}

void TreeView::RemoveTask()
{

}

void TreeView::StartTask()
{
    emit startTask();
    if(toolBar!=nullptr)
    {
        toolBar->actions().at(EToolBarItem_StartTask)->setEnabled(false);
        toolBar->actions().at(EToolBarItem_StopTask)->setEnabled(true);
        toolBar->actions().at(EToolBarItem_AddTask)->setEnabled(false);
    }
}

void TreeView::StopTask()
{
    emit stopTask();
    if(toolBar!=nullptr)
    {
        toolBar->actions().at(EToolBarItem_StartTask)->setEnabled(true);
        toolBar->actions().at(EToolBarItem_StopTask)->setEnabled(false);
        toolBar->actions().at(EToolBarItem_AddTask)->setEnabled(true);
    }
}

void TreeView::ViewColumn(int column)
{
    bool isHidden = ui->tree->isColumnHidden(column);
    ui->tree->setColumnHidden(column, !isHidden);
    if(isHidden)
    {
        int width = ui->tree->columnWidth(column);
        if(width<10)
            ui->tree->setColumnWidth(column, 100);
    }
}

void TreeView::hideColumn(int column, bool hide)
{
    if(!hide)
    {
        int width = ui->tree->columnWidth(column);
        if(width<10)
            ui->tree->setColumnWidth(column, 100);
    }
    ui->tree->setColumnHidden(column, hide);
}
