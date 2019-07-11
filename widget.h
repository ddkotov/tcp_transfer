#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_start_server_clicked(bool checked);
    void slotNewConnection();
    void slotServerRead();
    void slotConnectToServer();
    void slotDisconnected();
    void on_send_ip_clicked();
    QString MyIP();

private:
    Ui::Widget *ui;
    QTcpServer *server;
    QTcpSocket *socket_new = new QTcpSocket();
    quint16 blockSize = 0;
    QString server_ip, server_port;
    quint16 server_port_int;
};

#endif // WIDGET_H
