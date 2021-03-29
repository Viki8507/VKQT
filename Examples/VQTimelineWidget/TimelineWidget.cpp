#include "TimelineWidget.h"
#include "ui_TimelineWidget.h"

#include <QPainter>
#include <QTime>
#include <QMouseEvent>

#define ScaleTextFixedHeight    15  /* 刻度文字的高度 */
#define ScaleMarkHeight         8  /* 标尺刻度的高度 */
#define ColorSliceFixedHeight   _markSliderHeight  /* 着色条固定高度 */
#define ThumbFixedHeight        70  /* 缩略图固定高度 */

#define ThumbnailWidth          130 /* 缩略图绘制宽度 */
#define CursorFixedWidth        16  /* 游标固定宽度 */
#define CursorTopSpacing        3   /* 游标距顶部的距离 */     
#define TextFixedWidth          100  /* 标尺文本固定宽度 */

/* 每个格子表示的时长 */
static QList<int> DurationPer5Cell = {
    500, 1000, 2000, 5000, 10000, 15000, 
    20000, 25000, 30000, 35000, 40000, 
    45000, 50000, 55000, 60000,
    75000, 90000, 105000, 120000
};

VKQT::TimelineWidget::TimelineWidget(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::TimelineWidget();
    ui->setupUi(this);
    /* 暂时不允许动态调整精度(没有给出UI交互设计)，默认设为400 */
    ui->horizontalSlider->hide();

    ui->renderWidget->setMouseTracking(true);
    ui->renderWidget->installEventFilter(this);
    ui->horizontalScrollBar->installEventFilter(this);

    _pCursorWidget = new QWidget(ui->renderWidget);
    _pCursorWidget->setMouseTracking(true);
    _pCursorWidget->installEventFilter(this);
    _pCursorWidget->setFixedWidth(CursorFixedWidth);

    connect(ui->horizontalScrollBar, &QScrollBar::valueChanged, this, [this](int iValue){
        qint64 llOffset = iValue * _iRulerAccuracy;
        //qint64 llOffset = iValue * _llTotalDuration / 300;
        setOffset(llOffset);
    });

    connect(ui->horizontalSlider, &QSlider::valueChanged, this, [this](int iValue){
        setRulerAccuracy(iValue * 40);
    });

    setRulerAccuracy(400); 
}

VKQT::TimelineWidget::~TimelineWidget()
{
    delete ui;
}

void VKQT::TimelineWidget::setOffset(qint64 llOffset)
{
    _llPlayOffset = llOffset;
    ui->renderWidget->update();

    if (_progressGetter)
    {
        qint64 llPos = _progressGetter();
        int cursorPos = mapToWidgetPosition(llPos);
        _pCursorWidget->move(cursorPos - CursorFixedWidth / 2, CursorTopSpacing);
    }
}

void VKQT::TimelineWidget::setDuration(qint64 llDuration)
{
    _llTotalDuration = llDuration;
    updateScrollBar();
    ui->horizontalScrollBar->setValue(0);
    ui->renderWidget->update();
    _pCursorWidget->move(-(CursorFixedWidth / 2), CursorTopSpacing);
}

void VKQT::TimelineWidget::setRulerAccuracy(int iMillisecons)
{
    _iRulerAccuracy = qMax(16, iMillisecons);
    qint64 llDurationPerCell = _iRulerAccuracy * _fRulerCellWidth;

     /* 从DurationPerCell找到第一个值大于llDurationPerCell的元素 */
     auto it = std::find_if(DurationPer5Cell.begin(), DurationPer5Cell.end(), [llDurationPerCell](int iDuration){
         return iDuration >= llDurationPerCell;
     });
     _fTimePerCell = (it != DurationPer5Cell.end()) ? *it : _fTimePerCell;

    updateScrollBar();
    ui->renderWidget->update();

    if (_progressGetter)
    {
        qint64 llPos = _progressGetter();
        int cursorPos = mapToWidgetPosition(llPos);
        _pCursorWidget->move(cursorPos - CursorFixedWidth / 2, CursorTopSpacing);
    }
}

void VKQT::TimelineWidget::updatePosition(qint64 llPosition)
{
    //autoUpdateCursorPosition();
    if (!_bDragCursor)
    {
        int iWidgetPos = mapToWidgetPosition(llPosition);
        _pCursorWidget->move(QPoint{ iWidgetPos - CursorFixedWidth / 2, CursorTopSpacing });
        if (_bAutoAdjustOffset && (iWidgetPos < -CursorFixedWidth / 2 || iWidgetPos >= ui->renderWidget->width()))
        {
            ensurePlayPositionVisible(llPosition);
        }

        QPoint pos = _pCursorWidget->pos();
        if (!_bAutoAdjustOffset && !_bDragScrollBar
            && pos.rx() >= -_pCursorWidget->width()
            && pos.rx() <= ui->renderWidget->width())
        {
            _bAutoAdjustOffset = true;
        }
    }
}

void VKQT::TimelineWidget::setProgressGetter(ProgressGetter getter)
{
    _progressGetter = getter;
}

void VKQT::TimelineWidget::setThumbnailGetter(ThumbnailGetter getter)
{
    _thumbGetter = getter;
}

void VKQT::TimelineWidget::ensureCursorVisible()
{
    qint64 llPos = _progressGetter();
    int iWidgetPos = _pCursorWidget->pos().rx();
    if (iWidgetPos < -CursorFixedWidth / 2 || iWidgetPos > ui->renderWidget->width())
    {
        int offset = llPos / _iRulerAccuracy;
        ui->horizontalScrollBar->setValue(offset);
    }
}

void VKQT::TimelineWidget::ensurePlayPositionVisible(qint64 pos)
{
    int iWidgetPos = mapToWidgetPosition(pos);
    if (iWidgetPos < -CursorFixedWidth / 2 || iWidgetPos > ui->renderWidget->width())
    {
        int offset = pos / _iRulerAccuracy;
        ui->horizontalScrollBar->setValue(offset);
    }
}

void VKQT::TimelineWidget::seekTo(qint64 llPos)
{
    int iWidgetPos = mapToWidgetPosition(llPos);
    _pCursorWidget->move(QPoint{ iWidgetPos - CursorFixedWidth / 2, CursorTopSpacing });
}

void VKQT::TimelineWidget::addMarkPoint(const TimelineMarkPoint& point)
{
    auto it = std::find_if(_markPoints.begin(), _markPoints.end(), [&point](auto p){
        return p.start == point.start;
    });

    if (it != _markPoints.end())
    {
        if (it->end = point.end)
            return;
        else
        {
            it->start = point.start;
            it->end = point.end;
        }
    }
    else
    {
        _markPoints.append(point);
    }
    qSort(_markPoints.begin(), _markPoints.end(), [](auto point1, auto point2){
        return point1.start < point2.start;
    });
    update();
}

void VKQT::TimelineWidget::removeMarkPoint(qint64 pos)
{
    auto it = std::find_if(_markPoints.begin(), _markPoints.end(), [pos](auto point){
        return point.start == pos;
    });

    if (it != _markPoints.end())
    {
        _markPoints.erase(it);
        update();
    }
}

void VKQT::TimelineWidget::clearMarkPoint()
{
    _markPoints.clear();
    update();
}

void VKQT::TimelineWidget::repaintTimeline()
{
    //TODO: 时间线绘制刷新
    QPainter painter(ui->renderWidget);
    paintTimeText(&painter);
    paintRuler(&painter);
    paintColorSlice(&painter);
    paintThumbnail(&painter);
}

void VKQT::TimelineWidget::paintTimeText(QPainter* painter)
{
    ///////////////////////////////////////////////
    /* 一个文本宽度能够表示的时长 */
    //qint64 llTimePerTextWidth = TextFixedWidth * _iRulerAccuracy;
    int iWidthPer5Cells = _fRulerCellWidth * 5;
    int iCellsFillText = (TextFixedWidth + iWidthPer5Cells - 1) / iWidthPer5Cells;
    int iWidthPerTextBlock = iCellsFillText * iWidthPer5Cells;
    qint64 llDurationPerTextBlock = iWidthPerTextBlock * _iRulerAccuracy;

    int iBackwardBlocks = _llPlayOffset / llDurationPerTextBlock;
    qint64 llBackwardLeftLen = _llPlayOffset - iBackwardBlocks * llDurationPerTextBlock;
    llBackwardLeftLen = qMax(0LL, llBackwardLeftLen);
    qint64 llPlayPosStart = iBackwardBlocks * llDurationPerTextBlock;

    painter->save();
    static QFont textFont(tr("微软雅黑"));
    textFont.setPixelSize(12);
    int iHeight = painter->viewport().height();
    for (int iTextTotalWidth = -(llBackwardLeftLen / _iRulerAccuracy); iTextTotalWidth < width(); iTextTotalWidth += iWidthPerTextBlock)
    {
        painter->setPen(QColor("#666666"));
        painter->setFont(textFont);
        QString timeString = QTime::fromMSecsSinceStartOfDay(llPlayPosStart).toString("hh:mm:ss");
        painter->drawText(iTextTotalWidth, iHeight - ThumbFixedHeight - ColorSliceFixedHeight - ScaleMarkHeight - ScaleTextFixedHeight, 
            iWidthPerTextBlock, ScaleTextFixedHeight, Qt::AlignBottom | Qt::AlignLeft, timeString);
        llPlayPosStart += llDurationPerTextBlock;
    }
    painter->restore();
}

void VKQT::TimelineWidget::paintRuler(QPainter* painter)
{
    //TODO: 绘制标尺
    QRect geo = painter->viewport();
    painter->setPen(QColor("#989898"));
    int iHeight = painter->viewport().height();

    qint64 llTimerPer5Cell = _fRulerCellWidth * 5 * _iRulerAccuracy;
    int iBackwardBlocks = _llPlayOffset / llTimerPer5Cell;
    qint64 llBackwardLeftLen = _llPlayOffset - iBackwardBlocks * llTimerPer5Cell;
    llBackwardLeftLen = qMax(0LL, llBackwardLeftLen);
    qint64 llPlayPosStart = iBackwardBlocks * llTimerPer5Cell;

    /* 每5个格子画一个高刻度 */
    int iCounter = 0;
    for (int i = -(llBackwardLeftLen / _iRulerAccuracy); i < width(); i += lrint(_fRulerCellWidth))
    {
        if (iCounter % 5 == 0)
            painter->drawLine(QPoint{ i, iHeight - ThumbFixedHeight - ColorSliceFixedHeight - ScaleMarkHeight}, 
            QPoint{ i, iHeight - ThumbFixedHeight - ColorSliceFixedHeight - 2 });
        else
            painter->drawLine(QPoint{ i, iHeight - ThumbFixedHeight - ColorSliceFixedHeight - ScaleMarkHeight + 3 },
            QPoint{ i, iHeight - ThumbFixedHeight - ColorSliceFixedHeight - 2 });
        ++iCounter;
    }
}

void VKQT::TimelineWidget::paintColorSlice(QPainter* painter)
{
    painter->fillRect(QRect(0, painter->viewport().height() - ThumbFixedHeight - ColorSliceFixedHeight, width(),
        ColorSliceFixedHeight - 5), QColor("#666666"));
    auto it = _markPoints.begin(); 

    int iColorSelctor = 0;
    while (it != _markPoints.end())
    {
        int iStartPos = mapToWidgetPosition(it->start);
        int iEndPos = mapToWidgetPosition(it->end);
        auto itNext = std::next(it);
        if (itNext != _markPoints.end())
        {
            iEndPos = mapToWidgetPosition(itNext->end);
        }
        painter->fillRect(QRect(iStartPos, painter->viewport().height() - ThumbFixedHeight - ColorSliceFixedHeight,
            iEndPos - iStartPos, ColorSliceFixedHeight - 3), it->fillColor);

        it = std::next(it);
    }

    
}

void VKQT::TimelineWidget::paintThumbnail(QPainter* painter)
{
    //TODO: 绘制缩略图
    QRect rt = painter->viewport();
    QSize scaleTextSize = scaleTextSizeHint(painter);
    static QPixmap thumbDefault(":/TVM/Resources/default_keyframe.png");

    qint64 llLenPerBlock = ThumbnailWidth * _iRulerAccuracy;  /* 每张缩略图所占播放时时长(毫秒) */
    int iBackwardBlocks = _llPlayOffset / llLenPerBlock;             /* 左边超出控件的部分够填充多少张完整的缩略图 */
    qint64 llBackwardLeftLen = _llPlayOffset - iBackwardBlocks * llLenPerBlock;  
    llBackwardLeftLen = qMax(0LL, llBackwardLeftLen);
    qint64 llPlayPosStart = iBackwardBlocks * llLenPerBlock; /* 绘制的第一张缩略图对应是播放时间点 */
    for (int i = -(llBackwardLeftLen / _iRulerAccuracy); i < width(); i += ThumbnailWidth)
    {
        QPixmap thumb = _thumbGetter ? _thumbGetter(llPlayPosStart) : thumbDefault;
        QRect thumbRect(i, rt.height() - ThumbFixedHeight, ThumbnailWidth, ThumbFixedHeight);
        painter->drawTiledPixmap(thumbRect, thumb);
        llPlayPosStart += llLenPerBlock;
    }
}

void VKQT::TimelineWidget::paintCursor(QPainter* painter)
{
    //TODO: 绘制游标
    QRect rtOfCursor = _pCursorWidget->rect();
    QVector<QPoint> headPointers = {
        { rtOfCursor.topLeft() },
        { rtOfCursor.topRight() },
        { rtOfCursor.topRight() + QPoint{0, 16}},
        { rtOfCursor.topLeft() + QPoint{rtOfCursor.width() / 2, 20}},
        { rtOfCursor.topLeft() + QPoint{0, 16}},
    };
    QPolygonF pg(headPointers);
    QPainterPath path;
    path.addPolygon(pg);
    painter->fillPath(path, QColor("#0772FD"));
    painter->setPen(QColor("#0772FD"));
    painter->drawLine(rtOfCursor.topLeft() + QPoint{rtOfCursor.width() / 2, 0}, 
        rtOfCursor.bottomLeft() + QPoint{ rtOfCursor.width() / 2, 0});
}

QSize VKQT::TimelineWidget::scaleTextSizeHint(QPainter* painter)
{
    QFont font = painter->font();
    QFontMetrics fm(font);
    return fm.size(Qt::TextSingleLine, "00:00:00:000");
}

void VKQT::TimelineWidget::updateScrollBar()
{
    int displayRange = (_llTotalDuration + _iRulerAccuracy - 1) / _iRulerAccuracy;
    int iScrollRange = displayRange - width();
    ui->horizontalScrollBar->setRange(0, qMax(0, iScrollRange));
    ui->horizontalScrollBar->setSingleStep(width() / 20);
    ui->horizontalScrollBar->setPageStep(width());
}

qint64 VKQT::TimelineWidget::mapToPlayPostion(int iPos)
{
    int iMilliSeconds = iPos * _iRulerAccuracy;
    return iMilliSeconds + _llPlayOffset;
}

int VKQT::TimelineWidget::mapToWidgetPosition(qint64 llPos)
{
    int iPosAtWidget = (llPos - _llPlayOffset) / _iRulerAccuracy;
    return iPosAtWidget;
}

void VKQT::TimelineWidget::autoAdjustAccuracy()
{
    /* 大小调整时，也要调整一下精度或者偏移，保证缩略图能充满控件 */
    int iWidth = width();
    if (iWidth * _iRulerAccuracy > _llTotalDuration)
    {
        _llPlayOffset = 0;
        setRulerAccuracy(_llTotalDuration / iWidth);
    }
    else if (iWidth * _iRulerAccuracy > (_llTotalDuration - _llPlayOffset))
    {
        setOffset(_llTotalDuration - iWidth * _iRulerAccuracy);
    }
}

void VKQT::TimelineWidget::autoUpdateCursorPosition()
{
    if (_progressGetter)
    {
        qint64 llPos = _progressGetter();
        int iWidgetPos = mapToWidgetPosition(llPos);
        _pCursorWidget->move(QPoint{ iWidgetPos - CursorFixedWidth / 2, CursorTopSpacing });
        if (_bAutoAdjustOffset && (iWidgetPos < -CursorFixedWidth / 2 || iWidgetPos > ui->renderWidget->width()))
        {
            ensurePlayPositionVisible(llPos);
        }
    }
}

bool VKQT::TimelineWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->renderWidget)
    {
        if (event->type() == QEvent::Paint)
        {
            repaintTimeline();
            return true;
        }
        else if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if ((me->buttons() & Qt::LeftButton))
            {
                _pCursorWidget->move(QPoint{me->pos().x() - CursorFixedWidth / 2, CursorTopSpacing });
                qint64 llSeekPos = mapToPlayPostion(me->pos().x());
                emit seeked(llSeekPos);
            }
            return true;
        }
        else if (event->type() == QEvent::Resize)
        {
            updateScrollBar();
            QResizeEvent* re = static_cast<QResizeEvent*>(event);
            _markSliderHeight = qMin(80, re->size().height() - ThumbFixedHeight - ScaleMarkHeight - ScaleTextFixedHeight);
            _pCursorWidget->setFixedHeight(re->size().height() - CursorTopSpacing);
            autoUpdateCursorPosition();
            //_pCursorWidget->move(pos().x(), CursorTopSpacing);
        }
    }
    else if (obj == _pCursorWidget)
    {
        static QPoint mousePressedPoint{ 0, 0 };
        if (event->type() == QEvent::Paint)
        {
            QPainter painter(_pCursorWidget);
            paintCursor(&painter);
            return true;
        }
        else if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if ((me->buttons() & Qt::LeftButton))
            {
                mousePressedPoint = me->pos();
                _cursorMouseOriginPoint = _pCursorWidget->pos();
                _bCursorDragAvailable = true;
            }
            event->accept();
            return true;
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->buttons() & Qt::LeftButton)
            {
                if (_bCursorDragAvailable && (me->pos() - mousePressedPoint).manhattanLength() > 3)
                {
                    _bDragCursor = true;
                }

                if (_bDragCursor)
                {
                    QPoint movedPoint = _pCursorWidget->mapToParent(me->pos()) - QPoint{ CursorFixedWidth / 2, 0 };
                    movedPoint.setY(CursorTopSpacing);
                    _pCursorWidget->move(movedPoint);

                    qint64 llSeekPos = mapToPlayPostion(movedPoint.x() + CursorFixedWidth / 2);
                    emit seeked(llSeekPos);
                }
            }
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            _bCursorDragAvailable = false;
            _bDragCursor = false;
            return true;
        }
        else if (event->type() == QEvent::Move)
        {
            /* 限定游标的移动范围 */
            QMoveEvent* moveEvt = static_cast<QMoveEvent*>(event);
            int iMinLeftPos = mapToWidgetPosition(0);
            int iMaxRightPos = mapToWidgetPosition(_llTotalDuration);
            if (moveEvt->pos().x() < iMinLeftPos - (CursorFixedWidth / 2))
            {
                _pCursorWidget->move(iMinLeftPos - (CursorFixedWidth / 2), CursorTopSpacing);
                return true;
            }
            else if (moveEvt->pos().x() > iMaxRightPos - (CursorFixedWidth / 2))
            {
                _pCursorWidget->move(iMaxRightPos - (CursorFixedWidth / 2), CursorTopSpacing);
                return true;
            }
        }
    }
    else if (obj == ui->horizontalScrollBar)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            _bAutoAdjustOffset = false;
            _bDragScrollBar = true;
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            /* 松开滚动条时，只有游标当前可见才回复偏移自动调整功能 */
            if (_pCursorWidget->pos().x() >= -ui->renderWidget->width() && _pCursorWidget->pos().x() <= ui->renderWidget->width())
            {
                _bAutoAdjustOffset = true;
            }
            _bDragScrollBar = false;
        }
    }
    return __super::eventFilter(obj, event);
}

void VKQT::TimelineWidget::resizeEvent(QResizeEvent* event)
{
    __super::resizeEvent(event);
    autoAdjustAccuracy();
}

void VKQT::TimelineWidget::showEvent(QShowEvent *event)
{
    if (_iRulerAccuracy < 200)
    {
        int iMinAccuracy = _llTotalDuration / width();
        setRulerAccuracy(qMin(iMinAccuracy, 200));
    }

    if (_progressGetter)
    {
        qint64 llPos = _progressGetter();
        updatePosition(llPos);
    }
    __super::showEvent(event);
}
