#include <sstream>
#include "Date.hpp"

using namespace OpenVic2;

bool Timespan::operator<  (Timespan const& other) const { return days <  other.days; }
bool Timespan::operator>  (Timespan const& other) const { return days >  other.days; }
bool Timespan::operator<= (Timespan const& other) const { return days <= other.days; }
bool Timespan::operator>= (Timespan const& other) const { return days >= other.days; }
bool Timespan::operator== (Timespan const& other) const { return days == other.days; }
bool Timespan::operator!= (Timespan const& other) const { return days != other.days; }

Timespan Timespan::operator+ (Timespan const& other) const { return Timespan(days + other.days); }
Timespan Timespan::operator- (Timespan const& other) const { return Timespan(days - other.days); }
Timespan Timespan::operator* (int64_t const& factor) const { return Timespan(days * factor); }
Timespan Timespan::operator/ (int64_t const& factor) const { return Timespan(days / factor); }

Timespan& Timespan::operator+= (Timespan const& other) {
	days += other.days;
	return *this;
}
Timespan& Timespan::operator-= (Timespan const& other) {
	days -= other.days;
	return *this;
}

Timespan::operator std::string() const {
	return std::to_string(days);
}
std::ostream &operator<<(std::ostream &out, Timespan const& timespan) {
	return out << static_cast<std::string>(timespan);
}

Timespan fromYearZero(year_t year, month_t month, date_t day) {
	int64_t daysElapsed = year * DAYS_IN_YEAR;
	size_t daysSinceMonthStart = (day == 0) ? 0 : day - 1; //Underflow protection
	for (size_t x = 0; x < month && x < MONTHS_IN_YEAR; x++) {
		daysElapsed += DAYS_IN_MONTH[x];
	}
	daysElapsed += daysSinceMonthStart;
	return Timespan(daysElapsed);
}

//This function is not set up to handle dates before Year 0
YearMonthDayBundle toGregorianDate(Timespan const& timespan) {
	year_t year = 0;
	month_t month = 0;
	date_t day = 0;

	if (timespan >= 0) {
		year = timespan.days / DAYS_IN_YEAR;
		int64_t remainingDays = timespan.days % DAYS_IN_YEAR;

		for (size_t x = 0; x < MONTHS_IN_YEAR && remainingDays >= DAYS_IN_MONTH[x]; x++) {
			remainingDays -= DAYS_IN_MONTH[x];
			month++;
		}

		//Corrects month and day to be 1-indexed
		month++;
		day++;
	}
	return std::make_tuple(year, month, day);
}


Date::Date(Timespan const& timespan) : ts(timespan) { updateDate(ts); }

Date::Date(year_t year, month_t month, date_t day) {
	ts = fromYearZero(year, month, day);
	updateDate(ts);
}

void Date::updateDate(Timespan const& timespan) {
	gregorianDate = toGregorianDate(timespan);
}

size_t Date::getDay()   const { return std::get<2>(gregorianDate); }
size_t Date::getMonth() const { return std::get<1>(gregorianDate); }
size_t Date::getYear()  const { return std::get<0>(gregorianDate); }

bool Date::operator<  (Date const& other) const { return ts <  other.ts; }
bool Date::operator>  (Date const& other) const { return ts >  other.ts; }
bool Date::operator<= (Date const& other) const { return ts <= other.ts; }
bool Date::operator>= (Date const& other) const { return ts >= other.ts; }
bool Date::operator== (Date const& other) const { return ts == other.ts; }
bool Date::operator!= (Date const& other) const { return ts != other.ts; }

Date Date::operator+ (Timespan timespan) const { return Date(ts + timespan); }
Timespan Date::operator- (Date const& other) const { return ts - other.ts; }

Date& Date::operator+= (Timespan const& timespan) {
	ts += timespan;
	updateDate(ts);
	return *this;
}
Date& Date::operator-= (Timespan const& timespan) {
	ts -= timespan;
	updateDate(ts);
	return *this;
}
Date Date::operator++ (int) {
	Date oldCopy = *this;
	(*this) += 1;
	return oldCopy;
}

Date::operator std::string() const {
	std::stringstream ss;
	ss << getYear() << '.' << getMonth() << '.' << getDay();
	return ss.str();
}
std::ostream &operator<<(std::ostream &out, Date const& date) {
	return out << static_cast<std::string>(date);
}
