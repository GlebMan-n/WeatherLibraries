#ifndef REQMANAGER_H
#define REQMANAGER_H

#include <QObject>
#include "grib_lib/GribExStructures.h"
#include "grib_lib_global.h"
#include <QThread>
#include <QList>
#include <QTime>
#include <QTimer>
#include <QCoreApplication>
#include <grib_lib/GribIceFolderDataDriver.h>

class GribRequestData;
class GribLoaderQt;
class HttpClientQt;
class FileLoaderQt;
class RequestRegQt;
class IceFileLoaderQt;
class StatusLoaderQt;

//http://www.zygrib.org/noaa/getGfsRunLog.php - STATUS
class GRIB_LIB_EXPORT ReqManager : public QObject
{
    Q_OBJECT

public:
	ReqManager(const GribIceFolderDataDriver* driver, QObject *parent);
    ~ReqManager();

    bool getI_is_root() const
    {
        return m_i_is_root;
    }
    void setI_is_root(bool val)
    {
        m_i_is_root = val;
    }
    ConnectionInfo getConnectionInfo() const
    {
        return m_connectionInfo;
    }
    void setConnectionInfo(ConnectionInfo val)
    {
        m_connectionInfo = val;
    }
    void toLog(LogData logData);
    bool removeDirAndFilesReq(const ConnectionInfo &conInfo, const QString &uid);
    void setModificators(float modMin, float modMax);
    bool removeRequest(QString uid);
	QString getWorkInfo() const {return QString("!!!");}
	bool getWdc() const { return m_wdc; }
	void setWdc(bool val) { m_wdc = val; }
	QMap<QString, SeaData> createMapSea(bool bIsRoot, const QString &ip, const QString &port);
    bool eraseRequests();
private:
    FileLoaderQt*                   m_fileLoader;
	StatusLoaderQt*					m_statusLoader;
	const GribIceFolderDataDriver*	m_weatherDriver;
    QTimer*                         m_timer;
    QList<RequestRegQt*>            m_list_req;
    /* контейнер дл€ экземпл€ров классов загрузки с веб гио */
    QList<FileLoaderQt*>            m_list_fl;
	
	
	/* контейнер дл€ экземпл€ров классов загрузки из источников данных */
    QList<GribLoaderQt*>            m_list_gribLoad;
	
	
	QMap<QString,IceFileLoaderQt*>  m_map_iceLoad;
    ConnectionInfo                  m_connectionInfo;
    RawHeaderData                   m_rawHeaders;
    GribRequestData                 m_data;
    bool                            m_i_is_root;
	// флаг переключени€ адреса загрузки данных
	// true - http://wdc.aari.ru/datasets/d0004/#SEANAME/sigrid/#YEAR/#FILENAME
	// false - http://www.aari.ru/resources/d0004/index.php?dir=#SEANAME%2Fsigrid%2F#YEAR%2F&download=#FILENAME
	bool							m_wdc;

	IceFileLoaderQt*                m_iceFileLoaderQt;
	float                           m_modMin;
    float                           m_modMax;
    int                             m_autosaveLogs;
private:
    bool addRequest(GribRequestData data);
    bool addGribLoad(GribRequestData data, RawHeaderData rawHeader, ConnectionInfo conInfo);
    bool addForUpdate(GribRequestData data);
    bool addForDelete(GribRequestData data);

    bool removeFileLoader(QString uid);
    bool removeGribFileLoader(QString uid);
    void checkSettings();
	void deleteOldData();
	bool checkGribFileUpdateTime(const QString &uid, int updateTime);
	
private slots:
    void slotWorkCompleate();
    void slotCheckDb();
    void slotCheckForReg();
    void slotCheckForUpdate();
    void slotStatusGL(const QString &id,const QString &stat);
    void slotDebugRequest(const QString& log);

};

class GRIB_LIB_EXPORT BlockManager : public QObject
{
    Q_OBJECT

public:
	BlockManager(const GribIceFolderDataDriver* driver, int blockSize = 51200);
    ~BlockManager();
  
    Blocks  getBlocks(const QString& uid)
    {
        return m_blocksMap[uid];
    }
    Blocks::Block   getBlock(const QString &uid, const QString &hash);
    QStringList     getHashes(const QString &uid);
    void            setBlockSize(int blockSize)
    {
        m_block_size = blockSize;
    }
    QString         getInfo(const QString &uid);
    bool            isCurrect(const QString &uid);
    void            update();
private:
    QMap<QString,Blocks>    m_blocksMap;
    QTimer*                 m_timer;    
    int                     m_block_size;
	const GribIceFolderDataDriver*	m_weatherDriver;
private slots:
    void            m_slotUpdate();
};

#endif // REQMANAGER_H
