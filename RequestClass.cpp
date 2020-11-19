#include <RequestClass.h>

#include <common/include/tr_macros.h>

RequestClass::RequestClass(GribRequestData data, QObject* parent)
{
    this->setParent(parent);
    setData(data);
    m_timer = NULL;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotCheckActual()));
}

RequestClass::~RequestClass()
{
}

void RequestClass::slotCheckActual()
{
    if (m_acualMin > 0)
        m_acualMin -= 1;
    else
        setActual(false);
}

void RequestClass::setActual(bool bActual)
{
    if ( bActual && m_data.m_interval > 0 )
    {
        m_acualMin = m_data.m_interval * 60;
        if (m_timer)
            m_timer->start(1000);
    }
    else
    {
        m_acualMin = 0;
        if (m_timer)
            m_timer->stop();
    }
}

bool RequestClass::isActual()
{
    return (bool) m_acualMin;
}