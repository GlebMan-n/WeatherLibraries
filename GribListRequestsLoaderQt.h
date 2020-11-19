#ifndef GribListRequestsLoaderQt_H
#define GribListRequestsLoaderQt_H

#include <QObject>
#include "HttpClientQt.h"
#include "grib_lib_global.h"
#include "GribExStructures.h"
#include <QTimer>
#include <QString>
#include <QByteArray>

class GRIB_LIB_EXPORT GribListRequestsLoaderQt : public HttpClientQt
{
    Q_OBJECT
public:
    GribListRequestsLoaderQt(const QString &url,
        QObject *parent);
    ~GribListRequestsLoaderQt();
    void start();   
    bool getListRequest(QList<GribRequestData> &data);

private:
    QList<GribRequestData> m_listData;
    QString m_url;
private:
    void loadListRequest();   
    bool parseData(const QByteArray &ba, QList<GribRequestData> &data);
private slots:
    //void slotResponse(const QByteArray& ba);
    void slotFinished(const int &res);
    void slotListReqResponse(const QByteArray &ba);

};

#endif // GribListRequestsLoaderQt_H
