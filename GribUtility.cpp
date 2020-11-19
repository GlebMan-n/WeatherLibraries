#include <QHostInfo>
#include <QHostAddress>
#include <QAbstractSocket>

#include <GribUtility.h>

#include <gis/GisGrib/GribData.h>
#include <gis/GisGrib/GribData.h>
#include <gis/GisGrib/GriddedPlotter.h>
#include <gis/GisGrib/NetCdf2Grib.h>
#include <gis/GisGrib/GribPlot.h>
#include "GribExStructures.h"

#include <common/include/tr_macros.h>

using namespace GdiGis;

QMap<QString, QString> GribUtility::getMapSeaNames()
{
	QMap<QString, QString> res;
	res["Bal"] = tr("Балтийское");
	res["Bar"] = tr("Баренцево и Белое");
	res["Bea"] = tr("Бофорта");
	res["Ber"] = tr("Берингово");
	res["Cas"] = tr("Каспийское");
	res["Chu"] = tr("Чукотское");
	res["Ess"] = tr("Восточно-Сибирское");
	res["Gre"] = tr("Гренладское");
	res["Kar"] = tr("Карское");
	res["Lap"] = tr("Лаптевых");
	res["Okh"] = tr("Охотское");
	//res["Whi"] = tr("Белое");
	return res;
}

QString GribUtility::genSeaFileName(const QString &fileName, const QString &seaName, const QString &year, bool bWdc)
{
	if (bWdc)
		return GribUtility::genDataSetPath(seaName, year, bWdc) + fileName;
	else
		return GribUtility::genDataSetPath(seaName, year, bWdc) + "&download=" + fileName;
}

QString GribUtility::genDataSetPath(const QString &seaName, const QString &year /*yyyy*/, bool bWdc)
{
	if (bWdc)
		return QString("http://wdc.aari.ru/datasets/d0004/" + seaName + "/sigrid/" + year + "/");
	else
		return QString("http://www.aari.ru/resources/d0004/index.php?dir=" + seaName + "%2Fsigrid%2F" + year + "%2F");
}

QString GribUtility::parseSeaAnswer(const QString &answer, int &fileLoadedDate, bool isRoot, bool bWdc)
{
	QString result = "";
	QStringList list = answer.split("\n");
	QStringList tmpList;
	QStringList filelist;
	for (int i = 0; i < list.size(); i++)
	{
		QString current = list.at(i);
		if (current.contains("zip"))
			tmpList.append(current);
	}

	for (int i = 0; i < tmpList.size(); i++)
	{
		QString current = tmpList.at(i);
		int begin, end;
		if (bWdc)
		{
			begin = current.indexOf(">aari") + 1;
			end = current.indexOf("</a>");
		}
		else
		{
			begin = current.indexOf(">aari_") + 1;
			end = current.indexOf("</strong>");
		}

		if (begin == -1 || end == -1)
			continue;
		QString fName = current.mid(begin, end - begin);
		filelist.append(fName);
	}

	int dateJ = 0;
	QString file = "";
	for (int i = 0; i < filelist.size(); i++)
	{
		QString filename = filelist.at(i);

		if (checkData(filename, isRoot,dateJ))
		{
			QString dateStr = filename.section("_", 2, 2);
			QDate date = QDate::fromString(dateStr, "yyyyMMdd");
			int tmp = date.toJulianDay();
			if (tmp > dateJ)
			{
				dateJ = tmp;
				file = filename;
			}
		}
	}
	result = file;
	fileLoadedDate = dateJ;
	return result;
}


bool GribUtility::checkData(const QString &fileName, bool isRoot, int dt_last)
{
	bool result = false;
	QString dateStr;
	QDate date;
	if (isRoot)
	{
		dateStr = fileName.section("_", 2, 2);
		date = QDate::fromString(dateStr, "yyyyMMdd");
	}
	else
	{
		dateStr = fileName;
		date = QDate::fromJulianDay(dateStr.toInt());
	}


	if (date.isValid())
	{
		QDate lastDate;
		lastDate = QDate::fromJulianDay(dt_last);
		result = date > lastDate;
	}
	return result;
}
QString GribUtility::getPage(const GribRequestData &requestData, const ZygribConsts &gribConsts, float min_mod, float max_mod)
{
	QString result, currentTime, page, phpFileName;
	float modx, mody, y0t, y1t, x0t, x1t;
	currentTime = QTime::currentTime().toString("HHmmss");
	phpFileName = gribConsts.m_scriptpath + gribConsts.m_grbFileResolution;
	/*
	if (max_mod != 0 && min_mod != 0)
	{
		modx = (float)rand() / RAND_MAX;
		modx = min_mod + modx * (max_mod - min_mod);

		mody = (float)rand() / RAND_MAX;
		mody = min_mod + mody * (max_mod - min_mod);
	}

	*/
	y0t = requestData.m_y0;// -mody;// - rand() % 4;
	y1t = requestData.m_y1;// +mody;// + rand() % 4;
	x0t = requestData.m_x0;// -modx;// - rand() % 4;
	x1t = requestData.m_x1;// +modx;// + rand() % 4;

	QTextStream(&page) << phpFileName
		<< "but=prepfile"
		<< "&la1=" << y0t//floor(y0t)
		<< "&la2=" << y1t//ceil(y1t)
		<< "&lo1=" << x0t//floor(x0t)
		<< "&lo2=" << x1t//ceil(x1t)
		<< "&res=" << requestData.m_resolution
		<< "&hrs=" << requestData.m_interval
		<< "&jrs=" << requestData.m_days
		<< "&par=" << requestData.m_parametrs
		<< "&rungfs=" << requestData.m_runGFS
		<< "&l=" << gribConsts.m_zygriblog
		<< "&m=" << gribConsts.m_zygribpwd
		<< "&client=" << gribConsts.m_client
		<< "&tm=" << currentTime;

	result = "http://" + gribConsts.m_host + page;
	return result;
}

QString GribUtility::getProxyUrl(const QString &m_serverAddr, int m_serverPort)
{
	//return "http://ikonnikov:81/proxyapp/";
	QString ipPort = QString("%1:%2").arg(m_serverAddr).arg(m_serverPort);
	QString now = QTime::currentTime().toString("HHmmss");
	return QString("http://%1?tm=%2").arg(ipPort).arg(now);
}

QString GribUtility::getFileUrl(const ZygribConsts &consts, const QString &fileName)
{
	//http://www.zygrib.org/noaa/313O562/20160218_232605_%20
	QString tmp = fileName;
	tmp.replace(".grb", "%20");
	QString url;
	QTextStream(&url) << "http://" << consts.m_host << consts.m_scriptpath << consts.m_scriptstock << tmp;
	return url;
}


QByteArray GribUtility::makeAuthString(const QString &destAddr /*URL*/, const QString &srvAddr /*PAGE*/)
{
	QByteArray result;
	result.clear();
	QByteArray proxyHostHash = QCryptographicHash::hash(destAddr.toLocal8Bit(), QCryptographicHash::Md5);
	QByteArray bt2 = QCryptographicHash::hash(srvAddr.toLocal8Bit(), QCryptographicHash::Md5);
	for (int i = 0; i < proxyHostHash.count(); i++)
		result[i] = (int)(proxyHostHash[i] + (bt2[i] * pow(-1.0, i))) % 256;
	return result;
}

QString GribUtility::genIceFileName(const QString& seaName, int date)
{
	QString str;
	QDate qdate;
	qdate = QDate::fromJulianDay(date);
	str = "aari_" + seaName.toLower() + qdate.toString("_yyyyMMdd_pl_a") + ".zip";
	return str;
}

int GribUtility::getTimeTfromIceFileName(const QString& fileName)
{
	QDate date;
	QString str = fileName.section("_", 2, 2);
	date = QDate::fromString(str, "yyyyMMdd");
	if (date.isValid())
		return date.toJulianDay();
	else
		return -1;
}


//////////////////////////////////////////////////////////////////////////
int GribUtility::getFilesDirsLists(const QString &aRootDir, QStringList &aDirectories, QStringList &aFiles, bool aStripRoot)
{
	QDir vDir(aRootDir);

	QString vPath = aRootDir;
	int vLevel = 0;

	QList < QStringList > vDirs;
	QList < int > vIndex;
	QList < QString > vPaths;

	aDirectories.append("");

	vDirs.append(aDirectories);
	aDirectories.clear();
	vIndex.append(0);
	vPaths.append(vPath);

	while (vLevel > -1)
	{
		int i;
		bool vUp = false;
		for (i = vIndex.at(vLevel); i < vDirs.at(vLevel).size(); i++)
		{
			vPath = vPaths.at(vLevel) + vDirs.at(vLevel).at(i) + QLatin1String("\\");
			aDirectories.append(vPath);
			// get files
			QDir vLocDir(vPath);
			QStringList vLocFiles = vLocDir.entryList(QDir::Files);
			for (int j = 0; j < vLocFiles.size(); j++)
				aFiles.append(vPath + vLocFiles.at(j));
			//

			QStringList vNewList = vLocDir.entryList(QDir::AllDirs | QDir::Hidden
				| QDir::NoDotAndDotDot);
			if (vNewList.size() > 0)
			{
				vIndex[vLevel] = i + 1;
				vLevel++;
				if (vLevel >= vDirs.size())
				{
					vDirs.append(vNewList);
					vPaths.append(vPath);
					vIndex.append(0);
				}
				else
				{
					vDirs[vLevel] = vNewList;
					vPaths[vLevel] = vPath;
					vIndex[vLevel] = 0;
				}
				vUp = true;
				break;
			}
		}
		if (vUp) continue;
		if (i >= vDirs.at(vLevel).size()) vLevel--;
	}

	aDirectories.removeFirst();

	if (aStripRoot)
	{
		for (int i = 0; i < aDirectories.size(); i++)
		{
			aDirectories[i].replace(QDir::toNativeSeparators(QString("%1/").arg(aRootDir)),
				QLatin1String(""));
		}

		for (int i = 0; i < aFiles.size(); i++)
		{
			aFiles[i].replace(QDir::toNativeSeparators(QString("%1/").arg(aRootDir)),
				QLatin1String(""));
		}
	}

	return 1;
}

QString GribUtility::getIpV4()
{
	QString result = "";

	QList< QHostAddress > addresses = QHostInfo::fromName(QHostInfo::localHostName()).addresses();

	foreach(const QHostAddress & a, addresses)
	{
		QString protocol = "???";
		switch (a.protocol())
		{
		case QAbstractSocket::IPv4Protocol:
			protocol = "IPv4";
			result = a.toString();
			break;
		case QAbstractSocket::IPv6Protocol:
			protocol = "IPv6";
			break;
		}

	}

	return result;
}


bool GribUtility::saveFile(const QByteArray& byteArray, const QString &path)
{
	/*QFile file(path);
	if(file.open(QIODevice::WriteOnly))
	{
	QDataStream out(&file);
	out << byteArray;
	file.close();
	return true;
	}*/
	return false;
}

QByteArray GribUtility::openFileBytes(const QString &path, bool &bRes)
{
	QByteArray result;
	QFile file;
	bRes = openFile(path, file);
	if (!bRes)
		return result;
	result = file.readAll();
	bRes = true;
	return result;
}

bool GribUtility::openFile(const QString &path, QFile &result, QIODevice::OpenMode mode)
{
	bool bRes;
	result.setFileName(path);
	if (result.open(mode))
		bRes = true;
	else
		bRes = false;
	return bRes;
}

QString GribUtility::genHash(const QByteArray &ba)
{
	QCryptographicHash cryp(QCryptographicHash::Md5);
	cryp.addData(ba);
	return QString(cryp.result().toHex());
}

int GribUtility::getCurrentTimeZone()
{
	QDateTime local;
	local = QDateTime::currentDateTime();
	QString dtime = local.toString(Qt::RFC2822Date);
	QString timeZone = dtime.section(" ", 4, 4);
	return timeZone.left(3).toInt();
}

QString GribUtility::getCurrentTimeZoneStr()
{
	QString dtime = QDateTime::currentDateTime().toString(Qt::RFC2822Date);
	QString timeZone = dtime.section(" ", 4, 4);
	return timeZone;
}

int GribUtility::removeFolder(const QDir &dir)
{
	int res = 0;

	QStringList lstDirs = dir.entryList(QDir::Dirs |
		QDir::AllDirs |
		QDir::NoDotAndDotDot);

	QStringList lstFiles = dir.entryList(QDir::Files);
	foreach(QString entry, lstFiles)
	{
		QString entryAbsPath = dir.absolutePath() + QDir::separator() + entry;
		QFile::remove(entryAbsPath);
	}

	foreach(QString entry, lstDirs)
	{
		QString entryAbsPath = dir.absolutePath() + QDir::separator() + entry;
		removeFolder(QDir(entryAbsPath));
	}
	if (!QDir().rmdir(dir.absolutePath()))
	{
		res = 1;
	}
	return res;
}

long long GribUtility::getKey()
{
	uint tt = QDateTime::currentDateTime().toTime_t();
	double tmp = (double)(tt * qrand()) * 3.1415926;
	return (long long)tmp;
}

bool GribUtility::removeOldFilesInDirectory(const QString &path, int dayOld)
{
	bool result = false;
	QDir dir(path);
	QStringList lstFiles = dir.entryList(QDir::Files);
	QStringList deleteStringList;

	foreach(QString string, lstFiles)
	{
		bool result;
		int dt = string.section(".", 0, 0).toInt(&result);
		if (!result)
			continue;

		int currentDt = QDateTime::currentDateTime().toTime_t();
		if ((currentDt - dt) > dayOld * 86400)
		{
			deleteStringList.append(string);
		}
	}

	foreach(QString string, deleteStringList)
	{
		QString filePath = dir.absolutePath() + QDir::separator() + string;
		bool bRes = QFile::remove(filePath);
		if (bRes)
		{
			//toLog( tr("Удален файл: ") + filePath);
			result = true;
		}
	}

	return result;
}

bool GribUtility::invertIceFileName(const QString &oldName, QString &newName, const QString &letter)
{
	QString result = oldName;
	if (result.contains("_a.zip"))
	{
		newName = result.replace("_a.zip", "_" + letter + ".zip");
		return true;
	}	
    else if (result.contains("_b.zip"))
    {
        newName = result.replace("_b.zip", "_" + letter + ".zip");
        return true;
    }
    else if (result.contains("_c.zip"))
    {
        newName = result.replace("_c.zip", "_" + letter + ".zip");
        return true;
    }
    else if (result.contains("_d.zip"))
    {
        newName = result.replace("_d.zip", "_" + letter + ".zip");
        return true;
    }
    else if (result.contains("_e.zip"))
    {
        newName = result.replace("_e.zip", "_" + letter + ".zip");
        return true;
    }
    else if (result.contains("_f.zip"))
    {
        newName = result.replace("_f.zip", "_" + letter + ".zip");
        return true;
    }
	else
		return false;
}

bool GribUtility::copyFile(const QString &oldFileFullName, const QString &newFileFullName)
{
	QFile file(oldFileFullName);
	if (!file.open(QIODevice::ReadWrite))
		return false;
	return file.copy(newFileFullName);
}

bool GribUtility::renameFile(const QString &oldFileFullName, const QString &newFileFullName)
{
	QFile file(oldFileFullName);
	if (!file.open(QIODevice::ReadWrite))
		return false;
	bool bRes = file.rename(newFileFullName);
	file.close();
	return bRes;
}
