#include "HeterotypeButton.h"
#include <QAbstractButton>
#include <QBitmap>
#include <QPainter>

VKQT::HeterotypeButton::HeterotypeButton(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    setFixedSize(100, 100);

    _pixBkg.load(":/HeterotypeButton/Resources/HeterotypeButton/bg.png");
    _pixLeft.load(":/HeterotypeButton/Resources/HeterotypeButton/btn_centre_tuimu_left_nor_hot_press_03.png");
    _pixLeftTop.load(":/HeterotypeButton/Resources/HeterotypeButton/btn_centre_tuimu_upper-left_nor_hot_press_02.png");
    _pixTop.load(":/HeterotypeButton/Resources/HeterotypeButton/btn_centre_tuimu_up_nor_hot_press_02.png");
    _pixRightTop.load(":/HeterotypeButton/Resources/HeterotypeButton/btn_centre_tuimu_right-upper_nor_hot_press_02.png");
    _pixRight.load(":/HeterotypeButton/Resources/HeterotypeButton/btn_centre_tuimu_right_nor_hot_press_03.png");
    _pixRightBottom.load(":/HeterotypeButton/Resources/HeterotypeButton/btn_centre_tuimu_right-lower_nor_hot_press_02.png");
    _pixBottom.load(":/HeterotypeButton/Resources/HeterotypeButton/btn_centre_tuimu_lower_nor_hot_press_02.png");
    _pixLeftBottom.load(":/HeterotypeButton/Resources/HeterotypeButton/btn_centre_tuimu_-left-lower_nor_hot_press_02.gif");

    connect(ui.btnTop, SIGNAL(clicked()), this, SLOT(btnClicked()));
    connect(ui.btnBottom, SIGNAL(clicked()), this, SLOT(btnClicked()));
    connect(ui.btnLeft, SIGNAL(clicked()), this, SLOT(btnClicked()));
    connect(ui.btnRight, SIGNAL(clicked()), this, SLOT(btnClicked()));
    connect(ui.btnLeftTop, SIGNAL(clicked()), this, SLOT(btnClicked()));
    connect(ui.btnLeftBottom, SIGNAL(clicked()), this, SLOT(btnClicked()));
    connect(ui.btnRightTop, SIGNAL(clicked()), this, SLOT(btnClicked()));
    connect(ui.btnRightBottom, SIGNAL(clicked()), this, SLOT(btnClicked()));

    SetButtonStyle();
}

VKQT::HeterotypeButton::~HeterotypeButton()
{

}

void VKQT::HeterotypeButton::setText(const QString& text)
{
    ui.label->setText(text);
}

void VKQT::HeterotypeButton::btnClicked()
{
    QAbstractButton* pBtn = (QAbstractButton*)sender();

    if (pBtn == ui.btnTop){
        emit clicked(Direction::Top);
    }
    else if (pBtn == ui.btnBottom){
        emit clicked(Direction::Bottom);
    }
    else if (pBtn == ui.btnLeft){
        emit clicked(Direction::Left);
    }
    else if (pBtn == ui.btnRight) {
        emit clicked(Direction::Right);
    }
    else if (pBtn == ui.btnLeftTop) {
        emit clicked(Direction::LeftTop);
    }
    else if (pBtn == ui.btnLeftBottom) {
        emit clicked(Direction::LeftBottom);
    }
    else if (pBtn == ui.btnRightTop) {
        emit clicked(Direction::RightTop);
    }
    else if (pBtn == ui.btnRightBottom) {
        emit clicked(Direction::RightBottom);
    }
}

void VKQT::HeterotypeButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void VKQT::HeterotypeButton::SetButtonStyle()
{
    //左
    //QPixmap pixLeftBtnMask = _pixLeft.copy(_pixLeft.width() / 3, 0, _pixLeft.width() / 3, _pixLeft.height());
    ui.btnLeft->setMask(_pixLeft.mask());

    //左上
    //QPixmap pixLeftTopBtnMask = _pixLeftTop.copy(_pixLeftTop.width() / 3, 0, _pixLeftTop.width() / 3, _pixLeftTop.height());
    ui.btnLeftTop->setMask(_pixLeftTop.mask());

    //QPixmap pixTopBtnMask = _pixTop.copy(_pixTop.width() / 3, 0, _pixTop.width() / 3, _pixTop.height());
    ui.btnTop->setMask(_pixTop.mask());

    //右上
    //QPixmap pixRightTopBtnMask = _pixRightTop.copy(_pixRightTop.width() / 3, 0, _pixRightTop.width() / 3, _pixRightTop.height());
    ui.btnRightTop->setMask(_pixRightTop.mask());

    //右
    //QPixmap pixRightBtnMask = _pixRight.copy(_pixRight.width() / 3, 0, _pixRight.width() / 3, _pixRight.height());
    ui.btnRight->setMask(_pixRight.mask());

    //右下
    //QPixmap pixRightBottomBtnMask = _pixRightBottom.copy(_pixRightBottom.width() / 3, 0, _pixRightBottom.width() / 3, _pixRightBottom.height());
    ui.btnRightBottom->setMask(_pixRightBottom.mask());

    //下
    //QPixmap pixBottomBtnMask = _pixBottom.copy(_pixBottom.width() / 3, 0, _pixBottom.width() / 3, _pixBottom.height());
    ui.btnBottom->setMask(_pixBottom.mask());

    //左下
    //QPixmap pixLeftBottomBtnMask = _pixLeftBottom.copy(_pixLeftBottom.width() / 3, 0, _pixLeftBottom.width() / 3, _pixLeftBottom.height());
    ui.btnLeftBottom->setMask(_pixLeftBottom.mask());




    //ui->_pixLeft->setStyleSheet(
    //	"QPushButton{border-image:url(images/left.png);}"
    //	"QPushButton:hover{border-image:url(images/left2.png);}"
    //	"QPushButton:pressed{border-image:url(images/left3.png);}");
}

