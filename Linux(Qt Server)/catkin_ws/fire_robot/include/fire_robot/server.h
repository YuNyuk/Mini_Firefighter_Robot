#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QObject>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);

signals:
    void newConnect(QTcpSocket* socket);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // SERVER_H

