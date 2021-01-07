#include <QDockWidget>
#include <QtCharts/QLineSeries>
#include <QtCore/QDateTime>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include "MainWindow.h"
#include "ui_MainWindow.h"

void MainWindow::createGraphWindows()
{
    QDockWidget *dock = new QDockWidget(tr("Graph"), this);
    dock->setAllowedAreas(Qt::BottomDockWidgetArea);

    QLineSeries *series = new QLineSeries();

    chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setTitle("Meassurement Data");

    QValueAxis *axisX = new QValueAxis;
    axisX->setLabelFormat("%i");
    axisX->setTitleText("Samples");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%i");
    axisY->setTitleText("Value [dec]");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    dock->setWidget(chartView);
    dock->resize(600,300);

    addDockWidget(Qt::BottomDockWidgetArea, dock);
    ui->menuTools->addAction(dock->toggleViewAction());

    //resize Qt buck
    dock->setFloating(true);
    dock->hide();
    dock->setFloating(false);
}

void MainWindow::chartGraphData(QVector<uint16_t> data)
{
    chart->removeAllSeries();

    QLineSeries *series = new QLineSeries();

    uint16_t min = 0xffff;
    uint16_t max = 0;

    for(int i = 0; i< data.size(); i++)
    {
        uint16_t val = data.at(i);

        if(val<min) min = val;
        if(val>max) max = val;

        series->append(i, val);
    }

    chart->addSeries(series);
    chart->axisX()->setRange(0,series->count());
    chart->axisY()->setRange(min, max);
}
