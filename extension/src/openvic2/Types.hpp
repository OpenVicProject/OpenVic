#pragma once

#include <string>
#include <vector>

#include "openvic2/Logger.hpp"

namespace OpenVic2 {
	using return_t = bool;
	// This mirrors godot::Error, where `OK = 0` and `FAILED = 1`.
	static constexpr return_t SUCCESS = false, FAILURE = true;

	class HasIdentifier {
		std::string identifier;
	protected:
		HasIdentifier(std::string const& new_identifier);
	public:
		std::string const& get_identifier() const;
	};

	template<typename T, char const* name, std::enable_if<std::is_base_of<HasIdentifier, T>::value>::type* = nullptr>
	class IdentifierRegistry {
		std::vector<T> items;
		bool locked = false;
	public:
		return_t add_item(T&& item) {
			if (locked) {
				Logger::error("Cannot add item to the ", name, " registry - locked!");
				return FAILURE;
			}
			if (item.get_identifier().empty()) {
				Logger::error("Cannot add item to the ", name, " registry - empty identifier!");
				return FAILURE;
			}
			T const* old_item = get_item_by_identifier(item.get_identifier());
			if (old_item != nullptr) {
				Logger::error("Cannot add item to the ", name, " registry - an item with the identifier \"", item.get_identifier(), "\" already exists!");
				return FAILURE;
			}
			items.push_back(item);
			return SUCCESS;
		}
		void lock() {
			if (locked) {
				Logger::error("Failed to lock ", name, " registry - already locked!");
			} else {
				locked = true;
				Logger::info("Locked ", name, " registry after registering ", get_item_count(), " items");
			}
		}
		bool is_locked() const {
			return locked;
		}
		size_t get_item_count() const {
			return items.size();
		}
		T* get_item_by_identifier(std::string const& identifier) {
			if (!identifier.empty())
				for (T& item : items)
					if (item.get_identifier() == identifier) return &item;
			return nullptr;
		}
		T const* get_item_by_identifier(std::string const& identifier) const {
			if (!identifier.empty())
				for (T const& item : items)
					if (item.get_identifier() == identifier) return &item;
			return nullptr;
		}
		T* get_item_by_index(size_t index) {
			return index < items.size() ? &items[index] : nullptr;
		}
		T const* get_item_by_index(size_t index) const {
			return index < items.size() ? &items[index] : nullptr;
		}
		std::vector<T>& get_items() {
			return items;
		}
		std::vector<T> const& get_items() const {
			return items;
		}
	};
}
