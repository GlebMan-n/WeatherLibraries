#include "FileLoaderQt.h"

#include <grib_lib/GribUtility.h>

#include <common/include/tr_macros.h>

FileLoaderQt::FileLoaderQt(QObject *parent)
	: HttpClientQt(parent)
{
	connect(this,
		SIGNAL(signData(const QByteArray&)),
		SLOT(slotResponse(const QByteArray&)));
	connect(this,
		SIGNAL(signFinished(const int &)),
		SLOT(slotFinished(const int &)));
    m_timerTimeout = NULL;
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotStartLoad()));
	m_curCheckSumCounter = 0;
	m_startBuldFile = false;
	m_bLoaded = false;
    setTimeout(3600);
    m_single = false;
}

FileLoaderQt::FileLoaderQt(const QString &pathToData, const QString &url, QObject *parent)
    : HttpClientQt(parent)
{

    m_timerTimeout = NULL;
    m_connectionInfo.m_pathToGribData = pathToData;
    m_connectionInfo.m_url = url;

    connect(this,
        SIGNAL(signData(const QByteArray&)),
        SLOT(slotResponse(const QByteArray&)));
    connect(this,
        SIGNAL(signFinished(const int &)),
        SLOT(slotFinished(const int &)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotStartLoad()));
    m_curCheckSumCounter = 0;
    m_startBuldFile = false;
    m_bLoaded = false;
    setTimeout(3600);   
    m_single = true;
}

void FileLoaderQt::slotFinished(const int &res)
{
}

FileLoaderQt::~FileLoaderQt()
{
}

void FileLoaderQt::start(int wait)
{
	m_timer->start(wait);
    if (m_timerTimeout)
        delete m_timerTimeout;
    m_timerTimeout = new QTimer(this);
    connect(m_timerTimeout, SIGNAL(timeout()), this, SIGNAL(signTimeout()));
    m_timerTimeout->start(m_timeout);
}

void FileLoaderQt::slotResponse(const QByteArray& ba)
{
	try
	{
		GribUIDResp resp;
		if (!resp.fromJson(ba))
		{
			QString str = GribUtility::genHash(ba);	

			int index = m_checkSums.indexOf(str);	
			
			if(index < 0 || index > m_checkSums.size())
			{
				m_bLoaded = false;
				return;
			}

			toLog(tr("Получен блок файла, ожидаемая чек сумма: ") + m_checkSums.at(index) + tr(" фактическая чек сумма: ") + str);
	
			Blocks::Block block;
			block.setBlock(ba);
			block.setHash(str);
			block.setNumber(index);
			m_blocks.set(index, block);
			toLog(tr("Блок номер:") + QString::number(index + 1)  + tr(", всего блоков: ") + QString::number(m_checkSums.size()) );
			m_timer->setInterval(5000);
			m_bLoaded = true;					
			return;
		}

		if (resp.m_error == BLOCK_INFO)
		{
			QByteArray tmp = resp.m_info.toLatin1();
			QString hashOut = resp.m_key;
			QString hashMe = GribUtility::genHash(tmp);
			bool bRes = hashOut == hashMe;
			if (bRes && m_blocks.fromJson(tmp))
			{
				m_checkSums = m_blocks.getHashes();
				
				m_startBuldFile = true;
				m_bLoaded	= true;
				toLog(tr("Получена информация о блоках: количество блоков - ") + QString::number(m_blocks.getHashes().size()) + tr(", общий размер в байтах: ") + QString::number(m_blocks.getSizeInBytes()) );
				m_Data.m_date_time_t = m_blocks.getDateTime_t();
				m_timer->setInterval(5000);
			}

			if(!bRes)
			   toLog(tr("Получена информация о блоках: количество блоков - ") + QString::number(m_blocks.getHashes().size()) + tr(", общий размер в байтах: ") + QString::number(m_blocks.getSizeInBytes()) );
			return;
		}

		if(resp.m_error == SET_DEL_OK || resp.m_error == REQ_NOT_FOUND || resp.m_error == FILE_NOT_ACTUAL)
		{
			m_Data.m_error = resp.m_error;
			toLog(tr("Файл не актуален, удален или не найден ") + resp.m_info);
			m_startBuldFile = false;
			signFinished();
			return;
		}

		if (resp.m_error == BLOCKS_NOT_FOUND)
		{
			toLog(tr("Файлов по запросу нет ") + resp.m_info);
			m_startBuldFile = false;
			return;
		}

		if (resp.m_error == FAIL)
		{
			toLog(tr("Ошибка отправки файла с сервера webgio: ") + resp.m_info);
			m_startBuldFile = false;
			return;
		}


	}
	catch(...)
	{
	}
}

void FileLoaderQt::setData(const GribRequestData &data)
{
	GribRequestData dt = data;
	QByteArray ba = dt.toJson();
	m_Data.fromJson(ba);
	m_Data.setType(3);
}

void FileLoaderQt::setConnectionInfo(const ConnectionInfo &data)
{
	m_connectionInfo = data;
}	

void FileLoaderQt::slotStartLoad()
{
	try{
		if (m_startBuldFile)
		{
            if (m_blocks.size() == m_curCheckSumCounter)
            {
                if (!m_blocks.isCurrect() && m_bLoaded)
                {
                    m_curCheckSum = m_blocks.getCheckToReload();

                    if (m_curCheckSum.isEmpty() || m_checkSums.indexOf(m_curCheckSum) == -1)
                    {
                        toLog(tr("Не знаю что перезагружать, создаю новую сессию"));
                        signFinished();
                        return;
                    }

                    m_Data.m_error = GET_BLOCK;
                    m_Data.m_parametrs = m_curCheckSum;
                    m_Data.m_senderIp = GribUtility::getIpV4();
                    m_Data.genKey();
                    toLog(tr("Запрашиваю блок файла на перезагрузку: ") + QString::number(m_checkSums.indexOf(m_curCheckSum)));
                    m_bLoaded = false;
                    sendData(m_connectionInfo.m_url, RawHeaderData().getMap(), m_Data.toJson());
                    return;
                }
                else  if (m_blocks.isCurrect())
                {
                    toLog(tr("Перезагрузка закончена.."));
                }
                else
                {
                    toLog(tr("В процессе перегрузки.."));
                    return;
                }
                if (m_single)
                {
                    QString name;
                    m_Data.m_name.isEmpty() ? name = m_Data.m_uid_slave : name = m_Data.m_name;
                    setFileFullName(m_connectionInfo.m_pathToGribData + QDir::separator() + name + ".grb");
                }
                else
                    setFileFullName(m_connectionInfo.m_pathToGribData + QDir::separator() + m_Data.m_uid_slave + QDir::separator() + QString::number(m_blocks.getDateTime_t()) + ".grb");
				QDir dir;
                if (m_single)
				    dir.mkpath(m_connectionInfo.m_pathToGribData);
                else
                    dir.mkpath(m_connectionInfo.m_pathToGribData + QDir::separator() + m_Data.m_uid_slave);

				bool bRes = m_blocks.saveFile(m_fileFullName);

				if(bRes)
				{
					toLog(tr("Файл ") + getFileFullName()  +  tr( " сохранен"));
					signFinished();
				}
				else
				{
					toLog(tr("Файл ") + getFileFullName()  +  tr( " НЕ сохранен"));
					QFile file(m_fileFullName);
					file.remove();	 
					signFinished();

				}
				return;
			}
			
			if (m_bLoaded)
			{
				m_bLoaded = false;
				m_curCheckSum = m_checkSums.at(m_curCheckSumCounter);
				toLog(tr("Запрашиваю блок файла: ") + QString::number(m_curCheckSumCounter));
				m_curCheckSumCounter++;
				m_Data.m_error = GET_BLOCK;		
				m_Data.m_parametrs = m_curCheckSum;	
				if(!m_curCheckSum.isEmpty())
				{
					m_Data.m_senderIp = GribUtility::getIpV4();
					m_Data.genKey();
					sendData(m_connectionInfo.m_url, RawHeaderData().getMap(), m_Data.toJson());
				}
				else
				{
				  m_startBuldFile = false;
				  toLog(tr("Чек-сумма пустая (slotStartLoad2)"));
				}
			}
			else
			{
				//чек сумма не совпала, изменяем периодичность опроса
				m_timer->setInterval(10000);
				m_Data.m_senderIp = GribUtility::getIpV4();
				m_Data.genKey();
				sendData(m_connectionInfo.m_url, RawHeaderData().getMap(), m_Data.toJson());
			}
		}
		else
		{
			m_curCheckSumCounter = 0;
			m_timer->setInterval(40000);
			m_Data.m_error = GET_BLOCK_INFO;
			toLog(tr("Запрашиваю информацию о файле"));
			m_Data.m_senderIp = GribUtility::getIpV4();
			m_Data.genKey();
			sendData(m_connectionInfo.m_url, RawHeaderData().getMap(), m_Data.toJson());
		}
	}
	catch(...)
	{
	}
}
