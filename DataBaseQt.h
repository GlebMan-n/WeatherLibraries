#ifndef DATABASEQT_H
#define DATABASEQT_H

#include <QObject>
#include <QDebug>
#include <QString>
#include "grib_lib_global.h"
#include <grib_lib/GribExStructures.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>

class GRIB_LIB_EXPORT DataBaseQt : public QObject
{
    Q_OBJECT

public:
    DataBaseQt(QObject *parent = 0);
    ~DataBaseQt();

    // создать бд
    // подключиться к бд
    bool open(const QString &path);

    bool createTables() const;
    // добавить новую заявку
    bool addReq(const GribRequestData &data) const;

	bool setReqMasterUid(const QString &uid_master, const QString &uid_slave) const;
    bool setReqStatus(const QString &uid_slave, int status, uint date_time) const;
    bool setReqMasterUidNull(const QString &uid_slave) const;
	bool addResponse(const GribRequestData &resp,const QString &filename) const;
	bool setDeleted(int iDeleted, const QString &uid_slave) const;

    // обновить заявку
    bool init(const QString &path);
    QList<GribRequestData> getListRequests(int i_type) const;// 0 - все, 1 - не зарегестрированные
    QList<GribRequestData> getListRequests(const QString &qStrWhere = "") const;
    QList<GribRequestData> getListIceRequests();
    QList<GribFileResp> getListFiles(const QString &qStrWhere) const;
    GribFileResp getLast(const QString &uid) const;
	bool checkReqByUid(const QString &uid) const;
	bool deleteReq(const QString &uid) const;
	bool deleteResponses(const QString &uid) const;
	bool deleteRespons(const QString &uid, int dateTime);
	bool isDeletedReq(const QString &uid);
	bool getReqById(const QString &uid, GribRequestData &data) const;
    //установить настройки
	void setSettings(	const QString &serverAddr, 
						const QString &serverPort, 
						const QString &userLogin, 
						const QString &userPassword, 
						const QString &pathToGribData, 
						const ZygribConsts &zygrib,
						const QString &pathToIceData) const;
    //получить настройки
    bool getSettings(QString &serverAddr, QString &serverPort, QString &userLogin, QString &userPassword, QString &pathToGribData, ZygribConsts &zygrib,
                     QString &pathToIceData) const;
    //получить текущее состояние
    bool getStatus(StatusData &data) const;
    //установить текущее состояние
    bool setStatus(const StatusData &data) const;
    //добавить лог
    bool addLog(const LogData &data) const;
    //получить список логов
    std::vector<LogData> getLogs(const QDateTime &begin, const QDateTime &end) const;
    QString getLastLogs(int count = 100) const;
    //очистить лог до указанной даты
    bool clearLog(const QDateTime &date) const;
    QMap<QString,QString> getDrawParams() const;
    bool setDrawParams(QMap<QString,QString> drawParams) const;
    QMap<QString,SeaData> getMapSea() const;
    bool importSeaTextFile(const QString &fileName) const;
    bool insertSeaData(const QString &name, const QString &addr,const QString &rus_name, int check) const;
    bool updateSeaData(const SeaData &seaData) const;
    bool saveLogsToFile(const QString &logFile) const;
    bool isLogsEnable() const
    {
        return m_bLogsEnable;
    }
    void setLogsEnable(bool val)
    {
        m_bLogsEnable = val;
    }
	bool isValid() const { return m_db.isValid(); }
	bool clearSeas() const;
private:
    QSqlDatabase m_db;
    bool m_bLogsEnable;

};

#endif // DATABASEQT_H
