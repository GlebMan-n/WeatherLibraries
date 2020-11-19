#include "GribIceFolderDataDriver.h"
#include <gis_tool_qt/grib_lib/DataBaseQt.h>

//using namespace juce;

/** Фабрика для плагина данных */
class GribIceDriverPlugin : public DataAccess_DataDriverPlugin
{
public:
	GribIceDriverPlugin()
	{
		m_keys.push_back(GribIceFolderDataDriver::classNameStr());
	}
	virtual ~GribIceDriverPlugin()
	{
	}

	virtual DataAccess_DataDriver* create(const std::string &key) const
	{
		DataAccess_DataDriver *ptDrv = NULL;
		if (key.compare(GribIceFolderDataDriver::classNameStr()) == 0)
		{
			ptDrv = new GribIceFolderDataDriver();
		}
		return ptDrv;
	}
};

/** Инициализация библиотеки */
class GribIce_InitPrivate
{
public:
	GribIce_InitPrivate()
	{
		DataAccess_DataDriver::makeFactory().addPlugin(pGribIceDriverPlugin = new GribIceDriverPlugin());
	}
	~GribIce_InitPrivate()
	{
        if (DataAccess_DataDriver::makeFactory().takePlugin(pGribIceDriverPlugin))
            delete pGribIceDriverPlugin;
    }

private:
    GribIceDriverPlugin *pGribIceDriverPlugin;
};

static GribIce_InitPrivate init;

GribIceFolderDataDriver::GribIceFolderDataDriver()
{
	m_dataBase = NULL;
	m_className = GribIceFolderDataDriver::classNameStr();	
}

QString GribIceFolderDataDriver::crFolder(const QString &folderName) const
{
	try{
		juce::String str = createFolder(qt_juce(folderName));
		QString res = juce_qt(str) + "/";
		return res;
	}
	catch (...)
	{
	}
	return "";
}

GribIceFolderDataDriver::~GribIceFolderDataDriver()
{
	if (m_dataBase)
		delete m_dataBase;
}

juce::String GribIceFolderDataDriver::createFolder(const juce::String &folderName) const
{
	try{
		juce::String path = m_path + juce::String("/") + folderName;
		juce::File f(path);
		if (!f.exists())
		{
			if (!f.createDirectory())
			{
				return "";
			}
		}
		return path;
	}
	catch (...)
	{
	}
	return "";
}

void GribIceFolderDataDriver::deleteFolder(const juce::String &folderName) const
{
	try{
		juce::String path = m_path + juce::String("/") + folderName;
		juce::File f(path);
		if (f.exists())
		{
			f.deleteRecursively();
		}
	}
	catch (...)
	{
	}
	return;
}

DataBaseQt* GribIceFolderDataDriver::getDataBase() const
{
	DataBaseQt* result = new DataBaseQt(NULL);	
	result->init(getDataBaseFullName());
	return result;
}

bool GribIceFolderDataDriver::addReq(const GribRequestData &data) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->addReq(data);
}

bool GribIceFolderDataDriver::init(const juce::String &path)
{
	try{
		juce::File d(path);
		if (!d.exists())
		{
			//log_error_to(log_data, "Ошибка инициалазации каталога с данными приложения.\n Инфо: каталог " << path.toSrc8Bit().data() << " не существует.");
			return false;
		}

		m_path = path;
		correctionPath(m_path);
		m_path += juce::String("/") + dataName();

		juce::File dP(m_path);
		if (!dP.exists())
		{
			if (!dP.createDirectory())
			{
				return false;
			}
		}
		m_dataBase = getDataBase();
		Q_CHECK_PTR(m_dataBase);
		return true;
	}
	catch (...)
	{
	}

	return false;
}

bool GribIceFolderDataDriver::setDeleted(int iDeleted, const QString &uid_slave) const
{	
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->setDeleted(iDeleted, uid_slave);
}

void GribIceFolderDataDriver::setSettings(const QString serverAddr,
	const QString &serverPort,
	const QString &userLogin,
	const QString &userPassword,
	const QString &pathToGribData,
	const ZygribConsts &zygrib,
	const QString &pathToIceData) const
{
	Q_ASSERT(m_dataBase->isValid());
	return m_dataBase->setSettings(serverAddr, serverPort, userLogin, userPassword, pathToGribData, zygrib, pathToIceData);
}

bool GribIceFolderDataDriver::getSettings(QString &serverAddr,
	QString &serverPort,
	QString &userLogin,
	QString &userPassword,
	QString &pathToGribData,
	ZygribConsts &zygrib,
	QString &pathToIceData) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getSettings(serverAddr, serverPort, userLogin, userPassword, pathToGribData, zygrib, pathToIceData);
}

QMap<QString, QString> GribIceFolderDataDriver::getDrawParams() const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getDrawParams();
}

bool GribIceFolderDataDriver::setDrawParams(const QMap<QString, QString> &drawParams) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->setDrawParams(drawParams);
}

GribFileResp GribIceFolderDataDriver::getLast(const QString &uid) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getLast(uid);
}

std::vector<LogData> GribIceFolderDataDriver::getLogs(const QDateTime &begin, const QDateTime &end) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getLogs(begin, end);
}

QList<GribRequestData> GribIceFolderDataDriver::getListRequests(const QString &qStrWhere) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getListRequests(qStrWhere);
}

QMap<QString, SeaData> GribIceFolderDataDriver::getMapSea() const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getMapSea();
}

QMap<QString, QStringList> GribIceFolderDataDriver::getSeaMapAllFiles() const
{
    QMap<QString, QStringList> mapSeaFiles;
    QDir sea_dir(getIceDataPath());
    QStringList list = sea_dir.entryList(QDir::Dirs);
    foreach(QString dirName, list)
    {
        if (dirName == "." || dirName == "..")
            continue;

        QDir child_dir(getIceDataPath() + dirName);
        QStringList fileInfoList = child_dir.entryList();
        mapSeaFiles[dirName] = fileInfoList;
    }
    return mapSeaFiles;
}

QMap<QString, QString> GribIceFolderDataDriver::getSeaMapLastFiles() const
{
	QMap<QString, QString> map;
	//заполняем список последними файлами из папок морей
	QString path = getIceDataPath();
	QDir sea_dir(path);
	QStringList list = sea_dir.entryList(QDir::Dirs);
	foreach (QString dirName,list)
	{
		if (dirName == "." || dirName == "..")
			continue;
		QDir child_dir(getIceDataPath() + dirName);
		QStringList fileInfoList = child_dir.entryList();
		QDateTime lastMod;
		QFileInfo lastFile;
		foreach (QString fileName,fileInfoList)
		{
			if (fileName == "." || fileName == "..")
				continue;
			QString fileNameStr = getIceDataPath() + dirName + QDir::separator() + fileName;
			QFileInfo fileInfo(fileNameStr);
			if (lastMod < fileInfo.lastModified())
			{
				lastMod = fileInfo.lastModified();
				lastFile = fileInfo;
			}			
		}
		QString sea = child_dir.dirName();
		QString seaFilePath = lastFile.absoluteFilePath();
		map.insert(sea, seaFilePath);// [sea] = seaFilePath;
	}
	return map;
}

QStringList GribIceFolderDataDriver::getSeaLastFiles(const QStringList &seaNames) const
{
	QStringList result;
	foreach (QString string,seaNames)
	{
		QMap<QString, QString> map = getSeaMapLastFiles();
		result << map[string];
	}
	return result;
}


QList<GribRequestData> GribIceFolderDataDriver::getListRequests(int i_type) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getListRequests(i_type);
}

bool GribIceFolderDataDriver::importSeaTextFile(const QString &fileName) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->importSeaTextFile(fileName);
}

bool GribIceFolderDataDriver::updateSeaData(SeaData seaData) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->updateSeaData(seaData);
}

bool GribIceFolderDataDriver::addLog(LogData data) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->addLog(data);
}

QString GribIceFolderDataDriver::getLastLogs(int count) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getLastLogs(count);
}

bool GribIceFolderDataDriver::getStatus(StatusData &data) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getStatus(data);
}

bool GribIceFolderDataDriver::addResponse(const GribRequestData &resp, const QString &filename) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->addResponse(resp, filename);
}

bool GribIceFolderDataDriver::deleteReq(QString uid) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->deleteReq(uid);
}

bool GribIceFolderDataDriver::saveLogsToFile(const QString &logFile) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->saveLogsToFile(logFile);
}

bool GribIceFolderDataDriver::setReqMasterUid(const QString &uid_master, const QString &uid_slave) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->setReqMasterUid(uid_master,uid_slave);
}

bool GribIceFolderDataDriver::setReqMasterUidNull(const QString &uid_slave) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->setReqMasterUidNull(uid_slave);
}

bool GribIceFolderDataDriver::setReqStatus(const QString &uid_slave, int status, uint date_time) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->setReqStatus(uid_slave, status, date_time);
}

bool GribIceFolderDataDriver::setStatus(const StatusData &data) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->setStatus(data);
}

QList<GribFileResp> GribIceFolderDataDriver::getListFiles(const QString &qStrWhere) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getListFiles(qStrWhere);
}

QList<GribFileResp> GribIceFolderDataDriver::getListFiles(const GribRequestData &reqData) const
{
    Q_CHECK_PTR(m_dataBase);
    QString qStrWhere;
    qStrWhere = " WHERE uid_slave = '" + reqData.m_uid_slave + "' GROUP BY DATE_TIME ORDER BY DATE_TIME DESC ";
    return m_dataBase->getListFiles(qStrWhere);     
}

QString GribIceFolderDataDriver::getFreeGribFilesPath() const
{
    QString path = QString(gribFolderName()) + QString(QDir::separator()) + QString(gribFreeFolderName());
    return crFolder(path);
}

QStringList GribIceFolderDataDriver::getFreeGribFiles() const
{
    QString path = getFreeGribFilesPath();
    QDir sea_dir(path);
    QStringList list = sea_dir.entryList(QDir::Files);
    for (int i = 0; i < list.size(); i++)
    {
        QString string;
        string = list.at(i);
        string.insert(0, getFreeGribFilesPath() + QDir::separator());
        list[i] = string;
    }
    return list;
}

bool GribIceFolderDataDriver::getReqById(const QString &uid, GribRequestData &data) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->getReqById(uid, data);
}

bool GribIceFolderDataDriver::insertSeaData(const QString &name, const QString &addr, const QString &rus_name, int check) const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->insertSeaData(name, addr, rus_name, check);
}

bool GribIceFolderDataDriver::clearSeas() const
{
	Q_CHECK_PTR(m_dataBase);
	return m_dataBase->clearSeas();
}