#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QMediaPlayer>

#include "dbhandler.h"
#include "downloaddialog.h"
#include "dialogrestart.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    bool setAya(unsigned int sura_number, unsigned int aya_number, unsigned int tafsir_index);
    bool setAutoRecite(bool s);
    void loadInstalledRecitations();
    void read_selected_aya();

private:
    Ui::MainWindow *ui;
    DownloadDialog *downloadDialog = nullptr;
    DialogRestart *dialogRestart = nullptr;

    DbHandler *dbHandler;
    QMediaPlayer *media_player = nullptr;
    FileDownloader *fileDownloader = nullptr;

    void loadUiData();
    void save_settings();

    // app stats
    unsigned int selectedSuraNumber = 0;
    unsigned int selectedAyaNumber = 0;
    unsigned int selectedTafsirIndex = 0;
    bool readAyatOnClick = false;
    bool autoAudioDownload = true;
    QString selectedRecitationDir = "Abdul_Basit_Murattal_64kbps";
    bool autoRecite = false;
    // ui stats
    QString  appLang;
    bool uiDisplaySearchTool;
    bool uiDisplayTafsirTool;

    // consts

private slots:
    void on_sura_name_click(int row, int column);
    void on_aya_click(QListWidgetItem *item);
    void on_tafsir_change(int index);
    void on_aya_spinbox_changed(int i);
    void on_sura_spinbox_changed(int i);
    void on_aya_lineEdit_changed(QString text);
    void on_sura_lineEdit_changed(QString text);
    void on_recite_button_click(bool checked);
    // actions
    void copy_selected_ayat();
    void export_search_result();
    void on_show_search_act(bool checked);
    void on_show_tafsir_act(bool checked);
    void on_download_act();
    void on_readAyaOC_act(bool checked);
    void on_recitation_select(QAction *action);
    void on_autoDownload_act(bool checked);
    void on_langArabic_act();
    void on_langEnglish_act();
    // ui
    void on_spliter1_moved(int pos, int index);
    void on_spliter2_moved(int pos, int index);
    // search tool
    void on_searchTool_query(QString text);
    void on_searchTool_rowClick(int row, int column);
    // media
    void on_MediaStateChanged(QMediaPlayer::State state);

};
#endif // MAINWINDOW_H
