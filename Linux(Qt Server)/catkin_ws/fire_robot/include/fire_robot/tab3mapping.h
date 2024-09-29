#ifndef TAB3MAPPING_H
#define TAB3MAPPING_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QImage>
#include <QPixmap>
#include <opencv2/opencv.hpp>
#include <vector>
// config r/w
#include <QDir>
#include <QFile>
#include <QList>
#include <QIODevice>
#include <QStringList>
#include <QTextStream>
// fire message
#include <QTcpServer>
#include <QTcpSocket>
#include <QtEndian>

#include <QDateTime>

namespace Ui {
class Tab3Mapping;
}

class Tab3Mapping : public QWidget
{
    Q_OBJECT

public:
    explicit Tab3Mapping(QWidget *parent = nullptr);
    ~Tab3Mapping();

private:
    Ui::Tab3Mapping *ui;

    QString map_path = QDir::homePath().append("/map.png");

    cv::Mat cam_image;                     // CCTV 이미지
    cv::Mat map_image;                      // 지도 이미지

    /* 상태 변수 */
    int current_cam = 1;                    // 현재 선택된 카메라
    bool active_cam_click = false;          // 카메라 좌표 선택 모드
    bool active_map_click = false;          // 지도 좌표 선택 모드
    bool is_turtle_busy = false;            // 터틀봇 작동 상태

    /* 이벤트 플래그 */
    bool cam_map_connect_event_flag = false;
    bool map_area_set_event_flag = false;

    /* 카메라-지도 변환 */
    std::vector<cv::Point2f> cam1_points;
    std::vector<cv::Point2f> map1_points;
    std::vector<cv::Point2f> cam2_points;
    std::vector<cv::Point2f> map2_points;
    cv::Mat cam_map1_transform_array;       // 카메라-지도 좌표 변환 배열
    cv::Mat cam_map2_transform_array;

    /* 지도-터틀봇 변환 */
    std::vector<cv::Point2f> map_turtle1_points;
    std::vector<cv::Point2f> turtle1_points;
    std::vector<cv::Point2f> map_turtle2_points;
    std::vector<cv::Point2f> turtle2_points;
    cv::Mat map_turtle1_transform_array;     // 지도-터틀봇 좌표 변환 배열
    cv::Mat map_turtle2_transform_array;

    /* 상태 저장 */
    QList<std::vector<cv::Point2f>*> point_list;
    QString config_path = QDir::homePath().append("/aiot_map.csv");

    /* 화재 지점 수신 */
    QTcpServer *server;
    QTcpSocket *client;                     // 파이썬 추론 프로그램

    cv::Point2f test_point = cv::Point2f(0.0, 0.0);

public:
    bool isCamMapReady(int);                // 카메라-지도 변환의 모서리가 설정 되었는지 여부
    bool isMappingDone(int);                // 모든 설정이 완료되어 좌표 전송이 준비된 상태의 여부

private:
    bool eventFilter(QObject*, QEvent*);
    bool camMapConnectEvent(QObject*, QEvent*);
    bool mapAreaSetEvent(QObject*, QEvent*);

    void updateImageView(cv::Mat&, QLabel*);
    void drawCorners();
    void getPerspectiveTransform();
    bool readConfig();
    void writeConfig();
    QString convertPointToTurtle(cv::Point2f, int);

    QString currentTime();

private slots:
    void onBtnCam1SelectClicked();
    void onBtnCam2SelectClicked();
    void onBtnCamMapConnectClicked(bool);
    void onBtnMapAreaSetClicked(bool);
    void onBtnRestoreSettingClicked();
    void onMapEdited();

    void onClientConnect();
    void readClientSocket();
    void onClientDisconnected();

public slots:
    void slotReceiveFirePoint(cv::Point2f, int);
    void slotReceiveFinishMessage();

signals:
    void signalRequestCamImage(cv::Mat&, int, bool&);
    void signalTargetFound(QString);
    void signalMapEdited();
};

#endif // TAB3MAPPING_H
