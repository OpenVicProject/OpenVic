#pragma once

#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>
#include <openvic-simulation/types/Signal.hpp>

namespace OpenVic {
	struct CountryInstance;

	struct BudgetComponent {
	private:
		fixed_point_t balance = 0;

	protected:
		void set_balance(const fixed_point_t balance);

	public:
		signal_property<BudgetComponent> balance_changed;
		fixed_point_t get_balance() const;
		virtual void full_update(CountryInstance const& country) = 0;
	};
}