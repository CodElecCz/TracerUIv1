#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QWidget>
#include <QToolBar>
#include <QModelIndex>

enum EModelType
{
    EModelType_None = 0,
    EModelType_ModBus,
    EModelType_OMC8000,
    EModelType_ELM327
};

namespace Ui {
class TreeView;
}

class TreeView : public QWidget
{
    Q_OBJECT
    
public:
    explicit TreeView(QWidget *parent = nullptr);
    ~TreeView();

public slots:
    void StartTask();
    void StopTask();

    void AddTask();
    void ViewColumn(int column);
    void LoadConfig(QString fileName);
    void CloseConfig();

private slots:
    void on_tree_customContextMenuRequested(const QPoint &pos);
    void on_tree_clicked(const QModelIndex &index);
    void on_tree_doubleClicked(const QModelIndex &index);

    void AddNode();
    void AddArea();
    void RemoveNode();
    void RemoveArea();
    void RemoveTask();

signals:
    void startTask();
    void stopTask();

public:
    void setToolBar(QToolBar *toolBar) { this->toolBar = toolBar; }
    void hideColumn(int column, bool hide = false);

private:
    void writeSettings();
    void readSettings();
    EModelType readConfig(QString fileName);

private:
    Ui::TreeView *ui;
    QToolBar *toolBar;
};

#endif // TREEVIEW_H
