#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QNetworkInterface>


void processEventQueueSleep(int msec)
{
  QEventLoop loop;
  QTimer::singleShot(msec,&loop,SLOT(quit()));
  loop.exec();
}


MainWindow::MainWindow(QWidget *parent) :
  m_ownIPAddr("127.0.0.1"),
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
#define NO_OF_GRAPHS 10
  ui->setupUi(this);

  foreach (const QNetworkInterface &netInterface, QNetworkInterface::allInterfaces())
  {
      QNetworkInterface::InterfaceFlags flags = netInterface.flags();
      if( (bool)(flags & QNetworkInterface::IsRunning) && !(bool)(flags & QNetworkInterface::IsLoopBack))
      {
          foreach (const QNetworkAddressEntry &address, netInterface.addressEntries())
          {
              if(address.ip().protocol() == QAbstractSocket::IPv4Protocol)
              {
                m_ownIPAddr = address.ip().toString();
                  qDebug() << address.ip().toString();
              }
          }
      }
  }
  setWindowTitle(m_ownIPAddr);


  ui->customPlot->addGraph();

  m_pUDPSocket = new QUdpSocket(this);
  //m_pUDPSocket->bind(QHostAddress::LocalHost, 8888);
  QHostAddress hAddr("192.168.0.105");
  m_pUDPSocket->bind(hAddr,8888);
  //m_pUDPSocket->bind(8888);

  connect(m_pUDPSocket, SIGNAL(readyRead()), SLOT(readUDPSocket()));
  connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
  connect(ui->actionIP, SIGNAL(triggered(bool)), SLOT(enterIPAddr()));



}

MainWindow::~MainWindow()
{
  delete ui;
}



void MainWindow::updateGraph()
{
#define VECTOR_LENGTH 2048
  QVector<double> data;
  QVector<double> x;
  data.clear();
  x.clear();



  for (int i = 0; i < VECTOR_LENGTH; i+=2)
  {
    uint8_t low = m_vecData.at(i);
    uint8_t high = m_vecData.at(i+1);

    double dd =256*high+low;

    data.append(dd);
    x.append(i);
  }

  ui->customPlot->graph(0)->setData(x,data);
  ui->customPlot->rescaleAxes();
  ui->customPlot->yAxis->setRange(0,10000);
  ui->customPlot->replot();
}




void MainWindow::readUDPSocket()
{
  qDebug() << "In readUDP";


  m_vecData.resize(m_pUDPSocket->pendingDatagramSize());

  QHostAddress sender;
  quint16 senderPort;

  m_pUDPSocket->readDatagram(m_vecData.data(), m_vecData.size(), &sender, &senderPort);

  updateGraph();
  //qCritical() << m_vecData.length();

}


void MainWindow::enterIPAddr()
{
  QLineEdit* ipEdit = new QLineEdit();
  ipEdit->setInputMask("000.000.000.000;_");

  ipEdit->show();
  while (ipEdit->isVisible())
  {
    processEventQueueSleep(100);
  }

  qDebug() << ipEdit->text();

}
