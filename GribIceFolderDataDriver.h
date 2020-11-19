#ifndef GRIBICEFOLDERDATADRIVER_H
#define GRIBICEFOLDERDATADRIVER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QTimer>
#include "grib_lib_global.h"
#include <gis/GisBase/data/DataAccess.h>
#include <gis/GisBase/core/encoding.h>
#include <gis_tool_qt/grib_lib/GribExStructures.h>

/** Драйвер для Каталога погодных файлов приложения

Использование:
#include <gis/DataInterface/GribIceFolderDataDriver.h>

const GribIceFolderDataDriver* weatherDriver = m_pDataCollector->getDriver<GribIceFolderDataDriver>();
QString gribPath = juce_qt(weatherDriver->createFolder("grib"));
weatherDriver->deleteFolder("grib");
QString icePath = juce_qt(weatherDriver->createFolder("ice"));
weatherDriver->deleteFolder("ice");
*/

class DataBaseQt;

class GRIB_LIB_EXPORT GribIceFolderDataDriver :	public DataAccess_DataDriver
{
public:
	DEF_CLASS_NAME_STR(GribIceFolderDataDriver)
	/** конструктор */
	GribIceFolderDataDriver();
	/** деструктор */
	virtual ~GribIceFolderDataDriver();
	virtual const char * dataName() const
	{
		return "_weather";
	}

	const char * dataBaseFolderName() const
	{
		return "_data";
	}
	
	const char * gribFolderName() const
	{
		return "_gribData";
	}

    const char * gribFreeFolderName() const
    {
        return "_gribFreeFiles";
    }

	const char * iceFolderName() const
	{
		return "_iceData";
	}

	const char * iniFolderName() const
	{
		return "_iniData";
	}

	const char * dataBaseName() const
	{
		return "gribIceDb.db";
	}

	/** Создает папку folderName в каталоге _weather */
	virtual juce::String createFolder(const juce::String &folderName) const;
	/** Удаляет папку folderName в каталоге _weather */
	virtual void deleteFolder(const juce::String &folderName) const;
	/** инициализация */
	virtual bool init(const juce::String &path);

	QString getGribDataPath() const { return crFolder(gribFolderName()); }
	QString getIceDataPath() const { return crFolder(iceFolderName()); }
	QString getDataBasePath() const { return crFolder(dataBaseFolderName()); }
	QString getDataBaseFullName() const { return getDataBasePath() + dataBaseName(); }
	QString getIniPath() const { return crFolder(iniFolderName()); }
	QString crFolder(const QString &folderName) const;
	
	bool addReq(const GribRequestData &data) const;
	bool setDeleted(int iDeleted, const QString &uid_slave) const;
	void setSettings(const QString serverAddr,
		const QString &serverPort,
		const QString &userLogin,
		const QString &userPassword,
		const QString &pathToGribData,
		const ZygribConsts &zygrib,
		const QString &pathToIceData) const;

	bool getSettings(	QString &serverAddr, 
						QString &serverPort, 
						QString &userLogin, 
						QString &userPassword, 
						QString &pathToGribData, 
						ZygribConsts &zygrib,
						QString &pathToIceData) const;
	QMap<QString, QString> getDrawParams() const;
	bool setDrawParams(const QMap<QString, QString> &drawParams) const;
	GribFileResp getLast(const QString &uid) const;
	std::vector<LogData> getLogs(const QDateTime &begin, const QDateTime &end) const;
	QList<GribRequestData> getListRequests(const QString &qStrWhere = "") const;
	QList<GribRequestData> getListRequests(int i_type) const;
	QMap<QString, SeaData> getMapSea() const;
	QMap<QString,QString> getSeaMapLastFiles() const;
    QMap<QString, QStringList> getSeaMapAllFiles() const;
	QStringList getSeaLastFiles(const QStringList &seaNames) const;
	bool importSeaTextFile(const QString &fileName) const;
	bool updateSeaData(SeaData seaData) const;
	DataBaseQt* getDataBase() const;
	bool addLog(LogData data) const;
	QString getLastLogs(int count = 100) const;
	bool getStatus(StatusData &data) const;
	bool addResponse(const GribRequestData &resp, const QString &filename) const;
	bool deleteReq(QString uid) const;
	bool saveLogsToFile(const QString &logFile) const;
	bool setReqMasterUid(const QString &uid_master, const QString &uid_slave) const;
	bool setReqMasterUidNull(const QString &uid_slave) const;
	bool setReqStatus(const QString &uid_slave, int status, uint date_time) const;
	bool setStatus(const StatusData &data) const;
	QList<GribFileResp> getListFiles(const QString &qStrWhere) const;
    QList<GribFileResp> getListFiles(const GribRequestData &reqData) const;
    // каталог для погодных файлов без запроса
    QString getFreeGribFilesPath() const;
    QStringList getFreeGribFiles() const;
	bool getReqById(const QString &uid, GribRequestData &data) const;
	bool insertSeaData(const QString &name, const QString &addr, const QString &rus_name, int check) const;
	bool clearSeas() const;
private:
	juce::String m_gribPath;
	juce::String m_icePath;
	juce::String m_databaseFullPath;
	DataBaseQt *m_dataBase;
};

#endif // GRIBICEFOLDERDATADRIVER_H