#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "tab1camera.h"
#include "tab2roscontrol.h"
#include "tab3mapping.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidget; }
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(int argc, char** argv, QWidget *parent = nullptr);
    ~MainWidget();

private:
    Ui::MainWidget *ui;
    Tab1Camera *pTab1Camera;
    Tab2RosControl *pTab2RosControl;
    Tab3Mapping *pTab3Mapping;
};
#endif // MAINWIDGET_H
