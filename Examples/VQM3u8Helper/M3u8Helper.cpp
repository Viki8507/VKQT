#include "M3u8Helper.h"
#include <QtNetwork/QNetworkReply>
#include <QProcess>
#include <QDir>
#include <QCoreApplication>

#define FileDownloadTimeout     5000
#define TSDownloadThreadCount   4

VKQT::M3u8Helper::M3u8Helper(QObject *parent)
    : QObject(parent)
{
    _mainDownloader = new FileDownloader(this);     /* 专门用来下载m3u8文件 */
    connect(_mainDownloader, SIGNAL(downloadFinished(const QString&, const QString&, const QString&)),
        this, SLOT(onFileDownloadFinished(const QString&, const QString&, const QString&)));

    _tsDownloaders.resize(TSDownloadThreadCount);
    for (int i = 0; i < TSDownloadThreadCount; i++)
    {
        _tsDownloaders[i] = new FileDownloader(this);
        connect(_tsDownloaders[i], SIGNAL(downloadFinished(const QString&, const QString&, const QString&)),
            this, SLOT(onFileDownloadFinished(const QString&, const QString&, const QString&)));
    }
}

VKQT::M3u8Helper::~M3u8Helper()
{
    stopDownload();
}

void VKQT::M3u8Helper::download(const QString& m3u8Url, const QString& downloadDir)
{
    if (!QDir(downloadDir).exists())
    {
        QDir(downloadDir).mkpath(downloadDir);
    }
    _m3u8Url = m3u8Url;
    _downloadDir = downloadDir;
    bool bRemoved = QFile(_downloadDir + "/m3u8.m3u8").remove();    /* 移除旧的 */
    _bManualCancel = false;
    _bStopDownload = false;
    _mainDownloader->downloadFile(_m3u8Url,  _downloadDir + "/m3u8.m3u8", 2);
}

void VKQT::M3u8Helper::stopDownload()
{
    _bStopDownload = true;
    if (_mainDownloader->isRunning())
        _mainDownloader->wait(10000);

    for (int i = 0; i < TSDownloadThreadCount; i++)
    {
        if (_tsDownloaders[i]->isRunning())
            _tsDownloaders[i]->wait(10000);
    }
}

void VKQT::M3u8Helper::cancelDownload()
{
    _bStopDownload = true;
    _bManualCancel = true;
    if (_mainDownloader)
    {
        _mainDownloader->cancel();
    }

    for (int i = 0; i < TSDownloadThreadCount; i++)
    {
        if (_tsDownloaders[i])
            _tsDownloaders[i]->cancel();
    }
}

QStringList VKQT::M3u8Helper::tsFileNames()
{
    return _allTsFileNames;
}

void VKQT::M3u8Helper::parseTimestamp(const QString& url, qint64& begin, qint64& end)
{
    int iBeginIndexFrom = url.indexOf("starttime=") + strlen("starttime=");
    int iBeginIndexEnd = url.lastIndexOf("&");
    QString beginString = url.mid(iBeginIndexFrom, iBeginIndexEnd - iBeginIndexFrom);
    begin = beginString.toLongLong();

    int iEndIndexFrom = url.lastIndexOf("endtime=") + strlen("endtime=");
    QString endString = url.mid(iEndIndexFrom, -1);
    end = endString.toLongLong();
}

QString VKQT::M3u8Helper::modifyM3u8Begin(const QString& url, qint64 llBegin)
{
    QString modifiedUrl = url;
    int iBeginIndexFrom = url.indexOf("starttime=") + strlen("starttime=");
    int iBeginIndexEnd = url.lastIndexOf("&");
    modifiedUrl.replace(iBeginIndexFrom, iBeginIndexEnd - iBeginIndexFrom, QString::number(llBegin));
    return modifiedUrl;
}

QString VKQT::M3u8Helper::modifyM3u8End(const QString& url, qint64 llBegin)
{
    QString modifiedUrl = url;
    int iEndIndexFrom = url.lastIndexOf("endtime=") + strlen("endtime=");
    modifiedUrl.replace(iEndIndexFrom, 10000, QString::number(llBegin));    /* 10000只是为了替换到尾部 */
    return modifiedUrl;
}

void VKQT::M3u8Helper::merge(const QString& m3u8FilePath, const QString& dstFilePath)
{
    if (!QFile(m3u8FilePath).exists())
    {
        emit mergeFinished(tr("M3U8转码失败:找不到m3u8文件."));
        return;
    }

    QFileInfo dstFileInfo(dstFilePath);
    if (!dstFileInfo.dir().exists() && !QDir().mkpath(dstFileInfo.dir().path()))
    {
        emit mergeFinished(tr("M3U8转码失败:创建目标文件失败."));
        return;
    }

    /* 删除旧的合成文件 */
    QFile oldFile(dstFilePath);
    oldFile.remove();

    bool bMergeComplete = false;
    QProcess* ffmpegProc = new QProcess();
    connect(ffmpegProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, ffmpegProc, &bMergeComplete](int exitCode, QProcess::ExitStatus state){
        qDebug() << "FFmpeg.exe exit: " << exitCode;
        if (QProcess::NormalExit == state)
            emit mergeFinished("");
        else
            emit mergeFinished(tr("转码失败"));
        bMergeComplete = true;
        ffmpegProc->deleteLater();
    });

    QString cmdLine = QString("ffmpeg.exe -i \"%1\" -c copy \"%2\"").arg(m3u8FilePath).arg(dstFilePath);
    ffmpegProc->start(cmdLine);

    while (!bMergeComplete)
    {
        QCoreApplication::processEvents();
    }
}

Q_SLOT void VKQT::M3u8Helper::onFileDownloadFinished(const QString& url,
    const QString& filePath, const QString& errStr)
{
    static QString errorString = "";
    if (!errStr.isEmpty())
    {
        _bStopDownload = true;
        errorString = errStr;
    }

    if (_mainDownloader == sender())
    {
        if (errorString.isEmpty())
        {
            _tsUrls = parseTsUrls();
            _tsUrls.erase(std::remove_if(_tsUrls.begin(), _tsUrls.end(), [this](const QString& url){
                QString tsFileName = QUrl(url).fileName();
                QString tsDstPath = _downloadDir + "/" + tsFileName;
                return QFile(tsDstPath).exists();
            }), _tsUrls.end());

            emit m3u8FileDownloaded(_tsUrls.count());
            _downloadTotal = _tsUrls.count();
            _downloaded = 0;

            for (int i = 0; i < TSDownloadThreadCount; i++)
            {
                if (!_tsUrls.isEmpty())
                {
                    QString tsUrl = _tsUrls.front();
                    _tsUrls.pop_front();
                    QString tsDstPath = _downloadDir + "/" + QUrl(tsUrl).fileName();
                    _tsDownloaders[i]->downloadFile(tsUrl, tsDstPath, 2);
                }
            }

            while (!_bStopDownload && _downloaded < _downloadTotal)
            {
                QCoreApplication::processEvents();
                QThread::msleep(1);
            }
        }

        emit downloadFinished(_bManualCancel ? tr("手动取消下载") : errorString);
        errorString = "";
    }
    else
    {
        ++_downloaded;
        emit progressChanged(_downloadTotal, _downloaded, url);
        FileDownloader* tsDownloader = static_cast<FileDownloader*>(sender());
        if (errorString.isEmpty() && !_bStopDownload && !_tsUrls.isEmpty())
        {
            QString tsUrl = _tsUrls.front();
            QString tsDstPath = _downloadDir + "/" + QUrl(tsUrl).fileName();
            _tsUrls.pop_front();
            tsDownloader->downloadFile(tsUrl, tsDstPath, 2);
        }
    }
}

QStringList VKQT::M3u8Helper::parseTsUrls()
{
    QFile m3u8File(_downloadDir + "/m3u8.m3u8");
    if (!m3u8File.exists() || !m3u8File.open(QIODevice::ReadOnly))
        return QStringList();

    _allTsFileNames.clear();
    QStringList tsUrls;
    QByteArray replacedM3u8Data;
    while (!m3u8File.atEnd())
    {
        QByteArray lineData = m3u8File.readLine();
        if (lineData.startsWith("http://"))
        {
            QString tsUrlString = QString::fromUtf8(lineData).trimmed();
            QUrl url(tsUrlString);
            tsUrls << tsUrlString;

            replacedM3u8Data.append(url.fileName() + "\r\n");
            _allTsFileNames.append(url.fileName());
        }
        else
        {
            replacedM3u8Data.append(lineData);
        }
    }
    m3u8File.close();

    m3u8File.open(QIODevice::WriteOnly);
    m3u8File.write(replacedM3u8Data, replacedM3u8Data.length());
    m3u8File.close();
    return tsUrls;
}

VKQT::FileDownloader::FileDownloader(QObject* parent /* = nullptr */)
    :QThread(parent)
{
    _networkManger.moveToThread(this);
    _timer.moveToThread(this);
    connect(&_timer, &QTimer::timeout, this, [this]{
        if (_pDowloadReply)
            _pDowloadReply->abort();
        if (_pFile)
            _pFile->close();
        _err = tr("超时");
        qDebug() << "FileDownloader ERR:" << _err;
    });
}

void VKQT::FileDownloader::downloadFile(const QString& url, const QString& savePath, int iRetrycount /* = 0 */)
{
    _srcUrl = url;
    _savePath = savePath;
    _iRetryCount = iRetrycount;
    wait();
    start();
}

void VKQT::FileDownloader::cancel()
{
    _err = tr("手动取消下载");
    emit cancelSignal();
}

QString VKQT::FileDownloader::error()
{
    return _err;
}

bool VKQT::FileDownloader::downloadInit()
{
    Q_ASSERT(QThread::currentThread() == this);
    _err = "";
    _pFile = new QFile(_savePath + ".tmp");
    if (_pFile->exists())
        _pFile->remove();
    
    if (!_pFile->open(QIODevice::WriteOnly))
    {
        _err = tr("创建临时文件失败!");
        delete _pFile;
        _pFile = nullptr;
        downloadUnInit(tr("M3U8文件下载器创建临时文件失败!"));
        return false;
    }

    QNetworkRequest downloadRequest;
    downloadRequest.setUrl(_srcUrl);
    downloadRequest.setRawHeader(QByteArray("user-agent"), QByteArray("tm.2021.live"));
    _pDowloadReply = _networkManger.get(downloadRequest);
    connect(this, &FileDownloader::cancelSignal, _pDowloadReply, [this]{
        _pDowloadReply->abort();
    });
    return true;
}

void VKQT::FileDownloader::downloadUnInit(const QString& err)
{
    Q_ASSERT(QThread::currentThread() == this);
    _err = err;
    _timer.stop();
    _bTimeout = false;
    if (_pFile)
    {
        _pFile->close();
        if (_err.isEmpty())
        {
            QString oldName = _pFile->fileName();
            if (!_pFile->rename(oldName.remove(".tmp")))
            {
                qDebug() << "Rename failed.";
            }
        }
        else
        {
            _pFile->remove();
        }
        _pFile = nullptr;
    }

    _pDowloadReply = nullptr;
    emit downloadFinished(_srcUrl, _savePath, _err);
    quit();
}

void VKQT::FileDownloader::run()
{
    if (!downloadInit())
        return;
    
    connect(_pDowloadReply, &QNetworkReply::readyRead, [this]{
        _timer.stop();
        while (!_pDowloadReply->atEnd())
        {
            QByteArray ba = _pDowloadReply->readAll();
            _pFile->write(ba);
        }

        /* 如果5秒内还没有新数据到来，认为获取数据超时 */
        _timer.start(FileDownloadTimeout);
    });

    connect(_pDowloadReply, &QNetworkReply::finished, [this]{
        disconnect(_pDowloadReply);
        _pDowloadReply->deleteLater();
        _pFile->deleteLater();

        if (_bTimeout)
        {
            if (_iRetryCount > 0)
            {
                --_iRetryCount;
                if (!downloadInit())
                    return;
            }
            else
            {
                downloadUnInit(tr("文件下载超时!"));
            }
        }
        else
        {
            downloadUnInit(_err);
        }
    }); 

    /*! 异常 ***/
    connect(_pDowloadReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
        [this](QNetworkReply::NetworkError code){
        if (code != QNetworkReply::NoError)
        {
            static QMap<QNetworkReply::NetworkError, int> HttpErrorMap = {
                {QNetworkReply::ContentAccessDenied, 403 },
                { QNetworkReply::ContentNotFoundError, 404 }
            };
            /* 删除临时现在文件 */
            QFile(_savePath + ".tmp").remove();
            int httpError = HttpErrorMap.contains(code) ? HttpErrorMap[code] : (int)code;
            _err = tr("M3U8数据获取失败. 信息:") + _pDowloadReply->errorString();
        }
    });
    exec();
}
