#include "TimelineDemo.h"
#include "ui_TimelineDemo.h"
#include "../Examples.h"

RegistryDemoWidget(QString::fromLocal8Bit("Ê±¼äÖá"), TimelineDemo)
TimelineDemo::TimelineDemo(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::TimelineDemo();
    ui->setupUi(this);

    ui->widget_timline->setDuration(3600000);
    ui->widget_timline->setRulerAccuracy(400);
}

TimelineDemo::~TimelineDemo()
{
    delete ui;
}