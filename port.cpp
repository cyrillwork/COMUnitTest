#include "port.h"

#include "port.h"
#include <qdebug.h>

Port::Port(QObject *parent) :
    QObject(parent)
{
}

Port::~Port()
{
    qDebug() << "By in Thread!";

    emit finished_Port();//Сигнал о завершении работы
}

void Port::process_Port(){//Выполняется при старте класса
    qDebug() << "process_Port start";
    connect(&thisPort,SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError))); // подключаем проверку ошибок порта
    connect(&thisPort, SIGNAL(readyRead()),this,SLOT(ReadInPort()));//подключаем   чтение с порта по сигналу readyRead()
    qDebug() << "process_Port end";
}

void Port :: Write_Settings_Port(QString name, int baudrate,int DataBits,
                         int Parity,int StopBits, int FlowControl){//заносим параметры порта в структуру данных
    SettingsPort.name = name;
    SettingsPort.baudRate = (QSerialPort::BaudRate) baudrate;
    SettingsPort.dataBits = (QSerialPort::DataBits) DataBits;
    SettingsPort.parity = (QSerialPort::Parity) Parity;
    SettingsPort.stopBits = (QSerialPort::StopBits) StopBits;
    SettingsPort.flowControl = (QSerialPort::FlowControl) FlowControl;
}

void Port::Write_Settings_Port(QString name)
{
    SettingsPort.name = name;

    SettingsPort.baudRate = QSerialPort::Baud19200;
    SettingsPort.parity = QSerialPort::NoParity;
    SettingsPort.dataBits = QSerialPort::Data8;

}

void Port::ConnectPort(void)
{
    //процедура подключения
    thisPort.setPortName(SettingsPort.name);
    if (thisPort.open(QIODevice::ReadWrite))
    {
        if (thisPort.setBaudRate(SettingsPort.baudRate)
                && thisPort.setDataBits(SettingsPort.dataBits)//DataBits
                && thisPort.setParity(SettingsPort.parity)
                && thisPort.setStopBits(SettingsPort.stopBits)
                && thisPort.setFlowControl(SettingsPort.flowControl))
        {
            if (thisPort.isOpen())
            {
                error_((SettingsPort.name+ " >> Открыт!\r").toLocal8Bit());
                qDebug() << SettingsPort.name+ " >> Открыт!\r";
            }
        }
        else
        {
            thisPort.close();
            error_(thisPort.errorString().toLocal8Bit());
         }
    } else {
        thisPort.close();
        error_(thisPort.errorString().toLocal8Bit());
    }
}
void Port::handleError(QSerialPort::SerialPortError error)//проверка ошибок при работе
{
    if ( (thisPort.isOpen()) && (error == QSerialPort::ResourceError)) {
        error_(thisPort.errorString().toLocal8Bit());
        DisconnectPort();
    }
}//
void  Port::DisconnectPort(){//Отключаем порт
    if(thisPort.isOpen()){
        thisPort.close();
        error_(SettingsPort.name.toLocal8Bit() + " >> Закрыт!\r");
    }
}
void Port :: WriteToPort(QByteArray data){//Запись данных в порт
    if(thisPort.isOpen()){
        thisPort.write(data);
    }
}
//
void Port :: ReadInPort(){//Чтение данных из порта
    QByteArray data;
    data.append(thisPort.readAll());
    outPort(data);

    qDebug() << ((QString)(data.toInt())).toLatin1().toHex();
}
