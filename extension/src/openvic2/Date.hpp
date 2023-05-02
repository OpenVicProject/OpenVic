#pragma once

#include <cstdint>
#include <ostream>
#include <string>

namespace OpenVic2 {
	// A relative period between points in time, measured in days
	struct Timespan {
		using day_t = int64_t;

	private:
		day_t days;

	public:
		Timespan(day_t value = 0);

		bool operator<(Timespan other) const;
		bool operator>(Timespan other) const;
		bool operator<=(Timespan other) const;
		bool operator>=(Timespan other) const;
		bool operator==(Timespan other) const;
		bool operator!=(Timespan other) const;

		Timespan operator+(Timespan other) const;
		Timespan operator-(Timespan other) const;
		Timespan operator*(day_t factor) const;
		Timespan operator/(day_t factor) const;
		Timespan& operator+=(Timespan other);
		Timespan& operator-=(Timespan other);
		Timespan& operator++();
		Timespan operator++(int);

		explicit operator day_t() const;
		explicit operator double() const;
		explicit operator std::string() const;
	};
	std::ostream& operator<<(std::ostream& out, Timespan timespan);

	// Represents an in-game date
	// Note: Current implementation does not account for leap-years, or dates before Year 0
	struct Date {
		using year_t = uint16_t;
		using month_t = uint8_t;
		using day_t = uint8_t;

		static constexpr Timespan::day_t MONTHS_IN_YEAR = 12;
		static constexpr Timespan::day_t DAYS_IN_YEAR = 365;
		static constexpr Timespan::day_t DAYS_IN_MONTH[MONTHS_IN_YEAR] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		static constexpr Timespan::day_t DAYS_UP_TO_MONTH[MONTHS_IN_YEAR] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	private:
		// Number of days since Jan 1st, Year 0
		Timespan timespan;

		static Timespan _dateToTimespan(year_t year, month_t month, day_t day);

	public:
		// The Timespan is considered to be the number of days since Jan 1st, Year 0
		Date(Timespan total_days);
		// Year month day specification
		Date(year_t year = 0, month_t month = 1, day_t day = 1);

		year_t getYear() const;
		month_t getMonth() const;
		day_t getDay() const;

		bool operator<(Date other) const;
		bool operator>(Date other) const;
		bool operator<=(Date other) const;
		bool operator>=(Date other) const;
		bool operator==(Date other) const;
		bool operator!=(Date other) const;

		Date operator+(Timespan other) const;
		Timespan operator-(Date other) const;
		Date& operator+=(Timespan other);
		Date& operator-=(Timespan other);
		Date& operator++();
		Date operator++(int);

		explicit operator std::string() const;
		// Parsed from string of the form YYYY.MM.DD
		static Date from_string(std::string const& date);
	};
	std::ostream& operator<<(std::ostream& out, Date date);
}
