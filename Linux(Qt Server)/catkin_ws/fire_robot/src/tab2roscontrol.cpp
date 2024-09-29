#include "../include/fire_robot/tab2roscontrol.h"
#include "ui_tab2roscontrol.h"
//#include "../include/fire_robot/socketclient.h"

Tab2RosControl::Tab2RosControl(int argc, char **argv, Tab1Camera *tab1Camera, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab2RosControl),
    m_tab1Camera(tab1Camera)
{
    ui->setupUi(this);
    prosNode = new RosNode(argc, argv);
    prosNode->pLcamView = ui->pLcamView;
    prosNode->start();

    // Server setup
    // 잘못된 IP에 연결될 경우, 세그멘테이션 오류(코어 덤프)가 발생할 수 있기에 IP지정함 -> Port 번호 변경으로 해결.
    // 터틀봇 서버
    server = new Server(this);
    if (!server->listen(QHostAddress::Any, 5001)) {   // ROS IP : 192.168.100.108
        qCritical() << "Sever - tab2 - 5001번 포트 에러 : " << server->errorString();
    } else {
        qDebug() << "Sever - tab2 - 5001번 포트 연결";
    }
    // ROS Cam 이미지 전송하기 위한 서버
    imgServer = new QTcpServer(this);
    if (!imgServer->listen(QHostAddress::Any, 5010)) {
        qCritical() << "Sever - tab2 - 5010번 포트 에러 : " << imgServer->errorString();
    } else {
        qDebug() << "Sever - tab2 - 5010번 포트 연결";
    }
    // 화재 알림 장치 데이터 전송하기 위한 서버
    buzzerServer = new QTcpServer(this);
    if (!buzzerServer->listen(QHostAddress::Any, 5011)) {
        qCritical() << "Sever - tab2 - 5011번 포트 에러 : " << buzzerServer->errorString();
    } else {
        qDebug() << "Sever - tab2 - 5011번 포트 연결";
    }

    // 버튼 클릭 시, 하드 코딩된 좌표 값을 터틀봇에 전달
    connect(ui->sendBtn, SIGNAL(clicked()),this, SLOT(sendData()));
    connect(prosNode, SIGNAL(sigLdsReceive(float *)),this, SLOT(slotLdsReceive(float *)));
    connect(prosNode, SIGNAL(rosShutdown()),qApp, SLOT(quit()));

    // Ros 이미지 전달을 위한 이미지 캡처
    connect(this, SIGNAL(signalRequestRosImage(cv::Mat&)), prosNode, SLOT(slotCopyRosImage(cv::Mat&)));
    // 새로 들어온 터틀봇 소켓 저장
    connect(server, SIGNAL(newConnect(QTcpSocket*)), this, SLOT(saveSocket(QTcpSocket*)));
    // Ros 이미지 서버의 소켓 저장
    connect(imgServer, SIGNAL(newConnection()), this, SLOT(slotNewImageConnection()));
    // Ros 이미지 서버의 소켓 저장
    connect(buzzerServer, SIGNAL(newConnection()), this, SLOT(slotBuzzerConnection()));
}

Tab2RosControl::~Tab2RosControl()
{
    // 활성화가 되어 있다면 닫기
    if (buzzerClientSocket && buzzerClientSocket->isOpen()) {
        buzzerClientSocket->disconnectFromHost();
        buzzerClientSocket->close();
    }

    if (imgClientSocket && imgClientSocket->isOpen()) {
        imgClientSocket->disconnectFromHost();
        imgClientSocket->close();
    }

    if (clientSocket && clientSocket->isOpen()) {
        clientSocket->disconnectFromHost();
        clientSocket->close();
    }

    // 할당된 자원을 해제합니다.
    delete prosNode;
    delete server;
    delete imgServer;
    delete buzzerServer;
    delete ui;
}
// Ros 스캔 데이터 UI에 표시
void Tab2RosControl::slotLdsReceive(float *pscanData)
{
    ui->lcdNumber1->display(pscanData[0]);
    ui->lcdNumber2->display(pscanData[1]);
    ui->lcdNumber3->display(pscanData[2]);
    ui->lcdNumber4->display(pscanData[3]);
}

// 화재 감지 시그널 발생 시에 작동하는 부저 ON 메시지 전달 함수(매개변수는 따로 사용하지 않음)
void Tab2RosControl::sendBuzzerOn(QString temp)
{
    qDebug() << "Server - tab2 - sendBuzzerOn";

    QString message = "buzzer_on";

    // 소켓이 존재하고 열려 있는지 확인
    if (!buzzerClientSocket) {
        qCritical() << "Server - tab2 - sendBuzzerOn - 소켓 없음";
        return;
    }

    if (!buzzerClientSocket->isOpen()) {
        qCritical() << "Server - tab2 - sendBuzzerOn - 오픈 에러";
        return;
    }

    buzzerClientSocket->write(message.toUtf8());

    qDebug() << "Server - tab2 - sendBuzzerOn - message : " << message;

    /*
    // Client IP 주소와 포트 번호 정보
    QString clientInfo = QString("IP: %1, Port: %2")
        .arg(clientSocket->peerAddress().toString())
        .arg(clientSocket->peerPort());

    qDebug() << "Message sent to client:" << message << "->" << clientInfo;
    */
}
// 터틀봇 복귀 명령 시에 작동하는 부저 OFF 메시지 전달 함수
void Tab2RosControl::sendBuzzerOff()
{
    qDebug() << "Server - tab2 - sendBuzzerOff";

    QString message = "buzzer_off";

    // 소켓이 존재하고 열려 있는지 확인
    if (!buzzerClientSocket) {
        qCritical() << "Server - tab2 - sendBuzzerOff - 소켓 없음";
        return;
    }

    if (!buzzerClientSocket->isOpen()) {
        qCritical() << "Server - tab2 - sendBuzzerOff - 오픈 에러";
        return;
    }

    buzzerClientSocket->write(message.toUtf8());

    qDebug() << "Server - tab2 - sendBuzzerOff - message : " << message;
}
// 터틀봇에 하드 코딩된 좌표 전달
void Tab2RosControl::sendData()
{
    // 소켓이 존재하고 열려 있는지 확인
    if (!clientSocket) {
        qCritical() << "Server - tab2 - sendData - 소켓 없음";
        return;
    }

    if (!clientSocket->isOpen()) {
        qCritical() << "Server - tab2 - sendData - 오픈 에러";
        return;
    }

    QString message = "[18]GOAL@-1.2@-5.4";
    clientSocket->write(message.toUtf8());

    qDebug() << "Server - tab2 - sendData - message : " << message;
}
// 화재 감시 시에, 터틀봇에게 해당 좌표값 전달
void Tab2RosControl::sendGoalMessage(QString msg)
{
    if (clientSocket && clientSocket->isOpen())
    {
        qDebug() << "Goal message : " << msg;
        clientSocket->write(msg.toUtf8());
        QString clientInfo = QString("IP: %1, Port: %2")
            .arg(clientSocket->peerAddress().toString())
            .arg(clientSocket->peerPort());
    }
    else
    {
        qDebug() << "Message sent to client: failed";
        qCritical() << "No client connected or socket not open!";
    }
}
// 새로운 연결 시에, 소켓 저장 및 응답 대기
// 하나의 공간에서 서버 관리 할 때 유용할 것 같았지만, 현재는 필요할 때만 소켓 생성.
void Tab2RosControl::saveSocket(QTcpSocket* socket)
{
    clientSocket = socket;
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(slotRosReadData()));
}
// cv::Mat png 형식으로 인코딩 -> 바이트 배열로 변환
QByteArray Tab2RosControl::matToQByteArray(const cv::Mat& image) {
    cv::Mat rgbImage;
    std::vector<uchar> buffer;
    cv::cvtColor(image, rgbImage, cv::COLOR_BGR2RGB); // BRG -> RGB 변형
    cv::imencode(".jpg", rgbImage, buffer); // JPG 형식으로 인코딩
    return QByteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}
// **현재 사용하지 않음** -> 이미지 인코딩 후, 데이터 전송
void Tab2RosControl::sendImageViaTcp(QTcpSocket *socket, const cv::Mat &image) {
    if (!socket || !socket->isOpen()) {
        qCritical() << "Socket is not available or not open.";
        return;
    }

    // 이미지 인코딩
    std::vector<uchar> buffer;
    if (!cv::imencode(".png", image, buffer)) {
        qCritical() << "Failed to encode image.";
        return;
    }

    // 이미지 크기와 데이터를 준비
    quint32 imageSize = buffer.size();
    QByteArray sizeArray;
    sizeArray.setNum(imageSize);

    // 이미지 크기 및 이미지 데이터 전송
    socket->write(sizeArray);
    socket->write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    socket->flush();

    qDebug() << "Image sent with size:" << imageSize << "bytes.";
}
// **현재 사용하지 않음** -> 이미지 인코딩 후, 데이터를 나누어서 전송
void Tab2RosControl::sendImageInChunks(QTcpSocket *socket, const cv::Mat &image) {
    if (!socket || !socket->isOpen())
    {
        qCritical() << "Socket is not available or not open.";
        return;
    }

    // 1. 이미지 인코딩
    std::vector<uchar> buffer;
    if (!cv::imencode(".png", image, buffer))
    {
        qCritical() << "Failed to encode image.";
        return;
    }

    // 2. 청크 크기를 계산합니다.
    quint32 totalSize = buffer.size();
    int chunkCount = 3;
    quint32 chunkSize = std::ceil(static_cast<float>(totalSize) / chunkCount);

    // 3. 각 청크로 데이터를 나누어 보냅니다.
    for (int i = 0; i < chunkCount; ++i){
        quint32 startIdx = i * chunkSize;
        quint32 endIdx = std::min(startIdx + chunkSize, totalSize);
        QByteArray chunkData = QByteArray::fromRawData(reinterpret_cast<const char*>(buffer.data() + startIdx), endIdx - startIdx);

        socket->write(chunkData);
        socket->flush(); // 적절히 전송 완료를 보장하기 위해 flush 사용

        qDebug() << "Chunk" << i + 1 << "sent, size:" << chunkData.size() << "bytes.";
    }

    qDebug() << "Total image sent with size:" << totalSize << "bytes.";
}
// ROS 이미지 소켓 저장
void Tab2RosControl::slotNewImageConnection()
{
    imgClientSocket = imgServer->nextPendingConnection();

    connect(imgClientSocket, SIGNAL(readyRead()), this, SLOT(slotReadData()));
    connect(imgClientSocket, SIGNAL(disconnected()), imgClientSocket, SLOT(deleteLater()));

    qDebug() << "이미지 소켓 연결 완료" << imgClientSocket->peerAddress().toString();
}
// 화재 알림 장치 소켓 저장
void Tab2RosControl::slotBuzzerConnection()
{
    buzzerClientSocket = buzzerServer->nextPendingConnection();

    connect(buzzerClientSocket, SIGNAL(readyRead()), this, SLOT(slotReadData()));
    connect(buzzerClientSocket, SIGNAL(disconnected()), buzzerClientSocket, SLOT(deleteLater()));

    qDebug() << "부저 소켓 연결 완료" << buzzerClientSocket->peerAddress().toString();
}
// 화재 진압 후 복귀 명령 시, 시그널 발생 및 부저 OFF
void Tab2RosControl::slotRosReadData()
{
    // 소켓이 존재하는지 확인
    if (!clientSocket)
    {
        qCritical() << "Server - tab2 - slotRosReadData - 소켓 없음";
        return;
    }

    // 데이터 읽기
    QByteArray data = clientSocket->readAll();
    qDebug() << "받은 데이터 : " << data;
    if (data.trimmed() == "Situation Over!")
    {
        emit sigFireFinish();
        sendBuzzerOff();
    }
}
// ROS 이미지 전송 슬롯
void Tab2RosControl::slotReadData()
{
    // 소켓이 존재하고 열려 있는지 확인
    if (!imgClientSocket)
    {
        qCritical() << "Server - tab2 - slotReadData - 소켓 없음";
        return;
    }

    if (!imgClientSocket->isOpen())
    {
        qCritical() << "Server - tab2 - slotReadData - 소켓 없음";
        return;
    }
    // 데이터 읽기
    QByteArray data = imgClientSocket->readAll();

    if (data.trimmed() == "IMG")
    {
        // 해당 시그널 발생 시 -> rosImg에 이미지 1 프레임 복사함
        emit signalRequestRosImage(rosImg);
        // 복사해온 이미지 데이터 변환
        QByteArray imgData = matToQByteArray(rosImg);

        // 이미지 바이너리 전송
        imgClientSocket->write(imgData);
        imgClientSocket->flush();

        // 사이즈 확인(잘 전달되었는지)
        qDebug() << "Image sent with size:" << imgData.size() << "bytes.";
        qDebug() << "Server - tab2 - slotReadData - 전달 완료";

        // 데이터 전송 완료 후 소켓 연결 해제
        imgClientSocket->disconnectFromHost();

        if (imgClientSocket->state() == QAbstractSocket::UnconnectedState || imgClientSocket->waitForDisconnected(3000))
        {
            qDebug() << "Server - tab2 - slotReadData - 소켓 해제 완료";
        } else
        {
            qDebug() << "Server - tab2 - slotReadData - 소켓 해제 실패";
        }
    }
}

