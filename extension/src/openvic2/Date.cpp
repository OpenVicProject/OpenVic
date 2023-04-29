#include "Date.hpp"

#include <cctype>
#include <algorithm>

#include "Logger.hpp"

using namespace OpenVic2;

Timespan::Timespan(day_t value) : days{value} {}

bool Timespan::operator<(Timespan other) const { return days < other.days; };
bool Timespan::operator>(Timespan other) const { return days > other.days; };
bool Timespan::operator<=(Timespan other) const { return days <= other.days; };
bool Timespan::operator>=(Timespan other) const { return days >= other.days; };
bool Timespan::operator==(Timespan other) const { return days == other.days; };
bool Timespan::operator!=(Timespan other) const { return days != other.days; };

Timespan Timespan::operator+(Timespan other) const { return days + other.days; }

Timespan Timespan::operator-(Timespan other) const { return days - other.days; }

Timespan Timespan::operator*(day_t factor) const { return days * factor; }

Timespan Timespan::operator/(day_t factor) const { return days / factor; }

Timespan& Timespan::operator+=(Timespan other) {
	days += other.days;
	return *this;
}

Timespan& Timespan::operator-=(Timespan other) {
	days -= other.days;
	return *this;
}

Timespan& Timespan::operator++() {
	days++;
	return *this;
}

Timespan Timespan::operator++(int) {
	Timespan old = *this;
	++(*this);
	return old;
}

Timespan::operator day_t() const {
	return days;
}

Timespan::operator double() const {
	return days;
}

Timespan::operator std::string() const {
	return std::to_string(days);
}

std::ostream& OpenVic2::operator<<(std::ostream& out, Timespan timespan) {
	return out << static_cast<std::string>(timespan);
}

Timespan Date::_dateToTimespan(year_t year, month_t month, day_t day) {
	month = std::clamp<month_t>(month, 1, MONTHS_IN_YEAR);
	day = std::clamp<day_t>(day, 1, DAYS_IN_MONTH[month - 1]);
	return year * DAYS_IN_YEAR + DAYS_UP_TO_MONTH[month - 1] + day - 1;
}

Date::Date(Timespan total_days) : timespan{ total_days } {
	if (timespan < 0) {
		Logger::error("Invalid timespan for date: ", timespan, " (cannot be negative)");
		timespan = 0;
	}
}

Date::Date(year_t year, month_t month, day_t day) : timespan{ _dateToTimespan(year, month, day) } {}

Date::year_t Date::getYear() const {
	return static_cast<Timespan::day_t>(timespan) / DAYS_IN_YEAR;
}

Date::month_t Date::getMonth() const {
	return ((static_cast<Timespan::day_t>(timespan) % DAYS_IN_YEAR) / 32) + 1;
}

Date::day_t Date::getDay() const {
	const Timespan::day_t days_in_year = static_cast<Timespan::day_t>(timespan) % DAYS_IN_YEAR;
	return days_in_year - DAYS_UP_TO_MONTH[days_in_year / 32] + 1;
}


bool Date::operator<(Date other) const { return timespan < other.timespan; };
bool Date::operator>(Date other) const { return timespan > other.timespan; };
bool Date::operator<=(Date other) const { return timespan <= other.timespan; };
bool Date::operator>=(Date other) const { return timespan >= other.timespan; };
bool Date::operator==(Date other) const { return timespan == other.timespan; };
bool Date::operator!=(Date other) const { return timespan != other.timespan; };

Date Date::operator+(Timespan other) const { return timespan + other; }

Timespan Date::operator-(Date other) const { return timespan - other.timespan; }

Date& Date::operator+=(Timespan other) {
	timespan += other;
	return *this;
}

Date& Date::operator-=(Timespan other) {
	timespan -= other;
	return *this;
}

Date& Date::operator++() {
	timespan++;
	return *this;
}

Date Date::operator++(int) {
	Date old = *this;
	++(*this);
	return old;
}

Date::operator std::string() const {
	std::stringstream ss;
	ss << *this;
	return ss.str();
}

std::ostream& OpenVic2::operator<<(std::ostream& out, Date date) {
	return out << (int) date.getYear() << '.' << (int) date.getMonth() << '.' << (int) date.getDay();
}

// Parsed from string of the form YYYY.MM.DD
Date Date::from_string(std::string const& date) {
	year_t year = 0;
	month_t month = 1;
	day_t day = 1;

	size_t first_pos = 0;
	while (first_pos < date.length() && std::isdigit(date[first_pos++]));
	year = atoi(date.substr(0, first_pos).c_str());
	if (first_pos < date.length()) {
		if (date[first_pos] == '.') {
			size_t second_pos = first_pos + 1;
			while (second_pos < date.length() && std::isdigit(date[second_pos++]));
			month = atoi(date.substr(first_pos, second_pos - first_pos).c_str());
			if (second_pos < date.length()) {
				if (date[second_pos] == '.') {
					size_t third_pos = second_pos + 1;
					while (third_pos < date.length() && std::isdigit(date[third_pos++]));
					day = atoi(date.substr(second_pos, third_pos - second_pos).c_str());
					if (third_pos < date.length())
						Logger::error("Unexpected string \"", date.substr(third_pos), "\" at the end of date ", date);
				} else Logger::error("Unexpected character \"", date[second_pos], "\" in date ", date);
			}
		} else Logger::error("Unexpected character \"", date[first_pos], "\" in date ", date);
	}
	return _dateToTimespan(year, month, day);
};
