#include "dbhandler.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QCoreApplication>
#include <QDebug>
#include <QSqlError>
#include <QFile>

DbHandler::DbHandler(QString con_name) {
    this->db_con = QSqlDatabase::addDatabase("QSQLITE", con_name);
}

bool DbHandler::connect(QString db_path){
    // setup database
    QString full_path = QCoreApplication::applicationDirPath() + db_path;
    qDebug() << "Opening db:" << full_path;

    if (!QFile::exists(full_path)) {
        qDebug() << "[Error] db not exists !";
        return false;
    }

    // connect db
    this->db_con.setDatabaseName(full_path);
    if (!this->db_con.open()){
        qDebug() << "[Error]: " << db_con.lastError().text();
        return false;
    }

    return true;
}

QList<QString> DbHandler::getSuwarNames() {
    QList<QString> suwarNames;

    if( !this->connect(this->ayatDb_path) ) return suwarNames;

    // get query
    QSqlQuery query = QSqlQuery(this->db_con);
    QString query_cmd = "SELECT name FROM suwar ORDER BY number ASC";
    if (!query.exec(query_cmd)){
        qDebug() << "[Error]: " << query.lastError().text();
        return suwarNames;
    }

    while (query.next()) {
        suwarNames.append(query.value(0).toString());
    }

    query.finish();
    this->db_con.close();
    return suwarNames;
}

QList<Sura> DbHandler::getSuwarInfo(){
    QList<Sura> suwarInfo;

    if( !this->connect(this->ayatDb_path) ) return suwarInfo;

    // get query
    QSqlQuery query = QSqlQuery(this->db_con);
    QString query_cmd = "SELECT number, name, total_verses FROM suwar ORDER BY number ASC";
    if (!query.exec(query_cmd)){
        qDebug() << "[Error]: " << query.lastError().text();
        return suwarInfo;
    }

    while (query.next()) {
        Sura sura;
        sura.number = query.value(0).toInt();
        sura.name = query.value(1).toString();
        sura.total_verses = query.value(2).toInt();
        suwarInfo.append(sura);
    }

    query.finish();
    this->db_con.close();
    return suwarInfo;
}

QList<QString> DbHandler::getTafasirNames(){
    QList<QString> tafasirNames;

    // connect db
    if(!this->connect(this->tafsirDb_path)) return tafasirNames;

    // get query
    QSqlQuery query = QSqlQuery(this->db_con);
    QString query_cmd = "SELECT trans_name FROM trans ORDER BY id ASC";
    bool success = query.exec(query_cmd);
    if (!success){
        qDebug() << "[Error]: " << query.lastError().text();
        return tafasirNames;
    }

    while (query.next()) {
        tafasirNames.append(query.value(0).toString());
    }

    query.finish();
    this->db_con.close();
    return tafasirNames;
}

QList<QString> DbHandler::getSura(int sura_number){
    QList<QString> ayat_list;

    // connect db
    if(!this->connect(this->ayatDb_path)) return ayat_list;

    // get query
    QSqlQuery query = QSqlQuery(this->db_con);
    query.prepare("SELECT text FROM ayat WHERE sura=(:sura_number) ORDER BY aya ASC");
    query.bindValue(":sura_number", sura_number);
    bool success = query.exec();
    if (!success){
        qDebug() << "[Error]: " << query.lastError().text();
        return ayat_list;
    }

    while (query.next()) {
        ayat_list.append(query.value(0).toString());
    }

    query.finish();
    this->db_con.close();
    return ayat_list;
}

QString DbHandler::getTafsir(int sura_number, int aya_number, int tafsirID){
    QString trans;

    QString trans_key;
    QString trans_dir;
    // Connect db
    if(!this->connect(this->tafsirDb_path)) return trans;
    // query
    QSqlQuery query = QSqlQuery(this->db_con);
    query.prepare("SELECT trans_key, trans_dir FROM trans WHERE id=(:tafsirID) ");
    query.bindValue(":tafsirID", tafsirID);
    if (!query.exec()){
        qDebug() << "[Error]: " << query.lastError().text();
        return trans;
    }
    if(!query.first() || !query.isValid()){
        qDebug() << "query return nothing! : " << query.lastQuery();
        return trans;
    }
    trans_key = query.value(0).toString();
    trans_dir = query.value(1).toString();
    qDebug() << "trans_key: " << trans_key << " trans_dir: " << trans_dir << "tafsirID: " << tafsirID;

    QString tafsirDb_path = "/data/" + trans_dir + "/" + trans_key + ".ayt";
    if(!this->connect(tafsirDb_path)) return trans;
    // query
    query = QSqlQuery(db_con);
    query.prepare(" SELECT text FROM "+ trans_key +" WHERE sura=(:sura_number) AND aya=(:aya_number)");
    query.bindValue(":sura_number", sura_number);
    query.bindValue(":aya_number", aya_number);
    if (!query.exec()){
        qDebug() << "[Error]: " << query.lastError().text();
        return trans;
    }
    if(!query.first() || !query.isValid()){
        qDebug() << "query return nothing! : " << query.lastQuery();
        return trans;
    }
    trans = query.value(0).toString();

    query.finish();
    this->db_con.close();
    return trans;
}

QList<Recitation> DbHandler::getRecitationsList()
{
    QList<Recitation> recitations;

    // connect db
    if(!this->connect(this->recitationsDb_path)) return recitations;

    // get query
    QSqlQuery query = QSqlQuery(this->db_con);
    QString query_cmd = "SELECT recitation_dir, recitation FROM recitations ORDER BY id ASC";
    bool success = query.exec(query_cmd);
    if (!success){
        qDebug() << "[Error]: " << query.lastError().text();
        return recitations;
    }

    while (query.next()) {
        Recitation r;
        r.dir  = query.value(0).toString();
        r.name = query.value(1).toString();
        recitations.append(r);
    }

    query.finish();
    this->db_con.close();
    return recitations;
}

QList<Aya> DbHandler::searchAya(QString text){
    QList<Aya> result;
    if(text.length() == 0) return result;

    if(!this->connect(this->ayatDb_path)) return result;
    // query
    QSqlQuery query = QSqlQuery(this->db_con);
    query.prepare(" SELECT sura, aya, text FROM ayat WHERE nass_safy LIKE (:text)");
    query.bindValue(":text", "%" + text + "%");
    if (!query.exec()){
        qDebug() << "[Error]: " << query.lastError().text();
        return result;
    }
    if(!query.first() || !query.isValid()){
        qDebug() << "Query return nothing: " << query.lastQuery();
        return result;
    }

    do{
        Aya aya;
        aya.sura_number = query.value(0).toInt();
        aya.aya_number  = query.value(1).toInt();
        aya.aya_text    = query.value(2).toString();
        result.append(aya);

    }while(query.next());

    query.finish();
    this->db_con.close();
    return result;
}

QList<Aya> DbHandler::searchAya(int sura_number, QString text){
    QList<Aya> result;
    if(text.length() == 0) return result;

    if(!this->connect(this->ayatDb_path)) return result;
    // query
    QSqlQuery query = QSqlQuery(this->db_con);
    query.prepare(" SELECT sura, aya, text FROM ayat WHERE sura=(:sura_number) AND nass_safy LIKE (:text) ");
    query.bindValue(":sura_number", sura_number);
    query.bindValue(":text", "%" + text + "%");
    if (!query.exec()){
        qDebug() << "[Error]: " << query.lastError().text();
        return result;
    }
    if(!query.first() || !query.isValid()){
        qDebug() << "Query return nothing: " << query.lastQuery();
        return result;
    }

    do{
        Aya aya;
        aya.sura_number = query.value(0).toInt();
        aya.aya_number  = query.value(1).toInt();
        aya.aya_text    = query.value(2).toString();
        result.append(aya);

    }while(query.next());

    query.finish();
    this->db_con.close();
    return result;
}

