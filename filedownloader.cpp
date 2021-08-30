#include "filedownloader.h"

#include <QFile>

FileDownloader::FileDownloader(QObject *parent) :
 QObject(parent)
{
    //
}

FileDownloader::~FileDownloader() { }

void FileDownloader::download(QString fileUrl){
    this->isDownloading = true;
    this->fileData.clear();

    if(!this->n_manager){
        this->n_manager = new QNetworkAccessManager();
        connect(this->n_manager, &QNetworkAccessManager::finished, this, &FileDownloader::fileDownloaded);
    }

    qDebug() << "Downloading:" << fileUrl << "...";

    QUrl url = QUrl(fileUrl);
    QNetworkRequest request(url);
    this->reply = this->n_manager->get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, &FileDownloader::fileDownloadProgress);
}

void FileDownloader::downloadAndSave(QString fileUrl, QString filePath)
{
    if(this->isDownloading) return;

    this->saveDownloadedFile = true;
    this->targetFilePath = filePath;
    this->isDownloadedAndSaved = false;
    this->download(fileUrl);
}

void FileDownloader::stop()
{
    this->reply->abort();
    this->isDownloading = false;
    this->saveDownloadedFile = false;
}

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {

    if(pReply->error()){
        qDebug() << "Eroor Downloading!" << pReply->errorString();
    }
    else{
        this->fileData = pReply->readAll();
    }

    qDebug() << "Downloaded data size:" << this->fileData.length();
    pReply->deleteLater();

    //Save file
    if(this->saveDownloadedFile && this->fileData.length() > 0){
        QFile saveFile(this->targetFilePath);
        if( saveFile.open(QIODevice::WriteOnly) ){
            qint64 ret = saveFile.write(this->fileData);
            if(ret == -1){
                qDebug() << "Error saving file:" << this->targetFilePath;
                this->isDownloadedAndSaved = false;
            }
            else qDebug() << "File Saved:" << this->targetFilePath;
            this->isDownloadedAndSaved = true;
            emit this->downloadedAndSaved();
        }
        else{
            qDebug() << "Error opening file:" << this->targetFilePath;
            this->isDownloadedAndSaved = false;
        }

        saveFile.close();
    }

    this->saveDownloadedFile = false;
    this->isDownloading = false;
    emit this->downloaded();
}

void FileDownloader::fileDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if(bytesTotal <= 0) return;

    int percentage = bytesReceived / bytesTotal * 100;
    emit this->progress(percentage);
}

QByteArray FileDownloader::downloadedData() const {
    return this->fileData;
}

int FileDownloader::downloadedDataLength()
{
    return this->fileData.length();
}

