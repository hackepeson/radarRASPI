#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
#define NO_OF_GRAPHS 10
    ui->setupUi(this);


    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = (x[i]*x[i]); // let's plot a quadratic function
    }

    for (int i = 0; i < NO_OF_GRAPHS; i++)
    {
        ui->customPlot->addGraph();
        ui->customPlot->graph(i)->setData(x,y);
    }

    connect(ui->pushButton, SIGNAL(clicked(bool)), SLOT(updateGraph()));
    ui->customPlot->rescaleAxes();
    ui->customPlot->replot();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::updateGraph()
{
    static double scale = 0.1;

    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = scale * (x[i]*x[i]); // let's plot a quadratic function
    }

    for (int i = (ui->customPlot->graphCount()-1); i > 0; i--)
    {
        ui->customPlot->graph(i)->setData(ui->customPlot->graph(i-1)->data());
    }
    ui->customPlot->graph(0)->setData(x,y);
    scale+= 1.1;

    ui->customPlot->rescaleAxes();
    ui->customPlot->replot();
}
