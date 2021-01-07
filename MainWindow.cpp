#include <QSettings>
#include <QSignalMapper>
#include <QFileDialog>
#include <QMessageBox>
#include <QBitArray>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_AboutDialog.h"
#include "TreeItem.h"

#define WINDOWS_TITLE "Tracer"

MainWindow::MainWindow(QString xmlFile, QWidget *parent) :
    MainWindow(parent)
{
    openFile(xmlFile);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    treeView(nullptr)
{
    ui->setupUi(this);

    initializeTreeView();
    initializeRecentFiles();

    readSettings();
}

MainWindow::~MainWindow()
{
    if(treeView!=nullptr)
        treeView->setToolBar(nullptr);

    writeSettings();

    delete ui;
}

void MainWindow::aboutDialog()
{
   QDialog *info = new QDialog(this, nullptr);
   Ui_aboutDialog aboutUi;
   aboutUi.setupUi(info);
   info->show();
}

void MainWindow::initializeTreeView()
{
    //app icon
    setWindowIcon(QIcon(":/tracer.ico"));
    setWindowTitle(WINDOWS_TITLE);
    treeView = new TreeView(this);

    //tree
    setCentralWidget(treeView);
    treeView->show();
    treeView->setToolBar(ui->toolBar);

    //toolbar
    QAction* runAct = ui->toolBar->addAction(QIcon(":/images/start.ico"), QString("Start"), treeView, SLOT(StartTask()));
    runAct->setEnabled(false);

    QAction* stopAct = ui->toolBar->addAction(QIcon(":/images/close.ico"), QString("Stop"), treeView, SLOT(StopTask()));
    stopAct->setEnabled(false);

    ui->toolBar->addSeparator();

    QAction* taskAct = ui->toolBar->addAction(QIcon(":/images/task.ico"), QString("Add Task"), treeView, SLOT(AddTask()));
    taskAct->setEnabled(false);

    //treeview column view
    for(int i = 0; i< TreeUIColumnSize; i++)
    {
        QSignalMapper* signalMapper = new QSignalMapper (treeView) ;
        QAction* act = ui->menuView->addAction(QString(TreeUIColumnStr[i]), signalMapper, SLOT(map()));

        signalMapper->setMapping(act, i);
        connect (signalMapper, SIGNAL(mapped(int)), treeView, SLOT(ViewColumn(int))) ;

        act->setCheckable(true);
        act->setChecked(true);
    }

    //About dialog
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(aboutDialog()));

    //Tools
    createGraphWindows();
    createTraceWindows();
}

void MainWindow::initializeRecentFiles()
{
    //recent files
    recentFileActs[0] = ui->actionFile1;
    recentFileActs[1] = ui->actionFile2;
    recentFileActs[2] = ui->actionFile3;
    recentFileActs[3] = ui->actionFile4;
    recentFileActs[4] = ui->actionFile5;

    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

    separatorAct = ui->menuFile->actions().at(2);
    setWindowFilePath(QString());
    updateRecentFileActions();
}

void MainWindow::openFile(QString filePath)
{
    try
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        if (!filePath.isNull() && !filePath.isEmpty())
        {
            if(treeView)
                treeView->LoadConfig(filePath);

            setCurrentFile(filePath);
            setWindowTitle(QString(WINDOWS_TITLE) + QString(" (") + filePath + QString(")"));
            enableColumnMenu();
        }
    }
    catch(INodeException& e)
    {
        QApplication::restoreOverrideCursor();

        QString msg;
        msg.append(e.id());
        msg.append(": ");
        msg.append(e.what());
        msg.append("\r\n");
        msg.append(e.fnc());
        QMessageBox::warning(this, "Warning", msg);
    }
    QApplication::restoreOverrideCursor();
}

void MainWindow::on_actionOpen_triggered()
{
        QString filePath = QFileDialog::getOpenFileName(this,
                                                      tr("Open File"),
                                                      QDir::currentPath(),
                                                      tr("Setting files (*.xml)"));

        openFile(filePath);
}

void MainWindow::on_actionClose_triggered()
{
    try
    {
        if(treeView)
            treeView->CloseConfig();

        ui->toolBar->actions().at(EToolBarItem_StartTask)->setEnabled(false);
        ui->toolBar->actions().at(EToolBarItem_StopTask)->setEnabled(false);
        ui->toolBar->actions().at(EToolBarItem_AddTask)->setEnabled(false);

        setWindowTitle(WINDOWS_TITLE);
        enableColumnMenu(false);
    }
    catch(INodeException& e)
    {
        QMessageBox::warning(this, "Warning", QString(e.what()));
    }
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        openFile(action->data().toString());
    }
}


void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowFilePath(curFile);

    QSettings settings("QT", "TracerUI");
    QStringList files = settings.value("RecentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

void MainWindow::updateRecentFileActions()
{
     QSettings settings("QT", "TracerUI");
     QStringList files = settings.value("RecentFileList").toStringList();

     int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

     for (int i = 0; i < numRecentFiles; ++i) {
         QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
         recentFileActs[i]->setText(text);
         recentFileActs[i]->setData(files[i]);
         recentFileActs[i]->setVisible(true);
     }
     for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
         recentFileActs[j]->setVisible(false);

     separatorAct->setVisible(numRecentFiles > 0);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::writeSettings()
{
    QSettings settings("QT", "TracerUI");

    settings.beginGroup("MainWindow");

    settings.setValue("size", size());
    settings.setValue("pos", pos());

    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("QT", "TracerUI");

    settings.beginGroup("MainWindow");

    resize(settings.value("size", QSize(800, 600)).toSize());
    move(settings.value("pos", QPoint(100, 100)).toPoint());

    settings.endGroup();

    //treeview column view
    for(int i = 0; i< ui->menuView->actions().count(); i++)
    {
        //disable
        ui->menuView->actions().at(i)->setEnabled(false);
        ui->menuView->actions().at(i)->setChecked(false);
    }
}

void MainWindow::enableColumnMenu(bool enable)
{
    if(enable)
    {
        QSettings settings("QT", "TracerUI");

        settings.beginGroup("TreeView");

        QString s("");
        for(int i = 0; i<TreeUIColumnSize; i++)
        {
            s.sprintf("ColumnVisible%02d", i);
            bool visible = settings.value(s, true).toBool();
            if(i<ui->menuView->actions().count())
            {
                ui->menuView->actions().at(i)->setEnabled(true);
                ui->menuView->actions().at(i)->setChecked(visible);
            }
        }
        settings.endGroup();
    }
    else
    {
        for(int i = 0; i< ui->menuView->actions().count(); i++)
        {
            //disable
            ui->menuView->actions().at(i)->setEnabled(false);
            ui->menuView->actions().at(i)->setChecked(false);
        }
    }
}


