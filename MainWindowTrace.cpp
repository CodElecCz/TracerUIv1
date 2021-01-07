#include <QDockWidget>
#include <QTextBrowser>
#include "MainWindow.h"
#include "ui_MainWindow.h"

void MainWindow::createTraceWindows()
{
    QDockWidget *dock = new QDockWidget(tr("Log"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

    QTextBrowser* browser = new QTextBrowser(this);

    dock->setWidget(browser);
    dock->resize(600,300);

    addDockWidget(Qt::BottomDockWidgetArea, dock);
    ui->menuTools->addAction(dock->toggleViewAction());

    //resize Qt buck
    dock->setFloating(true);
    dock->hide();
    dock->setFloating(false);
}
