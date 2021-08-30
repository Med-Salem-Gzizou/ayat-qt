#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>


class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader(QObject *parent = 0);
    virtual ~FileDownloader();

    void download(QString fileUrl);
    void downloadAndSave(QString fileUrl, QString filePath);
    void stop();

    QByteArray downloadedData() const;
    int downloadedDataLength();

    bool isDownloadedAndSaved = false;

private:
    QNetworkAccessManager *n_manager = nullptr;
    QByteArray fileData;
    QNetworkReply *reply;

    bool isDownloading = false;
    bool saveDownloadedFile = false;
    QString targetFilePath;

signals:
    void progress(int percentage);
    void downloaded();
    void downloadedAndSaved();

private slots:
    void fileDownloaded(QNetworkReply* pReply);
    void fileDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

};

#endif // FILEDOWNLOADER_H
