#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    server = new QTcpServer(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_start_server_clicked(bool checked)
{
    if(checked)
    {
        if (!server->listen(QHostAddress::AnyIPv4, static_cast<quint16>(ui->input_port->value())))
        {
            ui->server_log->appendPlainText("ОШИБКА");
        }
        else {
            ui->server_log->appendPlainText("ГОТОВ");
            ui->start_server->setText("РАБОТАЕТ");
            connect(server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
        }
    }
    else {
        server->close();
        ui->start_server->setText("СТАРТ");
        ui->server_log->appendPlainText("НЕ ГОТОВ");
    }
}

void Widget::slotNewConnection()
{
    socket_new = server->nextPendingConnection();
    ui->server_log->appendPlainText("Новое подключение");

    connect(socket_new, SIGNAL(readyRead()), this, SLOT(slotServerRead()));
    connect(socket_new, SIGNAL(disconnected()), socket_new, SLOT(slotDisconnected()));
}

void Widget::slotServerRead()
{
    /*----Получение данных----*/
    QDataStream data_in(socket_new);
    data_in.setByteOrder(QDataStream::BigEndian);
    data_in >> blockSize;
    ui->server_log->appendPlainText("Получено сообщение размером: " + QString::number(blockSize));

    QVector<quint8> my_get_data (blockSize);
    QString out_data;
    for (int i = 0; i < blockSize; i++) {
        data_in >> my_get_data[i];
        out_data += QString::number(my_get_data[i]) + " ";
        qDebug() << my_get_data[i];
    }
    ui->server_log->appendPlainText("Данные: " + out_data);

    /*----Отправка данных----*/
    QByteArray back_message;
    QDataStream send_back(&back_message, QIODevice::WriteOnly);

    send_back << quint16(0);
    for (int i = 0; i < blockSize; i++){
    send_back << my_get_data[i];
    }
    send_back << quint8(48);
    send_back.device()->seek(0);
    send_back << quint16(back_message.size() - sizeof(quint16));
    socket_new -> write(back_message);
    qDebug() << back_message;

    ui->server_log->appendPlainText("Отправлено сообщение размером: " + QString::number(blockSize+1));

    /*----Обнуление----*/
    my_get_data.clear();
    blockSize = 0;
}



void Widget::slotConnectToServer()
{
    /*----Парсим IP/Port----*/
    QString in_ip = ui->input_ip->text();
    qDebug()<<in_ip;

    int i = 0;
    int j = 0;

    do
    {
        server_ip[i] = in_ip[i];
        i++;
    }
    while(in_ip[i]!=":");

    i++;

    do
    {
        server_port[j] = in_ip[i];
        i++;
        j++;
    }
    while(i < in_ip.length());

    server_port_int = static_cast<quint16>(server_port.toInt());

    /*----Подключаемся----*/
    QTcpSocket *socket = new QTcpSocket();
    socket->connectToHost(server_ip, server_port_int);
    ui->server_log->appendPlainText("Подключение к: " + server_ip + (":") + server_port);

    /*----Разбиваем IP----*/
    QString my_ip = MyIP();
    qDebug() << my_ip;
    QStringList server_ip_list = my_ip.split(".");
    quint8 server_ip_0 = static_cast<quint8>(server_ip_list[0].toInt());
    quint8 server_ip_1 = static_cast<quint8>(server_ip_list[1].toInt());
    quint8 server_ip_2 = static_cast<quint8>(server_ip_list[2].toInt());
    quint8 server_ip_3 = static_cast<quint8>(server_ip_list[3].toInt());

    /*----Формируем посылку----*/
    quint16 my_port = static_cast<quint16>(ui->input_port->value());
    QByteArray pocket;
    QDataStream send_ip(&pocket, QIODevice::WriteOnly);
    send_ip.setByteOrder(QDataStream::BigEndian);
    send_ip << quint16(0)
            << quint16(my_port)
            << quint8(server_ip_0)
            << quint8(server_ip_1)
            << quint8(server_ip_2)
            << quint8(server_ip_3);
    send_ip.device()->seek(0);
    send_ip << quint16(pocket.size() - sizeof(quint16));
    socket->write(pocket);
    qDebug() << pocket;
    ui->server_log->appendPlainText("Отправлено");

    socket->disconnected();

}

void Widget::on_send_ip_clicked()
{
    slotConnectToServer();
}

/*----Получение своего IP----*/
QString Widget::MyIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();

    for(int i=0; i<list.count(); i++)
    {
        if(!list[i].isLoopback())
            if (list[i].protocol() == QAbstractSocket::IPv4Protocol )
                return(list[i].toString());
    }
}

void Widget::slotDisconnected()
{
    socket_new->close();
}

