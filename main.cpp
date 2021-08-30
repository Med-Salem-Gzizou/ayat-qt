#include "mainwindow.h"

#include "settingsKeys.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Load Language settings
    QTranslator translator;
    QSettings settings(ORG_NAME_KEY, APP_NAME_KEY);
    QString appLang = settings.value(APP_LANG_KEY).toString();
    if(appLang == "AR"){
        QString translatinFile = QCoreApplication::applicationDirPath() + "/ayat-qt_ar_001";
        translator.load(translatinFile);
    }
    else{
        appLang = "EN";
        settings.setValue(APP_LANG_KEY, appLang);
    }
    app.installTranslator(&translator);

    MainWindow mw;
    mw.show();
    return app.exec();
}
