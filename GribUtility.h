#ifndef GRIBUTILITY_H
#define GRIBUTILITY_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QDir>
#include <QDateTime>
#include  <QTextStream>
#include <QDebug>
#include <QList>
#include <QDate>
#include <QFile>
#include "grib_lib_global.h"
#include <QCryptographicHash>
#include <cmath>

const double FOR_KEY = 3.1415926;


class GribData;
class GribRequestData;
class ZygribConsts;
class ZygribConsts;

class GRIB_LIB_EXPORT GribUtility
{
public:
	static QString getPage(const GribRequestData &requestData, const ZygribConsts &gribConsts, float min_mod, float max_mod);

	static QString getProxyUrl(const QString &m_serverAddr, int m_serverPort);

	static QString getFileUrl(const ZygribConsts &consts, const QString &fileName);

	static QByteArray makeAuthString(const QString &destAddr /*URL*/, const QString &srvAddr /*PAGE*/);

	

	static int getTimeTfromIceFileName(const QString& fileName);


    //////////////////////////////////////////////////////////////////////////
	static int getFilesDirsLists(const QString &aRootDir, QStringList &aDirectories, QStringList &aFiles, bool aStripRoot);

	static QString getIpV4();

	static bool saveFile(const QByteArray& byteArray, const QString &path);

	static QByteArray openFileBytes(const QString &path, bool &bRes);

	static bool openFile(const QString &path, QFile &result, QIODevice::OpenMode mode = QIODevice::ReadOnly);

	static QString genHash(const QByteArray &ba);

	static int getCurrentTimeZone();

	static QString getCurrentTimeZoneStr();

	static int removeFolder(const QDir &dir);
    
	static bool removeOldFilesInDirectory(const QString &path, int dayOld);

	static QMap<QString, QString> getMapSeaNames();

	static long long getKey();

	static QString genSeaFileName(const QString &fileName,/*, const QString &datasetPath,*/ const QString &seaName, const QString &year /*yyyy*/, bool bWdc);
	static QString genDataSetPath(const QString &seaName, const QString &year /*yyyy*/, bool bWdc);
	static QString genIceFileName(const QString& seaName, int date);
	static bool checkData(const QString &fileName, bool isRoot, int dt_last);
	static QString parseSeaAnswer(const QString &answer, int &fileLoadedDate, bool isRoot, bool bWdc);
	static bool renameFile(const QString &oldFileFullName, const QString &newFileFullName);
	static bool copyFile(const QString &oldFileFullName, const QString &newFileFullName);
	static bool invertIceFileName(const QString &oldName, QString &newName, const QString &letter);
	
};

#endif // GRIBUTILITY_H
