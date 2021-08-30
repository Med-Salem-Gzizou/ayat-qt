#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include "dbhandler.h"
#include "filedownloader.h"

#include <QDialog>
#include <QTreeWidgetItem>

struct DownloadTask {
    QString audioUrl;
    QString recitationDir;
    QString audioFilePath;
};

namespace Ui {
class DownloadDialog;
}

class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DownloadDialog(QWidget *parent = nullptr);
    ~DownloadDialog();

private:
    Ui::DownloadDialog *ui;
    DbHandler *dbHandler = nullptr;
    FileDownloader *fileDownloader = nullptr;

    // State
    QList<DownloadTask> downloadTasks;
    int currentDownloadTask;
    bool isDownloading = false;

public slots:
    void load_recitations_list();
    void update_recitations_list();
    void setDownloadMode(bool isDownloading);


signals:
    void recitationsListChanged();

private slots:
    void on_downloadButton_click();
    void on_uncheckButton_click();
    void on_deleteButton_click();
    void on_cancelButton_click();
    void on_recitItem_click(QTreeWidgetItem *item, int column);
    void on_fileDownloaded();

};

#endif // DOWNLOADDIALOG_H
