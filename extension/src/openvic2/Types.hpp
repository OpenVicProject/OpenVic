#pragma once

#include <string>
#include <vector>

#include "openvic2/Logger.hpp"

namespace OpenVic2 {
	using return_t = bool;
	// This mirrors godot::Error, where `OK = 0` and `FAILED = 1`.
	static constexpr return_t SUCCESS = false, FAILURE = true;

	/*
	 * Base class for objects with a non-empty string identifier,
	 * uniquely named instances of which can be entered into an
	 * IdentifierRegistry instance.
	 */
	class HasIdentifier {
		const std::string identifier;
	protected:
		HasIdentifier(std::string const& new_identifier);
	public:
		HasIdentifier(HasIdentifier const&) = delete;
		HasIdentifier(HasIdentifier&&) = default;
		HasIdentifier& operator=(HasIdentifier const&) = delete;
		HasIdentifier& operator=(HasIdentifier&&) = delete;

		std::string const& get_identifier() const;
	};

	/*
	 * Template for a list of objects with unique string identifiers that can
	 * be locked to prevent any further additions. The template argument T is
	 * the type of object that the registry will store, and the second part ensures
	 * that HasIdentifier is a base class of T.
	 */
	template<class T, typename std::enable_if<std::is_base_of<HasIdentifier, T>::value>::type* = nullptr>
	class IdentifierRegistry {
		const std::string name;
		std::vector<T> items;
		bool locked = false;
	public:
		IdentifierRegistry(std::string const& new_name) : name(new_name) {}
		return_t add_item(T&& item) {
			if (locked) {
				Logger::error("Cannot add item to the ", name, " registry - locked!");
				return FAILURE;
			}
			T const* old_item = get_item_by_identifier(item.get_identifier());
			if (old_item != nullptr) {
				Logger::error("Cannot add item to the ", name, " registry - an item with the identifier \"", item.get_identifier(), "\" already exists!");
				return FAILURE;
			}
			items.push_back(std::move(item));
			return SUCCESS;
		}
		void lock(bool log = true) {
			if (locked) {
				Logger::error("Failed to lock ", name, " registry - already locked!");
			} else {
				locked = true;
				if (log) Logger::info("Locked ", name, " registry after registering ", get_item_count(), " items");
			}
		}
		bool is_locked() const {
			return locked;
		}
		void reset() {
			items.clear();
			locked = false;
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
