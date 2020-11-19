#include "RequestRegQt.h"
#include <grib_lib/GribUtility.h>

#include <common/include/tr_macros.h>


RequestRegQt::RequestRegQt(QObject *parent)
    : HttpClientQt(parent)
{
    connect(this,
            SIGNAL(signData(const QByteArray&)),
            SLOT(slotData(const QByteArray&)));

    m_bFinished = false;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotRegistrate()));

}

RequestRegQt::~RequestRegQt()
{

}

void RequestRegQt::start()
{
    m_timer->start(15000);
}

void RequestRegQt::slotRegistrate()
{
    if (!m_bFinished)
    {
		m_Data.m_senderIp = GribUtility::getIpV4();
		m_Data.genKey();
        sendData(m_connectionInfo.m_url, m_rawHeaders.getMap() , m_Data.toJson());
    }
    else
        emit signFinished();
}

void RequestRegQt::slotData(const QByteArray& ba)
{
    m_bFinished = m_uidResp.fromJson(ba);
}

void RequestRegQt::setData(GribRequestData val)
{
    m_Data.fromJson(val.toJson());
    m_uidResp.m_uid_master  = m_Data.m_uid_master;
    m_uidResp.m_uid_slave   = m_Data.m_uid_slave;
}

