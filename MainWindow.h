#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>

QT_CHARTS_USE_NAMESPACE

#include "TreeView.h"

namespace Ui {
class MainWindow;
}

enum EToolBarItems
{
    EToolBarItem_StartTask = 0,
    EToolBarItem_StopTask,
    EToolBarItem_Separator,
    EToolBarItem_AddTask
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    explicit MainWindow(QString xmlFile, QWidget *parent = nullptr);
    ~MainWindow();
    
private:
    void initializeTreeView();
    void initializeRecentFiles();

    void openFile(QString file);
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

    //aplication settings
    void writeSettings();
    void readSettings();
    void enableColumnMenu(bool enable = true);

    void createGraphWindows();
    void createTraceWindows();

public slots:
    void chartGraphData(QVector<uint16_t> data);

private slots:
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void openRecentFile();
    void aboutDialog();

private:
    Ui::MainWindow  *ui;
    TreeView        *treeView;
    QChart          *chart;

    //open files history
    enum { MaxRecentFiles = 5 };
    QString         curFile;
    QAction         *separatorAct;
    QAction         *recentFileActs[MaxRecentFiles];
};

#endif // MAINWINDOW_H
