#include "Good.hpp"

using namespace OpenVic2;;

Good::Good() = default;

Good::Good(const godot::String& identifier, const godot::String& category, float_t cost, const godot::String& colour,
		   bool isAvailable, bool isTradable, bool isMoney, bool hasOverseasPenalty) {
	this->identifier = identifier;
	this->category = category;
	this->cost = cost;
	this->colour = colour;
	this->isAvailableAtStart = isAvailable;
	this->isTradable = isTradable;
	this->isMoney = isMoney;
	this->hasOverseasPenalty = hasOverseasPenalty;
}

Good::~Good() = default;