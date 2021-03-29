#pragma once

#include <QWidget>
#include <QRect>
#include <QMap>

namespace Ui { class ImageCutter; };

namespace VKQT{
    class ImageCutter : public QWidget
    {
        Q_OBJECT

        enum AnchorType
        {
            Invalid,
            TopLeft,
            TopRight,
            BottomLeft,
            BottomRight,
            Middle
        };

    public:
        struct ImageCutInfo
        {
            float   x = 0.f;
            float   y = 0.f;
            float   w = 1.f;
            float   h = 1.f;
        };

    public:
        ImageCutter(QWidget *parent = Q_NULLPTR);
        ~ImageCutter();

        void    loadImage(const QString& img);
        void    loadCutInfo(const ImageCutInfo& ci);
        int     borderWidth();
        ImageCutInfo cutInfo();

        QMap<VKQT::ImageCutter::AnchorType, QRect> _anchorRegion;

    protected:
        void    paintEvent(QPaintEvent* event) override;
        void    mousePressEvent(QMouseEvent* event) override;
        void    mouseMoveEvent(QMouseEvent* event) override;
        void    mouseReleaseEvent(QMouseEvent* event) override;
        void    showEvent(QShowEvent* event) override;

    private:
        AnchorType  hitTest(QPoint p);
        void    autoAdjustMiddleRect(AnchorType anchor);
        void    autoUpdateCursor(QPoint p);

    private:
        Ui::ImageCutter *ui;

        bool    _bFirstShow = true;
        ImageCutInfo    _originCutInfo{ 0.f, 0.f, 1.f, 1.f };
        QImage  _img;
        QPoint  _originMousePos{ 0, 0 };
        QPoint  _originRectPos;
        bool    _bDraging = false;
        AnchorType  _anchor = Invalid;
    };
}
