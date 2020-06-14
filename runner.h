#ifndef RUNNER_H
#define RUNNER_H

#include <QtSerialPort/QSerialPort>
#include <QThread>

#include <thread>
#include <vector>
#include <memory>

#include "record.h"
#include "serialportworker.h"

class Runner: public QObject
{   
    Q_OBJECT
public:    
    explicit Runner(QObject* parent = nullptr);
    ~Runner();

    bool Init(const char* port, const char* name);
    void printInputData();

    void run();

private slots:
    void finishTest();

private:
    std::unique_ptr<QThread> mainThread;
    std::unique_ptr<SerialPortWorker> worker;

    std::string fileName;
    std::vector<Record> inputDataHost;
    std::vector<Record> inputDataEGM;
};

#endif // RUNNER_H
