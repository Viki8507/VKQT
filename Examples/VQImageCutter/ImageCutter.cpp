#include "ImageCutter.h"
#include "ui_ImageCutter.h"
#include <QPainter>
#include <QMouseEvent>

#define MinWidth    20
#define MinHeight   20
#define BorderWidth 10

VKQT::ImageCutter::ImageCutter(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::ImageCutter();
    ui->setupUi(this);
    setMouseTracking(true);
    loadImage("");
}

VKQT::ImageCutter::~ImageCutter()
{
    delete ui;
}

void VKQT::ImageCutter::loadImage(const QString& img)
{
    _img.load(img);
}

void VKQT::ImageCutter::loadCutInfo(const ImageCutInfo& ci)
{
    _originCutInfo = ci;
}

int VKQT::ImageCutter::borderWidth()
{
    return BorderWidth;
}

VKQT::ImageCutter::ImageCutInfo VKQT::ImageCutter::cutInfo()
{
    ImageCutInfo ci;
    ci.x =  (float)(_anchorRegion[Middle].left() - BorderWidth) / ((float)width() - 2 * BorderWidth);
    ci.y = (float)(_anchorRegion[Middle].top() -  BorderWidth) / ((float)height() - 2 * BorderWidth);
    ci.w = (float)_anchorRegion[Middle].width() / ((float)width() - 2 * BorderWidth);
    ci.h = (float)_anchorRegion[Middle].height() / ((float)height() - 2 * BorderWidth);
    return ci;
}

void VKQT::ImageCutter::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    /* 背景色 */
    painter.fillRect(rect(), QBrush(QColor("#D7D7D7")));

    /* 图片 */
    painter.drawImage(rect().adjusted(BorderWidth, BorderWidth, -BorderWidth, -BorderWidth), _img);

    QPainterPath alphaPath;
    alphaPath.addRect(rect().adjusted(BorderWidth, BorderWidth, -BorderWidth, -BorderWidth));
    QPainterPath cutPath;
    cutPath.addRect(_anchorRegion[Middle]);
    painter.fillPath(alphaPath.subtracted(cutPath), QBrush(QColor(0, 0, 0, 150)));

#if 1
    /* 画四个角的锚点 */
    static QColor anchorColor("#ffffff");
    painter.fillRect(_anchorRegion[TopLeft], anchorColor);
    painter.fillRect(_anchorRegion[TopRight], anchorColor);
    painter.fillRect(_anchorRegion[BottomLeft], anchorColor);
    painter.fillRect(_anchorRegion[BottomRight], anchorColor);
#endif
}

void VKQT::ImageCutter::mousePressEvent(QMouseEvent* event)
{
    _anchor = hitTest(event->pos());
    if (_anchor != Invalid)
    {
        switch (_anchor)
        {
            case Middle:
                _originRectPos = _anchorRegion[Middle].topLeft();
                break;
            case TopLeft:
                _originRectPos = _anchorRegion[Middle].topLeft();
                break;
            case TopRight:
                _originRectPos = _anchorRegion[Middle].topRight();
                break;
            case BottomLeft:
                _originRectPos = _anchorRegion[Middle].bottomLeft();
                break;
            case BottomRight:
                _originRectPos = _anchorRegion[Middle].bottomRight();
                break;
        }
        
        _originMousePos = event->pos();
    }
}

void VKQT::ImageCutter::mouseMoveEvent(QMouseEvent* event)
{
    if (!_bDraging)
    {
        autoUpdateCursor(event->pos());
    }

    if (_anchor != Invalid && (_originMousePos - event->pos()).manhattanLength() > 3)
    {
        _bDraging = true;
        switch (_anchor)
        {
            case Middle:
            {
                _anchorRegion[Middle].moveTopLeft(_originRectPos + event->pos() - _originMousePos);
            }break;
            case TopLeft:
            {
                _anchorRegion[Middle].setTopLeft(_originRectPos + event->pos() - _originMousePos);
            }break;
            case TopRight:
            {
                _anchorRegion[Middle].setTopRight(_originRectPos + event->pos() - _originMousePos);
            }break;
            case BottomLeft:
            {
                _anchorRegion[Middle].setBottomLeft(_originRectPos + event->pos() - _originMousePos);
            }break;
            case BottomRight:
            {
                _anchorRegion[Middle].setBottomRight(_originRectPos + event->pos() - _originMousePos);
               
            }break;
        }
        autoAdjustMiddleRect(_anchor);
        _anchorRegion[TopLeft] = QRect(_anchorRegion[Middle].topLeft() + QPoint{ 0, 0 }, QSize(10, 10));
        _anchorRegion[TopRight] = QRect(_anchorRegion[Middle].topRight() + QPoint{ -10, 0 }, QSize(10, 10));
        _anchorRegion[BottomLeft] = QRect(_anchorRegion[Middle].bottomLeft() + QPoint{ 0, -10 }, QSize(10, 10));
        _anchorRegion[BottomRight] = QRect(_anchorRegion[Middle].bottomRight() + QPoint{ -10, -10 }, QSize(10, 10));
        update();
    }
}

void VKQT::ImageCutter::mouseReleaseEvent(QMouseEvent* event)
{
    _bDraging = false;
    _anchor = Invalid;
    autoUpdateCursor(event->pos());
}

void VKQT::ImageCutter::showEvent(QShowEvent* event)
{
    if (_bFirstShow)
    {
        _anchorRegion[Middle].setX((width() - 2 * BorderWidth) * _originCutInfo.x + BorderWidth);
        _anchorRegion[Middle].setY((height() - 2 * BorderWidth) * _originCutInfo.y + BorderWidth);
        _anchorRegion[Middle].setWidth((width() - 2 * BorderWidth) * _originCutInfo.w + 1);
        _anchorRegion[Middle].setHeight((height() - 2 * BorderWidth) * _originCutInfo.h + 1);

        _anchorRegion[TopLeft] = QRect(_anchorRegion[Middle].topLeft() + QPoint{ 0, 0 }, QSize(10, 10));
        _anchorRegion[TopRight] = QRect(_anchorRegion[Middle].topRight() + QPoint{ -10, 0 }, QSize(10, 10));
        _anchorRegion[BottomLeft] = QRect(_anchorRegion[Middle].bottomLeft() + QPoint{ 0, -10 }, QSize(10, 10));
        _anchorRegion[BottomRight] = QRect(_anchorRegion[Middle].bottomRight() + QPoint{ -10, -10 }, QSize(10, 10));
        _bFirstShow = false;
    }
    __super::showEvent(event);
}

VKQT::ImageCutter::AnchorType VKQT::ImageCutter::hitTest(QPoint p)
{
    if (_anchorRegion[TopLeft].adjusted(1, 1, -1, -1).contains(p))
        return TopLeft;
    else if (_anchorRegion[TopRight].adjusted(1, 1, -1, -1).contains(p))
        return TopRight;
    else if (_anchorRegion[BottomLeft].adjusted(1, 1, -1, -1).contains(p))
        return BottomLeft;
    else if (_anchorRegion[BottomRight].adjusted(1, 1, -1, -1).contains(p))
        return BottomRight;
    else if (_anchorRegion[Middle].adjusted(1, 1, -1, -1).contains(p))
        return Middle;
    return Invalid;
}

void VKQT::ImageCutter::autoAdjustMiddleRect(AnchorType anchor)
{
    QRect& middleRect = _anchorRegion[Middle];
    switch (anchor)
    {
        case Middle:    /* 整体挪动，在不改变尺寸的情况下，禁止超出图片区域 */
        {
            if (middleRect.topLeft().x() < BorderWidth)
                middleRect.moveLeft(BorderWidth);

            if (middleRect.topLeft().y() < BorderWidth)
                middleRect.moveTop(BorderWidth);

            if (middleRect.topRight().x() > width() - BorderWidth)
                middleRect.moveRight(width() - BorderWidth);

            if (middleRect.bottomRight().y() > height() - BorderWidth)
                middleRect.moveBottom(height() - BorderWidth);
        }break;
        case TopLeft:
        {
            if (middleRect.topLeft().x() < BorderWidth)
                middleRect.setLeft(BorderWidth);
            if (middleRect.width() < MinWidth)
                middleRect.setLeft(middleRect.right() - MinWidth);
            if (middleRect.topLeft().y() < BorderWidth)
                middleRect.setTop(BorderWidth);
            if (middleRect.height() < MinHeight)
                middleRect.setTop(middleRect.bottom() - MinHeight);
        }break;
        case TopRight:
        {
            if (middleRect.topRight().x() > width() - BorderWidth)
                middleRect.setRight(width() - BorderWidth);
            if (middleRect.width() < MinWidth)
                middleRect.setRight(middleRect.left() + MinWidth);
            if (middleRect.topRight().y() < BorderWidth)
                middleRect.setTop(BorderWidth);
            if (middleRect.height() < MinHeight)
                middleRect.setTop(middleRect.bottom() - MinHeight);
        }break;
        case BottomLeft:
        {
            if (middleRect.bottomLeft().x() < BorderWidth)
                middleRect.setLeft(BorderWidth);
            if (middleRect.width() < MinWidth)
                middleRect.setLeft(middleRect.right() - MinWidth);
            if (middleRect.bottomLeft().y() > height() - BorderWidth)
                middleRect.setBottom(height() - BorderWidth);
            if (middleRect.height() < MinHeight)
                middleRect.setBottom(middleRect.top() + MinHeight);
        }break;
        case BottomRight:
        {
            if (middleRect.bottomRight().x() > width() - BorderWidth)
                middleRect.setRight(width() - BorderWidth);
            if (middleRect.width() < MinWidth)
                middleRect.setRight(middleRect.left() + MinWidth);
            if (middleRect.bottomRight().y() > height() - BorderWidth)
                middleRect.setBottom(height() - BorderWidth);
            if (middleRect.height() < MinHeight)
                middleRect.setBottom(middleRect.top() + MinHeight);
        }break;
    }
}

void VKQT::ImageCutter::autoUpdateCursor(QPoint p)
{
    AnchorType anchor = hitTest(p);
    if (anchor != Invalid)
    {
        switch (anchor)
        {
            case Middle:
                setCursor(Qt::ArrowCursor);
                break;
            case TopLeft:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case TopRight:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case BottomLeft:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case BottomRight:
                setCursor(Qt::SizeFDiagCursor);
                break;
        }
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
}
