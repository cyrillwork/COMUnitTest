#include "record.h"
#include "extern/stringtokenizer.h"

void getStringTime(char *time_full, const timeval& tv)
{
    char time_string[64];
    struct tm* ptm;

    //gettimeofday(&tv, NULL);
    ptm = localtime(&tv.tv_sec);
    strftime(time_string, sizeof(time_string), "%F\t%H:%M:%S", ptm);

    long microseconds = tv.tv_usec;
    sprintf(time_full, "%s.%06ld", time_string, microseconds);
}


TypeRecord getTypeRecord(const std::string& s) {
    if(s == "EGM ") {
        return TypeRecord::EGM;
    } else if(s == "HOST") {
        return TypeRecord::HOST;
    } else {
        return TypeRecord::NONE;
    }
}



Record::Record(const std::string _data):
    data{_data}
{
    StringTokenizer token(data, "\t\n");

    timeval tv = {};
    tm curr_point = {};

    if(token.hasMoreTokens()) {
        date = token.nextToken();
        sscanf(date.c_str(), "%d-%d-%d", &curr_point.tm_year, &curr_point.tm_mon, &curr_point.tm_mday);
    }

    if(token.hasMoreTokens()) {
        time = token.nextToken();
        sscanf(time.c_str(), "%d:%d:%d.%ld", &curr_point.tm_hour, &curr_point.tm_min, &curr_point.tm_sec, &tv.tv_usec);
    }
    curr_point.tm_year -= 1900;
    --curr_point.tm_mon;
    tv.tv_sec = mktime(&curr_point);

    timePoint = tv;

    if(token.hasMoreTokens()) {
        type = getTypeRecord(token.nextToken());
    }

    if(token.hasMoreTokens()) {
        duration = std::chrono::milliseconds( std::stoi(token.nextToken()) );
    }

    if(token.hasMoreTokens()) {
        StringTokenizer tokenBytes(token.nextToken());
        while(tokenBytes.hasMoreTokens()) {
            auto str1 = tokenBytes.nextToken();
            if(str1.size() > 0 && (str1 != " ")) {
                bytes.push_back((unsigned char)std::stoul(str1.c_str(), nullptr, 16));
            }
        }
    }
}

Record::Record(const timeval& _time, const char* bytes, int size, TypeRecord _type)
{
    std::string str_time;
    char time_full[64];
    time_full[0] = 0;

    getStringTime(time_full, _time);
    str_time = time_full;

    Record(str_time, bytes, size, _type);
}

Record::Record(const std::string& _time, const char* bytes, int size, TypeRecord _type)
{
    data += _time;

    if(_type == TypeRecord::HOST) {
        data += "HOST";
    } else {
        data += "EGM ";
    }

    for(int i = 0; i < size; ++i)
    {
        char num1[8];
        num1[0] = 0;
        sprintf(num1, "%02X ", bytes[i]);
        data += num1;
    }

}

TypeRecord Record::getType() const
{
    return type;
}

std::vector<unsigned char>& Record::getBytes()
{
    return bytes;
}

timeval Record::getTimePoint() const
{
    return timePoint;
}

std::string& Record::getData()
{
    return data;
}

std::ostream& operator<<(std::ostream& s, const Record& r) {
    //auto ttt = system_clock::to_time_t(r.timePoint);
    //auto ttt = high_resolution_clock::now() - r.timePoint;

    s << std::dec << r.timePoint.tv_sec<<"."<< r.timePoint.tv_usec << " " << r.type << " " << std::dec << r.duration.count() << " ";

    s << std::hex;
    for(const auto iii: r.bytes) {
        s << (int)iii << " ";
    }

    return s;
}

std::ostream&operator<<(std::ostream& s, const TypeRecord& r) {
    if(r == TypeRecord::EGM) {
        s << "EGM ";
    } else if(r == TypeRecord::HOST) {
        s << "HOST";
    } else {
        s << "NONE";
    }
    return s;
}
