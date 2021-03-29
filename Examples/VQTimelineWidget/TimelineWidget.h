#pragma once

#include <QWidget>
#include <QPushButton>
#include <QColor>
#include <functional>
namespace Ui { class TimelineWidget; };

namespace VKQT{
    struct TimelineMarkPoint
    {
        qint64      start = 0;
        qint64      end = 0;
        QColor      fillColor = QColor("#989898");
    };

    class TimelineWidget : public QWidget
    {
        Q_OBJECT

    public:
        TimelineWidget(QWidget *parent = Q_NULLPTR);
        ~TimelineWidget();

    public:

        /*!************************************
         @Method:    setDuration 设置整个时间轴的播放时长
         @FullName:  TimelineWidget::setDuration
         @Returns:   void
         @Parameter: qint64 llDuration
        ************************************/
        void    setDuration(qint64 llDuration);

        /*!************************************
         @Method:    setRulerAccuracy   设置标尺精度
         @FullName:  TimelineWidget::setRulerAccuracy
         @Returns:   void
         @Parameter: int iPixels 1像素表示的毫秒数(16~1000)
        ************************************/
        void    setRulerAccuracy(int iMillisecons);

        void    updatePosition(qint64 llPosition);

        typedef std::function<qint64()> ProgressGetter;
        /*!************************************
         @Method:    setProgressGetter 设置进度回调函数
         @FullName:  TimelineWidget::setProgressGetter
         @Returns:   void
         @Parameter: ProgressGetter getter
        ************************************/
        void    setProgressGetter(ProgressGetter getter);

        typedef std::function<QPixmap(qint64 pos)> ThumbnailGetter;
        /*!************************************
         @Method:    setThumbnailGetter 设置缩略图回调函数
         @FullName:  TimelineWidget::setThumbnailGetter
         @Returns:   void
         @Parameter: ThumbnailGetter getter
        ************************************/
        void    setThumbnailGetter(ThumbnailGetter getter);

        void    ensureCursorVisible();
        void    ensurePlayPositionVisible(qint64 pos);
        void    seekTo(qint64 llPos);
        void    addMarkPoint(const TimelineMarkPoint& point);
        void    removeMarkPoint(qint64 llStart);
        void    clearMarkPoint();

        Q_SIGNAL    void    seeked(qint64 llSeekPos);
        Q_SIGNAL    void    backwardLoadSignal();
        Q_SIGNAL    void    forwardLoadSignal();

    private:

        /*!************************************
        @Method:    setBegin   设置时间轴偏移
        @FullName:  TimelineWidget::setOffset
        @Returns:   void
        @Parameter: qint64 llOffset  起始时间(单位:毫秒)
        ************************************/
        void    setOffset(qint64 llOffset);

        /*!************************************
         @Method:    repaintTimeline    刷新时间线绘制
         @FullName:  TimelineWidget::repaintTimeline
         @Returns:   void
        ************************************/
        void    repaintTimeline();

        /*!************************************
        @Method:    paintTimeText 绘制时间文本
        @FullName:  TimelineWidget::paintRuler
        @Returns:   void
        @Parameter: QPainter * painter
        ************************************/
        void    paintTimeText(QPainter* painter);

        /*!************************************
        @Method:    paintRuler 绘制标尺
        @FullName:  TimelineWidget::paintRuler
        @Returns:   void
        @Parameter: QPainter * painter
        ************************************/
        void    paintRuler(QPainter* painter);

        /*!************************************
         @Method:    paintColorSlice 绘制着色条
         @FullName:  TimelineWidget::paintColorSlice
         @Returns:   void
         @Parameter: QPainter * painter
        ************************************/
        void    paintColorSlice(QPainter* painter);

        /*!************************************
         @Method:    paintThumbnail 绘制缩略图
         @FullName:  TimelineWidget::paintThumbnail
         @Returns:   void
         @Parameter: QPainter * painter
        ************************************/
        void    paintThumbnail(QPainter* painter);

        /*!************************************
         @Method:    paintCursor    绘制游标
         @FullName:  TimelineWidget::paintCursor
         @Returns:   void
         @Parameter: QPainter * painter
        ************************************/
        void    paintCursor(QPainter* painter);

    private:
        QSize       scaleTextSizeHint(QPainter* painter);
        void        updateScrollBar();
        qint64      mapToPlayPostion(int iPos);             /* 映射播放时间点到控件像素位置 */
        int         mapToWidgetPosition(qint64 llPos);     /* 映射控件中的像素位置到播放时间点 */
        void        autoAdjustAccuracy();                   /* 自动校正精度 */
        void        autoUpdateCursorPosition();             /* 自动刷新游标位置 */

    protected:
        bool    eventFilter(QObject* obj, QEvent* event) override;
        void    resizeEvent(QResizeEvent* event) override;
        void    showEvent(QShowEvent *event) override;

    private:
        Ui::TimelineWidget *ui;
        QWidget*        _pCursorWidget = nullptr;   /* 游标控件 */

        int         _iRulerAccuracy = 100;      /* 默认1像素表示100ms */
        float       _fRulerCellWidth = 5.f;     /* 每个格子的像素宽度(范围 [5px,30px],动态变化效果还不太好，暂固定为5px)*/
        float       _fTimePerCell = 100.f;      /* 每个格子表示的时长 */

        qint64      _llTotalDuration = 60000;   /* 默认为60000毫秒(单位:毫秒) */
        qint64      _llPlayOffset = 0;          /* 时间轴偏移(单位:毫秒) */
        int         _iTimerID = 0;
        bool        _bAutoUpdateProgress = true;    /* 自动刷新进度 */
        bool        _bAutoAdjustOffset = true;      /* 自动校正偏移 */
        bool        _bDragScrollBar = false;

        ProgressGetter  _progressGetter = nullptr;
        ThumbnailGetter _thumbGetter = nullptr;

        QPoint      _cursorMouseOriginPoint;
        bool        _bCursorDragAvailable = false;
        bool        _bDragCursor = false;
        int         _iScaleSpacing;             /*! 刻度间距 */
        int         _markSliderHeight = 20;

        QList<TimelineMarkPoint>    _markPoints;
    };
};
