#ifndef REQUESTREGQT_H
#define REQUESTREGQT_H

#include <QObject>
#include "HttpClientQt.h"
#include "grib_lib_global.h"
#include "GribExStructures.h"
#include <QTimer>

class GRIB_LIB_EXPORT RequestRegQt : public HttpClientQt
{
    Q_OBJECT

public:
    RequestRegQt(QObject *parent);
    ~RequestRegQt();

    void setData(GribRequestData val);
    void setConnectionInfo(ConnectionInfo val)
    {
        m_connectionInfo = val;
    }
    void setRawHeaders(RawHeaderData val)
    {
        m_rawHeaders = val;
    }
    void start();

    GribUIDResp getUidResp() const
    {
        return m_uidResp;
    }

private:
    ConnectionInfo  m_connectionInfo;
    RawHeaderData   m_rawHeaders;
    GribUIDResp     m_uidResp;
    QTimer*         m_timer;
    bool            m_bFinished;
    bool            m_toDelete;
private slots:
    void slotData(const QByteArray& ba);
    void slotRegistrate();
};

#endif // REQUESTREGQT_H
