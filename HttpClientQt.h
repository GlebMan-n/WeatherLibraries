#ifndef HTTPCLIENTQT_H
#define HTTPCLIENTQT_H

#include <QObject>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QXmlStreamReader>
#include <QByteArray>
#include <QString>
#include <QList>
#include <QMap>
#include <QMapIterator>
#include <QNetworkRequest>
#include <QNetworkCacheMetaData>
#include <QElapsedTimer>
#include "grib_lib_global.h"
#include <grib_lib/GribExStructures.h>

/* Класс для обращению к серверу */
class GRIB_LIB_EXPORT HttpClientQt : public QObject
{
    Q_OBJECT

public:
    HttpClientQt(QObject *parent);
    ~HttpClientQt();
    GribRequestData    m_Data;
	QString m_lastError;
	qint64 m_lastOperationTime;
	QNetworkAccessManager* m_networkAccessManager;
    GribRequestData getData() const
    {
        return m_Data;
    }
    void setData(GribRequestData val)
    {
        m_Data = val;
    }
    /*обратиться к серверу (указать адрес прокси и заголовки)*/
    void sendData(const QString &url, const QMap<QString,QString> &rawHeaders, const QByteArray &ba = QByteArray());
    void sendData(const QNetworkRequest &request, const QByteArray &ba);
    void sendData(const QNetworkRequest &request);
	QString getLastError() const { return m_lastError; }
	qint64 getLastOperationTime() const { return m_lastOperationTime; }
private:
    
	QElapsedTimer m_eTimer;	
protected:
    void toLog(const QString &log);
    void toLog(const QByteArray &log);
private:
    void logRequest(const QNetworkRequest &request,const QString &body = QString());
private slots:
    void finished(QNetworkReply* reply);
    //NetworkReply
    void slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void slotUploadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void finishedSendData();
    void downloadReadyRead();
    void slotBytesWritten(qint64 bytes);
    void slotReadyRead();
signals:
    void signDebug(const QString &log);
    void signResult(const QString &qStrResult);
    void signData(const QByteArray &ba);
    void signFinished(const int &iRes);// 0 - fail, 1 - ok
    void signUploadProg(const qint64 &bytesReceived, const qint64 &bytesTotal);
    void signDownloadProg(const qint64 &bytesReceived, const qint64 &bytesTotal);
    void signFinished();

};

#endif // HTTPCLIENTQT_H
