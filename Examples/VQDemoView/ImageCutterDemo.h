#pragma once

#include <QWidget>
namespace Ui { class ImageCutterDemo; };

class ImageCutterDemo : public QWidget
{
    Q_OBJECT

public:
    ImageCutterDemo(QWidget *parent = Q_NULLPTR);
    ~ImageCutterDemo();

private:
    Ui::ImageCutterDemo *ui;
};
