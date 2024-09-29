#ifndef TAB2ROSCONTROL_H
#define TAB2ROSCONTROL_H

#include <QWidget>
#include <QTcpSocket>
#include <QTcpServer>
#include <QButtonGroup>
#include <QPushButton>
#include <QProcess>
#include <iostream>
#include <array>
#include <opencv2/opencv.hpp>
#include "rosnode.h"
#include "server.h"
#include "tab1camera.h"


namespace Ui {
class Tab2RosControl;
}

class Tab2RosControl : public QWidget
{
    Q_OBJECT

public:
    explicit Tab2RosControl(int argc, char **argv, Tab1Camera *tab1Camera, QWidget *parent = nullptr);
    ~Tab2RosControl();

private:
    Ui::Tab2RosControl *ui;
    RosNode *prosNode;
    Server *server;
    QTcpSocket *clientSocket = nullptr;
    char msg = 0;
    Tab1Camera *m_tab1Camera;
    cv::Mat rosImg;
    int connectionCount = 0;
    QTcpServer *imgServer;
    QTcpSocket *imgClientSocket = nullptr;
    QTcpServer *buzzerServer;
    QTcpSocket *buzzerClientSocket = nullptr;

private slots:
    void slotLdsReceive(float *);
    void sendData();
    void sendBuzzerOff();
    void sendGoalMessage(QString msg);
    void saveSocket(QTcpSocket*);
    void slotReadData();
    void slotRosReadData();
    QByteArray matToQByteArray(const cv::Mat&);
    void sendImageViaTcp(QTcpSocket*, const cv::Mat&);
    void sendImageInChunks(QTcpSocket*, const cv::Mat&);
    void slotNewImageConnection();
    void slotBuzzerConnection();
    void sendBuzzerOn(QString);

signals:
    void signalRequestRosImage(cv::Mat&);
    void sigFireFinish();
};

#endif // TAB2ROSCONTROL_H
