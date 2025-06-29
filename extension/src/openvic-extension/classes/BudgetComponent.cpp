#include "BudgetComponent.hpp"

#include "openvic-simulation/country/CountryInstance.hpp"
#include "openvic-simulation/types/fixed_point/FixedPoint.hpp"

using namespace OpenVic;

void BudgetComponent::set_country(CountryInstance& new_country) {
	country_ptr = &new_country;
	update();
}

fixed_point_t BudgetComponent::get_balance() const {
	return balance;
}

void BudgetComponent::set_balance(const fixed_point_t new_balance) {
	if (balance == new_balance) {;
		return;
	}

	balance = new_balance;
	balance_changed();
}