#ifndef FILELOADERQT_H
#define FILELOADERQT_H

#include <QObject>
#include <QCryptographicHash>
#include "HttpClientQt.h"
#include "grib_lib_global.h"
#include "GribExStructures.h"
#include <QTimer>

class GRIB_LIB_EXPORT FileLoaderQt : public HttpClientQt
{
    Q_OBJECT
public:
    FileLoaderQt(QObject *parent);
    FileLoaderQt(const QString &pathToData, const QString &url, QObject *parent);
    ~FileLoaderQt();

    void setConnectionInfo(const ConnectionInfo &data);
    void start(int wait = 40000);

    //запрос
    void setData(const GribRequestData &data);
    QString getFileFullName() const
    {
        return m_fileFullName;
    }
    void setBlockSize();
	qint64 getCreate_dt() const { return m_create_dt; }
    int getTimeout() const { return m_timeout; }
    void setTimeout(int val) 
    {    
        m_timeout = val;        
    }
private:
    void setFileFullName(QString val)
    {
        m_fileFullName = val;
    }
private:
    ConnectionInfo  m_connectionInfo;
    QString         m_fileFullName;
    QTimer*         m_timer;
    int             m_curBlock;
    QStringList     m_checkSums;
    QString         m_curCheckSum;
    int             m_curCheckSumCounter;
    bool            m_startBuldFile;
    Blocks          m_blocks;
    bool            m_bLoaded;
    int             m_error_counter;
	qint64          m_create_dt;
    int             m_timeout;
    QTimer*         m_timerTimeout;
    bool m_single;
private slots:
    void slotResponse(const QByteArray& ba);
    void slotFinished(const int &res);
    void slotStartLoad();
signals:
    void signTimeout();
};

#endif // FILELOADERQT_H
