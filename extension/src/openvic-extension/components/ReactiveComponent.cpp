#include "ReactiveComponent.hpp"

using namespace OpenVic;

void ReactiveComponent::add_connection(connection conn) {
	const std::lock_guard<std::mutex> lock_guard { connections_lock };
	connections.emplace_back(std::move(conn));
}

void ReactiveComponent::disconnect_all() {
	const std::lock_guard<std::mutex> lock_guard { connections_lock };
	connections.clear();
}

void ReactiveComponent::update_if_dirty() {
	if (!is_initialised) {
		initialise();
		is_initialised = true;
	}

	if (!is_dirty) {
		return;
	}

	const std::lock_guard<std::mutex> lock_guard { is_dirty_lock };
	if (!is_dirty) {
		return;
	}

	update();

	is_dirty = false;
}

void ReactiveComponent::mark_dirty() {
	if (is_dirty) {
		return;
	}

	const std::lock_guard<std::mutex> lock_guard { is_dirty_lock };
	if (is_dirty) {
		return;
	}

	disconnect_all();
	is_dirty = true;
	marked_dirty();
}