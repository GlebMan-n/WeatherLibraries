#ifndef REQUESTCLASS_H
#define REQUESTCLASS_H

#include <QObject>
#include <QTimer>

#include "grib_lib_global.h"
#include "GribExStructures.h"

class GRIB_LIB_EXPORT RequestClass : public QObject
{
    Q_OBJECT
public:
    RequestClass(GribRequestData data, QObject* parent = NULL);
    ~RequestClass();
    GribRequestData getData() const
    {
        return m_data;
    }
    void setActual(bool bActual);
    bool isActual();

private:
    void setData(GribRequestData val)
    {
        m_data = val;
    }
private:
    QTimer*     m_timer;
    GribRequestData m_data;
    int     m_acualMin;

private slots:
    void slotCheckActual();
};

#endif // REQUESTCLASS_H