#ifndef TAB1CAMERA_H
#define TAB1CAMERA_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QImage>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QBuffer>
#include <QLabel>
#include <opencv2/opencv.hpp>

namespace Ui {
class Tab1Camera;
}

class Tab1Camera : public QWidget
{
    Q_OBJECT

public:
    explicit Tab1Camera(QWidget *parent = nullptr);
    ~Tab1Camera();

    cv::Mat cam1_image;
    cv::Mat cam2_image;
    QTcpSocket* getClientSocket() const; // 소켓 접근 함수

private:
    Ui::Tab1Camera *ui;
    QTcpServer *server;
    QTcpServer *appServer;
    QList<QTcpSocket *> clients;
    QTcpSocket *appClient = nullptr;
    QMap<QTcpSocket*, QLabel*> clientLabelMap;
    void processFrame(QTcpSocket*, cv::Mat&);
    void sendFrame(const QByteArray &, int);

private slots:
    void slotNewConnection();
    void slotAppNewConnection();
    void slotReadData();
    void slotClientDisconnected();
    void slotAppDisconnected();
    void slotCopyCamImage(cv::Mat&, int, bool&);
};

#endif // TAB2CAMERA_H
