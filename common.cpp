#include "common.h"


QStringList getSuraAudiosPathsList(QString recitationDir, int suraNumber)
{
    QStringList paths;
    for(int ayaNumber=1; ayaNumber<=ayahCount[suraNumber-1]; ayaNumber++){
        paths << getAudioFilePath(recitationDir, suraNumber, ayaNumber);
    }
    return paths;
}

QString getAudioFilePath(QString recitationDir, int suraNumber, int ayaNumber)
{
    return getRecitationDirPath(recitationDir) + "/" + getAudioFileName(suraNumber, ayaNumber);
}

QString getAudioFileName(int suraNumber, int ayaNumber){
    QString fileName;

    if(suraNumber < 10) fileName += "00";
    else if(suraNumber < 100) fileName += "0";
    fileName += QString::number(suraNumber);
    if(ayaNumber < 10) fileName += "00";
    else if(ayaNumber < 100) fileName += "0";
    fileName += QString::number(ayaNumber);
    fileName += ".mp3";

    return fileName;
}

QString getRecitationDirPath(QString recitationDir)
{
    return QCoreApplication::applicationDirPath() + "/data/mp3/" + recitationDir;
}

QString getAudioUrl(QString recitationDir, int suraNumber, int ayaNumber)
{
    return "https://www.everyayah.com/data/" + recitationDir + "/" + getAudioFileName(suraNumber, ayaNumber);
}

QStringList getSuraAudiosUrlsList(QString recitationDir, int suraNumber)
{
    QStringList urls;
    for(int ayaNumber=1; ayaNumber<=ayahCount[suraNumber-1]; ayaNumber++){
        urls << getAudioUrl(recitationDir, suraNumber, ayaNumber);
    }
    return urls;
}

