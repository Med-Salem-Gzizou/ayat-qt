#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "common.h"
#include "settingsKeys.h"

#include <QListWidgetItem>
#include <QDebug>
#include <QSettings>
#include <QClipboard>
#include <QKeyEvent>
#include <QFileDialog>
#include <QDirIterator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QActionGroup *langActionGroup = new QActionGroup(this);
    langActionGroup->addAction(this->ui->langArabicAct);
    langActionGroup->addAction(this->ui->langEnglishAct);
    // Setup splitters
    this->ui->splitter_1->setCollapsible(0, false);
    this->ui->splitter_2->setCollapsible(0, false);

    // Setup suwar list
    this->ui->suwarList_tableWidget->setColumnCount(1);
    this->ui->suwarList_tableWidget->horizontalHeader()->setStretchLastSection(true);
    // Setup search tool
    this->ui->searchTool_tableWidget->setColumnCount(4);
    QStringList headerList = QStringList({tr("Sura"), tr("Sura Name"), tr("Aya"), tr("Aya Text")});
    this->ui->searchTool_tableWidget->setHorizontalHeaderLabels(headerList);
    this->ui->searchTool_tableWidget->setColumnWidth(0, 40);
    this->ui->searchTool_tableWidget->setColumnWidth(1, 150);
    this->ui->searchTool_tableWidget->setColumnWidth(2, 40);
    this->ui->searchTool_tableWidget->horizontalHeader()->setStretchLastSection(true);
    this->ui->searchTool_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    this->loadUiData();

    //
    connect(ui->suwarList_tableWidget, &QTableWidget::cellClicked, this, &MainWindow::on_sura_name_click);
    connect(ui->suwarList_tableWidget, &QTableWidget::cellActivated, this, &MainWindow::on_sura_name_click);
    connect(ui->ayatList_listWidget, &QListWidget::itemActivated, this, &MainWindow::on_aya_click);
    connect(ui->ayatList_listWidget, &QListWidget::itemClicked, this, &MainWindow::on_aya_click);
    connect(ui->ayatList_listWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::on_aya_selected);
    connect(ui->tafasirNames_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_tafsir_change);
    connect(ui->selectSura_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::on_sura_spinbox_changed);
    connect(ui->selectAya_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::on_aya_spinbox_changed);
    connect(ui->selectAya_lineEdit, &QLineEdit::textEdited, this, &MainWindow::on_aya_lineEdit_changed);
    connect(ui->selectSura_lineEdit, &QLineEdit::textEdited, this, &MainWindow::on_sura_lineEdit_changed);
    connect(ui->reciteButton, &QAbstractButton::clicked, this, &MainWindow::on_recite_button_click);
    // actions
    connect(ui->exitAct, &QAction::triggered, this, &MainWindow::close);
    connect(ui->copySelectedAct, &QAction::triggered, this, &MainWindow::copy_selected_ayat);
    connect(ui->exportSearchAct, &QAction::triggered, this, &MainWindow::export_search_result);
    connect(ui->showSearchAct, &QAction::toggled, this, &MainWindow::on_show_search_act);
    connect(ui->showTafsirAct, &QAction::toggled, this, &MainWindow::on_show_tafsir_act);
    connect(ui->downloadAudioAct, &QAction::triggered, this, &MainWindow::on_download_act);
    connect(ui->readAyatOnClickAct, &QAction::toggled, this, &MainWindow::on_readAyaOC_act);
    connect(ui->autoDownloadAct, &QAction::toggled, this, &MainWindow::on_autoDownload_act);
    connect(ui->langArabicAct, &QAction::triggered, this, &MainWindow::on_langArabic_act);
    connect(ui->langEnglishAct, &QAction::triggered, this, &MainWindow::on_langEnglish_act);
    // ui
    connect(ui->splitter_1, &QSplitter::splitterMoved, this, &MainWindow::on_spliter1_moved);
    connect(ui->splitter_2, &QSplitter::splitterMoved, this, &MainWindow::on_spliter2_moved);
    // search tool
    connect(ui->searchTool_lineEdit, &QLineEdit::textEdited, this, &MainWindow::on_searchTool_query);
    connect(ui->searchTool_tableWidget, &QTableWidget::cellClicked, this, &MainWindow::on_searchTool_rowClick);

}

void MainWindow::loadUiData(){

    // Load suwar List
    this->ui->suwarList_tableWidget->clear();
    this->dbHandler = new DbHandler("main_con");
    QList<QString> suwar_names = this->dbHandler->getSuwarNames();
    this->ui->selectSura_spinBox->setMaximum(suwar_names.length());
    this->ui->suwarList_tableWidget->setRowCount(suwar_names.length());
    for (int i=0; i<suwar_names.length(); i++) {
        QTableWidgetItem *item = new QTableWidgetItem(suwar_names.at(i));
        item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        this->ui->suwarList_tableWidget->setItem(i, 0, item);
    }

    // Load tafasir List
    this->ui->tafasirNames_comboBox->clear();
    QList<QString> tafasir_names = dbHandler->getTafasirNames();
    qDebug() << "tafasir_names length: " << tafasir_names.length();
    for (int i=0; i<tafasir_names.length(); i++) {
        this->ui->tafasirNames_comboBox->addItem(tafasir_names.at(i));
    }

    // Load user settings and ui state values
    QSettings settings(ORG_NAME_KEY, APP_NAME_KEY);
    qDebug() << "Settings file: " << settings.fileName() ;
    // Get values
    QString appLang = settings.value(APP_LANG_KEY).toString();
    this->restoreGeometry(settings.value(GEOMETRY_KEY).toByteArray());
    this->restoreState(settings.value(STATE_KEY).toByteArray());
    this->ui->splitter_1->restoreState(settings.value(SPLITER1_KEY).toByteArray());
    this->ui->splitter_2->restoreState(settings.value(SPLITER2_KEY).toByteArray());
    int selected_sura_number = settings.value(S_SURA_KEY).toInt();
    int selected_aya_number = settings.value(S_AYA_KEY).toInt();
    int selected_tafsir_index = settings.value(S_TAFSIR_KEY).toInt();
    bool read_aya_on_click = settings.value(READ_OC_KEY).toBool();
    bool auto_audio_download = settings.value(AUTO_DONWLOAD_KEY).toBool();
    QString selected_recitation = settings.value(S_RECIT_KEY).toString();

    // Check loaded values
    if(selected_sura_number <= 0 || selected_sura_number > 114)
        selected_sura_number = 1;
    if(selected_aya_number <= 0)
        selected_aya_number = 1;
    if(selected_tafsir_index < 0)
        selected_tafsir_index = 0;

    // lang
    if(appLang == "AR"){
        this->appLang = appLang;
        this->ui->langArabicAct->setChecked(true);
    }
    else{
        this->appLang = "EN";
        this->ui->langEnglishAct->setChecked(true);
    }
    // Read aya on click
    this->readAyatOnClick = read_aya_on_click;
    this->ui->readAyatOnClickAct->setChecked(read_aya_on_click);
    // Auto Download
    this->autoAudioDownload = auto_audio_download;
    this->ui->autoDownloadAct->setChecked(auto_audio_download);
    // Tools
    if(this->ui->splitter_1->sizes().at(1) == 0 && this->ui->splitter_1->sizes().at(0) != 0){
        this->ui->showTafsirAct->setChecked(false);
        this->uiDisplayTafsirTool = false;
    }
    else{
        this->ui->showTafsirAct->setChecked(true);
        this->uiDisplayTafsirTool = true;
    }
    if(this->ui->splitter_2->sizes().at(1) == 0 && this->ui->splitter_2->sizes().at(0) != 0){
        this->ui->showSearchAct->setChecked(false);
        this->uiDisplaySearchTool = false;
    }
    else{
        this->ui->showSearchAct->setChecked(true);
        this->uiDisplaySearchTool = true;
    }
    // Recitations
    if(!selected_recitation.isEmpty()){
        this->selectedRecitationDir = selected_recitation;
    }
    this->loadInstalledRecitations();
    // Aya
    this->setAya(selected_sura_number, selected_aya_number, selected_tafsir_index);
}

bool MainWindow::setAya(unsigned int sura_number, unsigned int aya_number, unsigned int tafsir_index){

    if(sura_number != this->selectedSuraNumber){
        qDebug() << "Set selected sura number: " << sura_number;
        if(sura_number > (unsigned) this->ui->selectSura_spinBox->maximum()){
            qDebug() << "[Error] Sura not exist";
            return false;
        }
        this->selectedSuraNumber = sura_number;
        this->selectedAyaNumber = 0;

        this->ui->selectSura_spinBox->setValue(sura_number);
        this->ui->suwarList_tableWidget->setCurrentCell(sura_number-1, 0);

        QList<QString> ayat_list = dbHandler->getSura(sura_number);
        this->ui->ayatList_listWidget->clear();
        for (int i=0; i<ayat_list.length(); i++) {
            QString aya_text = "{" + QString::number(i+1) + "} " + ayat_list.at(i);
            this->ui->ayatList_listWidget->addItem(aya_text);
        }

        this->ui->selectAya_spinBox->setMaximum(ayat_list.length());
    }

    bool update_tafsir = false;

    if(tafsir_index != this->selectedTafsirIndex){
        qDebug() << "Set selected tafsir: " << tafsir_index;
        if(tafsir_index >= (unsigned) this->ui->tafasirNames_comboBox->count()){
            qDebug() << "[Error] Tafsir not exist";
            return false;
        }
        this->selectedTafsirIndex = tafsir_index;
        this->ui->tafasirNames_comboBox->setCurrentIndex(tafsir_index);

        update_tafsir = true;
    }

    if(aya_number != this->selectedAyaNumber){
        qDebug() << "Set selected aya number: " << aya_number;
        if(aya_number > (unsigned) this->ui->selectAya_spinBox->maximum()){
            qDebug() << "[Error] Aya number not exist";
            return false;
        }
        this->selectedAyaNumber = aya_number;

        this->ui->ayatList_listWidget->setCurrentRow(aya_number - 1);
        this->ui->selectAya_spinBox->setValue(aya_number);

        update_tafsir = true;
    }

    if(update_tafsir)
        this->ui->tafsir_textBrowser->setText( dbHandler->getTafsir(sura_number, aya_number, tafsir_index + 1) );

    return true;
}

bool MainWindow::setAutoRecite(bool s){
    if(s) this->ui->reciteButton->setText(tr("Pause"));
    else this->ui->reciteButton->setText(tr("Recite"));
    this->autoRecite = s;
    return true;
}

void MainWindow::save_settings() {
    QSettings settings(ORG_NAME_KEY, APP_NAME_KEY);
    qDebug("[save_setting] ...");

    // save values
    settings.setValue(APP_LANG_KEY, this->appLang);
    settings.setValue(GEOMETRY_KEY, this->saveGeometry());
    settings.setValue(STATE_KEY, this->saveState());
    settings.setValue(SPLITER1_KEY, this->ui->splitter_1->saveState());
    settings.setValue(SPLITER2_KEY, this->ui->splitter_2->saveState());
    settings.setValue(S_SURA_KEY, this->selectedSuraNumber);
    settings.setValue(S_AYA_KEY, this->selectedAyaNumber);
    settings.setValue(S_TAFSIR_KEY, this->selectedTafsirIndex);
    settings.setValue(READ_OC_KEY, this->readAyatOnClick);
    settings.setValue(S_RECIT_KEY, this->selectedRecitationDir);
    settings.setValue(AUTO_DONWLOAD_KEY, this->autoAudioDownload);
}

void MainWindow::read_selected_aya() {
    qDebug("[read_selected_aya] sura: %d aya: %d", this->selectedSuraNumber, this->selectedAyaNumber);
    QString audioFilePath = getAudioFilePath(this->selectedRecitationDir, this->selectedSuraNumber, this->selectedAyaNumber);

    if( !QFile::exists(audioFilePath) ){

        // Download the missing audio file
        if(this->autoAudioDownload){
            this->ui->statusbar->showMessage(tr("Downloading missing audio ..."));

            // init fileDownloader
            if(!this->fileDownloader){
                this->fileDownloader = new FileDownloader();
                connect(this->fileDownloader, &FileDownloader::downloadedAndSaved, this, &MainWindow::read_selected_aya);
            }

            // Create the required folders
            QDir dir = QDir( getRecitationDirPath(this->selectedRecitationDir) );
            if( !dir.exists() ){
                qDebug() << "folder not exist:" << dir.path();
                if(!dir.mkpath(dir.path())){
                    qDebug() << "Error mkdir:" << dir.path();
                    return;
                }
            }
            this->loadInstalledRecitations();

            // Download audio file
            QString url = getAudioUrl(this->selectedRecitationDir, this->selectedSuraNumber, this->selectedAyaNumber);
            this->fileDownloader->downloadAndSave(url, audioFilePath);
            return;
        }
        else {
            qDebug() << "audio not exists:" << audioFilePath;
            this->ui->statusbar->showMessage(tr("Audio file missing Use CTR+D to download recitations."));
            return;
        }

    }

    //play audio file
    this->ui->statusbar->showMessage(tr("Playing Audio ..."));
    if(!this->media_player){
        this->media_player = new QMediaPlayer();
        connect(this->media_player, &QMediaPlayer::stateChanged, this, &MainWindow::on_MediaStateChanged);
    }
    QMediaContent media = QMediaContent(QUrl::fromLocalFile(audioFilePath));
    this->media_player->setMedia(media);
    this->media_player->play();
}

void MainWindow::loadInstalledRecitations()
{
    this->ui->recitationsMenu->clear();
    QActionGroup *recitActionGroup = new QActionGroup(this);
    QList<Recitation> recitations = dbHandler->getRecitationsList();
    QString mp3dir = QCoreApplication::applicationDirPath() + "/data/mp3/";
    for(int r=0; r<recitations.length(); r++){
        if( !QDir(mp3dir + recitations.at(r).dir).exists() ) continue;
        QAction *recitItem = new QAction(this);
        recitItem->setText(recitations.at(r).dir);
        recitItem->setCheckable(true);
        this->ui->recitationsMenu->addAction(recitItem);
        recitActionGroup->addAction(recitItem);
        if(this->selectedRecitationDir == recitations.at(r).dir) recitItem->setChecked(true);
    }

    connect(recitActionGroup, &QActionGroup::triggered, this, &MainWindow::on_recitation_select);
}

MainWindow::~MainWindow()
{
    this->save_settings();
    delete ui;
}

// MainWindow Slots

void MainWindow::on_sura_name_click(int row, int column) {
    this->setAya(row+1, 1, this->selectedTafsirIndex);
}

void MainWindow::on_aya_click( QListWidgetItem *item ){
    int index = this->ui->ayatList_listWidget->currentIndex().row();
    this->setAya(this->selectedSuraNumber, index+1, this->selectedTafsirIndex);
    //Read aya
    if(this->readAyatOnClick) this->read_selected_aya();
}

void MainWindow::on_aya_selected()
{
    //QListWidgetItem *item = this->ui->ayatList_listWidget->currentItem();
    //emit this->on_aya_click( item );
    int index = this->ui->ayatList_listWidget->currentIndex().row();
    this->setAya(this->selectedSuraNumber, index+1, this->selectedTafsirIndex);
}

void MainWindow::on_tafsir_change(int index){
    this->setAya(this->selectedSuraNumber, this->selectedAyaNumber, index);
}

void MainWindow::on_aya_spinbox_changed(int i){
    this->setAya(this->selectedSuraNumber, i, this->selectedTafsirIndex);
}

void MainWindow::on_sura_spinbox_changed(int i){
    this->setAya(i, 1, this->selectedTafsirIndex);
}

void MainWindow::on_aya_lineEdit_changed(QString text){
    if(text.length() <= 0) return;

    QList<Aya> result = dbHandler->searchAya(this->selectedSuraNumber, text);
    qDebug() << "result len: " << result.length();

    //display result
    this->ui->searchTool_lineEdit->clear();
    this->ui->searchTool_tableWidget->setRowCount(result.length());
    if(result.length() <= 0) return;
    for (int row = 0; row < result.length(); row++) {
        QString sura_number = QString::number( result.at(row).sura_number );
        QString sura_name = this->ui->suwarList_tableWidget->item( result.at(row).sura_number - 1, 0)->text();
        QString aya_number = QString::number( result.at(row).aya_number );
        QString aya_text = result.at(row).aya_text;
        this->ui->searchTool_tableWidget->setItem(row, 0, new QTableWidgetItem(sura_number));
        this->ui->searchTool_tableWidget->setItem(row, 1, new QTableWidgetItem(sura_name));
        this->ui->searchTool_tableWidget->setItem(row, 2, new QTableWidgetItem(aya_number));
        this->ui->searchTool_tableWidget->setItem(row, 3, new QTableWidgetItem(aya_text));
    }

    // Select first result (aya)
    this->ui->ayatList_listWidget->setCurrentRow(result.at(0).aya_number-1);
}

void MainWindow::on_sura_lineEdit_changed(QString text){
    if(text.length() <= 0) return;

    QList<QTableWidgetItem *> result = this->ui->suwarList_tableWidget->findItems(text, Qt::MatchContains);
    if(result.length() <= 0) return;
    this->ui->suwarList_tableWidget->setCurrentItem(result.at(0));
}

void MainWindow::on_recite_button_click(bool checked){
    this->setAutoRecite(!this->autoRecite);
    if(this->autoRecite){
        if (!this->media_player || this->media_player->state() == QMediaPlayer::StoppedState) this->read_selected_aya();
    }
}

void MainWindow::copy_selected_ayat(){
    qDebug() << "[copy_selected_ayat] ...";
    QString text;
    QList<QListWidgetItem *> ayat = this->ui->ayatList_listWidget->selectedItems();
    for (int i=0; i<ayat.length(); i++) {
        QStringList l = ayat.at(i)->text().split("} ");
        text += l.at(1) + " " + l.at(0) +  "}" + "\n";
    }
    text += "\n-- " + this->ui->suwarList_tableWidget->selectedItems().at(0)->text();

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    this->ui->statusbar->showMessage(tr("Selected Ayat was copied"));
}

void MainWindow::export_search_result(){
    int rowCount = this->ui->searchTool_tableWidget->rowCount();
    if(rowCount <= 0){
        qDebug() << "Nothing to save !";
        this->ui->statusbar->showMessage(tr("Nothing to save!"), 3000);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
                this, tr("Save File"),
                "export.csv",
                "Text files (*.csv *.txt)");

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream stream(&file);
        stream << tr("Sura Number") << " ; ";
        stream << tr("Sura Name") << " ; ";
        stream << tr("Aya Number")   << " ; ";
        stream << tr("Aya Text")    << " \n";
        for (int i=0; i<rowCount; i++) {
            stream << this->ui->searchTool_tableWidget->item(i, 0)->text() << " ; ";
            stream << this->ui->searchTool_tableWidget->item(i, 1)->text() << " ; ";
            stream << this->ui->searchTool_tableWidget->item(i, 2)->text() << " ; ";
            stream << this->ui->searchTool_tableWidget->item(i, 3)->text() << " \n";
        }

        file.close();
        qDebug() << "Writing finished";
        this->ui->statusbar->showMessage(tr("Search saved in:") + fileName);
    }
}

void MainWindow::on_spliter1_moved(int pos, int index){
    QList<int> sizes = this->ui->splitter_1->sizes();
    if( this->uiDisplayTafsirTool && sizes.at(1) == 0 ){
        this->ui->showTafsirAct->setChecked(false);
        this->uiDisplayTafsirTool = false;
    }
    else if( !this->uiDisplayTafsirTool && sizes.at(1) > 0 ){
        this->ui->showTafsirAct->setChecked(true);
        this->uiDisplayTafsirTool = true;
    }
}

void MainWindow::on_spliter2_moved(int pos, int index){
    QList<int> sizes = this->ui->splitter_2->sizes();
    if( this->uiDisplaySearchTool && sizes.at(1) == 0 ){
        this->ui->showSearchAct->setChecked(false);
        this->uiDisplaySearchTool = false;
    }
    else if( !this->uiDisplaySearchTool && sizes.at(1) > 0 ){
        this->ui->showSearchAct->setChecked(true);
        this->uiDisplaySearchTool = true;
    }
}

void MainWindow::on_show_search_act(bool checked){
    if(checked){
        this->ui->splitter_2->setSizes(QList<int>({1, 1}));
        this->uiDisplaySearchTool = true;
    }
    else{
        this->ui->splitter_2->setSizes(QList<int>({1, 0}));
        this->uiDisplaySearchTool = false;
    }
}

void MainWindow::on_show_tafsir_act(bool checked){
    if(checked){
        this->ui->splitter_1->setSizes(QList<int>({1, 1}));
        this->uiDisplayTafsirTool = true;
    }
    else{
        this->ui->splitter_1->setSizes(QList<int>({1, 0}));
        this->uiDisplayTafsirTool = false;
    }
}

void MainWindow::on_download_act(){
    if(this->downloadDialog){
        this->downloadDialog->show();
    }
    else{
        this->downloadDialog = new DownloadDialog(this);
        connect(this->downloadDialog, &DownloadDialog::recitationsListChanged, this, &MainWindow::loadInstalledRecitations);
        this->downloadDialog->show();
    }
}

void MainWindow::on_readAyaOC_act(bool checked)
{
    qDebug() << "read aya on click:" << checked;
    this->readAyatOnClick = checked;
}

void MainWindow::on_recitation_select(QAction *action)
{
    this->selectedRecitationDir = action->text();
}

void MainWindow::on_autoDownload_act(bool checked)
{
    this->autoAudioDownload = checked;
}

void MainWindow::on_langArabic_act()
{
    if(this->appLang != "AR"){
        this->appLang = "AR";
        this->save_settings();
        if(!this->dialogRestart) this->dialogRestart = new DialogRestart(this);
        this->dialogRestart->show();
    }
}

void MainWindow::on_langEnglish_act()
{
    if(this->appLang != "EN"){
        this->appLang = "EN";
        this->save_settings();
        if(!this->dialogRestart) this->dialogRestart = new DialogRestart(this);
        this->dialogRestart->show();
    }
}

void MainWindow::on_searchTool_query(QString text){
    QList<Aya> result = dbHandler->searchAya(text);

    //display result
    this->ui->searchTool_tableWidget->setRowCount(result.length());
    for (int row = 0; row < result.length(); row++) {
        QString sura_number = QString::number( result.at(row).sura_number );
        QString sura_name = this->ui->suwarList_tableWidget->item( result.at(row).sura_number - 1, 0)->text();
        QString aya_number = QString::number( result.at(row).aya_number );
        QString aya_text = result.at(row).aya_text;
        this->ui->searchTool_tableWidget->setItem(row, 0, new QTableWidgetItem(sura_number));
        this->ui->searchTool_tableWidget->setItem(row, 1, new QTableWidgetItem(sura_name));
        this->ui->searchTool_tableWidget->setItem(row, 2, new QTableWidgetItem(aya_number));
        this->ui->searchTool_tableWidget->setItem(row, 3, new QTableWidgetItem(aya_text));
    }
}

void MainWindow::on_searchTool_rowClick(int row, int column){
    int sura_number = this->ui->searchTool_tableWidget->item(row, 0)->text().toInt();
    int aya_number = this->ui->searchTool_tableWidget->item(row, 2)->text().toInt();
    this->setAya(sura_number, aya_number, this->selectedTafsirIndex);
}

void MainWindow::on_MediaStateChanged(QMediaPlayer::State state){
    if(state == QMediaPlayer::StoppedState){
        if(this->autoRecite){
            int ret = this->setAya(this->selectedSuraNumber, this->selectedAyaNumber + 1, this->selectedTafsirIndex);
            if(ret) this->read_selected_aya();
            else this->setAutoRecite(false);
        }
    }
    else if (state == QMediaPlayer::PlayingState) {}
    else if (state == QMediaPlayer::PausedState) {}
}
