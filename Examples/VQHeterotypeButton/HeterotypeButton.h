#ifndef KEIGHTDIRECTIONBUTTON_H
#define KEIGHTDIRECTIONBUTTON_H

#include <QWidget>
#include <QPixmap>
#include "ui_HeterotypeButton.h"

namespace VKQT{
    class HeterotypeButton : public QWidget
    {
        Q_OBJECT

    public:
        HeterotypeButton(QWidget *parent = 0);
        ~HeterotypeButton();

        void	setText(const QString& text);

        enum Direction
        {
            Top,
            Bottom,
            Left,
            Right,
            LeftTop,
            LeftBottom,
            RightTop,
            RightBottom
        };

    signals:
        void	clicked(int);

    public slots:
        void	btnClicked();

    protected:
        void	paintEvent(QPaintEvent *event);

    private:
        void	SetButtonStyle();

    private:
        Ui::HeterotypeButton ui;

        QPixmap		_pixBkg;
        QPixmap		_pixLeft;
        QPixmap		_pixLeftTop;
        QPixmap		_pixTop;
        QPixmap		_pixRightTop;
        QPixmap		_pixRight;
        QPixmap		_pixRightBottom;
        QPixmap		_pixBottom;
        QPixmap		_pixLeftBottom;
    };
}
#endif // KEIGHTDIRECTIONBUTTON_H
