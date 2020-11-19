#include "IceFileLoaderQt.h"
#include <grib_lib/GribUtility.h>

#include <common/include/tr_macros.h>

IceFileLoaderQt::IceFileLoaderQt(bool bWdc, QObject *parent)
    : HttpClientQt(parent)
{
    m_listFilesLoader = NULL;
    m_listFilesLoader = new HttpClientQt(this);
    m_listIceLoader =  new HttpClientQt(this);
    connect(m_listFilesLoader,
            SIGNAL(signData(const QByteArray &)),
            SLOT(slotListFilesResponse(const QByteArray &)));

    connect(m_listFilesLoader,
            SIGNAL(signDebug(QString)),
            SIGNAL(signDebug(QString)));

    connect(m_listIceLoader,
            SIGNAL(signData(const QByteArray &)),
            SLOT(slotIceFileResponse(const QByteArray &)));

    connect(m_listIceLoader,
            SIGNAL(signDebug(QString)),
            SIGNAL(signDebug(QString)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotStartLoad()));
    m_isRoot = true;
    m_fileLoadedDate = 0;
    m_timeUpdate = 86400000;
	m_wdc = bWdc;
}

void IceFileLoaderQt::slotFinished(const int &res)
{
	HttpClientQt* client = qobject_cast<HttpClientQt*>(sender());
	if (client)
	{
		m_lastError = client->getLastError();
		m_lastOperationTime = client->getLastOperationTime();
	}
	signFinished();
}

IceFileLoaderQt::~IceFileLoaderQt()
{
}

void IceFileLoaderQt::start()
{
    slotStartLoad();
    m_timer->start(m_timeUpdate);
}

void IceFileLoaderQt::slotResponse(const QByteArray& ba)
{

    if (ba == QString("QT_NETWORK_ERROR"))
    {
		HttpClientQt* client = qobject_cast<HttpClientQt*>(sender());
		if (client)
		{
			 m_lastError = client->getLastError();
			 m_lastOperationTime = client->getLastOperationTime();
		}
		signFinished();
        return;
    }
}

void IceFileLoaderQt::slotStartLoad()
{
	if (m_isRoot)
	{
		QString year = QString::number(QDate::currentDate().year());
		QString seaDataSetAddr = GribUtility::genDataSetPath(m_seaData.sea_name, year,m_wdc);
		m_listFilesLoader->sendData(QNetworkRequest(QUrl(seaDataSetAddr)));
	}
    else
    {
        QString str;
        str += m_seaData.sea_name;
        str += ";";
        str += "get_date";
        m_listFilesLoader->sendData(QNetworkRequest(QUrl(m_seaData.data_addr)),str.toUtf8());
    }
}

void IceFileLoaderQt::slotListFilesResponse(const QByteArray &ba)
{
	HttpClientQt* client = qobject_cast<HttpClientQt*>(sender());
	if (client)
	{
		m_lastError = client->getLastError();
		m_lastOperationTime = client->getLastOperationTime();
	}
	if (ba == QString("QT_NETWORK_ERROR"))
	{
		signFinished();
		return;
	}
    QString resp;
    QDate date;
    resp = ba;
    QString filename;
    if (m_isRoot)
		filename = GribUtility::parseSeaAnswer(resp, m_fileLoadedDate, m_isRoot, m_wdc);
    else
    {
        filename = resp;
        m_fileLoadedDate = filename.toInt();
    }

    if (!filename.isEmpty())
    {
        bool bRes = GribUtility::checkData(filename,m_isRoot,m_seaData.date);
        if (bRes)
        {
            loadFile(filename);
        }
    }
}

void IceFileLoaderQt::slotIceFileResponse(const QByteArray &ba)
{
	HttpClientQt* client = qobject_cast<HttpClientQt*>(sender());
	if (client)
	{
		m_lastError = client->getLastError();
		m_lastOperationTime = client->getLastOperationTime();
	}
    //сохраняем файл
	if (ba == "QT_NETWORK_ERROR")
	{
		signFinished();
		return;
	}
    QDir dir;
    if (m_fileLoadedDate == 0)
        m_fileLoadedDate = QDate::currentDate().toJulianDay();

	m_fileName = m_pathToIceData + "/" + m_seaData.sea_name + "/" + GribUtility::genIceFileName(m_seaData.sea_name, m_fileLoadedDate);

    dir.mkpath(m_pathToIceData + "/" + m_seaData.sea_name + "/");
	bool bRes = saveFile(m_fileName, ba);
    if (bRes)
    {
        m_seaData.date = m_fileLoadedDate;       
    }
	signFinished();
}

bool IceFileLoaderQt::saveFile(const QString &fullFileName, const QByteArray &ba)
{
    bool result = false;

    QFile file(fullFileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    result = file.write(ba) > 0;
    file.close();
    if (result)
        result = checkIceFile(fullFileName);

    return result;
}

bool IceFileLoaderQt::checkIceFile(const QString &fileName)
{
    bool res = true;
    //TODO
    //добавить проверку ледового файла
    return res;
}

void IceFileLoaderQt::loadFile(const QString &fileName)
{
	if (m_isRoot)
	{
		QString year = QString::number(QDate::currentDate().year());
		QString pathFileDataset = GribUtility::genSeaFileName(fileName, m_seaData.sea_name, year,m_wdc);
		m_listIceLoader->sendData(QNetworkRequest(QUrl(pathFileDataset)));
	}
    else
    {
        QString str;
        str += m_seaData.sea_name;
        str += ";";
        str += "get_last_file";
        m_listIceLoader->sendData(m_seaData.data_addr,RawHeaderData().getMap(),str.toUtf8());
    }
}
