#pragma once

#include <QObject>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtCore/QFile>
#include <QtCore/QThread>
#include <QtCore/QTimer>

namespace VKQT{
    class FileDownloader;
    class M3u8Helper : public QObject
    {
        Q_OBJECT

    public:
        M3u8Helper(QObject *parent);
        ~M3u8Helper();

        void            download(const QString& url, const QString& downloadDir);
        void            stopDownload();     /*! 不强制终止正在下载的文件，但是没有开始下载的不再下载 */
        void            cancelDownload();   /*! 强制终止正在现在的文件，没有开始现在的也不再下载*/
        QStringList     tsFileNames();
        void            merge(const QString& m3u8FilePath, const QString& dstFilePath);

        static void     parseTimestamp(const QString& url, qint64& begin, qint64& end);
        static QString  modifyM3u8Begin(const QString& url, qint64 llBegin);
        static QString  modifyM3u8End(const QString& url, qint64 llBegin);

        Q_SLOT   void   onFileDownloadFinished(const QString& url, const QString& filePath, const QString& err);
        Q_SIGNAL void   m3u8FileDownloaded(int iFileTotal);
        Q_SIGNAL void   progressChanged(int iFileTotal, int iCurrentIndex, QString iCurrFileName);
        Q_SIGNAL void   error(const QString& filePath, const QString&);
        Q_SIGNAL void   downloadFinished(const QString& err);
        Q_SIGNAL void   mergeFinished(const QString& err);

    private:

        QStringList     parseTsUrls();

    private:

        QString         _m3u8Url;
        QString         _downloadDir;
        QStringList     _tsUrls;
        int             _downloadTotal = 0;
        int             _downloaded = 0;
        FileDownloader* _mainDownloader = nullptr;
        QVector<FileDownloader*> _tsDownloaders;        /* TS文件下载线程*/
        bool            _bStopDownload = true;
        QStringList     _allTsFileNames;
        bool            _bManualCancel = false; /* 是否手动取消 */
    };

    class FileDownloader : public QThread
    {
        Q_OBJECT

    public:
        FileDownloader(QObject* parent = nullptr);

        void            downloadFile(const QString& url, const QString& savePath, int iRetrycount = 0);
        void            cancel();
        QString         error();

        Q_SIGNAL void   downloadFinished(const QString& srcUrl, const QString& dstFile, const QString& err);
        Q_SIGNAL void   cancelSignal();

    private:
        bool            downloadInit();
        void            downloadUnInit(const QString& err);

    protected:
        void            run();

    private:
        QNetworkAccessManager   _networkManger;
        QNetworkReply*          _pDowloadReply = nullptr;
        QFile*                  _pFile = nullptr;

        QString         _srcUrl;
        QString         _savePath;
        QString         _err = "";

        QTimer          _timer;
        bool            _bTimeout = false;
        int             _iRetryCount = 0;
    };
}
