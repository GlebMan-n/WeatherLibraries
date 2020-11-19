#ifndef STATUSLOADERQT_H
#define STATUSLOADERQT_H

#include "GribExStructures.h"
#include <QObject>
#include <QTimer>
#include "HttpClientQt.h"
#include "grib_lib_global.h"

class GRIB_LIB_EXPORT StatusLoaderQt : public HttpClientQt
{
	Q_OBJECT

public:
	StatusLoaderQt(QObject *parent);
	~StatusLoaderQt();
	void start();
	void startOnce();
	StatusData getData() {return m_data;}
	void setUrl(QString url) { m_url = url; }
	QString getUrl() { return m_url; }
	void setInterval(qint64 interval);
	bool isRoot() const { return m_isRoot; }
	void setRoot(bool val) { m_isRoot = val; }
	int getInterval() const { return m_interval; }
	void setInterval(int val) { m_interval = val; }
private:
	StatusData	m_data;
	QTimer*		m_timer;
	qint64      m_dt;
	QString		m_url;
	bool		m_isRoot;
	int			m_interval;
private:
	void parseData(const QByteArray &data);
	private slots:
		void slotResponse(const QByteArray& ba);
		void slotFinished(const int &res);
		void slotStartLoad();
};

#endif // STATUSLOADERQT_H
