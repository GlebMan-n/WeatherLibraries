#ifndef ICEFILELOADERQT_H
#define ICEFILELOADERQT_H

#include <QObject>
#include "HttpClientQt.h"
#include "grib_lib_global.h"
#include "GribExStructures.h"
#include <QTimer>
#include <QString>
#include <QByteArray>

class GRIB_LIB_EXPORT IceFileLoaderQt : public HttpClientQt
{
    Q_OBJECT
public:
    IceFileLoaderQt(bool bWdc, QObject *parent);
    ~IceFileLoaderQt();
    void start();
    QString getPathToIceData() const
    {
        return m_pathToIceData;
    }
    void setPathToIceData(QString val)
    {
        m_pathToIceData = val;
    }
    SeaData getSeaData() const
    {
        return m_seaData;
    }
    void setSeaData(SeaData val)
    {
        m_seaData = val;
    }
    bool isRoot() const
    {
        return m_isRoot;
    }
    void setRoot(bool val)
    {
        m_isRoot = val;
    }
	QString getFileName() const { return m_fileName; }
	void setFileName(QString val) { m_fileName = val; }
private:
    QTimer*         m_timer;
    HttpClientQt*   m_listFilesLoader;
    HttpClientQt*   m_listIceLoader;
    QString         m_pathToIceData;
	QString			m_fileName;

	int             m_fileLoadedDate;
    SeaData         m_seaData;
    int             m_timeUpdate;
    bool            m_isRoot;
	bool			m_wdc;
private:
    void loadFile(const QString &fileName);
    bool checkIceFile(const QString &fileName);
    bool saveFile(const QString &fullFileName, const QByteArray &ba);
private slots:
    void slotResponse(const QByteArray& ba);
    void slotFinished(const int &res);
    void slotStartLoad();
    void slotListFilesResponse(const QByteArray &ba);
    void slotIceFileResponse(const QByteArray &ba);
};

#endif // ICEFILELOADERQT_H
