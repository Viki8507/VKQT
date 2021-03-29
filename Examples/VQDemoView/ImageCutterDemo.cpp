#include "ImageCutterDemo.h"
#include "ui_ImageCutterDemo.h"
#include "../Examples.h"

RegistryDemoWidget(QString::fromLocal8Bit("ͼƬ�ü���"), ImageCutterDemo)
ImageCutterDemo::ImageCutterDemo(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::ImageCutterDemo();
    ui->setupUi(this);

    ui->widget_imageCutter->loadImage(":/VKQT/Resources/ImageCutter/imagecutter.jpg");
}

ImageCutterDemo::~ImageCutterDemo()
{
    delete ui;
}
