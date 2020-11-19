#include "DataBaseQt.h"
#include <QDateTime>

#include <common/include/tr_macros.h>

DataBaseQt::DataBaseQt(QObject *parent)
    : QObject(parent)
{
    m_bLogsEnable = true;			
}

DataBaseQt::~DataBaseQt()
{
    try
    {
        m_db.close();
    }
    catch (...)
    {
    }
}

bool DataBaseQt::init(const QString &path)
{
    if (path.isEmpty())
        return false;
    bool result;
    result = open(path);
    createTables();
    if (!result)
        return result;

    QString addr;
    QString login;
    QString pwd;
    QString port;
    QString fpath;
    QString ice_path;
    ZygribConsts zg;

    bool bRes;
    bRes = getSettings(addr, port, login, pwd, fpath, zg, ice_path);

    if (!bRes)
    {
        QFileInfo fi(path);
        ConnectionInfo info;
        setSettings(    info.m_serverAddr,
                        QString::number(info.m_serverPort),
                        "login",
                        "password",
                        fi.path(),
                        ZygribConsts(),
                        ice_path);
    }
    return result;
}

// создать бд
// подключиться к бд
bool DataBaseQt::open(const QString &path)
{
    bool result = false;
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open())
    {
        return result;
    }


    result = m_db.isValid();

    return result;
}

bool DataBaseQt::createTables() const
{
    bool result = false;

    QString qStrQueryTableCli =
        " CREATE TABLE IF NOT EXISTS clients (  "
        " id				INTEGER		PRIMARY KEY NOT NULL, "
        " client_login		TEXT		,	"
        " client_password	TEXT		,	"
        " client_ip			TEXT		,	"
        " client_info		TEXT		,	"
        " client_key		TEXT );	";


    QString qStrQueryTableReqPar =
        " CREATE TABLE IF NOT EXISTS request_param (	"
        " id			INTEGER		PRIMARY KEY NOT NULL, "
        " uid_slave		TEXT		,	"
        " uid_master	TEXT		,	"
        " creators_ip	TEXT		,	"
		" sender_ip		TEXT		,	"
        " client_id		TEXT		,	"
        " date_time		INTEGER		,	"
        " is_deleted	INTEGER		,	"
        " x0			REAL		,	"
        " y0			REAL		,	"
        " x1			REAL		,	"
        " y1			REAL		,	"
        " resolution	REAL		,	"
        " interval		INTEGER		,	"
        " days			INTEGER		,	"
        " req_type		INTEGER		,	"
        " runGFS		TEXT		,	"
        " parametrs		TEXT		,	"
        " region_name	TEXT		,	"
        " status		INTEGER			"
        " ); ";

    QString qStrQueryTableResponse =
        " CREATE TABLE  IF NOT EXISTS response (  "
        " id				INTEGER PRIMARY KEY NOT NULL, "
        " uid_master		TEXT,			"
        " uid_slave			TEXT,			"
        " date_time			INTEGER,		"
        " filename			TEXT);		";

    QString qStrQueryTableSettings =
        " CREATE TABLE IF NOT EXISTS settings (		"
        " id				INTEGER PRIMARY KEY NOT NULL, "
        " zygrib_host				TEXT,			"
        " zygrib_scritpath			TEXT,			"
        " zygrib_scriptstock		TEXT,			"
        " zygrib_zygriblog			TEXT,			"
        " zygrib_zygribpwd			TEXT,			"
        " zygrib_grbFileResolution	TEXT,			"
        " zygrib_client				TEXT,			"
        " user_login				TEXT,			"
        " user_password				TEXT,			"
        " server_port				TEXT,			"
        " path_to_grib_data			TEXT,			"
        " path_to_ice_data			TEXT,			"
        " server_addr				TEXT);			";

    QString qStrQueryTableLogs =
        " CREATE TABLE IF NOT EXISTS logs (		"
        " id						INTEGER PRIMARY KEY NOT NULL, "
        " date_time					INTEGER,			"
        " source					TEXT,			"// titanic || saturn || webgio || filetransferservice
        " type						INTEGER,			"// 0 - unknown || 1 - info || 2 - warning || 3 - error
        " text						TEXT);			"  ;

    QString qStrQueryTableStatus =
        " CREATE TABLE IF NOT EXISTS status (		"
        " id						INTEGER PRIMARY KEY NOT NULL, "
        " date_time					INTEGER,			"
        " ping_server				INTEGER,			"  // 0 || 1
        " status					TEXT);			"  ;

    QString qStrQueryTableDrawParam =
        " CREATE TABLE IF NOT EXISTS drawParam (		"
        " id						INTEGER PRIMARY KEY NOT NULL, "
        " slave_uid					TEXT,			"
        " draw_param				TEXT);			"  ;

    QString qStrQueryTableSea =
        " CREATE TABLE IF NOT EXISTS seaTable (		"
        " id						INTEGER PRIMARY KEY NOT NULL,	"
        " sea_name					TEXT,							"
        " sea_rus_name				TEXT,							"
        " data_addr					TEXT,							"
        " date_time					INTEGER,						"
        " check_data				INTEGER);			"  ;

    QSqlQuery   queryTableCli(m_db),
                queryTableReqPar(m_db),
                queryTableResponse(m_db),
                queryTableLog(m_db),
                queryTableStatus(m_db),
                queryTableSettings(m_db),
                queryTableDrawParam(m_db),
                queryTableSea(m_db);

    queryTableCli.prepare(qStrQueryTableCli);
    queryTableReqPar.prepare(qStrQueryTableReqPar);
    queryTableResponse.prepare(qStrQueryTableResponse);
    queryTableSettings.prepare(qStrQueryTableSettings);
    queryTableStatus.prepare(qStrQueryTableStatus);
    queryTableLog.prepare(qStrQueryTableLogs);
    queryTableDrawParam.prepare(qStrQueryTableDrawParam);
    queryTableSea.prepare(qStrQueryTableSea);

    result = queryTableCli.exec() && queryTableReqPar.exec() && queryTableStatus.exec()&& queryTableLog.exec() && queryTableSettings.exec()
             && queryTableResponse.exec() && queryTableDrawParam.exec() && queryTableSea.exec();
    return result;
}

// добавить новую заявку
bool DataBaseQt::addReq(const GribRequestData &data) const
{
    bool result = false;

    if (checkReqByUid(data.m_uid_slave))
    {
        return result;
    }

    QSqlQuery query(m_db);

    query.prepare("INSERT INTO request_param (	uid_slave, uid_master, creators_ip, sender_ip, client_id, date_time, is_deleted, x0, y0, x1, y1, resolution, interval, days, req_type, runGFS, parametrs, status, region_name) "
                  "VALUES (	:uid_slave, :uid_master, :creators_ip, :sender_ip, :client_id, :date_time, :is_deleted, :x0, :y0, :x1, :y1, :resolution, :interval, :days, :req_type, :runGFS, :parametrs, :status, :region_name)");

    if (!data.m_uid_slave.isEmpty())
        query.bindValue(":uid_slave",   data.m_uid_slave);
    if (!data.m_uid_master.isEmpty())
        query.bindValue(":uid_master",  data.m_uid_master);
    query.bindValue(":creators_ip", data.m_creatorIp);
	query.bindValue(":sender_ip",	data.m_senderIp);
    query.bindValue(":client_id",   data.m_clients_id);
    query.bindValue(":date_time",   data.m_date_time_t);
    query.bindValue(":is_deleted",  data.m_deleted);
    query.bindValue(":x0",          data.m_x0);
    query.bindValue(":y0",          data.m_y0);
    query.bindValue(":x1",          data.m_x1);
    query.bindValue(":y1",          data.m_y1);
    query.bindValue(":resolution",  data.m_resolution);
    query.bindValue(":interval",    data.m_interval);
    query.bindValue(":days",        data.m_days);
    query.bindValue(":req_type",    data.m_requestType);
    query.bindValue(":runGFS",      data.m_runGFS);
    query.bindValue(":parametrs",   data.m_parametrs);
    query.bindValue(":status",      data.m_status);
    query.bindValue(":region_name", data.m_name);

    result = query.exec();
    return result;
}

bool DataBaseQt::setReqMasterUid(const QString &uid_master, const QString &uid_slave) const
{
    bool result = false;

    QSqlQuery query(m_db);

    query.prepare("UPDATE request_param SET uid_master = :uid_master_val WHERE uid_slave = :uid_slave_val ");
    query.bindValue(":uid_master_val",  uid_master);
    query.bindValue(":uid_slave_val",   uid_slave);

    result = query.exec();
    return result;
}

bool DataBaseQt::setDeleted(int iDeleted, const QString &uid_slave) const
{
    bool result = false;

    GribRequestData data;
    bool bRes = getReqById(uid_slave, data);
    /*если запись есть и уид мастера не назначен, т.е запрос не зарегистрирован*/
    if (bRes && ( data.m_uid_master.isEmpty() || data.m_uid_master == "" ) )
    {
        /*удаляем сразу*/
        if (deleteReq(uid_slave))
            return true;
    }

    QSqlQuery query(m_db);

    query.prepare("UPDATE request_param SET is_deleted = :is_deleted_val WHERE uid_slave = :uid_slave_val ");

    query.bindValue(":is_deleted_val",      iDeleted);
    query.bindValue(":uid_slave_val",       uid_slave);

    result = query.exec();

    if (result)
        deleteResponses(uid_slave);

    return result;
}

bool DataBaseQt::setReqStatus(const QString &uid_slave, int status, uint date_time) const
{
    bool result = false;

    QSqlQuery query(m_db);

    query.prepare("UPDATE request_param SET status = :status_val, date_time = :date_time_val WHERE uid_slave = :uid_slave_val ");
    query.bindValue(":status_val",      status);
    query.bindValue(":date_time_val",   date_time);
    query.bindValue(":uid_slave_val",   uid_slave);

    result = query.exec();

    return result;
}


bool DataBaseQt::setReqMasterUidNull(const QString &uid_slave) const
{
    bool result = false;

    QSqlQuery query(m_db);

    query.prepare("UPDATE request_param SET uid_master = null WHERE uid_slave = :uid_slave_val ");
    query.bindValue(":uid_slave_val",   uid_slave);

    result = query.exec();
    return result;
}


bool DataBaseQt::isDeletedReq(const QString &uid)
{
    bool result = false;

    return result;
}

bool DataBaseQt::getReqById(const QString &uid, GribRequestData &data) const
{
    bool result = false;
    QList<GribRequestData> list = getListRequests("WHERE uid_slave = '" + uid + "'");

    if (list.isEmpty())
        return result;

    data.fromJson(list[0].toJson());
    return true;
}

bool DataBaseQt::addResponse(const GribRequestData &resp, const QString &filename) const
{
    bool result = false;


    QSqlQuery query(m_db);

    query.prepare("INSERT INTO response ( uid_master, uid_slave, date_time, filename) "
                  "VALUES ( :uid_master, :uid_slave, :date_time, :filename)");

    query.bindValue(":uid_master",  resp.m_uid_master);
    query.bindValue(":uid_slave",   resp.m_uid_slave);
    query.bindValue(":date_time",   resp.m_date_time_t);
    query.bindValue(":filename",    filename);

    result = query.exec();
    return result;
}

bool DataBaseQt::deleteReq(const QString & uid) const
{
    bool result = false;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM request_param WHERE uid_slave = :uid_slave_val ");
    query.bindValue(":uid_slave_val", uid);
    result = query.exec();

    if (result)
        deleteResponses(uid);


    return result;

}

bool DataBaseQt::deleteRespons(const QString & uid, int dateTime)
{
    bool result = false;

    QSqlQuery query1(m_db);

    query1.prepare("DELETE FROM response WHERE uid_slave = :uid_slave_val AND date_time = :date_time");
    query1.bindValue(":uid_slave_val", uid);
    query1.bindValue(":date_time", dateTime);
    result = query1.exec();
    return result;
}

bool DataBaseQt::deleteResponses(const QString & uid) const
{
    bool result = false;
    QSqlQuery query1(m_db);
    query1.prepare("DELETE FROM response WHERE uid_slave = :uid_slave_val ");
    query1.bindValue(":uid_slave_val", uid);
    result = query1.exec();
    return result;
}

QList<GribRequestData> DataBaseQt::getListRequests(int i_type) const
{
    QList<GribRequestData> result, tmp;
    switch (i_type)
    {
        case 0:
            result = getListRequests();
            break;
        case 1:
            result = getListRequests("WHERE uid_master IS NULL OR is_deleted = '1' OR uid_master == ''");
            break;
        case 2:
            tmp = getListRequests("WHERE uid_master IS NOT NULL");

            for (int i = 0; i < tmp.size(); i++)
            {
                GribRequestData dt = tmp.at(i);
                if (tmp.at(i).m_deleted == 1 || dt.m_status == 0)
                    result.append(tmp.at(i));
            }
            break;
        case 3:
            tmp = getListRequests("WHERE status = '0' ");

            for (int i = 0; i < tmp.size(); i++)
            {
                    result.append(tmp.at(i));
            }
            break;
        case 4:
            result = getListRequests("WHERE is_deleted = '1' ");
            break;
        default:
            break;
    }
    return result;
}

QList<GribRequestData> DataBaseQt::getListRequests(const QString &qStrWhere) const
{
    QList<GribRequestData> result;

    QSqlQuery query("SELECT * FROM request_param " + qStrWhere);
    while (query.next())
    {
        int ind_uid_slave       = query.record().indexOf("uid_slave");
        int ind_uid_master      = query.record().indexOf("uid_master");
        int ind_creators_ip     = query.record().indexOf("creators_ip");
		int ind_sender_ip		= query.record().indexOf("sender_ip");
        int ind_client_id       = query.record().indexOf("client_id");
        int ind_date_time       = query.record().indexOf("date_time");
        int ind_is_deleted      = query.record().indexOf("is_deleted");
        int ind_x0              = query.record().indexOf("x0");
        int ind_y0              = query.record().indexOf("y0");
        int ind_x1              = query.record().indexOf("x1");
        int ind_y1              = query.record().indexOf("y1");
        int ind_resolution      = query.record().indexOf("resolution");
        int ind_interval        = query.record().indexOf("interval");
        int ind_days            = query.record().indexOf("days");
        int ind_req_type        = query.record().indexOf("req_type");
        int ind_runGFS          = query.record().indexOf("runGFS");
        int ind_parametrs       = query.record().indexOf("parametrs");
        int ind_status          = query.record().indexOf("status");
        int ind_region_name     = query.record().indexOf("region_name");

        GribRequestData data;
        data.m_x0           = query.value(ind_x0).toDouble();
        data.m_y0           = query.value(ind_y0).toDouble();
        data.m_x1           = query.value(ind_x1).toDouble();
        data.m_y1           = query.value(ind_y1).toDouble();
        data.m_resolution   = query.value(ind_resolution).toDouble();
        data.m_interval     = query.value(ind_interval).toInt();
        data.m_days         = query.value(ind_days).toInt();
        data.m_requestType  = query.value(ind_req_type).toInt();
        data.m_runGFS       = query.value(ind_runGFS).toString();
        data.m_parametrs    = query.value(ind_parametrs).toString();
        data.m_creatorIp    = query.value(ind_creators_ip).toString();
		data.m_senderIp		= query.value(ind_sender_ip).toString();
        data.m_uid_master   = query.value(ind_uid_master).toString();
        data.m_uid_slave    = query.value(ind_uid_slave).toString();
        data.m_date_time_t  = query.value(ind_date_time).toInt();
        data.m_status       = query.value(ind_status).toInt();
        data.m_clients_id   = query.value(ind_client_id).toString();
        data.m_deleted      = query.value(ind_is_deleted).toInt();
        data.m_name         = query.value(ind_region_name).toString();

        result.append(data);
    }
    return result;
}


QList<GribFileResp> DataBaseQt::getListFiles(const QString &qStrWhere) const
{
    QList<GribFileResp> result;

    QSqlQuery query("SELECT * FROM response " + qStrWhere);

    while (query.next())
    {
        int ind_uid_master      = query.record().indexOf("uid_master");
        int ind_uid_slave       = query.record().indexOf("uid_slave");
        int ind_date_time       = query.record().indexOf("date_time");
        int ind_filename        = query.record().indexOf("filename");

        GribFileResp data;

        data.m_uid_master   = query.value(ind_uid_master).toString();
        data.m_uid_slave    = query.value(ind_uid_slave).toString();
        data.m_date_time    = query.value(ind_date_time).toInt();
        data.m_filename     = query.value(ind_filename).toString();

        result.append(data);
    }
    return result;
}

GribFileResp DataBaseQt::getLast(const QString &  uid) const
{
    GribFileResp result;
    QList<GribFileResp> list = getListFiles("WHERE uid_slave = '" + uid + "' GROUP BY DATE_TIME ORDER BY DATE_TIME DESC ");
    if (list.size() > 0)
        result = list.at(0);
    return result;
}

bool DataBaseQt::checkReqByUid(const QString &  uid) const
{
    bool result = true;
    QList<GribRequestData> list = getListRequests("WHERE uid_slave = '" + uid + "'");
    result = list.size() > 0;
    return result;
}

void DataBaseQt::setSettings(	const QString &serverAddr,
								const QString &serverPort,
								const QString &userLogin,
								const QString &userPassword,
								const QString &pathToGribData,
								const ZygribConsts &zygrib,
								const QString &pathToIceData) const
{
    bool result;
    QSqlQuery query(m_db);

    query.prepare("DELETE FROM settings");
    query.exec();

    query.prepare(  "INSERT INTO settings (zygrib_host, zygrib_scritpath, zygrib_scriptstock, zygrib_zygriblog, zygrib_zygribpwd, zygrib_grbFileResolution, zygrib_client, user_login, user_password, server_port, server_addr, path_to_grib_data, path_to_ice_data) "
                    "VALUES (:zygrib_host, :zygrib_scritpath, :zygrib_scriptstock, :zygrib_zygriblog, :zygrib_zygribpwd, :zygrib_grbFileResolution, :zygrib_client, :user_login, :user_password, :server_port, :server_addr, :path_to_grib_data, :path_to_ice_data)");

    query.bindValue(":zygrib_host",             zygrib.m_host);
    query.bindValue(":zygrib_scritpath",        zygrib.m_scriptpath);
    query.bindValue(":zygrib_scriptstock",      zygrib.m_scriptstock);
    query.bindValue(":zygrib_zygriblog",        zygrib.m_zygriblog);
    query.bindValue(":zygrib_zygribpwd",        zygrib.m_zygribpwd);
    query.bindValue(":zygrib_grbFileResolution",zygrib.m_grbFileResolution);
    query.bindValue(":zygrib_client",           zygrib.m_client);
    query.bindValue(":user_login",              userLogin);
    query.bindValue(":user_password",           userPassword);
    query.bindValue(":server_port",             serverPort);
    query.bindValue(":server_addr",             serverAddr);
    query.bindValue(":path_to_grib_data",       pathToGribData);
    query.bindValue(":path_to_ice_data",        pathToIceData);

    result = query.exec();
}

bool DataBaseQt::getSettings(QString &serverAddr, QString &serverPort, QString &userLogin, QString &userPassword, QString &pathToGribData, ZygribConsts &zygrib,
	QString &pathToIceData) const
{
    QSqlQuery query("SELECT * FROM settings ");

    while (query.next())
    {
        int ind_zygrib_host     = query.record().indexOf("zygrib_host");
        int ind_zygrib_scritpath        = query.record().indexOf("zygrib_scritpath");
        int ind_zygrib_scriptstock      = query.record().indexOf("zygrib_scriptstock");
        int ind_zygrib_zygriblog        = query.record().indexOf("zygrib_zygriblog");

        int ind_zygrib_zygribpwd        = query.record().indexOf("zygrib_zygribpwd");
        int ind_zygrib_grbFileResolution        = query.record().indexOf("zygrib_grbFileResolution");
        int ind_zygrib_client       = query.record().indexOf("zygrib_client");

        int ind_user_login      = query.record().indexOf("user_login");
        int ind_user_password       = query.record().indexOf("user_password");
        int ind_server_port     = query.record().indexOf("server_port");
        int ind_server_addr     = query.record().indexOf("server_addr");
        int ind_path_to_grib_data       = query.record().indexOf("path_to_grib_data");
        int ind_path_to_ice_data = query.record().indexOf("path_to_ice_data");

        serverAddr      = query.value(ind_server_addr).toString();
        serverPort      = query.value(ind_server_port).toString();
        userLogin       = query.value(ind_user_login).toString();
        userPassword    = query.value(ind_user_password).toString();
        pathToGribData  = query.value(ind_path_to_grib_data).toString();
        pathToIceData = query.value(ind_path_to_ice_data).toString();
        ZygribConsts zgrib;

        zgrib.m_host = query.value(ind_zygrib_host).toString();
        zgrib.m_scriptpath = query.value(ind_zygrib_scritpath).toString();
        zgrib.m_scriptstock = query.value(ind_zygrib_scriptstock).toString();
        zgrib.m_zygriblog = query.value(ind_zygrib_zygriblog).toString();

        zgrib.m_zygribpwd = query.value(ind_zygrib_zygribpwd).toString();
        zgrib.m_grbFileResolution = query.value(ind_zygrib_grbFileResolution).toString();
        zgrib.m_client = query.value(ind_zygrib_client).toString();

        zygrib = zgrib;
        return true;
    }
    return false;
}

//установить текущее состояние
bool DataBaseQt::setStatus(const StatusData &data) const
{
    bool result = false;

    QSqlQuery query(m_db);

    query.prepare("DELETE FROM status");
    result = query.exec();

    query.prepare(  "INSERT INTO status (date_time, ping_server, status) "
                    "VALUES (:date_time, :ping_server, :status)");

	query.bindValue(":date_time",	data.m_dateTime);
	query.bindValue(":ping_server", data.m_responseTime);
    query.bindValue(":status",      data.toJson());


    result = query.exec();   
    return result;
}

bool DataBaseQt::getStatus(StatusData &data) const
{
	QSqlQuery query("SELECT * FROM status ");

	while (query.next())
	{
		int ind_date_time     = query.record().indexOf("date_time");
		int ind_ping_server        = query.record().indexOf("ping_server");
		int ind_status      = query.record().indexOf("status");		
		QString json = query.value(ind_status).toString();
		
		data.fromJson(json.toLatin1());
		return true;
	}
	return false;

	/*data.m_gfs_run_date = QDate::currentDate();
	data.m_gfs_run_hour = 0;
	data.m_gfs_update_time_utc = QDateTime::currentDateTime();
	data.m_current_job = "job!";
	return true;*/
}

//добавить лог
bool DataBaseQt::addLog(const LogData &data) const
{
    bool result = false;
    if (!m_bLogsEnable)
        return false;
    QSqlQuery query(m_db);

    query.prepare(  "INSERT INTO logs (date_time, source, type, text) "
                    "VALUES (:date_time, :source, :type, :text)");

    query.bindValue(":date_time",   data.m_time_t);
    query.bindValue(":source",      data.m_source);
    query.bindValue(":type",        data.m_type);
    query.bindValue(":text",        data.m_text);

    result = query.exec();
    return result;
}

//получить список логов
std::vector<LogData> DataBaseQt::getLogs(const QDateTime &begin, const QDateTime &end) const
{
    std::vector<LogData> result;
    QSqlQuery query("SELECT * FROM logs order by id desc limit 100");
    while (query.next())
    {
        int ind_date_time   = query.record().indexOf("date_time");
        int ind_source      = query.record().indexOf("source");
        int ind_type        = query.record().indexOf("type");
        int ind_text        = query.record().indexOf("text");
        LogData data;
        data.m_source       = query.value(ind_source).toString();
        data.m_text         = query.value(ind_text).toString();
        data.m_time_t       = query.value(ind_date_time).toInt();
        data.m_type         = query.value(ind_type).toInt();
        result.insert(result.begin(),data);
        //result.push_back(data);
    }
    return result;
}

QString DataBaseQt::getLastLogs(int count) const
{
    QString result;
    QSqlQuery query("SELECT * FROM logs order by id desc limit " + QString::number(count));
    while (query.next())
    {
        int ind_date_time   = query.record().indexOf("date_time");
        int ind_source      = query.record().indexOf("source");
        int ind_type        = query.record().indexOf("type");
        int ind_text        = query.record().indexOf("text");
        LogData data;
        data.m_source       = query.value(ind_source).toString();
        data.m_text         = query.value(ind_text).toString();
        data.m_time_t       = query.value(ind_date_time).toInt();
        data.m_type         = query.value(ind_type).toInt();
        result += data.toQString() + " <p>\n";
        //result.insert(result.begin(),data);
        //result.push_back(data);
    }
    return result;
}

//очистить лог до указанной даты
bool DataBaseQt::clearLog(const QDateTime &date) const
{
    bool result = false;
    QSqlQuery query(m_db);
    int dateTime_t =  date.toTime_t();
    query.prepare("DELETE FROM log WHERE date_time < :date_time");
    query.bindValue(":date_time", dateTime_t);
    result = query.exec();
    return result;
}

QMap<QString,QString> DataBaseQt::getDrawParams() const
{
    QMap<QString,QString> result;

    QSqlQuery query("SELECT * FROM drawParam ");
    while (query.next())
    {
        int ind_slave_uid   = query.record().indexOf("slave_uid");
        int ind_draw_param  = query.record().indexOf("draw_param");
        QString uid         = query.value(ind_slave_uid).toString();
        QString draw_param  = query.value(ind_draw_param).toString();
        result.insert(uid,draw_param);
    }
    return result;
}

bool DataBaseQt::setDrawParams(QMap<QString,QString> drawParams) const
{
    QStringList keys = drawParams.keys();
    bool result = false;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM drawParam");
    result = query.exec();

    for (int i = 0; i < keys.size(); i++)
    {
        QString uid = keys.at(i);
        QString drawParam = drawParams.value(uid, QString());
        if (drawParam.isEmpty())
            continue;

        query.prepare(  "INSERT INTO drawParam (slave_uid, draw_param) "
                        "VALUES (:slave_uid, :draw_param)");
        query.bindValue(":slave_uid",   uid);
        query.bindValue(":draw_param",  drawParam);
        result = query.exec();
    }





    return result;
}

QMap<QString,SeaData> DataBaseQt::getMapSea() const
{

    QMap<QString,SeaData> result;

    QSqlQuery query("SELECT * FROM seaTable ");
    while (query.next())
    {
        int ind_sea_name    = query.record().indexOf("sea_name");
        int ind_data_addr   = query.record().indexOf("data_addr");
        int ind_sea_rus_name= query.record().indexOf("sea_rus_name");
        int ind_date_time   = query.record().indexOf("date_time");
        int ind_check_data  = query.record().indexOf("check_data");

        SeaData data;
        data.sea_name       = query.value(ind_sea_name).toString();
        data.data_addr      = query.value(ind_data_addr).toString();
        data.sea_rus_name   = query.value(ind_sea_rus_name).toString();
        data.date               = query.value(ind_date_time).toInt();
        data.check_data         = query.value(ind_check_data).toInt();
        result[data.sea_name]  = data;
        //result.insert(uid,draw_param);
    }
    return result;
}

bool DataBaseQt::importSeaTextFile(const QString &fileName) const
{
    bool result = false;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

    if (file.size() > 0)
    {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM seaTable");
        result = query.exec();
    }

    while (!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString string(line);
        QString name, addr, rus_name;
        int checkData;
        name = string.section(';', 0, 0);
        addr = string.section(';', 1, 1);
        rus_name = string.section(';', 2, 2);
        bool bRes = false;
        checkData = string.section(';', 3, 3).toInt(&bRes);
        if (!bRes)
            checkData = 0;
        bRes = insertSeaData(name, addr, rus_name, checkData);
    }
    file.close();
    return result;
}

bool DataBaseQt::insertSeaData(const QString &name, const QString &addr,const QString &rus_name, int check) const
{
    bool result = false;

    QSqlQuery query(m_db);
    query.prepare(  "INSERT INTO seaTable (sea_name, sea_rus_name, data_addr, date_time, check_data) "
                    "VALUES (:sea_name, :sea_rus_name, :data_addr, :date_time, :check_data)");
    query.bindValue(":sea_name",        name);
    query.bindValue(":sea_rus_name",    rus_name);
    query.bindValue(":data_addr",       addr);
    query.bindValue(":date_time",       0);
    query.bindValue(":check_data",      check);

    result = query.exec();
    return result;
}

bool DataBaseQt::updateSeaData(const SeaData &seaData) const
{
    bool result = false;

    QSqlQuery query(m_db);

    query.prepare(  "UPDATE seaTable SET sea_rus_name = :sea_rus_name, "
                    " data_addr = :data_addr,	"
                    " date_time = :date_time,	"
                    " check_data = :check_data	"
                    " WHERE sea_name = :sea_name ");

    query.bindValue(":sea_name",        seaData.sea_name);
    query.bindValue(":sea_rus_name",    seaData.sea_rus_name);
    query.bindValue(":data_addr",       seaData.data_addr);
    query.bindValue(":date_time",       seaData.date);
    query.bindValue(":check_data",      seaData.check_data);

    result = query.exec();
    return result;
}


bool DataBaseQt::saveLogsToFile(const QString &logFile) const
{
    bool result = false;
    try
    {
        QSqlQuery query("SELECT * FROM logs ORDER BY id DESC ");
        QByteArray ba;

        while (query.next())
        {
            int ind_date_time   = query.record().indexOf("date_time");
            int ind_source      = query.record().indexOf("source");
            int ind_type        = query.record().indexOf("type");
            int ind_text        = query.record().indexOf("text");
            LogData data;
            data.m_source       = query.value(ind_source).toString();
            data.m_text         = query.value(ind_text).toString();
            data.m_time_t       = query.value(ind_date_time).toInt();
            data.m_type         = query.value(ind_type).toInt();
            QString string = data.toQString() + "\n";
            ba.append(string);
        }

        if (ba.size() > 0)
        {
            QSqlQuery query1(m_db);
            query1.prepare("DELETE  FROM logs");
            result = query1.exec();

            query1.prepare("VACUUM");
            query1.exec();
        }
        else
            return false;

        QFile file(logFile);
        result = file.open(QFile::WriteOnly | QFile::Text);
        if (!result)
            return false;
        file.write(ba);
        file.close();
    }
    catch (...)
    {
    }
    return result;
}

bool DataBaseQt::clearSeas() const
{
	QSqlQuery query1(m_db);
	query1.prepare("DELETE  FROM seaTable");
	return query1.exec();
}
