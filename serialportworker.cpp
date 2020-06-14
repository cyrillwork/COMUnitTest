/****************************************************************************
**
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "serialportworker.h"

#include <QCoreApplication>
#include <QDebug>

#include <iostream>

QT_USE_NAMESPACE

static high_resolution_clock::time_point to_time_point(timeval tv)
{
    return high_resolution_clock::time_point{seconds{tv.tv_sec} + microseconds{tv.tv_usec}};
}


SerialPortWorker::SerialPortWorker(const char* namePort, std::vector<Record>& host, std::vector<Record>& egm, QObject *parent)
    : QObject(parent)
    , isLastEmpty{false}
    //, m_serialPort(namePort)
//    , m_standardOutput(stdout)
{
    m_serialPort.setPortName(namePort);
    m_serialPort.setBaudRate(QSerialPort::Baud19200);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setDataBits(QSerialPort::Data8);
    isPortOpen = m_serialPort.open(QIODevice::ReadWrite);

    //std::copy(in.begin(), in.end(), std::back_inserter(inputDataHost));
    for(const auto &i: host) {
        inputDataHost.emplace(i);
    }

    for(const auto &i: egm) {
        inputDataEGM.emplace(i);
    }

    connect(&m_serialPort, SIGNAL(readyRead()), SLOT(handleReadyRead()));
    connect(&m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleError(QSerialPort::SerialPortError)));
    connect(&read_timer,   SIGNAL(timeout()), SLOT(handleTimeout()));
    connect(&write_timer,  SIGNAL(timeout()), SLOT(writeTimeout()));

    connect(this, SIGNAL(stopReadTimer()),      &read_timer, SLOT(stop()) );
    connect(this, SIGNAL(startReadTimer(int)),  &read_timer, SLOT(start(int)) );

    connect(this, SIGNAL(stopWriteTimer()),      &write_timer, SLOT(stop()) );
    connect(this, SIGNAL(startWriteTimer(int)),  &write_timer, SLOT(start(int)) );

    read_timer.start(15);
    write_timer.start(100);
}

SerialPortWorker::~SerialPortWorker()
{
}

QSerialPort* SerialPortWorker::getSerialPort()
{
    return &m_serialPort;
}

void SerialPortWorker::handleReadyRead()
{
    if( (m_commandData.size() == 0) && (m_readData.size() == 0))
    {
        timeFirstByte = QDateTime::currentDateTime();
    }

    m_readData.append(m_serialPort.readAll());

    if (!read_timer.isActive())
    {
        read_timer.start(5000);
    }
}

void SerialPortWorker::handleTimeout()
{
    if (m_readData.isEmpty())
    {
        if(m_commandData.size() > 0) {
            if(!isLastEmpty) {
                isLastEmpty = true;
                return;
            } else {
                isLastEmpty = false;
                const auto &qstr = this->timeFirstByte.toString("yyyy-MM-dd\thh:mm:ss.zzz000");

                dataGet.emplace_back(qstr.toStdString(), m_commandData.data(), m_commandData.size());
                m_commandData.clear();
            }
        }
    } else {
        m_commandData += m_readData;
        m_readData.clear();
    }
}

void SerialPortWorker::writeTimeout()
{

    //std::cout << "writeTimeout" << std::endl;

    if(!inputDataHost.empty()) {
        auto &record = inputDataHost.front();
        auto& vec = record.getBytes();

        for(const auto& i: vec){
            std::cout << std::hex << (int)i << " ";
        }
        std::cout << std::endl;

        m_serialPort.write((const char*)vec.data(), vec.size());
        inputDataHost.pop();

        if(!inputDataHost.empty()) {
            auto &next = inputDataHost.front();

            auto t1 = to_time_point(record.getTimePoint());
            auto t2 = to_time_point(next.getTimePoint());

            emit startWriteTimer( (int)duration_cast<milliseconds>(t2 - t1).count() );

        } else {
            //std::cout << "write last element" << std::endl;
            emit startWriteTimer(3000);
        }

    } else {
        //std::cout << "write queue empty" << std::endl;
        emit startWriteTimer(3000);
        emit finishTest();
        std::cout << "emit SerialPortWorker::finishTest()" << std::endl;
    }
}

void SerialPortWorker::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError) {
        //qDebug() << QObject::tr("An I/O error occurred while reading the data from port %1, error: %2").arg(m_serialPort->portName()).arg(m_serialPort->errorString()) << endl;
        QCoreApplication::exit(1);
    }
}

bool SerialPortWorker::getPortState() const
{
    return isPortOpen;
}

std::vector<Record>& SerialPortWorker::getData()
{
    return dataGet;
}
