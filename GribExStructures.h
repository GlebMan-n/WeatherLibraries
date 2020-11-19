#ifndef GRIBEXSTRUCTURES_H
#define GRIBEXSTRUCTURES_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDate>
#include <QTextStream>
#include <QDateTime>
#include <QDataStream>
#include <QMap>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QVariant>
#include <QCryptographicHash>
#include "grib_lib_global.h"
#include "GribUtility.h"
#include <gis_2D/gis/GisBase/core/encoding.h>
#include <common/include/helper_string.h>

enum REQ_ERRORS {	FAIL, 
					OK, 
					NO_BODY, 
					BAD_AUTH, 
					BAD_PARSE, 
					FILE_READY, 
					FILE_NOT_READY, 
					SET_DEL_FAIL, 
					SET_DEL_OK, 
					REQ_NOT_FOUND, 
					FILE_NOT_ACTUAL, 
					FILE_NOT_FOUND, 
					GET_BLOCK,
					GET_BLOCK_INFO, 
					BLOCK_INFO,
					BLOCKS_NOT_FOUND,
					GET_STATUS,
					DATA_STATUS,
                    GET_REQ_LIST};

/* Класс данных для формирования запроса к ZyGrib **/
class GRIB_LIB_EXPORT ZygribConsts
{
public:
	ZygribConsts()
    {
		m_host = "www.zygrib.org";
		m_scriptpath = "/noaa/";
		m_scriptstock = "313O562/";
		m_zygriblog = "a07622b82b18524d2088c9b272bb3feeb0eb1737";
		m_zygribpwd = "61c9b2b17db77a27841bbeeabff923448b0f6388";
		m_grbFileResolution = "getzygribfile025.php?";
		m_client = "zyGrib_win-7.0.0";
    }

    QString m_host;
    QString m_scriptpath;
    QString m_scriptstock;
    QString m_zygriblog;
    QString m_zygribpwd;
    QString m_grbFileResolution;
    QString m_client;
    QString m_srvAddr;
};

/* Базовый класс данных запроса **/
class GRIB_LIB_EXPORT GribRequest 
{
public:	
	GribRequest() 
	{
		
		m_type = 0;
	} // 0 - базовый класс
								// 1 - запрос погоды для нашего сервера
								// 2 - ответ с UID
								// 3 - запрос файла
								// 4 - запрос на удаление
	
	virtual QByteArray toJson() 
	{
		QJsonObject jsonObj;
		insertAuthToJson(jsonObj);		
		QJsonDocument jsonOutDoc;
		jsonOutDoc.setObject(jsonObj);			
		return jsonOutDoc.toJson();
	}
	virtual bool fromJson(const QByteArray &ba) 
	{
		QJsonObject jsonObj;
		QJsonDocument jsonOutDoc;
		QJsonParseError error;

		jsonOutDoc = QJsonDocument::fromJson(ba,&error);
		jsonObj = jsonOutDoc.object();

		if(error.offset > 0)
			return false;

		takeAuthFromJson(jsonObj);
		return true;
	}
	int getType() const {return m_type;}
	void setType(const int &type){m_type = type;}

	void genKey()
	{
		QCryptographicHash cryp(QCryptographicHash::Md5);
		cryp.addData(toJson());
		m_key = cryp.result().toHex();
	}

	bool checkKey(QString &error)
	{
		if (m_key.isEmpty())
		{
			error = "_m_key is empty_";
			return false;
		}
		QCryptographicHash cryp(QCryptographicHash::Md5);
		cryp.addData(toJson());
		QString key = cryp.result().toHex();
		if (key != m_key)
			error = "_m_key is invalid_";
		return key == m_key;
	}

public:
	QString m_login;	/* логин */
	QString m_password;	/* пароль */
	QString m_key;		/* ключ */
	QString	m_senderIp;		/* ip адрес отправившего запрос */
	REQ_ERRORS m_error;
protected:
	void insertAuthToJson(QJsonObject& jsonObj)
	{
		jsonObj.insert("login", QJsonValue(m_login));
		jsonObj.insert("password", QJsonValue(m_password));
		jsonObj.insert("key", QJsonValue(m_key));
		jsonObj.insert("type", QJsonValue(m_type));	
		jsonObj.insert("senderIp", QJsonValue(m_senderIp));	
		jsonObj.insert("error", QJsonValue((int)m_error));
	}
	
	void takeAuthFromJson(QJsonObject& jsonObj)
	{
		m_login		= jsonObj.take("login").toString();
		m_password	= jsonObj.take("password").toString();
		m_key		= jsonObj.take("key").toString();
		m_type		= jsonObj.take("type").toInt();
		m_senderIp	= jsonObj.take("senderIp").toString();
		m_error		= static_cast<REQ_ERRORS>(jsonObj.take("error").toInt());
	}	
	
protected:
	int		m_type;		
};

/* Класс данных запроса **/
class GRIB_LIB_EXPORT GribRequestData : public GribRequest
{
public:	
	GribRequestData()
	{
		m_type			= 1;
		m_x0			= 21.0;
		m_y0			= 64.0;
		m_x1			= 44.0;
		m_y1			= 52.0;
		m_resolution	= 0.25;
		m_interval		= 3;
		m_days			= 3;
		m_requestType	= 0;
		m_runGFS		= "last";
		m_parametrs		= "W;P;R;C;T;H;I;S;s;Z;c;i;G;";
		m_date_time_t	= 0;
		m_status		= 0;
		m_deleted		= 0;
		m_error			= FAIL;
		m_clients_id	= QString();
		m_name			= QString();
		m_uid_slave		= QString();
		m_uid_master	= QString();
	}

	bool operator == (const GribRequestData  &src)
	{
		return(
			m_x0 == src.m_x0 &&
			m_y0  == src.m_y0 &&
			m_x1  == src.m_x1 &&
			m_y1  == src.m_y1 &&
			m_resolution == src.m_resolution &&
			m_interval == src.m_interval &&
			m_days == src.m_days &&
			m_requestType == src.m_requestType &&
			m_runGFS == src.m_runGFS &&
			m_parametrs == src.m_parametrs &&
			m_creatorIp == src.m_creatorIp &&	
			m_uid_master == src.m_uid_master &&
			m_uid_slave == src.m_uid_slave &&
			m_date_time_t == src.m_date_time_t &&
			m_status == src.m_status &&
			m_clients_id == src.m_clients_id &&
			m_deleted == src.m_deleted &&
			m_name == src.m_name &&
			m_login == src.m_login &&
			m_password == src.m_password &&
			m_key == src.m_key &&
			m_senderIp == src.m_senderIp &&
			m_error == src.m_error
			);
	}

	const GribRequestData &operator = (const GribRequestData  &src)
	{

		if(this == &src)
			return *this;
		m_x0			= src.m_x0;
		m_y0			= src.m_y0;
		m_x1			= src.m_x1;
		m_y1			= src.m_y1;
		m_resolution	= src.m_resolution;
		m_interval		= src.m_interval;
		m_days			= src.m_days;
		m_requestType	= src.m_requestType;
		m_runGFS		= src.m_runGFS;
		m_parametrs		= src.m_parametrs;
		m_creatorIp		= src.m_creatorIp;	
		m_uid_master	= src.m_uid_master;
		m_uid_slave		= src.m_uid_slave;
		m_date_time_t	= src.m_date_time_t;
		m_status		= src.m_status;
		m_clients_id	= src.m_clients_id;
		m_deleted		= src.m_deleted;
		m_name			= src.m_name;
		m_login			= src.m_login;
		m_password		= src.m_password;
		m_key			= src.m_key;
		m_senderIp		= src.m_senderIp;
		m_error			= src.m_error;
		return *this;
	}

	QString htmlHeadersTableString()
	{
		QString	names;
		names.append("<tr><span	style=\"font-size:20px\">");
		names.append("<td nowrap=\"nowrap\"	align=\"center\">");
        
		names += juce_qt(tr_juce("имя региона"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">"); 
		names += juce_qt(tr_juce("идентификатор клиента"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("IP	адрес создавшего"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("IP	адрес отправителя"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("последнее обновление (LOC)"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("главный идентификатор"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("свой идентификатор"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("X0"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("Y0"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("X1"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("Y1"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("разрешение"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("интервал"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("дней"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("тип запроса"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("GFS"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("параметры"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("статус"));
		names.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		names += juce_qt(tr_juce("удален	ли"));
		names.append("</td>");
		names.append("</span></tr>");

		return names;
	}
	  	  
	QString toHtmlTableString()
	{
		QString request;
        if (m_status == 0)
		    request.append("<tr id=\"tr1\" style=\"background-color:rgba(255, 0, 0, 0.2);\">");
        else
            request.append("<tr><span style=\"font-size:18px\">");
		request.append("<td nowrap=\"nowrap\">");
		request += m_name;
		request.append("</td><td nowrap=\"nowrap\">");
		request += m_clients_id;
		request.append("</td><td nowrap=\"nowrap\">");
		request += m_creatorIp;
		request.append("</td><td nowrap=\"nowrap\">");
		request += m_senderIp;
		request.append("</td><td nowrap=\"nowrap\">");		
		int tt = m_date_time_t;
		request += QDateTime::fromTime_t(tt).toString("HH:mm dd.MM.yyyy");
		request.append("</td><td nowrap=\"nowrap\">");
		request += m_uid_master;
		request.append("</td><td nowrap=\"nowrap\">");
		request += m_uid_slave;
		request.append("</td><td nowrap=\"nowrap\">");
		request += QString::number(m_x0,'f',6);
		request.append("</td><td nowrap=\"nowrap\">");
		request += QString::number(m_y0,'f',6);
		request.append("</td><td nowrap=\"nowrap\">");
		request += QString::number(m_x1,'f',6);
		request.append("</td><td nowrap=\"nowrap\">");
		request += QString::number(m_y1,'f',6);
		request.append("</td><td nowrap=\"nowrap\">");
		request += QString::number(m_resolution,'f',6);
		request.append("</td><td nowrap=\"nowrap\">");
		request += QString::number(m_interval);
		request.append("</td><td nowrap=\"nowrap\">");
		request += QString::number(m_days);
		request.append("</td><td nowrap=\"nowrap\">");
		request += QString::number(m_type);
		request.append("</td><td nowrap=\"nowrap\">");
		request += m_runGFS;
		request.append("</td><td nowrap=\"nowrap\">");
		request += m_parametrs;
        request.append("</td><td nowrap=\"nowrap\">");
		request += QString::number(m_status);		
        request.append("</td><td nowrap=\"nowrap\">");		
		request += m_deleted ? juce_qt(tr_juce("удален")) : juce_qt(tr_juce("не удален"));
		request.append("</td>");
		request.append("</span></tr>");
		return request;
	}

	bool checkActuality()
	{ 
        QDateTime local;
		local = QDateTime::currentDateTime();
		QString dtime = local.toString(Qt::RFC2822Date);
		QString timeZone = dtime.section(" ",4,4);
		int tz = timeZone.left(3).toInt();

		uint time = m_date_time_t + ( (tz - 3) * 3600);
		qint64 timet = QDateTime::currentDateTime().toTime_t();
		int delta = timet - time;
		int raznD = (m_interval + 6) * 3600;
		if(m_date_time_t == 0 || delta > raznD)
			return false;
		return true;
	}
	
	/** получение строки JSON из класса*/
	virtual QByteArray toJson() 
	{
		QJsonObject jsonObj;
		insertAuthToJson(jsonObj);				
		jsonObj.insert("x0", QJsonValue(m_x0));
		jsonObj.insert("y0", QJsonValue(m_y0));
		jsonObj.insert("x1", QJsonValue(m_x1));
		jsonObj.insert("y1", QJsonValue(m_y1));
		jsonObj.insert("resolution", QJsonValue(m_resolution));
		jsonObj.insert("interval", QJsonValue(m_interval));
		jsonObj.insert("requestType", QJsonValue(m_requestType));
		jsonObj.insert("days", QJsonValue(m_days));
		jsonObj.insert("runGFS", QJsonValue(m_runGFS));
		jsonObj.insert("parametrs", QJsonValue(m_parametrs));	
		jsonObj.insert("creatorIp", QJsonValue(m_creatorIp));
		jsonObj.insert("uid_slave", QJsonValue(m_uid_slave));
		jsonObj.insert("uid_master", QJsonValue(m_uid_master));	
		jsonObj.insert("date_time", QJsonValue(m_date_time_t));	
		jsonObj.insert("status", QJsonValue(m_status));
		jsonObj.insert("clients_id", QJsonValue(m_clients_id));	
		jsonObj.insert("deleted", QJsonValue(m_deleted));		
		jsonObj.insert("name", QJsonValue(m_name));
		QJsonDocument jsonOutDoc;
		jsonOutDoc.setObject(jsonObj);			
		return jsonOutDoc.toJson();
	}
	
	/** заполнение класса из строки JSON */
	virtual bool fromJson(const QByteArray &ba)
	{
		QJsonObject jsonObj;
		QJsonDocument jsonOutDoc;
		QJsonParseError error;
		
		jsonOutDoc = QJsonDocument::fromJson(ba,&error);
		jsonObj = jsonOutDoc.object();

		if(error.offset > 0)
			return false;
				
		takeAuthFromJson(jsonObj);
				
		m_x0			= jsonObj.take("x0").toDouble();
		m_y0			= jsonObj.take("y0").toDouble();
		m_x1			= jsonObj.take("x1").toDouble();
		m_y1			= jsonObj.take("y1").toDouble();
		m_resolution	= jsonObj.take("resolution").toDouble();
		m_interval		= jsonObj.take("interval").toInt();
		m_days			= jsonObj.take("days").toInt();
		m_requestType	= jsonObj.take("requestType").toInt();	
		m_runGFS		= jsonObj.take("runGFS").toString();
		m_parametrs		= jsonObj.take("parametrs").toString();
		m_creatorIp		= jsonObj.take("creatorIp").toString();	
		m_uid_master	= jsonObj.take("uid_master").toString();	
		m_uid_slave		= jsonObj.take("uid_slave").toString();					
		m_date_time_t	= jsonObj.take("date_time").toInt();					
		m_status		= jsonObj.take("status").toInt();
		m_clients_id	= jsonObj.take("clients_id").toString();
		m_deleted		= jsonObj.take("deleted").toInt();	
		m_name			= jsonObj.take("name").toString();	
		return true;
	}
		
	float		m_x0;			/* координаты квадрата */
	float		m_y0;			/* координаты квадрата */
	float		m_x1;			/* координаты квадрата */
	float		m_y1;			/* координаты квадрата */
	float		m_resolution;	/* разрешение графического слоя */
	int			m_interval;		/* интервал запросов в часах */
	int			m_days;			/* запрос на количество дней */
	int			m_requestType;	/* тип запроса */
	QString		m_runGFS;		/* *** */
	QString		m_parametrs;	/* параметры запроса */
	QString		m_creatorIp;	/* ip адрес создавшего запрос */	
	QString		m_uid_master;	/* uid, назначенный сервером */
	QString		m_uid_slave;	/* собственный uid */
	qint64		m_date_time_t;	/* дата и время изменения*/
	short		m_status;		/* статус заявки */
	QString		m_clients_id;	/* идентификатор клиента, приславшего запрос */
	short		m_deleted;		/* признак удаления заявки */
	QString		m_name;			/* имя запроса */
};



/* Класс ответа от сервера **/
class GRIB_LIB_EXPORT GribUIDResp : public GribRequest
	{
	public:
		GribUIDResp() 
		{ 
			setType(2); 
		}

		/** получение строки JSON из класса*/
		virtual QByteArray toJson()
		{
			QJsonObject jsonObj;
			insertAuthToJson(jsonObj);				

			jsonObj.insert("uid_master", QJsonValue(m_uid_master));	
			jsonObj.insert("uid_slave", QJsonValue(m_uid_slave));
			jsonObj.insert("info", QJsonValue(m_info));	
			
			QJsonDocument jsonOutDoc;
			jsonOutDoc.setObject(jsonObj);			
			return jsonOutDoc.toJson();
		}

		/** заполнение класса из строки JSON */
		virtual bool fromJson(const QByteArray &ba)
		{
			QJsonObject jsonObj;
			QJsonDocument jsonOutDoc;
			QJsonParseError error;

			jsonOutDoc = QJsonDocument::fromJson(ba,&error);
			jsonObj = jsonOutDoc.object();

			if(error.offset > 0)
				return false;

			takeAuthFromJson(jsonObj);
			m_uid_master		= jsonObj.take("uid_master").toString();
			m_uid_slave			= jsonObj.take("uid_slave").toString();
			m_info				= jsonObj.take("info").toString();			
			return true;
		}
		
		QString m_uid_master;
		QString m_uid_slave;
		QString m_info;		
	};

/* Класс ответа от сервера **/
class GRIB_LIB_EXPORT GribFileResp : public GribRequest
{
public:
	GribFileResp() 
	{ 
		setType(3); 
		m_date_time = 0;
		m_uid_master = QString();
		m_uid_slave = QString();
	}


	/*bool checkActuality(int actHours)
	{
		if(m_date_time == 0 || m_date_time / 3600 > actHours)
			return false;
		return true;
	}*/
	
/** получение строки JSON из класса*/
virtual QByteArray toJson()
{
	QJsonObject jsonObj;
	insertAuthToJson(jsonObj);				
	
	
	jsonObj.insert("uid_master", QJsonValue(m_uid_master));
	jsonObj.insert("uid_slave", QJsonValue(m_uid_slave));
	jsonObj.insert("date_time", QJsonValue(m_date_time));
	jsonObj.insert("filename", QJsonValue(m_filename));
	
	
	QJsonDocument jsonOutDoc;
	jsonOutDoc.setObject(jsonObj);				
	return jsonOutDoc.toJson();
}
	int createDateTime_T()
	{
		return QFileInfo(m_filename).lastModified().toTime_t();
	}

/** заполнение класса из строки JSON */
virtual bool fromJson(const QByteArray &ba)
	{
		QJsonObject jsonObj;
		QJsonDocument jsonOutDoc;
		QJsonParseError error;

		jsonOutDoc = QJsonDocument::fromJson(ba,&error);
		jsonObj = jsonOutDoc.object();

		if(error.offset > 0)
			return false;
			
		takeAuthFromJson(jsonObj);		
			
		m_uid_master	= jsonObj.take("uid_master").toString();
		m_uid_slave		= jsonObj.take("m_uid_slave").toString();
		m_date_time		= jsonObj.take("m_date_time").toInt();
		m_filename		= jsonObj.take("m_filename").toString();
		return true;
	}
	
	QString		m_uid_master;
	QString		m_uid_slave;
	qint64		m_date_time;
	QString		m_filename;
};

/* Класс параметров подключения к серверу **/
class GRIB_LIB_EXPORT ConnectionInfo
{
public:
    ConnectionInfo()
    {
		m_serverAddr		= "";
		m_serverPort		= 0;
		m_regDate			= QDateTime::currentDateTime();
		m_lastCheckDate		= QDateTime::currentDateTime();
		m_lastFileDate		= QDateTime::currentDateTime();	   
		m_lastFileName		= "";	
		m_url				= "";
		m_databaseFullName	= QString();
		m_pathToGribData	= QString();
		m_login				= QString();
		m_pathToIceData		= QString();
    }
    
	bool operator == (const ConnectionInfo  &src)
		{
		return(
			m_serverAddr		== src.m_serverAddr &&
			m_serverPort		== src.m_serverPort &&
			m_regDate			== src.m_regDate &&
			m_lastCheckDate		== src.m_lastCheckDate &&
			m_lastFileDate		== src.m_lastFileDate &&
			m_lastFileName		== src.m_lastFileName &&
			m_url				== src.m_url &&
			m_databaseFullName	== src.m_databaseFullName &&
			m_pathToGribData	== src.m_pathToGribData &&
			m_login				== src.m_login &&
			m_password			== src.m_password &&
			m_pathToIceData		== src.m_pathToIceData
			);
		}

	const ConnectionInfo &operator = (const ConnectionInfo  &src)
		{

		if(this == &src)
			return *this;

		m_serverAddr		= src.m_serverAddr;
		m_serverPort		= src.m_serverPort;
		m_regDate			= src.m_regDate;
		m_lastCheckDate		= src.m_lastCheckDate;
		m_lastFileDate		= src.m_lastFileDate;
		m_lastFileName		= src.m_lastFileName;
		m_url				= src.m_url;
		m_databaseFullName	= src.m_databaseFullName;
		m_pathToGribData	= src.m_pathToGribData;
		m_login				= src.m_login;
		m_password			= src.m_password;
		m_pathToIceData		= src.m_pathToIceData;
		return *this;
		} 
    
    QString     m_serverAddr;
    int         m_serverPort;
    QDateTime	m_regDate;
    QDateTime	m_lastCheckDate;
    QDateTime	m_lastFileDate;   
    QString		m_lastFileName;    
    QString		m_url;
    QString		m_databaseFullName;
	QString		m_pathToGribData;
	QString		m_pathToIceData;
	QString		m_login;
	QString		m_password;
};

/** Класс параметров Raw заголовка */
class GRIB_LIB_EXPORT RawHeaderData
{
public:
    RawHeaderData()
        {
        m_connection = QObject::tr("Keep-Alive").toUtf8();
        m_mirrorHttp = QObject::tr("").toUtf8();
        m_lastIp = QObject::tr("0").toUtf8();
        m_codedHeader = QObject::tr("").toUtf8();
        m_sessionGUID = QObject::tr("").toUtf8();
        m_acceptEncoding = QObject::tr("no-compression").toUtf8();
        m_contentType = QObject::tr("text/xml;charset=UTF-8").toUtf8();
        m_userAgent = QObject::tr("zyGrib_win/7.0.0").toUtf8();
        m_saveData = QObject::tr("").toUtf8();
		m_login			= QObject::tr("login").toUtf8(); //"login";
		m_password		= QObject::tr("passwd").toUtf8(); //"passwd";
		m_key			= QObject::tr("key1").toUtf8(); //"key1";
        }

    RawHeaderData	(
			QString connection,
			QString mirrorHttp,
			QString lastIp,
			QString codedHeader,
			QString sessionGUID,
			QString acceptEncoding,
			QString contentType,
			QString userAgent,
			QString saveData,
			QString login,
			QString password,
			QString key
					)
        {
			m_connection = connection.toUtf8();
			m_mirrorHttp = mirrorHttp.toUtf8();
			m_lastIp = lastIp.toUtf8();
			m_codedHeader = codedHeader.toUtf8();
			m_sessionGUID = sessionGUID.toUtf8();
			m_acceptEncoding = acceptEncoding.toUtf8();
			m_contentType = contentType.toUtf8();
			m_userAgent = userAgent.toUtf8();
			m_saveData = saveData.toUtf8();
			m_login			= login.toUtf8();
			m_password		= password.toUtf8();
			m_key			= key.toUtf8();
        }	

    bool operator == (const RawHeaderData  &src)
        {
        return(
            m_connection		== src.m_connection &&
            m_mirrorHttp		== src.m_mirrorHttp &&
            m_lastIp			== src.m_lastIp &&
            m_codedHeader		== src.m_codedHeader &&
            m_sessionGUID		== src.m_sessionGUID &&
            m_acceptEncoding	== src.m_acceptEncoding &&
            m_contentType		== src.m_contentType &&
            m_userAgent			== src.m_userAgent &&
            m_saveData			== src.m_saveData &&
			m_login				== src.m_login &&
			m_password			== src.m_password &&
			m_key				== src.m_key
            );
        }

    const RawHeaderData &operator = (const RawHeaderData  &src)
        {
        
			if(this == &src)
				return *this;
				
			m_connection		= src.m_connection;
			m_mirrorHttp		= src.m_mirrorHttp;
			m_lastIp			= src.m_lastIp;
			m_codedHeader		= src.m_codedHeader;
			m_sessionGUID		= src.m_sessionGUID;
			m_acceptEncoding	= src.m_acceptEncoding;
			m_contentType		= src.m_contentType;
			m_userAgent			= src.m_userAgent;
			m_saveData			= src.m_saveData;
			m_login				= src.m_login;
			m_password			= src.m_password;
			m_key				= src.m_key;
			
			return *this;
        } 

	QMap<QString, QString> getMap()
	{
		QMap<QString, QString> rawHeadersMap;

		#pragma region INIT_RAW_HEADERS_MAP
		{
			if(!m_connection.isEmpty())
				rawHeadersMap.insert("Connection",QString::fromUtf8(m_connection));

			if(!m_contentType.isEmpty())
				rawHeadersMap.insert("Content-Type",QString::fromUtf8(m_contentType));
				
			if(!m_mirrorHttp.isEmpty())
				rawHeadersMap.insert("MirrorHttp",QString::fromUtf8(m_mirrorHttp));

			if(!m_lastIp.isEmpty()) 
				rawHeadersMap.insert("LastIp",QString::fromUtf8(m_lastIp));	 

			if(!m_codedHeader.isEmpty())   
				rawHeadersMap.insert("CodedHeader",QString::fromUtf8(m_codedHeader));	

			if(!m_sessionGUID.isEmpty())  
				rawHeadersMap.insert("SessionGUID",QString::fromUtf8(m_sessionGUID));	

			if(!m_userAgent.isEmpty()) 
				rawHeadersMap.insert("User-Agent",QString::fromUtf8(m_userAgent));	

			if(!m_acceptEncoding.isEmpty())
				rawHeadersMap.insert("Accept-Encoding",QString::fromUtf8(m_acceptEncoding)); 

			if(!m_saveData.isEmpty())
				rawHeadersMap.insert("SaveToFile",QString::fromUtf8(m_saveData));  
				
			if(!m_login.isEmpty())
				rawHeadersMap.insert("LOGIN",QString::fromUtf8(m_login));  
				
			if(!m_password.isEmpty())
				rawHeadersMap.insert("PASSWORD",QString::fromUtf8(m_password));  
				
			if(!m_key.isEmpty())
				rawHeadersMap.insert("KEY",QString::fromUtf8(m_key));  
		}	
		#pragma endregion END_INIT_RAW_HEADERS_MAP 

		return rawHeadersMap;
	}
    QByteArray m_connection;
    QByteArray m_mirrorHttp;
    QByteArray m_lastIp;
    QByteArray m_codedHeader;
    QByteArray m_sessionGUID;
    QByteArray m_acceptEncoding;
    QByteArray m_contentType;
    QByteArray m_userAgent;
    QByteArray m_saveData;
	QByteArray m_login;
	QByteArray m_password;
	QByteArray m_key;
};

/* класс с данными ответа от сервера Аргун-Софт **/    
class GRIB_LIB_EXPORT GribDataA
{
public:
    GribDataA()
    {
    }

    GribDataA(  QString status,
        QString file,
        QString size,
        QString checksum)
        {
			m_status = status;
			m_file = file;
			m_size = size;
			m_checksum = checksum;
        }

    QString m_status;
    QString m_file;
    QString m_size;
    QString m_checksum;
};

//namespace ExStruct
//{
	/* класс для передачи записи с логом в бд **/   
//	enum TYPE_LOG_DATA { UNKNOWN,INFO,WARNING,ERROR };
class GRIB_LIB_EXPORT LogData
{	
public:	
	
	void setType(int type)
	{
		m_type = type;
	}

	LogData()
	{
		setType(1);
		setText("TEST");
	} 

	LogData(int type, QString text)
	{
		setType(type);
		setText(text);
		setCurrentDt();
	} 
	
	void setText(const QString &text)
	{
		m_text = text;
	} 
	void setCurrentDt()
	{
		m_time_t = QDateTime::currentDateTime().toTime_t();
	}

	QString toQString()
	{
		QString result;
		result = QDateTime::fromTime_t(m_time_t).toString(Qt::LocalDate) + " " + m_source + " " + m_text ;
		return result;
	}

	int m_time_t;
	QString m_source;
	int m_type;   // 0 - UNKNOWN 1 - INFO 2 - WARNING 3 - ERROR
	QString m_text;
};   
//}
/* класс для передачи записей справочника моря **/    
class GRIB_LIB_EXPORT SeaData
{
public:
	QString getHtmlTableString()
	{
		QString res;

		res.append("<tr><span	style=\"font-size:20px\">");
		res.append("<td nowrap=\"nowrap\"	align=\"center\">");
		res += sea_rus_name;
		res.append("</td><td nowrap=\"nowrap\" align=\"center\">"); 
		res += QDate::fromJulianDay(date).toString("dd.MM.yyyy");
		res.append("</td><td nowrap=\"nowrap\" align=\"center\">"); 
		if(data_addr.startsWith("http://wdc.aari.ru"))
			res += "DATASETS";//data_addr;
		else
			res += data_addr;
		res.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		res.append(sea_name);
		res.append("</td><td nowrap=\"nowrap\" align=\"center\">");
		res.append(QString::number(check_data));  
		res.append("</td></tr>");
		return res;
	}

	QString sea_name;
	QString data_addr;
	QString sea_rus_name;
	int	date;
	int	check_data;
};

class GRIB_LIB_EXPORT Blocks
{
public:
	/*класс блока файла*/
	class Block
	{
	public:
		/*получить номер блока*/
		int getNumber() const { return m_number; }
		/*установить номер блока*/
		void setNumber(int val) { m_number = val; }
		/*получить чексумму блока*/
		QString getHash() const { return m_hash; }
		/*установить чексумму блока*/
		void setHash(const QString &val) { m_hash = val; }
		/*получить блок*/
		QByteArray getBlock() const { return m_block; }
		/*установить блок*/
		void setBlock(const QByteArray &val) { m_block = val; }

	private:
		//номер блока
		int m_number;
		//чек сумма блока
		QString m_hash;
		//блок
		QByteArray m_block;
	};
	//конструктор класса блоков файла
	//на входе 
	// - имя файла
	// - размер блоков на которые файл бьется
	Blocks()
	{
		m_size = 0;
		m_dt = 0;
		m_bCurrect = false;
	}
	Blocks(const QString &file_name, int block_size )
	{
		m_size = 0;
		m_dt = 0;
		m_block_size = block_size;
		m_bCurrect = init(file_name);
	}
	//блок по номеру
	Block at(int index)
	{
		if (m_size > index && index > -1)
			return m_blocks.value(index);
		return Block();
	}
	//блок по чековой сумме
	Block getBlock(const QString &hash)
	{
		int index = m_hashes.key(hash);
		return at(index);
	}
	//список чек сумм
	QStringList getHashes()
	{
		return m_hashes.values();
	}	
	//количество блоков
	int size()
	{
		return m_size;
	}
	//очистить блоки и информацию о них
	void clear()
	{
		m_blocks.clear();
		m_hashes.clear();
	}
	//получить чековую сумму файла
	QString getHash()
	{
		return m_hash;
	}
	//получить дату и время файла
	int getDateTime_t()
	{
		return m_dt;
	}

	void setDateTime_t(int dt)
	{
		m_dt = dt;
	}
	int getSizeInBytes() const { return m_bytes; }
	void setSizeInBytes(int val) { m_bytes = val; }
	//проверка на корректность
	bool isCurrect()
	{ 
		m_needToReload.clear();
		for(int i = 0; i < m_blocks.size(); i++)
		{
			QString hash = m_blocks.value(i).getHash();;
			QString hashCurrect = genHash(m_blocks.value(i).getBlock());
			if(hash != hashCurrect)
			{
				m_needToReload.append(hash);
			}
		}
		return m_needToReload.isEmpty();
	}
	//добавить в конец
	void append(const Block &block)
	{
		m_hashes[m_size] = block.getHash();
		m_blocks[m_size] = block;
		m_blocks[m_size].setNumber(m_size);
		m_size++;
	}
	//установить блок в позиции..
	void set(int index, const Block &block)
	{
		if (m_size > index && index > -1)
		{
			m_blocks[index] = block;
			m_hashes[index] = block.getHash();
			m_blocks[index].setNumber(index);
		}
	}
	//получить информацию о блоках в строке JSON
	QByteArray toJson()
	{
		QJsonObject jsonObj;
		jsonObj.insert("block_size", QJsonValue(m_block_size));
		jsonObj.insert("dt", QJsonValue(m_dt));
		jsonObj.insert("hash", QJsonValue(m_hash));
		jsonObj.insert("size", QJsonValue(m_size));
		jsonObj.insert("bytes", QJsonValue(m_bytes));
		jsonObj.insert("hashes", QJsonArray::fromStringList(m_hashes.values()));
		QJsonDocument jsonOutDoc;
		jsonOutDoc.setObject(jsonObj);
		return jsonOutDoc.toJson();
	}
	//задать информацию о блоках в строке JSON
	bool fromJson(const QByteArray &ba)
	{		
		QJsonObject jsonObj;
		QJsonDocument jsonOutDoc;
		QJsonParseError error;
		jsonOutDoc = QJsonDocument::fromJson(ba, &error);
		jsonObj = jsonOutDoc.object();
		if (error.offset > 0)
			return false;
		clear();
		QJsonArray ar;

		m_block_size = jsonObj.take("block_size").toInt(0);
		m_dt = jsonObj.take("dt").toInt(0);
		m_hash = jsonObj.take("hash").toString("");
		m_size = jsonObj.take("size").toInt(0);
		m_bytes = jsonObj.take("bytes").toInt(0);
		ar = jsonObj.take("hashes").toArray();

		QVariantList list = ar.toVariantList();

		for (int i = 0; i < list.size(); i++)
		{
			m_hashes.insert(i, list.at(i).toString());
			Block block;
			block.setNumber(i);			
			m_blocks.insert(i, block);
		}

		if (m_size == m_blocks.size() && m_size == m_hashes.size())
			return true;
		else
			return false;
	}

	bool saveFile(const QString &path)
	{			
		QFile file(path);
		int sz = 0;
		QByteArray baAll;
		if(file.open(QIODevice::WriteOnly))
		{
			for(int i = 0; i < m_blocks.size(); i++)
			{				
				QByteArray ba = at(i).getBlock();				
				sz += file.write(ba);
				baAll.append(ba);
			}			   			
			file.close();
			QString hash = genHash(baAll);
			return hash == m_hash;			
		}
		return false;
	}

	QString getCheckToReload()
	{
		if(!m_needToReload.isEmpty())
			return m_needToReload.at(0);

		return QString();
	}
private:
	//инициализация класса
	bool init(const QString &filename)
	{
		try{
			QFile file(filename);
			if (!file.open(QIODevice::ReadOnly))
				return false;
			QByteArray ba_file = file.readAll();
			m_hash = genHash(ba_file);
			m_bytes = ba_file.size();
			file.seek(0);
			while (!file.atEnd())
			{
				QByteArray ba = file.read(m_block_size);
				if (ba.size() > 0)
				{
					QString hash = genHash(ba);
					Blocks::Block block;
					block.setHash(hash);
					block.setBlock(ba);
					append(block);
				}
			}
			file.close();
			return true;
		}
		catch (...)
		{			
		}
		return false;
	}	

	//расчет чексуммы
	QString genHash(const QByteArray &ba)
	{
		QCryptographicHash cryp(QCryptographicHash::Md5);
		cryp.addData(ba);
		return QString(cryp.result().toHex());
	}	

private:
	//нумерованые блоки файла
	QMap<int, Block> m_blocks; 
	//нумерованые хэши блоков файла
	QMap<int, QString> m_hashes;
	//количество блоков
	int m_size;
	//хэш файла
	QString m_hash;
	QString m_hash_info;
	//дата и время файла
	int m_dt;
	//размер блока
	int m_block_size;
	//корректность
	bool m_bCurrect;
	//размер в байтах
	int m_bytes;
	//список чек-сумм на перезагрузку
	QStringList m_needToReload;

};

/* класс для передачи записи статуса сервера **/    
class GRIB_LIB_EXPORT StatusData
{
public:
	StatusData()
	{
		m_dateTime = 0;
		m_responseTime = 0;
	}

    int compare(const StatusData &status)
    {
        bool bRes;
        int hour = status.m_gfs_run_hour.toInt(&bRes);
        if (!bRes)
            return -1;

        int hourSrc = m_gfs_run_hour.toInt(&bRes);
        if (!bRes)
            return -1;

        if(status.m_gfs_run_date != m_gfs_run_date)
            return 0;

        if (hourSrc == hour)
            return 1;
        else
            return 0;
    }

	qint64 m_dateTime;
	qint64 m_responseTime;
	QString m_start_load;		// : 2016 - 10 - 17 09 : 25 : 01 UTC
	QString m_server;			// server: http ://nomads.ncep.noaa.gov/pub/data/nccf/com/gfs/prod/ 
	QString m_gfs_run_date;// : 20161017 gfs_run_hour : 06
	QString m_gfs_run_hour;
	QString m_gfs_update_time;// : 2016 - 10 - 17 11 : 12 : 24
	QString m_current_job;// : waiting for new data
	QString m_FNMOC_WW3_run_date;// : 20161016
	QString m_FNMOC_WW3_run_hour;// : 00
	QString m_FNMOC_WW3_update_time;// : 2016 - 10 - 16 06 : 44 : 18 UTC
	QString m_FNMOC_WW3_current_job;// : waiting for new data
	QString m_FNMOC_WW3_MED_run_date;// : 20161016
	QString m_FNMOC_WW3_MED_run_hour;// : 00
	QString m_FNMOC_WW3_MED_update_time;// : 2016 - 10 - 16 04 : 52 : 28 UTC
	QString m_FNMOC_WW3_MED_current_job;// : waiting for new data

	virtual QByteArray toJson() const
	{
		QJsonObject jsonObj;
		insertToJson(jsonObj);
		QJsonDocument jsonOutDoc;
		jsonOutDoc.setObject(jsonObj);
		return jsonOutDoc.toJson();
	}
	virtual bool fromJson(const QByteArray &ba)
	{
		QJsonObject jsonObj;
		QJsonDocument jsonOutDoc;
		QJsonParseError error;

		jsonOutDoc = QJsonDocument::fromJson(ba, &error);
		jsonObj = jsonOutDoc.object();

		if (error.offset > 0)
			return false;

		takeFromJson(jsonObj);
		return true;
	}
	void insertToJson(QJsonObject& jsonObj) const
	{		
		jsonObj.insert("m_start_load", QJsonValue(m_start_load));
		jsonObj.insert("m_server", QJsonValue(m_server));
		jsonObj.insert("m_gfs_run_date", QJsonValue(m_gfs_run_date));
		jsonObj.insert("m_gfs_run_hour", QJsonValue(m_gfs_run_hour));
		jsonObj.insert("m_gfs_update_time", QJsonValue(m_gfs_update_time));
		jsonObj.insert("m_current_job", QJsonValue(m_current_job));
		jsonObj.insert("m_FNMOC_WW3_run_date", QJsonValue(m_FNMOC_WW3_run_date));
		jsonObj.insert("m_FNMOC_WW3_run_hour", QJsonValue(m_FNMOC_WW3_run_hour));
		jsonObj.insert("m_FNMOC_WW3_update_time", QJsonValue(m_FNMOC_WW3_update_time));
		jsonObj.insert("m_FNMOC_WW3_current_job", QJsonValue(m_FNMOC_WW3_current_job));
		jsonObj.insert("m_FNMOC_WW3_MED_run_date", QJsonValue(m_FNMOC_WW3_MED_run_date));
		jsonObj.insert("m_FNMOC_WW3_MED_run_hour", QJsonValue(m_FNMOC_WW3_MED_run_hour));
		jsonObj.insert("m_FNMOC_WW3_MED_update_time", QJsonValue(m_FNMOC_WW3_MED_update_time));
		jsonObj.insert("m_FNMOC_WW3_MED_current_job", QJsonValue(m_FNMOC_WW3_MED_current_job));
		jsonObj.insert("m_dateTime", QJsonValue(m_dateTime));
		jsonObj.insert("m_responseTime", QJsonValue(m_responseTime));
	}

	void takeFromJson(QJsonObject& jsonObj)
	{
		m_start_load = jsonObj.take("m_start_load").toString();
		m_server = jsonObj.take("m_server").toString();
		m_gfs_run_date = jsonObj.take("m_gfs_run_date").toString();
		m_gfs_run_hour = jsonObj.take("m_gfs_run_hour").toString();
		m_gfs_update_time = jsonObj.take("m_gfs_update_time").toString();
		m_current_job = jsonObj.take("m_current_job").toString();
		m_FNMOC_WW3_run_date = jsonObj.take("m_FNMOC_WW3_run_date").toString();
		m_FNMOC_WW3_run_hour = jsonObj.take("m_FNMOC_WW3_run_hour").toString();
		m_FNMOC_WW3_update_time = jsonObj.take("m_FNMOC_WW3_update_time").toString();
		m_FNMOC_WW3_current_job = jsonObj.take("m_FNMOC_WW3_current_job").toString();
		m_FNMOC_WW3_MED_run_date = jsonObj.take("m_FNMOC_WW3_MED_run_date").toString();
		m_FNMOC_WW3_MED_run_hour = jsonObj.take("m_FNMOC_WW3_MED_run_hour").toString();
		m_FNMOC_WW3_current_job = jsonObj.take("m_FNMOC_WW3_current_job").toString();
		m_FNMOC_WW3_MED_update_time = jsonObj.take("m_FNMOC_WW3_MED_update_time").toString();
		m_FNMOC_WW3_MED_current_job = jsonObj.take("m_FNMOC_WW3_MED_current_job").toString();
		m_dateTime = jsonObj.take("m_dateTime").toInt();
		m_responseTime = jsonObj.take("m_responseTime").toInt();
	}
	QString getHtml() const
	{
		QString html;
		html.append("<table>");
		html.append("<tr><span	style=\"font-size:20px\">");
		html.append("<td nowrap=\"nowrap\"	align=\"left\">");
		html.append(juce_qt(tr_juce("Сервер загрузки данных (NOAA):")));
		html.append("</td><td nowrap=\"nowrap\" align=\"left\">");
		html.append("<a href=\"" + m_server + juce_qt(tr_juce("\"> сервер NOAA </a>")));
		html.append("</td></tr>");
		html.append("<tr><span	style=\"font-size:20px\">");
		html.append("<td nowrap=\"nowrap\"	align=\"left\">");
		html.append(juce_qt(tr_juce("Дата загрузки данных:")));
		html.append("</td><td nowrap=\"nowrap\" align=\"left\">");
		html.append(QDate::fromString(m_gfs_run_date,"yyyyMMdd").toString("dd.MM.yyyy"));
		html.append("</td></tr>");		
		html.append("<tr><span	style=\"font-size:20px\">");
		html.append("<td nowrap=\"nowrap\"	align=\"left\">");
		html.append(juce_qt(tr_juce("Данные от (часы UTC):")));
		html.append("</td><td nowrap=\"nowrap\" align=\"left\">");
		html.append(m_gfs_run_hour);
		html.append("</td></tr>");
		html.append("<tr><span	style=\"font-size:20px\">");
		html.append("<td nowrap=\"nowrap\"	align=\"left\">");
		html.append(juce_qt(tr_juce("Время обновления (UTC):")));
		html.append("</td><td nowrap=\"nowrap\" align=\"left\">");
		QDateTime dt;
		dt = QDateTime::fromString(m_gfs_update_time,"yyyy-MM-dd HH:mm:ss UTC");
		html.append(dt.toString("HH:mm dd.MM.yyyy"));
		html.append("</td></tr>");
		html.append("<tr><span	style=\"font-size:20px\">");
		html.append("<td nowrap=\"nowrap\"	align=\"left\">");
		html.append(juce_qt(tr_juce("Текущая работа сервера:")));
		html.append("</td><td nowrap=\"nowrap\" align=\"left\">");
		if (m_current_job == "waiting for new data")
			html.append(juce_qt(tr_juce("ожидание новых данных - ")) + m_current_job);
		else if (m_current_job == "trying to retrieve new data...")
		{
			html.append(juce_qt(tr_juce("пытаюсь получить новые данные - ")) + m_current_job);
		}
		else if (m_current_job.startsWith("loading run"))
		{
			//loading run 06 - 27.3% ETA:11 : 04Z
			QString work = m_current_job.left(12);
			QString tmp = m_current_job.section("-", 0, 0);
			QString hour;
			hour = tmp.right(3);
			tmp = m_current_job.section("-", 1, 1);
			QString procent = tmp.section("%", 0, 0);
			QString file = tmp.section("%", 1, 1);

			html.append(juce_qt(tr_juce("загрузка данных ")) + procent + juce_qt(tr_juce("%, от ")) + hour + QString::fromLocal8Bit("ч. UTC для файлов ") + file);
		}
		else
			html.append(m_current_job);
		html.append("</td></tr>");		
		html.append("</table>");
		return html;
	}
};


#endif // GRIBEXSTRUCTURES_H
