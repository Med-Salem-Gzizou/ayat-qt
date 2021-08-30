#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <QtSql/QSqlDatabase>
#include <QList>
#include <QString>

struct Aya {
    int sura_number;
    int aya_number;
    QString aya_text;
};

struct Sura {
    QString name;
    int number;
    int total_verses;
};

struct Recitation {
    QString name;
    QString dir;
};

class DbHandler
{
public:
    DbHandler(QString con_name);

    QList<QString> getSuwarNames();
    QList<Sura> getSuwarInfo();
    QList<QString> getTafasirNames();
    QList<QString> getSura(int sura_number);
    QString getTafsir(int sura_number, int aya_number, int tafsirID);
    QList<Recitation> getRecitationsList();
    QList<Aya> searchAya(QString text);
    QList<Aya> searchAya(int sura_number, QString text);

private:
    QSqlDatabase db_con;
    QString ayatDb_path = "/data/ayat.ayt";
    QString tafsirDb_path = "/data/tafasir.ayt";
    QString recitationsDb_path = "/data/recitations.db";

    bool connect(QString db_path);
};

#endif // DBHANDLER_H
