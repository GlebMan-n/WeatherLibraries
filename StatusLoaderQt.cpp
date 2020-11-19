#include "StatusLoaderQt.h"

#include <grib_lib/GribUtility.h>

#include <common/include/tr_macros.h>
StatusLoaderQt::StatusLoaderQt(QObject *parent)
	: HttpClientQt(parent)
{
	m_isRoot = true;
	connect(this,
		SIGNAL(signData(const QByteArray&)),
		SLOT(slotResponse(const QByteArray&)));
	connect(this,
		SIGNAL(signFinished(const int &)),
		SLOT(slotFinished(const int &)));
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotStartLoad()));	
	m_interval = 60000; 
	m_dt = QDateTime::currentDateTime().toTime_t();
}

StatusLoaderQt::~StatusLoaderQt()
{
}

void StatusLoaderQt::slotFinished(const int &res)
{
}

void StatusLoaderQt::start()
{
	if (!m_timer)
		return;
	m_timer->start(m_interval);
}

void StatusLoaderQt::startOnce()
{
	QTimer::singleShot(0, this, SLOT(slotStartLoad()));
}

void StatusLoaderQt::slotResponse(const QByteArray& ba)
{
	if(ba.isEmpty())
		return;

	if (isRoot())
	{
		parseData(ba);
		signFinished();
		return;
	}

	GribUIDResp resp;
	bool bRes = resp.fromJson(ba);
	if (!bRes)
	{
		signFinished();
		return;
	}

	QString hash = GribUtility::genHash(resp.m_info.toLatin1());
	if (hash != resp.m_key)
	{
		signFinished();
		return;
	}

	StatusData data;
	data.fromJson(resp.m_info.toLatin1());
	m_data = data;
	signFinished();
}

void StatusLoaderQt::slotStartLoad()
{
	if (isRoot())
		sendData(QNetworkRequest(QUrl("http://zygrib.org/noaa/getGfsRunLog.php")));
	else
	{
		GribRequestData data;
		data.m_error = GET_STATUS;
		data.m_senderIp = GribUtility::getIpV4();
		data.genKey();
		sendData(m_url, RawHeaderData().getMap(), data.toJson());
	}
}

void StatusLoaderQt::parseData(const QByteArray &data)
{
	StatusData status_data;
	QString strbuf (data);
	QStringList lsbuf = strbuf.split("\n");
 	for (int i = 0; i < lsbuf.size(); i++)
	{
		QStringList lsval = lsbuf.at(i).split(":");
		if (lsval.size() > 1) 
		{
			QString val;
			for (int i = 1; i<lsval.size(); i++) {
				if (i > 1) val += ":";
				val += lsval.at(i);
			}
			val = val.trimmed ();

			if (lsval.at(0) == "start_load")
			{
				status_data.m_start_load = val;
			}

			if (lsval.at(0) == "server")
			{
				status_data.m_server = val;
			}

			if (lsval.at(0) == "gfs_run_date")
			{
				status_data.m_gfs_run_date = val;
			}

			if (lsval.at(0) == "gfs_run_hour")
			{
				status_data.m_gfs_run_hour = val;
			}			

			if (lsval.at(0) == "gfs_update_time")
			{
				status_data.m_gfs_update_time = val;
			}

			if (lsval.at(0) == "current_job")
			{
				status_data.m_current_job = val;
			}

			if (lsval.at(0) == "FNMOC-WW3_run_date")
			{
				status_data.m_FNMOC_WW3_run_date = val;
			}

			if (lsval.at(0) == "FNMOC-WW3_run_hour")
			{
				status_data.m_FNMOC_WW3_run_hour = val;
			}

			if (lsval.at(0) == "FNMOC-WW3_update_time")
			{
				status_data.m_FNMOC_WW3_update_time = val;
			}

			if (lsval.at(0) == "FNMOC-WW3_current_job")
			{
				status_data.m_FNMOC_WW3_current_job = val;
			}

			if (lsval.at(0) == "FNMOC-WW3-MED_run_date")
			{
				status_data.m_FNMOC_WW3_MED_run_date = val;
			}

			if (lsval.at(0) == "FNMOC-WW3-MED_run_hour")
			{
				status_data.m_FNMOC_WW3_MED_run_hour = val;
			}

			if (lsval.at(0) == "FNMOC-WW3-MED_update_time")
			{
				status_data.m_FNMOC_WW3_MED_update_time = val;
			}

			if (lsval.at(0) == "FNMOC-WW3-MED_current_job")
			{
				status_data.m_FNMOC_WW3_MED_current_job = val;
			}
		}
	}
	status_data.m_dateTime = QDateTime::currentDateTime().toTime_t();
	status_data.m_responseTime = getLastOperationTime();
	m_data = status_data;
}
