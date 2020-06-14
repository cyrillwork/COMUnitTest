#ifndef RECORD_H
#define RECORD_H

#include <iostream>
#include <string>
#include <stdint.h>
#include <chrono>
#include <vector>
#include <time.h>

enum class TypeRecord: uint8_t {
    NONE = 0,
    HOST = 1,
    EGM  = 2
};

std::ostream& operator<< (std::ostream& s, const TypeRecord& r);


using namespace std::chrono;

class Record
{
public:
    Record(const std::string _data);

    Record(const timeval& _time, const char*bytes, int size, TypeRecord _type = TypeRecord::EGM);

    Record(const std::string &_time, const char*bytes, int size, TypeRecord _type = TypeRecord::EGM);

    friend std::ostream& operator<< (std::ostream& s, const Record& r);

    TypeRecord getType() const;

    std::vector<unsigned char>& getBytes();

    timeval getTimePoint() const;

    std::string& getData();

private:

    void stringToData();

    std::string data;
    TypeRecord typeRecord;

    std::string date;
    std::string time;

    timeval timePoint;
    TypeRecord type;
    milliseconds duration;
    std::vector<unsigned char> bytes;
};


#endif // RECORD_H
