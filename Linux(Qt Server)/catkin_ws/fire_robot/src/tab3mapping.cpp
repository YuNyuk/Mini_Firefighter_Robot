#include "../include/fire_robot/tab3mapping.h"
#include "ui_tab3mapping.h"

#include <QDebug>

Tab3Mapping::Tab3Mapping(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Tab3Mapping)
{
    ui->setupUi(this);

    // 좌표 목록
    point_list.append(&cam1_points);
    point_list.append(&map1_points);
    point_list.append(&cam2_points);
    point_list.append(&map2_points);
    point_list.append(&map_turtle1_points);

    // 터틀봇1 좌표
    turtle1_points.push_back(cv::Point2f(-0.84, -1.3));     // Top-Left
    turtle1_points.push_back(cv::Point2f(2.0, -1.18));      // Top-Right
    turtle1_points.push_back(cv::Point2f(2.0, -6.3));     // Bottom-Right
    turtle1_points.push_back(cv::Point2f(-0.96, -6.37));      // Bottom-Left

    map_image = cv::imread(map_path.toStdString());
    drawCorners();

    ui->labelCamView->installEventFilter(this);
    ui->labelMapView->installEventFilter(this);
    ui->labelInfoText->setText("준비");

    // 객체 탐지 프로그램 연결
    server = new QTcpServer(this);
    server->listen(QHostAddress::Any, 5020);
    connect(server, SIGNAL(newConnection()), this, SLOT(onClientConnect()));
    qDebug().noquote() << currentTime() << "YOLO client listen on port 5020";

    connect(ui->btnCamMapConnect, SIGNAL(clicked(bool)), this, SLOT(onBtnCamMapConnectClicked(bool)));
    connect(ui->btnMapAreaSet, SIGNAL(clicked(bool)), this, SLOT(onBtnMapAreaSetClicked(bool)));
    connect(ui->btnCam1Select, SIGNAL(clicked()), this, SLOT(onBtnCam1SelectClicked()));
    connect(ui->btnCam2Select, SIGNAL(clicked()), this, SLOT(onBtnCam2SelectClicked()));
    connect(ui->btnRestoreSetting, SIGNAL(clicked()), this, SLOT(onBtnRestoreSettingClicked()));
    connect(this, SIGNAL(signalMapEdited()), this, SLOT(onMapEdited()));
}

Tab3Mapping::~Tab3Mapping()
{
    delete ui;
}

/* 이벤트 처리 */
bool Tab3Mapping::eventFilter(QObject *watched, QEvent *event)
{
    if (cam_map_connect_event_flag)
    {
        return camMapConnectEvent(watched, event);
    }
    else if (map_area_set_event_flag)
    {
        return mapAreaSetEvent(watched, event);
    }
    else
    {
        // 마우스 우클릭 (For Test)
        if (event->type() == QMouseEvent::MouseButtonPress){
            QMouseEvent *m_event = (QMouseEvent*)event;
            if (m_event->button() == Qt::RightButton)
            {
                if (watched == ui->labelCamView)
                {
                    test_point = cv::Point2f(m_event->pos().x(), m_event->pos().y());
                    drawCorners();
                    emit signalTargetFound(convertPointToTurtle(test_point, current_cam));
                }
            }
        }

        return false;
    }
}

/* 카메라-지도 사이의 좌표 변환을 위한 영역을 설정 */
bool Tab3Mapping::camMapConnectEvent(QObject *watched, QEvent *event)
{
    // 마우스 클릭 이벤트
    if (event->type() == QMouseEvent::MouseButtonPress)
    {
        QMouseEvent *m_event = (QMouseEvent*)event;

        // 마우스 좌클릭
        if (m_event->button() == Qt::LeftButton)
        {
            // 1. 카메라 좌표 선택
            if (active_cam_click && watched == ui->labelCamView)
            {
                std::vector<cv::Point2f> *temp_points;
                if (current_cam == 1)
                    temp_points = &cam1_points;
                else
                    temp_points = &cam2_points;

                // 4개가 추가될 때까지 반복
                temp_points->push_back(cv::Point2f(m_event->pos().x(), m_event->pos().y()));
                if (temp_points->size() == 4)
                {
                    active_cam_click = false;
                    active_map_click = true;
                    ui->labelInfoText->setText("카메라 영역에 대응하는 지도 영역 선택");
                }
            }
            // 2. 지도 좌표 선택
            else if (active_map_click && watched == ui->labelMapView)
            {
                std::vector<cv::Point2f> *temp_points;
                if (current_cam == 1)
                    temp_points = &map1_points;
                else
                    temp_points = &map2_points;

                temp_points->push_back(cv::Point2f(m_event->pos().x(), m_event->pos().y()));
                if (temp_points->size() == 4) {
                    emit signalMapEdited();
                    ui->btnCamMapConnect->click();
                }
            }

            drawCorners();
        }
    }

    return false;
}

/* 지도-터틀봇 사이의 좌표 변환을 위한 영역을 설정 */
bool Tab3Mapping::mapAreaSetEvent(QObject *watched, QEvent *event)
{
    // 마우스 클릭 이벤트
    if (event->type() == QMouseEvent::MouseButtonPress)
    {
        QMouseEvent *m_event = (QMouseEvent*)event;

        // 마우스 좌클릭
        if (m_event->button() == Qt::LeftButton)
        {
            // 지도 좌표 선택
            if (active_map_click && watched == ui->labelMapView)
            {
                std::vector<cv::Point2f> *temp_points;
                temp_points = &map_turtle1_points;

                temp_points->push_back(cv::Point2f(m_event->pos().x(), m_event->pos().y()));
                if (temp_points->size() == 4) {
                    emit signalMapEdited();
                    ui->btnMapAreaSet->click();
                }
            }

            drawCorners();
        }
    }
    return false;
}

/* 이미지 갱신 */
void Tab3Mapping::updateImageView(cv::Mat &image, QLabel *label)
{
    if (image.empty())
    {
        return;
    }

    cv::Mat rgb_image;
    cv::cvtColor(image, rgb_image, cv::COLOR_BGR2RGB);

    QImage q_image(rgb_image.data, rgb_image.cols, rgb_image.rows, rgb_image.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(q_image);

    label->setFixedSize(pixmap.size());
    label->setPixmap(pixmap);
}

/* 선택한 영역을 이미지에 표시 */
void Tab3Mapping::drawCorners()
{
    cv::Mat temp_cam_image = cam_image.clone();
    cv::Mat temp_map_image = map_image.clone();
    std::vector<cv::Point2f> temp_cam_points;
    std::vector<cv::Point2f> temp_map_points;
    std::vector<cv::Point2f> temp_turtle_points;
    cv::Mat temp_transform_array;
    cv::Mat temp_turtle_transform_array;

    // select cam
    if (current_cam == 1)
    {
        temp_cam_points = cam1_points;
        temp_map_points = map1_points;
        temp_transform_array = cam_map1_transform_array;
    }
    else if (current_cam == 2)
    {
        temp_cam_points = cam2_points;
        temp_map_points = map2_points;
        temp_transform_array = cam_map2_transform_array;
    }
    else
    {
        return;
    }

    temp_turtle_points = map_turtle1_points;
    temp_turtle_transform_array = map_turtle1_transform_array;

    // draw circle
    for (int i = 0; i < (int)temp_cam_points.size(); ++i)
    {
        cv::circle(temp_cam_image, temp_cam_points[i], 2, cv::Scalar(0, 255, 0), 2);
    }
    for (int i = 0; i < (int)temp_map_points.size(); ++i)
    {
        cv::circle(temp_map_image, temp_map_points[i], 2, cv::Scalar(0, 255, 0), 2);
    }
    for (int i = 0; i < (int)map_turtle1_points.size(); ++i)
    {
        cv::circle(temp_map_image, map_turtle1_points[i], 2, cv::Scalar(255, 0, 255), 2);
    }

    // draw line
    if (temp_cam_points.size() == 4)
    {
        cv::line(temp_cam_image, temp_cam_points[0], temp_cam_points[1], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_cam_image, temp_cam_points[1], temp_cam_points[2], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_cam_image, temp_cam_points[2], temp_cam_points[3], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_cam_image, temp_cam_points[3], temp_cam_points[0], cv::Scalar(255, 0, 0), 1);
    }
    if (temp_map_points.size() == 4)
    {
        cv::line(temp_map_image, temp_map_points[0], temp_map_points[1], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_map_image, temp_map_points[1], temp_map_points[2], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_map_image, temp_map_points[2], temp_map_points[3], cv::Scalar(255, 0, 0), 1);
        cv::line(temp_map_image, temp_map_points[3], temp_map_points[0], cv::Scalar(255, 0, 0), 1);
    }
    if (temp_turtle_points.size() == 4)
    {
        cv::line(temp_map_image, temp_turtle_points[0], temp_turtle_points[1], cv::Scalar(0, 0, 255), 1);
        cv::line(temp_map_image, temp_turtle_points[1], temp_turtle_points[2], cv::Scalar(0, 0, 255), 1);
        cv::line(temp_map_image, temp_turtle_points[2], temp_turtle_points[3], cv::Scalar(0, 0, 255), 1);
        cv::line(temp_map_image, temp_turtle_points[3], temp_turtle_points[0], cv::Scalar(0, 0, 255), 1);
    }

    // draw target (For Test)
    if (isCamMapReady(current_cam) && temp_turtle_points.size() == 4)
    {
        std::vector<cv::Point2f> src, dst, test;
        src.push_back(test_point);

        cv::circle(temp_cam_image, test_point, 2, cv::Scalar(0, 0, 255), 2);
        cv::perspectiveTransform(src, dst ,temp_transform_array);
        cv::circle(temp_map_image, dst[0], 2, cv::Scalar(0, 0, 255), 2);
        cv::perspectiveTransform(dst, test, temp_turtle_transform_array);
    }

    // update images
    updateImageView(temp_cam_image, ui->labelCamView);
    updateImageView(temp_map_image, ui->labelMapView);
}

/* 변환 배열 구하기 */
void Tab3Mapping::getPerspectiveTransform()
{
    // cam1-map1
    if (cam1_points.size() == 4 && map1_points.size() == 4)
    {
        cam_map1_transform_array = cv::getPerspectiveTransform(cam1_points, map1_points);
    }
    // cam2-map2
    if (cam2_points.size() == 4 && map2_points.size() == 4)
    {
        cam_map2_transform_array = cv::getPerspectiveTransform(cam2_points, map2_points);
    }
    // map-turtle1
    if (map_turtle1_points.size() == 4)
    {
        map_turtle1_transform_array = cv::getPerspectiveTransform(map_turtle1_points, turtle1_points);
    }
}

/* 설정 읽기 */
bool Tab3Mapping::readConfig()
{
    // open file
    QFile file(config_path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->labelInfoText->setText("설정 파일이 존재하지 않습니다.");
        return false;
    }

    // read file
    QTextStream stream(&file);
    QList<QStringList> data;
    QString sep(",");

    while (stream.atEnd() == false)
    {
        QString line = stream.readLine();
        data << line.split(sep);
    }

    // parsing data
    int idx = 0;

    foreach (std::vector<cv::Point2f> *p, point_list)
    {
        QStringList str_list = data.at(idx);
        if (str_list.at(0).toInt() != -1)
        {
            p->push_back(cv::Point2f(str_list.at(0).toInt(), str_list.at(1).toInt()));
            p->push_back(cv::Point2f(str_list.at(2).toInt(), str_list.at(3).toInt()));
            p->push_back(cv::Point2f(str_list.at(4).toInt(), str_list.at(5).toInt()));
            p->push_back(cv::Point2f(str_list.at(6).toInt(), str_list.at(7).toInt()));
        }
        idx += 1;
    }

    if (isCamMapReady(1))
    {
        cam_map1_transform_array = cv::getPerspectiveTransform(cam1_points, map1_points);
        qDebug() << "CamMap1";
    }
    if (isCamMapReady(2))
    {
        cam_map2_transform_array = cv::getPerspectiveTransform(cam2_points, map2_points);
        qDebug() << "CamMap2";
    }
    if (map_turtle1_points.size() == 4)
    {
        map_turtle1_transform_array = cv::getPerspectiveTransform(map_turtle1_points, turtle1_points);
        qDebug() << "Turtle";
    }

    file.close();

    return true;
}

/* 설정 기록 */
void Tab3Mapping::writeConfig()
{
    // open file
    QFile file(config_path);
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    // write file
    QTextStream stream(&file);
    QString sep(",");

    foreach (std::vector<cv::Point2f> *p, point_list)
    {
        if (p->size() != 4)
        {
            stream << -1 << endl;
            continue;
        }

        for (int i = 0; i < 4; ++i)
        {
            stream << p->at(i).x << sep << p->at(i).y;
            if (i == 3)
                stream << endl;
            else
                stream << sep;
        }
    }

    stream.flush();
    file.close();
}

/* 카메라-지도 변환의 모서리가 설정 되었는지 여부 */
bool Tab3Mapping::isCamMapReady(int num)
{
    switch (num)
    {
    case 1:
        return (cam1_points.size() == 4) && (map1_points.size() == 4);
    case 2:
        return (cam2_points.size() == 4) && (map2_points.size() == 4);
    default:
        return false;
    }
}

/* 모든 설정이 완료되어 좌표 전송이 준비된 상태의 여부 */
bool Tab3Mapping::isMappingDone(int cam)
{
    switch (cam)
    {
    case 1:
        return isCamMapReady(1) && map_turtle1_points.size() == 4;
    case 2:
        return isCamMapReady(2) && map_turtle1_points.size() == 4;
    default:
        return false;
    }
}

/* 터틀봇의 지도 좌표로 변환 */
QString Tab3Mapping::convertPointToTurtle(cv::Point2f cam_point, int cam_number)
{
    QString str;
    std::vector<cv::Point2f> cam, map, turtle;  // 카메라 좌표, 지도 위 좌표, 터틀봇 좌표
    cam.push_back(cam_point);

    if (cam_number == 1)
    {
        cv::perspectiveTransform(cam, map, cam_map1_transform_array);       // cam1 -> map1
        cv::perspectiveTransform(map, turtle, map_turtle1_transform_array);  // map1 -> turtle1
    }
    else
    {
        cv::perspectiveTransform(cam, map, cam_map2_transform_array);       // cam2 -> map2
        cv::perspectiveTransform(map, turtle, map_turtle1_transform_array);  // map2 -> turtle1
    }

    str = QString("GOAL@%1@%2").arg(turtle[0].x).arg(turtle[0].y);
    return str;
}

/* SLOT : "카메라-지도 연결" 버튼 클릭 이벤트 */
void Tab3Mapping::onBtnCamMapConnectClicked(bool checked)
{
    if (checked)
    {
        if (current_cam == 1)
        {
            cam1_points.clear();
            cam1_points.shrink_to_fit();
//            std::vector<cv::Point2f>().swap(cam1_points);   // swap memory with empty vector
            map1_points.clear();
            map1_points.shrink_to_fit();
//            std::vector<cv::Point2f>().swap(map1_points);
        }
        else if (current_cam == 2)
        {
            cam2_points.clear();
            std::vector<cv::Point2f>().swap(cam2_points);
            map2_points.clear();
            std::vector<cv::Point2f>().swap(map2_points);
        }

        cam_map_connect_event_flag = true;
        active_cam_click = true;      // 카메라 좌표부터 설정
        ui->labelInfoText->setText("카메라 영역을 좌상단부터 시계방향으로 지정");
    }
    else {
        cam_map_connect_event_flag = false;
        active_cam_click = false;
        active_map_click = false;

        if ((current_cam == 1) && !isCamMapReady(1)) {
            ui->labelInfoText->setText("취소되었습니다.");
        }
        else if ((current_cam == 2) && !isCamMapReady(2))
        {
            ui->labelInfoText->setText("취소되었습니다.");
        }
        else {
            QString str = QString("카메라%1 맵핑이 완료되었습니다.").arg(current_cam);
            ui->labelInfoText->setText(str);
        }
    }
}

/* SLOT : "지도 영역 설정" 버튼 클릭 이벤트 */
void Tab3Mapping::onBtnMapAreaSetClicked(bool checked)
{
    if (checked)
    {
        map_turtle1_points.clear();
        std::vector<cv::Point2f>().swap(map_turtle1_points);

        map_area_set_event_flag = true;
        active_map_click = true;
        ui->labelInfoText->setText("지도 영역을 좌상단부터 시계방향으로 지정");
    }
    else
    {
        map_area_set_event_flag = false;
        active_map_click = false;

        if (map_turtle1_points.size() != 4)
        {
            ui->labelInfoText->setText("취소되었습니다.");
        }
        else
        {
            ui->labelInfoText->setText("지도 영역 설정이 완료되었습니다.");
        }
    }
}

/* SLOT : "카메라1" 버튼 클릭 이벤트 */
void Tab3Mapping::onBtnCam1SelectClicked()
{
    bool ok;

    emit signalRequestCamImage(cam_image, 1, ok);

    if (ok)
    {
        current_cam = 1;
        drawCorners();
        ui->labelInfoText->setText("카메라1이(가) 선택되었습니다.");
    }
    else
    {
        ui->labelInfoText->setText("카메라1 이미지를 가져올 수 없습니다.");
    }
}

/* SLOT : "카메라2" 버튼 클릭 이벤트 */
void Tab3Mapping::onBtnCam2SelectClicked()
{
    bool ok;

    emit signalRequestCamImage(cam_image, 2, ok);

    if (ok)
    {
        current_cam = 2;
        drawCorners();
        ui->labelInfoText->setText("카메라2이(가) 선택되었습니다.");
    }
    else
    {
        ui->labelInfoText->setText("카메라2 이미지를 가져올 수 없습니다.");
    }
}

void Tab3Mapping::onBtnRestoreSettingClicked()
{
    bool ok;

    // get camera image
    emit signalRequestCamImage(cam_image, current_cam, ok);
    if (!ok)
    {
        ui->labelInfoText->setText(QString("카메라%1 이미지를 가져올 수 없습니다.").arg(current_cam));
        return;
    }

    // read config from file
    if (readConfig())
    {
        getPerspectiveTransform();
        drawCorners();
        ui->labelInfoText->setText("설정을 가져왔습니다.");
    }
}

/* SLOT : 지도 영역 수정 시 이벤트 */
void Tab3Mapping::onMapEdited()
{
    writeConfig();
    getPerspectiveTransform();
    drawCorners();
}

/* SLOT : 파이썬 클라이언트 접속 시 */
void Tab3Mapping::onClientConnect()
{
    QTcpSocket *newClient = server->nextPendingConnection();

    newClient->setSocketOption(QAbstractSocket::LowDelayOption, 1);  // 지연 최소화
    newClient->setSocketOption(QAbstractSocket::KeepAliveOption, 1); // 연결 유효 확인

    connect(newClient, SIGNAL(readyRead()), this, SLOT(readClientSocket()));
    connect(newClient, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));

    qDebug().noquote() << currentTime() << "Python client connected";
}

/* SLOT : 소켓으로부터 데이터 읽기 */
void Tab3Mapping::readClientSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    static int messageLength = -1;

    while (socket->bytesAvailable() > 0) {
        if (messageLength == -1 && socket->bytesAvailable() >= 4) {
            // 4바이트 길이 읽기
            QByteArray lengthBytes = socket->read(4);
            messageLength = qFromBigEndian<qint32>(reinterpret_cast<const uchar*>(lengthBytes.constData()));
        }

        if (messageLength != -1 && socket->bytesAvailable() >= messageLength) {
            // 메시지 내용 읽기
            QByteArray data = socket->read(messageLength);

            // 문자열 파싱
            QStringList str_list;

            str_list = QString(data).split("@");
            if (str_list[0].contains("FIRE") && str_list.length() == 4)
            {
                cv::Point2f fire = cv::Point2f(str_list[1].toFloat(), str_list[2].toFloat());
                int cam = str_list[3].toInt();

                slotReceiveFirePoint(fire, cam);
            }

            // 다음 메시지 준비
            messageLength = -1;
        }
    }
}

/* SLOT : 파이썬 클라이언트 연결 해제 시 */
void Tab3Mapping::onClientDisconnected()
{
    QTcpSocket *sender_client = qobject_cast<QTcpSocket*>(sender());

    if (sender_client)
    {
        sender_client->deleteLater();
        qDebug().noquote() << currentTime() << "Python client disconnected";
    }
}

/* SLOT : 화재 좌표 수신 시 */
void Tab3Mapping::slotReceiveFirePoint(cv::Point2f fire, int cam)
{
    if (is_turtle_busy)
    {
        qDebug().noquote() << currentTime() << "Received fire point, but turtlebot is busy now.";
    }
    else if (cam == 1 && !isMappingDone(1))
    {
        qDebug().noquote() << currentTime() << "Received fire point, but cam1 is not mapped.";
    }
    else if (cam == 2 && !isMappingDone(2))
    {
        qDebug().noquote() << currentTime() << "Received fire point, but cam2 is not mapped.";
    }
    else
    {
        emit signalTargetFound(convertPointToTurtle(fire, cam));
        qDebug().noquote() << currentTime() << convertPointToTurtle(fire, cam);
        is_turtle_busy = true;
    }
}

/* SLOT : 소화 완료 메시지 수신 시 */
void Tab3Mapping::slotReceiveFinishMessage()
{
    is_turtle_busy = false;
}

QString Tab3Mapping::currentTime()
{
    QDateTime date_time = QDateTime::currentDateTime();

    return date_time.toString("[yyyy-MM-dd hh:mm:ss.zzz]");
}
