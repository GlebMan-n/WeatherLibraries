#ifndef GRIBLOADERQT_H
#define GRIBLOADERQT_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QTextStream>
#include <QTimer>
#include <QMap>
#include <QMapIterator>
#include <QDir>
#include <QDate>

#include "HttpClientQt.h"
#include "GribUtility.h"
#include "grib_lib_global.h"

class GRIB_LIB_EXPORT GribLoaderQt : public QObject
{
    Q_OBJECT
public:
    GribLoaderQt(QObject *parent = 0);
    ~GribLoaderQt();
    void execute(const ConnectionInfo &connectionInfo, const GribRequestData &reqData);
    GribRequestData getReqData() const
    {
        return m_Data;
    }
    void setFileDirectory(QString val)
    {
        m_fileDirectory = val;
    }
    QString getFileFullName() const
    {
        return m_fileFullName;
    }
    void setModificators(float modMin, float modMax);
    QString         m_error;
    ConnectionInfo getConnectionInfo() const
    {
        return m_connectionInfo;
    }
    void setConnectionInfo(ConnectionInfo val)
    {
        m_connectionInfo = val;
    }

	qint64 getCreate_dt() const { return m_create_dt; }
private:
    ConnectionInfo  m_connectionInfo;
    GribRequestData m_Data;
    ZygribConsts    m_zygribConsts;
    RawHeaderData   m_rawData;
    GribDataA       m_gribFileInfo;
    HttpClientQt*   m_gribFileInfoRequest;
    HttpClientQt*   m_gribFileStatRequest;
    HttpClientQt*   m_gribFileDownloadRequest;
    QTimer*         m_timer;
    QString         m_fileDirectory;
    QString         m_fileFullName;
    float           m_modMin;
    float           m_modMax;
    qint64          m_create_dt;
private:
    void fileInfoRequest(   const ConnectionInfo &connectionInfo,
                            const GribRequestData &reqData);
    void fileDownloadRequest();
    void fileStatRequest();
    void toLog(const QByteArray &log);
    void toLog(const QString &log);
private slots:
    void slotFileInfoResponse(const QByteArray &ba);
    void slotFileStatResponse(const QByteArray &ba);
    void slotFileDownloadResponse(const QByteArray &ba);

    void slotLoadFile();
signals:
    void signalWorkCompleate(bool result);
    void signStatus(const QString &sessionId, const QString &status);
    void signDebug(const QString &log);
    void signFinished();
};

#endif // GRIBLOADERQT_H
