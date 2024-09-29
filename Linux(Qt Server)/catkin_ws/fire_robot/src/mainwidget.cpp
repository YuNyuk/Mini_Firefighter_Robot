#include "../include/fire_robot/mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(int argc, char** argv, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    //Tab1
    pTab1Camera = new Tab1Camera(ui->pTab1);
    ui->pTab1->setLayout(pTab1Camera->layout());

    pTab2RosControl = new Tab2RosControl(argc, argv, pTab1Camera, ui->pTab2);
    ui->pTab2->setLayout(pTab2RosControl->layout());

    pTab3Mapping = new Tab3Mapping(ui->pTab3);
    ui->pTab3->setLayout(pTab3Mapping->layout());

    ui->pTabWidget->setCurrentIndex(0);

    // copy cam1 to tab3
    connect(pTab3Mapping, SIGNAL(signalRequestCamImage(cv::Mat&, int, bool&)), pTab1Camera, SLOT(slotCopyCamImage(cv::Mat&, int, bool&)));
    // send goal message
    connect(pTab3Mapping, SIGNAL(signalTargetFound(QString)), pTab2RosControl, SLOT(sendGoalMessage(QString)));
    // send goal message to tab2
    connect(pTab3Mapping, SIGNAL(signalTargetFound(QString)), pTab2RosControl, SLOT(sendBuzzerOn(QString)));
    // On receive finish message, resume detection.
    connect(pTab2RosControl, SIGNAL(sigFireFinish()), pTab3Mapping, SLOT(slotReceiveFinishMessage()));
}

MainWidget::~MainWidget()
{
    delete ui;
}

