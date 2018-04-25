#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QUdpSocket* m_pUDPSocket;
    QByteArray m_vecData;


private slots:
  void updateGraph();
  void readUDPSocket();
  void enterIPAddr();

};

#endif // MAINWINDOW_H
