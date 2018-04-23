#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
#define NO_OF_GRAPHS 10
    ui->setupUi(this);

    ui->customPlot->addGraph();

    connect(ui->pushButton, SIGNAL(clicked(bool)), SLOT(updateGraph()));


    m_pTCPSocket = new QTcpSocket(this);

    m_pTCPSocket->connectToHost("127.0.0.1", 11999);

    connect(m_pTCPSocket, SIGNAL(readyRead()), SLOT(readTCPSocket()));




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
      x[i] = (i/50.0 - 1);
      y[i] = scale*(x[i]*x[i]);

    }

    ui->customPlot->graph(0)->setData(x,y);

    scale+= 0.3;

    ui->customPlot->rescaleAxes();
    ui->customPlot->replot();
}


void MainWindow::readTCPSocket()
{
  QByteArray data = m_pTCPSocket->readAll();
  qDebug() << data.data();

}
