#include "ReqManager.h"

#include "grib_lib/GribProxyLoaderQt.h"
#include "grib_lib/FileLoaderQt.h"
#include "grib_lib/RequestRegQt.h"
#include "grib_lib/IceFileLoaderQt.h"
#include "grib_lib/StatusLoaderQt.h"


#include <log4cplus.h>
#include <QTimeZone>
#include "gis/GisGrib/Util.h"
#include "gis/GisGrib/GribData.h"
#include <gis_2D/gis/GisBase/core/encoding.h>

log_init_static("log_WeatherReqManager.ini")
log_define_instance(log_weather_req_manager, "weather_req_manager")
#define MINIMUM_GRIB_UPDATE_TIME 3600
#include <common/include/tr_macros.h>

ReqManager::ReqManager(const GribIceFolderDataDriver* driver, QObject *parent)
    : QObject(parent)
{
    m_i_is_root = false;
	m_wdc = true;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotCheckDb()));
    m_timer->start(40000);
	m_weatherDriver = driver;
    m_iceFileLoaderQt = NULL;
	m_statusLoader = NULL;
	m_modMin = 0.000001;
	m_modMax = 0.00001;
	m_autosaveLogs = 0;
}

void ReqManager::setModificators(float modMin, float modMax)
{
	m_modMin = modMin;
	m_modMax = modMax;
}

ReqManager::~ReqManager()
{
	if(m_timer)
		m_timer->stop();
}


void ReqManager::slotWorkCompleate()
{
	FileLoaderQt* fileloader			= qobject_cast<FileLoaderQt*>(sender());
	RequestRegQt* requestRegQt			= qobject_cast<RequestRegQt*>(sender());
	GribLoaderQt* gl					= qobject_cast<GribLoaderQt*>(sender());
	IceFileLoaderQt* iceFileLoaderQt	= qobject_cast<IceFileLoaderQt*>(sender());
	StatusLoaderQt* m_statusLoader		= qobject_cast<StatusLoaderQt*>(sender());
	
	if(m_statusLoader)
	{
       StatusData data_old;
	   StatusData data_new = m_statusLoader->getData();
       m_weatherDriver->getStatus(data_old);

       if (data_old.compare(data_new) == 0)
       {
           toLog(LogData(0,tr("Данные сервера-источника обновлены")));
           QList<GribRequestData> list =  m_weatherDriver->getListRequests();
           foreach(GribRequestData data, list)
               m_weatherDriver->setReqStatus(data.m_uid_slave, 0, data.m_date_time_t);
       }
       m_weatherDriver->setStatus(data_new);
	}

	if(iceFileLoaderQt)
	{
		SeaData data = iceFileLoaderQt->getSeaData();
		bool bRes = m_weatherDriver->updateSeaData(data);

		LogData ldata;
		QString str;
		str += tr("Получены данные по ледовой обстановке: ");
		str += data.sea_rus_name;
		str += tr(" море, от ");
		str += QDate::fromJulianDay(data.date).toString();
		ldata = LogData(1,str);
		toLog(ldata);
	}

	if(fileloader)
	{		
		GribRequestData resp;
		resp.fromJson(fileloader->getData().toJson());		
		try
		{
			QString filename = fileloader->getFileFullName(); 
			
			if(resp.m_error == SET_DEL_OK)
			{
				m_weatherDriver->deleteReq(resp.m_uid_slave);	
				LogData data;
				data = LogData(1,tr("Запрос удален: ") + resp.m_uid_slave);
				toLog(data);
				removeDirAndFilesReq(m_connectionInfo,resp.m_uid_slave);
			}
			else if(resp.m_error == REQ_NOT_FOUND)
			{
				m_weatherDriver->setReqMasterUidNull(resp.m_uid_slave);
				removeRequest(resp.m_uid_slave);
				//removeFileLoader(resp.m_uid_slave);
				LogData data;
				data = LogData(1,tr("Запрос не найден на сервере, перерегистрирую. Локальный ID : ") + resp.m_uid_slave + tr("ID на сервере:") + resp.m_uid_master);
				toLog(data);
			}
			else if(resp.m_error == FILE_NOT_ACTUAL)
			{
				LogData data;
				data = LogData(1,tr("Файл не актуален. Локальный ID : ") + resp.m_uid_slave +  tr("ID на сервере:") + resp.m_uid_master);
				toLog(data);
			}
			else if(resp.m_error == FAIL)
			{
				//removeFileLoader(resp.m_uid_slave);
			}
			else
			{	
				GribData* data = new GribData();
				juce::String error;
				bool bRes = Util::openGribFile(qt_juce(filename), data,error);
				
				if (bRes)
				{
					time_t ct = data->getFirstDate();
					//uint fileUtcTD = ct - (3600*3);
					//resp.m_date_time_t = (ct - (3600*3) + (GribUtility::getCurrentTimeZone()*3600));
					
					resp.m_date_time_t = ct;
					LogData ldata;						
					bRes = resp.checkActuality();

					if(bRes)
					{
						m_weatherDriver->addResponse(resp, filename);
						
						ldata = LogData(1,tr("Файл GRIB загружен: ") + filename + tr(" для id:") + resp.m_uid_slave);
						toLog(ldata);
						uint time = ct;
						m_weatherDriver->setReqStatus(resp.m_uid_slave, 1, time);
					}
					else
					{
						m_weatherDriver->addResponse(resp, filename);
						uint time = ct;
						m_weatherDriver->setReqStatus(resp.m_uid_slave, 1, time);
						LogData ldata;
						time = ct + ( (GribUtility::getCurrentTimeZone() - 3) *3600);
						ldata = LogData(1,tr("Получен устаревший GRIB файл: ") + filename + tr(" для id:") 
							+ resp.m_uid_slave 
							+ tr(" от (как есть): ") + QDateTime::fromTime_t(ct).toString("HH:mm ddMMyyyy")
							+ tr(" от (поправки): ") + QDateTime::fromTime_t(time).toString("HH:mm ddMMyyyy")
							) ;
						toLog(ldata);
						
					}
					//removeFileLoader(resp.m_uid_slave);
				}
				removeFileLoader(resp.m_uid_slave);
				LogData ldata;
				ldata = LogData(1,tr("Удаляю загрузчик ") + tr(" для id:") + resp.m_uid_slave); 
				toLog(ldata);
				delete data;
				data = NULL;
			}
		
		}
		catch(...)
		{
			LogData data;
			data = LogData(1,tr("Исключение при получении файла: ") + resp.m_uid_slave);
			toLog(data);
		}
		removeFileLoader(resp.m_uid_slave);
		return;
	}

    if(requestRegQt)
    {
        GribUIDResp reqReg = requestRegQt->getUidResp();

        try
        {
            if(reqReg.m_error == SET_DEL_OK)
            {
                m_weatherDriver->deleteReq(reqReg.m_uid_slave);
                LogData data;
                data = LogData(1,tr("Запрос удален: ") + reqReg.m_uid_slave);
                toLog(data);
                removeDirAndFilesReq(m_connectionInfo,reqReg.m_uid_slave);
            }
            else
            {
                m_weatherDriver->setReqMasterUid(reqReg.m_uid_master, reqReg.m_uid_slave);
                LogData data;
                data = LogData(1,tr("Запрос зарегестрирован на сервере локальный id: ") + reqReg.m_uid_master  + tr(", id на сервере: ") + reqReg.m_uid_slave);
                toLog(data);
            }

            removeRequest(reqReg.m_uid_slave);
        }
        catch(...)
        {
            LogData data;
            data = LogData(1,tr("Исключение при обработке запроса: ") + reqReg.m_uid_slave);
            toLog(data);
        }

        return;
    }

    if(gl)
    {
        GribRequestData resp;
        resp.fromJson(gl->getReqData().toJson());
        try
        {
			QString filename = gl->getFileFullName();
            GribData* data = new GribData();
            juce::String error;
            bool bRes = false;
            if(gl->m_error == "OK")
            {
                bRes = Util::openGribFile(qt_juce(filename),data,error);
                if(bRes)
                {
                    time_t time = data->getFirstDate();				
                    m_weatherDriver->setReqStatus(resp.m_uid_slave,1, time);
                    resp.m_date_time_t = time;
                    m_weatherDriver->addResponse(resp, filename);
					LogData data;
                    data = LogData(1,tr("Файл загружен и корректен: ") + filename + tr(" для id:") + resp.m_uid_slave);
                    toLog(data);
                }
                else
                {
                    LogData data;
                    data = LogData(1,tr("Файл загружен,но не корректен: ") + filename + tr(" для id:") + resp.m_uid_slave);
                    toLog(data);
					QFile(filename).remove();
                }
            }            
            delete data;
            data = NULL;
        }
        catch(...)
        {
            LogData data;
            data = LogData(1,tr("Исключение при получении файла: ") + resp.m_uid_slave);
            toLog(data);
        }
		removeGribFileLoader(resp.m_uid_slave);
        return;
    }
}
void ReqManager::slotCheckDb()
{
    QTimer::singleShot(1000, this, SLOT(slotCheckForReg()));
    QTimer::singleShot(2000, this, SLOT(slotCheckForUpdate()));

	if( (QDateTime::currentDateTime().toTime_t() - m_autosaveLogs) > 3600*24 && m_weatherDriver)
	{
		QString pathToLogs = m_connectionInfo.m_pathToGribData + QDir::separator() + "LOGS";
		QDir dir;
		dir.mkpath(pathToLogs);
		bool bRes = m_weatherDriver->saveLogsToFile(pathToLogs + QDir::separator() + QDateTime::currentDateTime().toString("HHmm_ddMMyyyy.log") );
		if(bRes)
			m_autosaveLogs = QDateTime::currentDateTime().toTime_t();
	}
    //следим за временем создания лоадеров и удаляем каждый час
    for (int i = 0; i < m_list_gribLoad.size(); i++)
    {
        GribLoaderQt* glq = m_list_gribLoad.at(i);
        if (glq && (QDateTime::currentDateTime().toTime_t() - glq->getCreate_dt()) > 3600)
            removeGribFileLoader(glq->getReqData().m_uid_slave);
    }

	if(!m_statusLoader)
	{
		m_statusLoader = new StatusLoaderQt(this);
		m_statusLoader->setUrl(m_connectionInfo.m_url);
		m_statusLoader->setRoot(m_i_is_root);
		connect(m_statusLoader, SIGNAL(signFinished()), SLOT(slotWorkCompleate()));
		m_statusLoader->start();
	}
	deleteOldData();
}

QMap<QString, SeaData> ReqManager::createMapSea(bool bIsRoot, const QString &ip, const QString &port)
{
	if (m_weatherDriver)
		m_weatherDriver->clearSeas();
	foreach(IceFileLoaderQt* iceLoader, m_map_iceLoad)
	{
		if (!iceLoader)
			continue;
		iceLoader->deleteLater();
		iceLoader = NULL;
	}
	m_map_iceLoad.clear();
	QMap<QString, SeaData> map;
	if (!m_weatherDriver)
		return map;
	QMap<QString, QString> res = GribUtility::getMapSeaNames();
	m_weatherDriver->clearSeas();
	foreach(QString val, res)
	{
		SeaData seaData;
		seaData.check_data = 1; 
		if (bIsRoot)
			seaData.data_addr = "";
		else
		{
			QString addr = "http://" + ip + ":" + port + "/webgio.ashx/grib/data";
			seaData.data_addr = addr;
		}
		seaData.date = 0;
		seaData.sea_name = res.key(val);
		seaData.sea_rus_name = val;
		m_weatherDriver->insertSeaData(seaData.sea_name, seaData.data_addr, seaData.sea_rus_name, seaData.check_data);
		map[seaData.sea_name] = seaData; 
	}
	return map;
}

bool ReqManager::eraseRequests()
{
        LogData data;
        data = LogData(1, juce_qt(tr_juce("Обновление загрузчиков..\n")));
        toLog(data);

        for (int i = 0; i < m_list_req.size(); i++)
        {
            RequestRegQt* req = m_list_req.at(i);
            if (req)
                removeRequest(req->getUidResp().m_uid_slave);
        }

        for (int i = 0; i < m_list_fl.size(); i++)
        {
            FileLoaderQt* flq = m_list_fl.at(i);
            if (flq)
                removeFileLoader(flq->getData().m_uid_slave);
        }

        for (int i = 0; i < m_list_gribLoad.size(); i++)
        {
            GribLoaderQt* glq = m_list_gribLoad.at(i);
            if (glq)
                removeGribFileLoader(glq->getReqData().m_uid_slave);
        }
        return true;
}

void ReqManager::slotCheckForReg()
{
    if(!m_weatherDriver)
        return;

    QMap<QString,SeaData> map;
    map = m_weatherDriver->getMapSea();
	if (map.isEmpty())
	{
		map = createMapSea(m_i_is_root, m_connectionInfo.m_serverAddr, QString::number(m_connectionInfo.m_serverPort));
		
	}
    QStringList keys = map.keys();
    for(int i = 0; i < keys.size(); i++)
    {
        IceFileLoaderQt* m_iceFileLoaderQt = m_map_iceLoad.value(keys.at(i), NULL);
        if(!m_iceFileLoaderQt)
        {
            SeaData data =	map[keys.at(i)];
            if(data.check_data == 1)
            {
				m_iceFileLoaderQt = new IceFileLoaderQt(m_wdc, this);
                m_iceFileLoaderQt->setPathToIceData(m_connectionInfo.m_pathToIceData);
                m_iceFileLoaderQt->setRoot(m_i_is_root);
                m_iceFileLoaderQt->setSeaData(data);
                m_iceFileLoaderQt->start();
                connect(m_iceFileLoaderQt, SIGNAL(signFinished()), SLOT(slotWorkCompleate()));
                m_map_iceLoad[keys.at(i)] = m_iceFileLoaderQt;
                LogData ldata;

                QString str;
                str += juce_qt(tr_juce("Добавлен запрос на обновление данных ледовой обстановки: "));
                str += data.sea_rus_name;
                str += juce_qt(tr_juce((" море")));

                ldata = LogData(1,str);
                toLog(ldata);
            }
        }
    }

    if(m_i_is_root)
    {
        QList<GribRequestData> list = m_weatherDriver->getListRequests(3);

        //обновляем список заявок
        for (int i = 0; i < list.size(); i++)
        {			
            if(list.at(i).m_deleted == 0)
			{				
                addGribLoad( list.at(i) , RawHeaderData(), m_connectionInfo);
			}
		}

        list = m_weatherDriver->getListRequests(4);

        //удаляем лишние
        for (int i = 0; i < list.size(); i++)
        {
            m_weatherDriver->deleteReq(list.at(i).m_uid_slave);
            removeDirAndFilesReq(m_connectionInfo,list.at(i).m_uid_slave);
        }
    }
    else
    {
        QList<GribRequestData> list = m_weatherDriver->getListRequests(1);
        //обвновляем список заявок
        for (int i = 0; i < list.size(); i++)	
            addRequest( list.at(i) );
    }
}

bool ReqManager::removeRequest(QString uid)
{
    bool result = false;

    int iRes = -1;

    for(int i = 0; i < m_list_req.size(); i++)
    {
        if( m_list_req.at(i)->getUidResp().m_uid_slave == uid)
            iRes = i;
    }

    if(iRes > -1)
    {
        RequestRegQt* req = m_list_req.at(iRes);
        m_list_req.removeOne(req);
        delete req;
        req = NULL;
        result = true;
    }

    return result;
}


bool ReqManager::removeFileLoader(QString uid)
{
    bool result = false;
    int iRes = -1;

    for(int i = 0; i < m_list_fl.size(); i++)
    {
        if( m_list_fl.at(i)->getData().m_uid_slave == uid)
        {
            iRes = i;
            break;
        }
    }

    if(iRes > -1)
    {
        FileLoaderQt* fl = m_list_fl.at(iRes);
        m_list_fl.removeOne(fl);
        delete fl;
        fl = NULL;
        result = true;
    }

    return result;
}

bool ReqManager::removeGribFileLoader(QString uid)
{
    bool result = false;
    int iRes = -1;

    for(int i = 0; i < m_list_gribLoad.size(); i++)
    {
        if( m_list_gribLoad.at(i)->getReqData().m_uid_slave == uid)
        {
            iRes = i;
            break;
        }
    }

    if(iRes > -1)
    {
        GribLoaderQt* gl = m_list_gribLoad.at(iRes);
        m_list_gribLoad.removeOne(gl);
        gl->deleteLater();
        result = true;
    }

    return result;
}

bool ReqManager::addGribLoad(GribRequestData data, RawHeaderData rawHeader, ConnectionInfo conInfo)
{
    bool result = false;

    int threadCount = -1;

    if(QThread::idealThreadCount() > 0)
        threadCount = QThread::idealThreadCount();
    else
        threadCount = 2;

    if(m_list_fl.size() + m_list_gribLoad.size() > threadCount - 1)
        return result;

    bool bRes = true;

    for(int i = 0; i < m_list_gribLoad.size(); i++)
    {
        if( m_list_gribLoad.at(i)->getReqData().m_uid_slave == data.m_uid_slave )
            bRes = false;
    }

    if(bRes)
    {
		bool updateOk = checkGribFileUpdateTime(data.m_uid_slave ,MINIMUM_GRIB_UPDATE_TIME);
		if(!updateOk)
			return false;

        GribLoaderQt* gl = new GribLoaderQt(this);

        connect(gl, SIGNAL(signStatus(const QString &,const QString &)), SLOT(slotStatusGL(const QString &,const QString &)));
        connect(gl, SIGNAL(signDebug(const QString &)), SLOT(slotDebugRequest(const QString &)));
        connect(gl, SIGNAL(signFinished()), SLOT(slotWorkCompleate()));

        gl->setFileDirectory(m_connectionInfo.m_pathToGribData);
		gl->setModificators(m_modMin,m_modMax);
        gl->execute(m_connectionInfo ,data);

        LogData ldata;
        ldata = LogData(1, juce_qt(tr_juce("Добавлен запрос на загрузку (proxy): ")) + data.m_uid_slave);
        toLog(ldata);

        m_list_gribLoad.append(gl);
    }

    return result;
}

bool ReqManager::addRequest(GribRequestData data)
{
    bool result = false;

    int threadCount = -1;
    if(QThread::idealThreadCount() > 0)
        threadCount = QThread::idealThreadCount();
    else
        threadCount = 2;

    if(m_list_fl.size() + m_list_req.size() > threadCount - 1)
        return result;

    bool bRes = true;

    for(int i = 0; i < m_list_req.size(); i++)
    {
        if( m_list_req.at(i)->getUidResp().m_uid_slave == data.m_uid_slave)
            bRes = false;
    } 

    if(bRes)
    {	 
		bool updateOk = checkGribFileUpdateTime(data.m_uid_slave,MINIMUM_GRIB_UPDATE_TIME);
		if(!updateOk)
			return result;

		LogData ldata;
        ldata = LogData(1, juce_qt(tr_juce("Добавлен запрос на загрузку (webgio): "))  + data.m_uid_slave);
        toLog(ldata);

        RequestRegQt* req = new RequestRegQt(this);
        connect(req, SIGNAL(signDebug(const QString &)), SLOT(slotDebugRequest(const QString &)));
        req->setData(data);
        req->setRawHeaders(m_rawHeaders);
        req->setConnectionInfo(m_connectionInfo);
        m_list_req.append(req);
        connect(req, SIGNAL(signFinished()), SLOT(slotWorkCompleate()));
        req->start();
        result = true;
    }
    return result;
}

void ReqManager::slotCheckForUpdate()
{
    QList<GribRequestData> list = m_weatherDriver->getListRequests(2);
    for (int i = 0; i < list.size(); i++)
    {
        if(list.at(i).m_deleted == 0)
            addForUpdate( list.at(i) );
    }
}

bool ReqManager::addForUpdate(GribRequestData data)
{
    bool result = false;

    int threadCount = -1;
    if(QThread::idealThreadCount() > 0)
        threadCount = QThread::idealThreadCount();
    else
        threadCount = 2;

    if(m_list_fl.size() + m_list_req.size() > threadCount - 1)
        return result;

    bool bRes = true;

    for(int i = 0; i < m_list_fl.size(); i++)
    {

        if(m_list_fl.at(i)->getData().m_uid_slave == data.m_uid_slave)
            bRes = false;
    }

    if(bRes)
    {
		//bool updateOk = checkGribFileUpdateTime(data.m_uid_slave,MINIMUM_GRIB_UPDATE_TIME) || ;
		if(data.m_status == 1)
			return result;

        LogData ldata;
        ldata = LogData(1, juce_qt(tr_juce("Добавлен запрос на обновление данных по запросу: "))  + data.m_uid_slave);
        toLog(ldata);

		FileLoaderQt* fl = new FileLoaderQt(this);
        connect(fl, SIGNAL(signDebug(const QString &)), SLOT(slotDebugRequest(const QString &)));
        connect(fl,SIGNAL(signFinished()),SLOT(slotWorkCompleate()));
        fl->setData(data);
        m_list_fl.append(fl);
        fl->setConnectionInfo(m_connectionInfo); 
        fl->start();
        result = true;
    }

    return result;
}

void ReqManager::checkSettings()
{
    if(m_weatherDriver)
    {
        QString	 serverAddr, serverPort, userLogin, userPassword, pathToGribData, pathToIceData;
        ZygribConsts zygrib;
        m_weatherDriver->getSettings(serverAddr, serverPort, userLogin, userPassword, pathToGribData, zygrib, pathToIceData);
        bool bRes = false;
        QString setUpdated;
        if(m_connectionInfo.m_serverAddr != serverAddr)
        {
            bRes = true;
            m_connectionInfo.m_serverAddr = serverAddr;
            setUpdated += juce_qt(tr_juce("Адрес: ")) + serverAddr + "\n";
        }

        if(m_connectionInfo.m_serverPort !=  serverPort.toInt())
        {
            bRes = true;
            m_connectionInfo.m_serverPort =  serverPort.toInt();
            setUpdated += juce_qt(tr_juce("Порт: ")) + serverPort+ "\n";
        }

        if(m_connectionInfo.m_pathToGribData != pathToGribData)
        {
            bRes = true;
            m_connectionInfo.m_pathToGribData = pathToGribData;
            setUpdated += juce_qt(tr_juce("Путь к данным: ")) + pathToGribData + "\n";
        }

        if (m_connectionInfo.m_pathToIceData != pathToIceData)
        {
            bRes = true;
            m_connectionInfo.m_pathToIceData = pathToIceData;
            setUpdated += juce_qt(tr_juce("Путь к ледовым данным: ")) + pathToIceData + "\n";
        }

        if(m_connectionInfo.m_login != userLogin)
        {
            bRes = true;
            m_connectionInfo.m_login = userLogin;
            setUpdated += juce_qt(tr_juce("Логин: ")) + userLogin + "\n";
        }

        if(m_connectionInfo.m_password != userPassword)
        {
            bRes = true;
            m_connectionInfo.m_password = userPassword;
            setUpdated += juce_qt(tr_juce("Пароль обновлен")) + "\n";
        }
        if(bRes)
        {
            m_connectionInfo.m_url = "http://" + m_connectionInfo.m_serverAddr +  ":" + QString::number(m_connectionInfo.m_serverPort) + "/webgio.ashx/grib/data";
            LogData data;
            data = LogData(1, juce_qt(tr_juce("Настройки подключения обновлены\n")) + setUpdated);
            toLog(data);
            for(int i = 0; i < m_list_req.size(); i++)
            {
               RequestRegQt* req = m_list_req.at(i);
               if(req)
                    removeRequest(req->getUidResp().m_uid_slave);
            }

            for(int i = 0; i < m_list_fl.size(); i++)
            {
                FileLoaderQt* flq = m_list_fl.at(i);
                if(flq)
                   removeFileLoader(flq->getData().m_uid_slave);
            }

            for(int i = 0; i < m_list_gribLoad.size(); i++)
            {
                GribLoaderQt* glq = m_list_gribLoad.at(i);
                if(glq)
                    removeGribFileLoader(glq->getReqData().m_uid_slave);
            }
        }
    }
}

void ReqManager::toLog(LogData logData)
{
    if(!m_weatherDriver)
        return;
    logData.m_source = this->objectName();
    m_weatherDriver->addLog(logData);
	
	juce::String tmp = qt_juce(QDateTime::fromTime_t(logData.m_time_t).toString()) + " | " + qt_juce(logData.m_source) + " | "+ qt_juce(logData.m_text);
	switch(logData.m_type)
	{
	case 0:
        log_trace_to(log_weather_req_manager, tmp.toSrc8Bit().data());
		break;
	case 1:
        log_info_to(log_weather_req_manager, tmp.toSrc8Bit().data());
		break;
	case 2:
		log_warn_to(log_weather_req_manager, tmp.toSrc8Bit().data());
		break;
	case 3:
		log_error_to(log_weather_req_manager, tmp.toSrc8Bit().data());
		break;
	}
}

void ReqManager::slotStatusGL(const QString &id,const QString &stat)
{
    LogData data;
    data.setCurrentDt();
    data.setType(1);
    QString text = id + " " + stat;
    data.setText(text);
    toLog(data);
}

void ReqManager::slotDebugRequest(const QString& log)
{
    LogData data;
    data.setCurrentDt();
    data.setType(2);
    data.setText(log);
    toLog(data);
}

bool ReqManager::removeDirAndFilesReq(const ConnectionInfo &conInfo, const QString &uid)
{
	QDir dir(conInfo.m_pathToGribData + QDir::separator() + uid);
	LogData data;
	data = LogData(1,tr("Удаляю директорию с файлами: ") + dir.path());
	toLog(data);
	int iRes = GribUtility::removeFolder(dir);    
	return (bool)iRes;
}

void ReqManager::deleteOldData()
{
	QList<GribRequestData> requests = m_weatherDriver->getListRequests(0);
	
	foreach(GribRequestData req,requests)
	{
		QString directory = m_connectionInfo.m_pathToGribData + QDir::separator() + req.m_uid_slave;
		GribUtility::removeOldFilesInDirectory(directory,30);
	}	
}

bool ReqManager::checkGribFileUpdateTime(const QString &uid, int updateTime)
{
	if(!m_weatherDriver)
		return false;
	GribFileResp respData = m_weatherDriver->getLast(uid);
	int curDt = QDateTime::currentDateTime().toTime_t();
	int fileDt = respData.createDateTime_T();
	bool updateOk = (curDt - fileDt) > updateTime; 
	return updateOk;
}

BlockManager::BlockManager(const GribIceFolderDataDriver* driver, int blockSize)
{
	setBlockSize(blockSize * 1024);
	m_weatherDriver = driver;
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), SLOT(m_slotUpdate()));
	m_slotUpdate();
	m_timer->start(120000);
}

BlockManager::~BlockManager()
{
	try
	{
		if(m_timer)
			m_timer->stop();
		
	}
	catch(...)
	{
	}
}

void BlockManager::m_slotUpdate()
{
	if (!m_weatherDriver)
		return;
	try
	{
		//получаем список запросов
		QList<GribRequestData> reqs = m_weatherDriver->getListRequests(0);        

		for (int i = 0; i < reqs.size(); i++)
		{     
            if (reqs.at(i).m_status == 0)
                continue;
            //получаем уид
			QString uid = reqs.at(i).m_uid_slave;
			//по нему запрашиваем самый последний файл
			GribFileResp data = m_weatherDriver->getLast(uid);
			//если он более актуален, чем тот, что уже разобран - разбераем.
			if (m_blocksMap[uid].getDateTime_t() < data.m_date_time)
			{
				Blocks blocks(data.m_filename, m_block_size);
				if (blocks.isCurrect())
				{
					blocks.setDateTime_t(data.m_date_time);
					m_blocksMap[uid] = blocks;
				}
			}
		}
	}
	catch (...)
	{ }
}

Blocks::Block BlockManager::getBlock(const QString &uid, const QString &hash)
{
	Blocks::Block result;
	try
	{		
		result = m_blocksMap[uid].getBlock(hash);		
	}
	catch (...)
	{ }
	return result;
}

QStringList BlockManager::getHashes(const QString &uid)
{
	try
	{
		return m_blocksMap[uid].getHashes();
	}
	catch (...)
	{

	}
	return QStringList();
}

QString BlockManager::getInfo(const QString &uid)
{
	try
	{
		return QString(m_blocksMap[uid].toJson());
	}
	catch (...)
	{

	}
	return QString();
}

bool BlockManager::isCurrect(const QString &uid)
{
	try
	{
		return m_blocksMap[uid].isCurrect();
	}
	catch (...)
	{

	}
	return false;
}

void BlockManager::update()
{
    m_blocksMap.clear();
}



