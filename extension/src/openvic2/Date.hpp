#pragma once

#include <cstdint>
#include <cstddef>
#include <tuple>
#include <string>
#include <iostream>

namespace OpenVic2 {
	//A relative period between points in time, measured in days
	struct Timespan {
		int64_t days;

		Timespan() : days(0) {}
		Timespan(int64_t const& value) : days(value) {}

		bool operator<  (Timespan const& other) const;
		bool operator>  (Timespan const& other) const;
		bool operator<= (Timespan const& other) const;
		bool operator>= (Timespan const& other) const;
		bool operator== (Timespan const& other) const;
		bool operator!= (Timespan const& other) const;

		Timespan operator+ (Timespan const& other) const;
		Timespan operator- (Timespan const& other) const;
		Timespan operator* (int64_t const& factor) const;
		Timespan operator/ (int64_t const& factor) const;

		Timespan& operator+= (Timespan const& other);
		Timespan& operator-= (Timespan const& other);
	};

	static constexpr size_t MONTHS_IN_YEAR = 12;
	static constexpr size_t DAYS_IN_YEAR = 365;
	static constexpr size_t DAYS_IN_MONTH[MONTHS_IN_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	using date_t = uint8_t;
	using month_t = uint8_t;
	using year_t = uint16_t;
	using YearMonthDayBundle = std::tuple<year_t, month_t, date_t>;

	//Represents an in-game date
	//Note: Current implementation does not account for leap-years, or dates before Year 0
	struct Date {
		private:
		YearMonthDayBundle gregorianDate;
		//Number of days since Jan 1st, Year 0
		Timespan ts;

		public:
		//The Timespan is considered to be the number of days since Jan 1st, Year 0
		Date(Timespan const& timespan);

		//Year month day specification
		Date(year_t year = 1836, month_t month = 1, date_t day = 1);

		private:
		void updateDate(Timespan const& timespan);

		public:
		size_t getDay() const;
		size_t getMonth() const;
		size_t getYear() const;

		bool operator<  (Date const& other) const;
		bool operator>  (Date const& other) const;
		bool operator<= (Date const& other) const;
		bool operator>= (Date const& other) const;
		bool operator== (Date const& other) const;
		bool operator!= (Date const& other) const;

		Date operator+ (Timespan timespan) const;
		Timespan operator- (Date const& other) const;

		Date& operator+= (Timespan const& timespan);
		Date& operator-= (Timespan const& timespan);
		//Postfix increment
		Date operator++ (int);

		explicit operator std::string() const;
		friend std::ostream& operator<< (std::ostream& out, Date const& date);
	};
}
