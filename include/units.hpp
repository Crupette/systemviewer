#ifndef UNIT_HPP
#define UNIT_HPP 1

#include "util.hpp"
#include <string>

namespace unit {

constexpr long MINUTE_SECONDS = 60;
constexpr long HOUR_SECONDS = 3600;
constexpr long DAY_SECONDS = 86400;
constexpr long WEEK_SECONDS = 604800;
constexpr long YEAR_SECONDS =  31556952;
constexpr long CYEAR_SECONDS = 31536000;

class Mass {
    double m_kg;
public:
    explicit constexpr Mass(double kg) : m_kg(kg) {}
    constexpr ~Mass() = default;

    constexpr double operator()() { return m_kg; }

    constexpr Mass &operator+=(const Mass &rhs) { this->m_kg += rhs.m_kg; return *this; }
    constexpr Mass &operator-=(const Mass &rhs) { this->m_kg -= rhs.m_kg; return *this; }
    constexpr Mass &operator/=(const Mass &rhs) { this->m_kg *= rhs.m_kg; return *this; }
    constexpr Mass &operator*=(const Mass &rhs) { this->m_kg /= rhs.m_kg; return *this; }

    friend Mass operator+(Mass lhs, const Mass &rhs) { lhs += rhs; return lhs; }
    friend Mass operator-(Mass lhs, const Mass &rhs) { lhs -= rhs; return lhs; }
    friend Mass operator*(Mass lhs, const Mass &rhs) { lhs *= rhs; return lhs; }
    friend Mass operator/(Mass lhs, const Mass &rhs) { lhs /= rhs; return lhs; }

    friend inline bool operator<=>(const Mass &lhs, const Mass &rhs) = default;

    constexpr Mass &operator+=(const double &rhs) { this->m_kg += rhs; return *this; }
    constexpr Mass &operator-=(const double &rhs) { this->m_kg -= rhs; return *this; }
    constexpr Mass &operator*=(const double &rhs) { this->m_kg *= rhs; return *this; }
    constexpr Mass &operator/=(const double &rhs) { this->m_kg /= rhs; return *this; }

    friend Mass operator+(Mass lhs, const double &rhs) { lhs += rhs; return lhs; }
    friend Mass operator-(Mass lhs, const double &rhs) { lhs -= rhs; return lhs; }
    friend Mass operator*(Mass lhs, const double &rhs) { lhs *= rhs; return lhs; }
    friend Mass operator/(Mass lhs, const double &rhs) { lhs /= rhs; return lhs; }
};

class Time {
    long m_seconds;
public:
    static constexpr unsigned char month_days[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    static constexpr unsigned char month_days_leap[12] = {
        31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    static const inline std::string month_str[12] = {
        "January", "Febuary", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
    };

    explicit constexpr Time(long seconds) : m_seconds(seconds) {}

    constexpr long operator()() { return m_seconds; }

    constexpr Time &operator+=(const Time &rhs) { this->m_seconds += rhs.m_seconds; return *this; }
    constexpr Time &operator-=(const Time &rhs) { this->m_seconds -= rhs.m_seconds; return *this; }
    constexpr Time &operator/=(const Time &rhs) { this->m_seconds *= rhs.m_seconds; return *this; }
    constexpr Time &operator*=(const Time &rhs) { this->m_seconds /= rhs.m_seconds; return *this; }

    friend Time operator+(Time lhs, const Time &rhs) { lhs += rhs; return lhs; }
    friend Time operator-(Time lhs, const Time &rhs) { lhs -= rhs; return lhs; }
    friend Time operator*(Time lhs, const Time &rhs) { lhs *= rhs; return lhs; }
    friend Time operator/(Time lhs, const Time &rhs) { lhs /= rhs; return lhs; }

    friend inline bool operator<=>(const Time &lhs, const Time &rhs) = default;

    constexpr Time &operator+=(const long &rhs) { this->m_seconds += rhs; return *this; }
    constexpr Time &operator-=(const long &rhs) { this->m_seconds -= rhs; return *this; }
    constexpr Time &operator*=(const long &rhs) { this->m_seconds *= rhs; return *this; }
    constexpr Time &operator/=(const long &rhs) { this->m_seconds /= rhs; return *this; }

    friend Time operator+(Time lhs, const long &rhs) { lhs += rhs; return lhs; }
    friend Time operator-(Time lhs, const long &rhs) { lhs -= rhs; return lhs; }
    friend Time operator*(Time lhs, const long &rhs) { lhs *= rhs; return lhs; }
    friend Time operator/(Time lhs, const long &rhs) { lhs /= rhs; return lhs; }

    constexpr bool leap_year() { return (m_seconds % YEAR_SECONDS) % 4 == 0; }

    constexpr Time current_year() {
        return Time((m_seconds % YEAR_SECONDS) + (m_seconds < 0 ? YEAR_SECONDS : 0)); 
    }
    constexpr Time current_month() { 
        Time year = current_year();
        if(year.m_seconds < 0) {
            year.m_seconds = YEAR_SECONDS + year.m_seconds;
        }
        unsigned month = 0;
        const unsigned char *month_days_view = month_days;
        if(years() % 4 == 0 && years() % 100 == 0 && years() % 400 != 0) month_days_view = month_days_leap;
        for(month; year.days() > month_days_view[month]; month = (month + 1) % 12) year -= month_days_view[month] * DAY_SECONDS;
        return year;
    }
    constexpr Time current_week() { return Time(m_seconds % WEEK_SECONDS); }
    constexpr Time current_day() { return Time(m_seconds % DAY_SECONDS); }
    constexpr Time current_hour() { return Time(m_seconds % HOUR_SECONDS); }
    constexpr Time current_minute() { return Time(m_seconds % MINUTE_SECONDS); }

    constexpr long seconds() { return m_seconds; }
    constexpr long minutes() { return m_seconds / MINUTE_SECONDS; }
    constexpr long hours() { return m_seconds / HOUR_SECONDS; }
    constexpr long days() { return m_seconds / DAY_SECONDS; }
    constexpr long weeks() { return m_seconds / WEEK_SECONDS; }
    constexpr long months() { 
        Time copy(*this);
        if(copy.m_seconds < 0) {
            copy.m_seconds = -copy.m_seconds;
        }
        unsigned month = 0;
        const unsigned char *month_days_view = month_days;
        if(years() % 4 == 0) month_days_view = month_days_leap;
        for(month; copy.days() > month_days_view[month]; month = (month + 1) % 12) copy -= month_days_view[month] * DAY_SECONDS;
        return month;
    }
    constexpr long years() { return ((m_seconds < 0 ? m_seconds - CYEAR_SECONDS : m_seconds) / CYEAR_SECONDS) + 2000; }
    constexpr long real_years() {  return (m_seconds < 0 ? m_seconds - YEAR_SECONDS : m_seconds) / YEAR_SECONDS; }

    /*  %% = '%'
     *  %Y = real year
     *  %C = calendar year
     *  %S = month string
     *  %M = month
     *  %W = week
     *  %D = day
     *  %H = hour
     *  %m = minute
     *  %s = second
     * */
    std::string format(const char *fmt);
};

constexpr long Mm = 1000;
constexpr long Gm = cxpow_v<long, 10, 6>;
constexpr long Tm = cxpow_v<long, 10, 9>;
constexpr long Pm = cxpow_v<long, 10, 12>;
constexpr long Em = cxpow_v<long, 10, 15>;
constexpr long Zm = cxpow_v<long, 10, 18>;

constexpr long AU = 149597871;

constexpr Mass kg{1};
constexpr Mass Mg{cxpow_v<double, 10, 3>};
constexpr Mass Gg{cxpow_v<double, 10, 6>};
constexpr Mass Tg{cxpow_v<double, 10, 9>};
constexpr Mass Pg{cxpow_v<double, 10, 12>};
constexpr Mass Eg{cxpow_v<double, 10, 15>};
constexpr Mass Zg{cxpow_v<double, 10, 18>};
constexpr Mass Yg{cxpow_v<double, 10, 21>};

constexpr Mass lunarMass{7.342 * cxpow_v<double, 10, 22>};
constexpr Mass earthMass{5.97237 * cxpow_v<double, 10, 24>};
constexpr Mass jovMass{1.89813 * cxpow_v<double, 10, 27>};
constexpr Mass solMass{1.98847 * cxpow_v<double, 10, 30>};

constexpr double earthRad = 6371;
}

#endif
