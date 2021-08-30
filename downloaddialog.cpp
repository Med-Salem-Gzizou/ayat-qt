#include "downloaddialog.h"
#include "ui_downloaddialog.h"

#include "common.h"

#include <QDebug>
#include <QDir>

DownloadDialog::DownloadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DownloadDialog)
{
    ui->setupUi(this);
    this->ui->recitations_treeWidget->setColumnWidth(0, 400);
    this->load_recitations_list();

    this->fileDownloader = new FileDownloader(this);
    connect(this->fileDownloader, &FileDownloader::progress, this->ui->file_progressBar, &QProgressBar::setValue);
    connect(this->fileDownloader, &FileDownloader::downloaded, this, &DownloadDialog::on_fileDownloaded);

    // Signals
    connect(this->ui->downloadButton, &QAbstractButton::clicked, this, &DownloadDialog::on_downloadButton_click);
    connect(this->ui->fetchButton, &QAbstractButton::clicked, this, &DownloadDialog::update_recitations_list);
    connect(this->ui->uncheckButton, &QPushButton::clicked, this, &DownloadDialog::on_uncheckButton_click);
    connect(this->ui->deleteButton, &QPushButton::clicked, this, &DownloadDialog::on_deleteButton_click);
    connect(this->ui->cancelButton, &QAbstractButton::clicked, this, &DownloadDialog::on_cancelButton_click);
    connect(this->ui->recitations_treeWidget, &QTreeWidget::itemClicked, this, &DownloadDialog::on_recitItem_click);
}

DownloadDialog::~DownloadDialog()
{
    delete ui;
}

void DownloadDialog::load_recitations_list()
{
    this->ui->recitations_treeWidget->clear();

    if(!this->dbHandler) this->dbHandler = new DbHandler("download_con");
    QList<Recitation> recitations = this->dbHandler->getRecitationsList();
    QList<QString> suwar_names = this->dbHandler->getSuwarNames();

    for(int r=0; r<recitations.length(); r++){
        QTreeWidgetItem *rItem = new QTreeWidgetItem(this->ui->recitations_treeWidget);
        rItem->setText(0, recitations.at(r).name);
        rItem->setText(1, recitations.at(r).dir);

        // Check if recitation exists
        QDir recitationDir = QDir( getRecitationDirPath(recitations.at(r).dir) ) ;
        bool rDirExists = recitationDir.exists();
        if( rDirExists ) rItem->setCheckState(0, Qt::PartiallyChecked);
        else rItem->setCheckState(0, Qt::Unchecked);

        //load suwar list
        for(int s=0; s<suwar_names.length(); s++){
            QTreeWidgetItem *suraItem = new QTreeWidgetItem(rItem);
            suraItem->setText(0,  suwar_names.at(s));
            suraItem->setText(1, QString::number(s+1));

            if(!rDirExists){
                suraItem->setCheckState(0, Qt::Unchecked);
                continue;
            }

            bool allfilesExists = true;
            bool oneFileExists = false;
            QStringList audiosFiles = getSuraAudiosPathsList(recitations.at(r).dir, s+1);
            for (int f=0; f<audiosFiles.length(); f++) {
                if( QFile::exists(audiosFiles.at(f)) ) oneFileExists = true;
                else allfilesExists = false;
            }

            //if(allfilesExists) suraItem->setCheckState(0, Qt::Checked);
            if(oneFileExists) suraItem->setCheckState(0, Qt::PartiallyChecked);
            else suraItem->setCheckState(0, Qt::Unchecked);

        }
    }

}

void DownloadDialog::update_recitations_list()
{
    int recitCount = this->ui->recitations_treeWidget->topLevelItemCount();
    for (int r=0; r<recitCount; r++) {
        QTreeWidgetItem *recitItem = this->ui->recitations_treeWidget->topLevelItem(r);
        QDir dir( getRecitationDirPath(recitItem->text(1)) );
        bool rDirExists = dir.exists();
        if(rDirExists) recitItem->setCheckState(0, Qt::PartiallyChecked);
        else recitItem->setCheckState(0, Qt::Unchecked);

        for (int s=0; s<recitItem->childCount(); s++) {
            if(rDirExists){
                bool oneFileExists = false;
                QStringList audioFiles = getSuraAudiosPathsList(recitItem->text(1), s+1);
                for(int a=0; a<audioFiles.length(); a++){
                    if( QFile::exists(audioFiles.at(a)) ){
                        oneFileExists = true;
                        break;
                    }
                }
                if(oneFileExists){
                    recitItem->child(s)->setCheckState(0, Qt::PartiallyChecked);
                    continue;
                }
            }
            recitItem->child(s)->setCheckState(0, Qt::Unchecked);
        }
    }
}

void DownloadDialog::setDownloadMode(bool isDownloading)
{
    this->ui->downloadButton->setEnabled(!isDownloading);
    this->ui->fetchButton->setEnabled(!isDownloading);
    this->ui->uncheckButton->setEnabled(!isDownloading);
    this->ui->deleteButton->setEnabled(!isDownloading);
    this->ui->cancelButton->setEnabled(isDownloading);
    this->ui->recitations_treeWidget->setEnabled(!isDownloading);
    if(isDownloading) this->ui->global_progressBar->setStyleSheet("");
    this->isDownloading = isDownloading;
    if(!isDownloading) this->update_recitations_list();
}

void DownloadDialog::on_downloadButton_click()
{
    if(this->isDownloading) return;
    this->setDownloadMode(true);

    // Create download tasks list
    this->downloadTasks.clear();
    int recitCount = this->ui->recitations_treeWidget->topLevelItemCount();
    for (int r=0; r<recitCount; r++) {
        QTreeWidgetItem *recitItem = this->ui->recitations_treeWidget->topLevelItem(r);

        for(int s=0; s<recitItem->childCount(); s++){
            if(!recitItem->child(s)->checkState(0)) continue;

            for(int a=0; a<ayahCount[s]; a++){
                DownloadTask task;
                task.recitationDir = recitItem->text(1);
                task.audioFilePath = getAudioFilePath(task.recitationDir, s+1, a+1);
                task.audioUrl = getAudioUrl(task.recitationDir, s+1, a+1);
                this->downloadTasks.append(task);
            }
        }
    }

    // Create the required folders
    for(int t=0; t<this->downloadTasks.length(); t++){
        QDir dir( getRecitationDirPath(this->downloadTasks.at(t).recitationDir) );
        if( !dir.exists() ){
            qDebug() << "Folder not exist:" << dir.path();
            if( !dir.mkpath(dir.path()) ) qDebug() << "Error mkpath:" << dir.path();
        }
    }
    emit this->recitationsListChanged();

    // Check for the first missing file
    int i;
    for (i=0; i < this->downloadTasks.length(); i++) {
        qDebug() << "Checking:" << this->downloadTasks.at(i).audioFilePath;
        bool exists = QFile::exists(this->downloadTasks.at(i).audioFilePath);
        if(!exists) break;
        qDebug() << "File Exists";
    }
    this->currentDownloadTask = i;

    if( this->currentDownloadTask >= this->downloadTasks.length() ){
        qDebug() << "All Files Exists: nothing to download!";
        // Update ui
        this->setDownloadMode(false);
        this->ui->global_progressBar->setValue(100);
        return;
    }

    // Start downloading
    qDebug() << "downloading ... ";
    const QString *audioUrl = &this->downloadTasks.at(this->currentDownloadTask).audioUrl;
    const QString *audioPath = &this->downloadTasks.at(this->currentDownloadTask).audioFilePath;
    this->fileDownloader->downloadAndSave(*audioUrl, *audioPath);
}

void DownloadDialog::on_uncheckButton_click()
{
    for (int i=0; i<this->ui->recitations_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = this->ui->recitations_treeWidget->topLevelItem(i);
        item->setCheckState(0, Qt::Unchecked);
        item->setExpanded(false);
        for (int s=0; s<item->childCount(); s++) {
            item->child(s)->setCheckState(0, Qt::Unchecked);
        }
    }
}

void DownloadDialog::on_deleteButton_click()
{

    int itemsCount = this->ui->recitations_treeWidget->topLevelItemCount();
    for(int i=0; i<itemsCount; i++){
        QTreeWidgetItem *recitItem = this->ui->recitations_treeWidget->topLevelItem(i);
        if(recitItem->checkState(0) == Qt::Checked){
            //delete recitation directory
            QDir dir( getRecitationDirPath(recitItem->text(1)) );
            qDebug() << "Deleting" << dir.path() << "...";
            if( dir.removeRecursively() ) qDebug("Deleted");
            else qDebug("Error Deleting");
            continue;
        }

        for (int s=0; s<recitItem->childCount(); s++) {
            if(recitItem->child(s)->checkState(0) == Qt::Checked){
                // delete sura audios
                QStringList audiosPaths = getSuraAudiosPathsList(recitItem->text(1), s+1);
                for (int f=0; f<audiosPaths.length(); f++) {
                    if(QFile(audiosPaths.at(f)).remove()) qDebug() << "Deleted:" << audiosPaths.at(f);
                    else qDebug() << "Error removing file:" << audiosPaths.at(f);
                }
            }
        }

    }

    emit this->recitationsListChanged();
    this->update_recitations_list();
}

void DownloadDialog::on_cancelButton_click()
{
    qDebug() << "Canceleding Download Tasks ...";

    this->isDownloading = false;
    this->fileDownloader->stop();

    this->setDownloadMode(false);
}

void DownloadDialog::on_recitItem_click(QTreeWidgetItem *item, int column)
{
    Qt::CheckState state = item->checkState(0);
    for (int i=0; i<item->childCount(); i++) {
        item->child(i)->setCheckState(0, state);
    }
}

void DownloadDialog::on_fileDownloaded()
{
    if(!this->isDownloading) return;

    //Check for download Errors
    if(!this->fileDownloader->isDownloadedAndSaved){
        this->ui->global_progressBar->setStyleSheet("selection-background-color: rgb(237, 51, 59);");
        //qDebug() << "Error Downloading:" << this->downloadTasks.at(this->currentDownloadTask).audioFilePath;
    }

    this->currentDownloadTask += 1;

    // Check for the first missing file
    int i;
    for (i=currentDownloadTask; i<this->downloadTasks.length(); i++) {
        qDebug() << "Checking:" << this->downloadTasks.at(i).audioFilePath;
        bool exists = QFile::exists(this->downloadTasks.at(i).audioFilePath);
        if(!exists) break;
        qDebug() << "File Exists";
    }
    this->currentDownloadTask = i;

    // update progress bar
    float progress = (float) this->currentDownloadTask / (float) this->downloadTasks.length() * 100 ;
    int globalProgress = (int) progress;
    this->ui->global_progressBar->setValue(globalProgress);
    qDebug() << "global progress: %" << globalProgress << this->currentDownloadTask << this->downloadTasks.length();

    // run the next task if available
    if(this->currentDownloadTask >= this->downloadTasks.length()){
        qDebug() << "Downloads Tasks completed!";
        this->setDownloadMode(false);
        return;
    }
    const QString *audioUrl = &this->downloadTasks.at(this->currentDownloadTask).audioUrl;
    const QString *audioPath = &this->downloadTasks.at(this->currentDownloadTask).audioFilePath;
    this->fileDownloader->downloadAndSave(*audioUrl, *audioPath);
}
