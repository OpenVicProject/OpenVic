#pragma once

#include <mutex>

#include <openvic-simulation/types/Signal.hpp>

namespace OpenVic {
	struct ReactiveComponent {
	private:
		memory::vector<scoped_connection> connections;
		std::mutex connections_lock;
		bool is_dirty = true;
		bool is_initialised = false;
		std::mutex is_dirty_lock;
	protected:
		ReactiveComponent() {}
		virtual ~ReactiveComponent() {
			disconnect_all();
		}

		constexpr bool has_no_connections() const {
			return connections.empty();
		}

		void add_connection(connection conn);

		template<typename... Args>
		auto connect_to_mark_dirty() {
			return [this](signal<Args...>& dependency_changed) mutable -> void {
				connect_to_mark_dirty(dependency_changed);
			};
		}

		template<typename... Args>
		auto connect_to_mark_dirty(signal<Args...>& dependency_changed) {
			add_connection(dependency_changed.connect([this](Args...) mutable -> void { mark_dirty(); }));
		}

		auto connect_property_to_mark_dirty() {
			return [this](signal_property<ReactiveComponent>& dependency_changed) mutable -> void {
				add_connection(dependency_changed.connect(&ReactiveComponent::mark_dirty, this));
			};
		}

		template<typename T, typename... Args>
		auto connect_to_mark_dirty(signal_property<T, Args...>& dependency_changed) {
			add_connection(dependency_changed.connect([this](Args...) mutable -> void { mark_dirty(); }));
		}

		void disconnect_all();
		virtual void initialise() {}
		virtual void update() = 0;
	public:
		signal_property<ReactiveComponent> marked_dirty;

		ReactiveComponent(ReactiveComponent&&) = delete;
		ReactiveComponent(ReactiveComponent const&) = delete;
		ReactiveComponent& operator=(ReactiveComponent&&) = delete;
		ReactiveComponent& operator=(ReactiveComponent const&) = delete;
		
		void update_if_dirty();
		void mark_dirty();
	};
}