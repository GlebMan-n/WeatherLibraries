#include "HttpClientQt.h"
#include <QDebug>

#include <common/include/tr_macros.h>

HttpClientQt::HttpClientQt(QObject *parent)
    : QObject(parent)
{
    m_networkAccessManager = NULL;
    m_networkAccessManager = new QNetworkAccessManager(this);
    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

HttpClientQt::~HttpClientQt()
{
}

void HttpClientQt::sendData(const QString &url, const QMap<QString,QString> &rawHeaders, const QByteArray &ba)
{
	
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    QMap<QString, QString>::const_iterator i = rawHeaders.constBegin();
    while (i != rawHeaders.constEnd())
    {
        request.setRawHeader(i.key().toUtf8(),i.value().toUtf8());
        ++i;
    }
    m_lastError.clear();

    if (ba.isEmpty())
        sendData(request);
    else
        sendData(request, ba);
}

void HttpClientQt::sendData(const QNetworkRequest &request, const QByteArray &ba)
{
	m_eTimer.start();
    QNetworkReply* m_networkReply = m_networkAccessManager->post(request, ba);

    connect(m_networkReply, SIGNAL(finished()),
            SLOT(finishedSendData()));
    connect(m_networkReply, SIGNAL(readyRead()),
            SLOT(slotReadyRead()));
    connect(m_networkReply, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(slotDownloadProgress(qint64,qint64)));
    connect(m_networkReply, SIGNAL(uploadProgress(qint64,qint64)),
            SLOT(slotUploadProgress(qint64,qint64)));
    connect(m_networkReply, SIGNAL(downloadProgress(qint64,qint64)),
            SIGNAL(signDownloadProg(qint64,qint64)));
    connect(m_networkReply, SIGNAL(uploadProgress(qint64,qint64)),
            SIGNAL(signUploadProg(qint64,qint64)));

    connect(m_networkReply, SIGNAL(bytesWritten(qint64)),
            SLOT(slotBytesWritten(qint64)));

    connect(m_networkReply, SIGNAL(readyRead()),
            SLOT(downloadReadyRead()));
}

void HttpClientQt::slotBytesWritten(qint64 bytes)
{
}

void HttpClientQt::sendData(const QNetworkRequest &request)
{
	m_eTimer.start();
    QNetworkReply* m_networkReply = m_networkAccessManager->get(request);

    connect(m_networkReply, SIGNAL(finished()),
            SLOT(finishedSendData()));
    connect(m_networkReply, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(slotDownloadProgress(qint64,qint64)));
    connect(m_networkReply, SIGNAL(uploadProgress(qint64,qint64)),
            SLOT(slotUploadProgress(qint64,qint64)));
    connect(m_networkReply, SIGNAL(readyRead()),
            SLOT(downloadReadyRead()));
}

void HttpClientQt::logRequest(const QNetworkRequest &request, const QString &body)
{
    QString log;
    log.clear();
    QTextStream(&log) << "\nBEGIN REQUEST  ==============================\n" << "\n URL: " << request.url().toDisplayString() ;
    QTextStream(&log) << "\n\n Headers:\n\n ";
    if (!request.header(QNetworkRequest::ContentTypeHeader).toString().isEmpty())
        QTextStream(&log) << QString(" >QNetworkRequest::ContentTypeHeader: ") << request.header(QNetworkRequest::ContentTypeHeader).toString() << "\n ";
    if (!request.header(QNetworkRequest::ContentLengthHeader).toString().isEmpty())
        QTextStream(&log) << QString(">QNetworkRequest::ContentLengthHeader: ") << request.header(QNetworkRequest::ContentLengthHeader).toString() << "\n ";
    if (!request.header(QNetworkRequest::LocationHeader).toString().isEmpty())
        QTextStream(&log) << QString(">QNetworkRequest::LocationHeader: ") << request.header(QNetworkRequest::LocationHeader).toString() << "\n ";
    if (!request.header(QNetworkRequest::CookieHeader).toString().isEmpty())
        QTextStream(&log) << QString(">QNetworkRequest::CookieHeader: ") << request.header(QNetworkRequest::CookieHeader).toString() << "\n ";
    if (!request.header(QNetworkRequest::SetCookieHeader).toString().isEmpty())
        QTextStream(&log) << QString(">QNetworkRequest::SetCookieHeader: ") << request.header(QNetworkRequest::SetCookieHeader).toString() << "\n ";
    if (!request.header(QNetworkRequest::UserAgentHeader).toString().isEmpty())
        QTextStream(&log) << QString(">QNetworkRequest::UserAgentHeader: ") << request.header(QNetworkRequest::UserAgentHeader).toString() << "\n ";
    if (!request.header(QNetworkRequest::ServerHeader).toString().isEmpty())
        QTextStream(&log) << QString(">QNetworkRequest::ServerHeader: ") << request.header(QNetworkRequest::ServerHeader).toString() << "\n ";
    if (!request.header(QNetworkRequest::ContentDispositionHeader).toString().isEmpty())
        QTextStream(&log) << QString(">QNetworkRequest::ContentDispositionHeader: ") << request.header(QNetworkRequest::ContentDispositionHeader).toString() << "\n ";
    QTextStream(&log) << "\n Raw headers:\n\n ";
    for (int i = 0; i < request.rawHeaderList().length(); i++)
        QTextStream(&log) << ">" << QString::fromUtf8(request.rawHeaderList().at(i)) << ": " << request.rawHeader(request.rawHeaderList().at(i) ) << "\n ";

    QTextStream(&log) << "\n BODY DATA : " << body << "\n";

    QTextStream(&log) << "\nEND REQUEST  ================================\n";
}

void HttpClientQt::toLog(const QString &log)
{
    QByteArray ba;
    ba.insert(0,log);//= QByteArray(log);
    toLog(ba);
}

void HttpClientQt::toLog(const QByteArray &log)
{    
    //signDebug(log + " ID: " + m_Data.m_uid_slave);
}

void HttpClientQt::finished(QNetworkReply* reply)
{
	m_lastOperationTime = m_eTimer.elapsed();
    //QNetworkAccessManager* accessManager = qobject_cast<QNetworkAccessManager*>(sender());
    if (reply->error())
    {
        QString tmp;
		m_lastError = reply->errorString();
        switch (reply->error())
        {			
            case 203:
                toLog(tr("Файл в процессе загрузки на прокси-сервере"));
                emit signResult("ERROR 404 - " + reply->errorString());
                emit signData("ERROR404");
                break;
            case 401:
                tmp = tr("Ошибка загрузки файла на прокси сервере");
                toLog(tmp);
                emit signResult("ERROR 500 - " + reply->errorString());
                emit signData("ERROR500");
                break;
            default:
                toLog(tr("Другая ошибка - ") + reply->errorString());
                emit signResult("QT_NETWORK_ERROR: " + QString::number(reply->error()) + " - " + reply->errorString());
                emit signData("QT_NETWORK_ERROR");
                break;
        }
        emit signFinished(0);
    }
    else
    {
		m_lastError.clear();
        try
        {
            QByteArray  bytes = reply->read(reply->size());
            emit signFinished(1);
            m_lastError = "OK";
            emit signData(bytes);
        }
        catch (...)
        {
        }
    }
    try
    {
        if (reply)
            reply->deleteLater();
    }
    catch (...)
    {
    }
}

void HttpClientQt::slotDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
}

void HttpClientQt::slotUploadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
}

void HttpClientQt::finishedSendData()
{
}

void HttpClientQt::downloadReadyRead()
{
}

void HttpClientQt::slotReadyRead()
{
}
