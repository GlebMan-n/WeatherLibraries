#include "GribListRequestsLoaderQt.h"
#include <grib_lib/GribUtility.h>

#include <common/include/tr_macros.h>

GribListRequestsLoaderQt::GribListRequestsLoaderQt(const QString &url,
    QObject *parent)
    : HttpClientQt(parent)
{    
    m_url = url;

    connect(this,
            SIGNAL(signData(const QByteArray &)),
            SLOT(slotListReqResponse(const QByteArray &)));

    connect(this,
            SIGNAL(signDebug(QString)),
        SIGNAL(signDebug(QString)));
}

void GribListRequestsLoaderQt::slotFinished(const int &res)
{
	//signFinished();
}

GribListRequestsLoaderQt::~GribListRequestsLoaderQt()
{
}

void GribListRequestsLoaderQt::start()
{
    loadListRequest();
}

bool GribListRequestsLoaderQt::getListRequest(QList<GribRequestData> &data)
{
    data = m_listData;
    return true;
}

/*void GribListRequestsLoaderQt::slotResponse(const QByteArray& ba)
{
    if (ba == QString("QT_NETWORK_ERROR"))
    {
		signFinished();
        return;
    }
}*/

void GribListRequestsLoaderQt::slotListReqResponse(const QByteArray &ba)
{
    parseData(ba, m_listData);
    emit signFinished();
}

void GribListRequestsLoaderQt::loadListRequest()
{
    GribRequestData data;
    data.m_error = REQ_ERRORS::GET_REQ_LIST;
    data.m_senderIp = GribUtility::getIpV4();
    data.genKey();
    QString str, url;
   
    sendData(m_url, RawHeaderData().getMap(), data.toJson());
}

bool GribListRequestsLoaderQt::parseData(const QByteArray &ba, QList<GribRequestData> &dataList)
{
    if (ba == "QT_NETWORK_ERROR")
        return false;
    QString tmp = "||";
    QList<QByteArray> listArray = ba.split(tmp.data()->toLatin1());

    QList<GribRequestData> result;

    foreach (QByteArray reqAr, listArray)
    {
        if (reqAr.isEmpty())
            continue;
        GribRequestData data; 
        bool bRes = data.fromJson(reqAr);
        if (bRes)
            result.append(data);
    }
    dataList = result;
    return true;
}
