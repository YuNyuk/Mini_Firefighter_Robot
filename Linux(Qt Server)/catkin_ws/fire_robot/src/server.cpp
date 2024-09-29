#include "../include/fire_robot/server.h"
#include <QDebug>

Server::Server(QObject *parent)
    : QTcpServer(parent)
{
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);

    if (socket->setSocketDescriptor(socketDescriptor)) {
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
        emit newConnect(socket);
        //socket->write("Welcome to the server!\n");
		qDebug() << "New client connected from:" << socket->peerAddress().toString();
    } else {
        delete socket;
    }
}
