#include "GribProxyLoaderQt.h"
#include <math.h>
#include "gis/GisGrib/Util.h"
#include "gis/GisGrib/GribData.h"
//#include "gis/GisBase/core/encoding.h"

#include <common/include/tr_macros.h>

#define TIMEOUT_GRIB_LOAD 3600

GribLoaderQt::GribLoaderQt(QObject *parent)
{
    m_timer = NULL;

    m_gribFileInfoRequest       = new HttpClientQt(this);
    m_gribFileStatRequest       = new HttpClientQt(this);
    m_gribFileDownloadRequest   = new HttpClientQt(this);

    connect(m_gribFileInfoRequest,
            SIGNAL(signData(const QByteArray &)),
            SLOT(slotFileInfoResponse(const QByteArray &)));

    connect(m_gribFileStatRequest,
            SIGNAL(signData(const QByteArray &)),
            SLOT(slotFileStatResponse(const QByteArray &)));

    connect(m_gribFileDownloadRequest,
            SIGNAL(signData(const QByteArray &)),
            SLOT(slotFileDownloadResponse(const QByteArray &)));


    connect(m_gribFileInfoRequest,
            SIGNAL(signDebug(QString)),
            SIGNAL(signDebug(QString)));

    connect(m_gribFileStatRequest,
            SIGNAL(signDebug(QString)),
            SIGNAL(signDebug(QString)));

    connect(m_gribFileDownloadRequest,
            SIGNAL(signDebug(QString)),
            SIGNAL(signDebug(QString)));

    m_modMin = 0.000001;
    m_modMax = 0.00001;
    m_create_dt = QDateTime::currentDateTime().toTime_t();
}

GribLoaderQt::~GribLoaderQt()
{
}

void GribLoaderQt::execute(const ConnectionInfo &connectionInfo, const GribRequestData &reqData)
{    m_connectionInfo = connectionInfo;
    m_Data = reqData;

    if (m_gribFileInfoRequest)
        m_gribFileInfoRequest->setData(reqData);
    if (m_gribFileStatRequest)
        m_gribFileStatRequest->setData(reqData);
    if (m_gribFileDownloadRequest)
        m_gribFileDownloadRequest->setData(reqData);
    fileInfoRequest(connectionInfo,reqData);
}

void GribLoaderQt::fileInfoRequest(const ConnectionInfo &connectionInfo, const GribRequestData &reqData)
{
    m_rawData.m_mirrorHttp = GribUtility::getPage(reqData, m_zygribConsts,m_modMin,m_modMax).toUtf8();
    m_rawData.m_sessionGUID = QString(GribUtility::makeAuthString(connectionInfo.m_serverAddr, m_rawData.m_mirrorHttp).toHex()).toUtf8();
    m_rawData.m_codedHeader = m_rawData.m_sessionGUID;

    QString qStrUrl;
    if (connectionInfo.m_url.isEmpty())
        qStrUrl = GribUtility::getProxyUrl(connectionInfo.m_serverAddr, connectionInfo.m_serverPort);
    else
        qStrUrl = connectionInfo.m_url;

    toLog(tr("Отправляю запрос с модификаторами мин/макс :") + QString::number(m_modMin, 'f', 10) + tr(" / ") + QString::number(m_modMax, 'f', 10));
    m_gribFileInfoRequest->sendData(qStrUrl, m_rawData.getMap());
}

void GribLoaderQt::fileStatRequest()
{
    RawHeaderData rawData = m_rawData;
    rawData.m_lastIp = QString("1").toUtf8();
    rawData.m_userAgent = QString("Mozilla/5.0").toUtf8();
    rawData.m_mirrorHttp = GribUtility::getFileUrl(m_zygribConsts, m_gribFileInfo.m_file).toUtf8();
    rawData.m_saveData = QString("true").toUtf8();
    rawData.m_codedHeader = QString(GribUtility::makeAuthString(m_connectionInfo.m_serverAddr, rawData.m_mirrorHttp).toHex()).toUtf8();

    QString qStrUrl = GribUtility::getProxyUrl(m_connectionInfo.m_serverAddr, m_connectionInfo.m_serverPort);
    toLog(tr("Запрашиваю статус файла"));
    m_gribFileStatRequest->sendData(qStrUrl,rawData.getMap());
}


void GribLoaderQt::slotFileInfoResponse(const QByteArray &ba)
{
    QString string = QString(ba);
    QStringList lsbuf = string.split("\n");

    QString message;
    for (int i=0; i < lsbuf.size(); i++)
    {
        QStringList lsval = lsbuf.at(i).split(":");
        if (lsval.size() >= 2)
        {
            if (lsval.at(0) == "status")
                m_gribFileInfo.m_status = lsval.at(1);
            else if (lsval.at(0) == "file")
                m_gribFileInfo.m_file  = QString(lsval.at(1));
            else if (lsval.at(0) == "size")
                m_gribFileInfo.m_size = lsval.at(1);
            else if (lsval.at(0) == "checksum")
                m_gribFileInfo.m_checksum = lsval.at(1);
            else if (lsval.at(0) == "message")
                message = QUrl::fromPercentEncoding (lsval.at(1).toUtf8());
        }
    }

    if (m_gribFileInfo.m_status == "ok")
    {
        toLog(tr("Файл загружен на прокси-сервер, подготавливаюсь к получению ") + m_gribFileInfo.m_status );
        fileStatRequest();
    }
    else
    {	
		if(string.isEmpty())
			toLog(tr("Ошибка получения информации о файле (Проверте сервер - источник данных) "));
		else
			toLog(tr("Ошибка получения информации о файле: ") + string);
        emit signFinished();
    }
}


void GribLoaderQt::slotFileStatResponse(const QByteArray &ba)
{
    QString string = QString(ba);

    if (string == "start downloading")
        fileDownloadRequest();
    else
    {
        toLog(tr("Ошибка ответа.. (slotFileStatResponse): "));

        if (string == "QT_NETWORK_ERROR")
        {
            m_error = "ERROR500";
            emit signFinished();
            return;
        }
    }
}

void GribLoaderQt::slotFileDownloadResponse(const QByteArray &ba)
{
    QString string = QString(ba);
    if (string == "ERROR500")
    {
        m_error = "ERROR500";
        emit signFinished();
        return;
    }

    if (string == "ERROR404")
    {
        m_error = "ERROR404";
        return;
    }

    if (string == "QT_NETWORK_ERROR")
    {
        m_error = "QT_NETWORK_ERROR";
        emit signFinished();
        return;
    }

    if (string.isEmpty())
    {
        m_error = "QT_NETWORK_ERROR";
        toLog(tr("Пустая строка ответа по запросу:"));
        emit signFinished();
        return;
    }

    QDir dir;

    if (!dir.exists(m_fileDirectory))
        dir.mkdir(m_fileDirectory);

    if (!dir.exists(m_fileDirectory + "/" + m_Data.m_uid_slave))
        dir.mkdir(m_fileDirectory + "/" + m_Data.m_uid_slave);

    QString str = m_fileDirectory + "/" + m_Data.m_uid_slave + "/" + QString::number(QDateTime::currentDateTime().toTime_t()) + ".grb";
    m_fileFullName = QDir::cleanPath(str);

    QFile file(m_fileFullName);
    file.open(QIODevice::WriteOnly);
    file.write(ba);
    file.close();

    if (file.exists() && ba.size() > 0)
    {
        m_error = "OK";
        emit signFinished();
    }
    else
    {
        m_error = "FAIL";
        toLog(tr("Ошибка проверки файла: ") + m_fileFullName);
        QFile file(m_fileFullName);
        if (file.exists())
            file.remove();
        emit signFinished();
    }
}

void GribLoaderQt::fileDownloadRequest()
{
    RawHeaderData rawData = m_rawData;
    rawData.m_mirrorHttp = QString("get_answer").toUtf8();
    rawData.m_lastIp = QString("1").toUtf8();
    QString qStrUrl = GribUtility::getProxyUrl(m_connectionInfo.m_serverAddr, m_connectionInfo.m_serverPort);

    rawData.m_codedHeader = QString(GribUtility::makeAuthString(m_connectionInfo.m_serverAddr,rawData.m_mirrorHttp).toHex()).toUtf8();
    toLog(tr("Начинаю загрузку данных"));
    m_gribFileDownloadRequest->sendData(qStrUrl,rawData.getMap());

    if (!m_timer)
    {
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(slotLoadFile()));
        m_timer->start(30000);
    }
}

void GribLoaderQt::slotLoadFile()
{

	if ((QDateTime::currentDateTime().toTime_t() - m_create_dt) > TIMEOUT_GRIB_LOAD)
	{
		toLog(tr("Запрос создан более ")+ QString::number( TIMEOUT_GRIB_LOAD / 60 ) + (" мин. назад. Пересоздать."));
		signFinished();
		return;
	}

    fileDownloadRequest();
}

void GribLoaderQt::setModificators(float modMin, float modMax)
{
    m_modMin = modMin;
    m_modMax = modMax;
}

void GribLoaderQt::toLog(const QByteArray &log)
{
    signDebug(log + " ID: " + m_Data.m_uid_slave);
}

void GribLoaderQt::toLog(const QString &log)
{
#ifdef _DEBUG
    qDebug() << log << " ID: " << m_Data.m_uid_slave;
#endif /* _DEBUG */

    signDebug(log + " ID: " + m_Data.m_uid_slave);
}
