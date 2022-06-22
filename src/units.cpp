#include "units.hpp"
#include <sstream>
#include <cstring>
#include <iostream>

namespace unit {

std::string 
Time::format(const char *fmt)
{
    std::stringstream ss;
    for(; *fmt; fmt++) {
        if(*fmt != '%') {
            ss << *fmt;
            continue;
        }
        fmt++;
        switch(*fmt) {
            case '%':
                ss << '%';
                break;
            case 'Y':
                ss << real_years();
                break;
            case 'C':
                ss << years();
                break;
            case 'S': {
                Time year = current_year();
                ss << month_str[year.months()];
                break; }
            case 'M': {
                Time year = current_year();
                ss << year.months();
                break; }
            case 'W': {
                Time month = current_month();
                ss << month.weeks();
                break; }
            case 'D': {
                Time month = current_month();
                ss << month.days();
                break; }
            case 'H': {
                Time day = current_day();
                ss << day.hours();
                break; }
            case 'm': {
                Time hour = current_hour();
                ss << hour.minutes();
                break; }
            case 's': {
                Time minute = current_minute();
                ss << minute.seconds();
                break; }
        }
    }
    return ss.str();
}

}
