#pragma once

#include <QWidget>
namespace Ui { class TimelineDemo; };

class TimelineDemo : public QWidget
{
    Q_OBJECT

public:
    TimelineDemo(QWidget *parent = Q_NULLPTR);
    ~TimelineDemo();

private:
    Ui::TimelineDemo *ui;
};
