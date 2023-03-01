#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <vector>
#include <chrono>
#include "openvic2/Date.hpp"

namespace OpenVic2 {
	class Simulation : public godot::Object {
		GDCLASS(Simulation, godot::Object)
		std::vector<uint64_t> exampleProvinces;

		enum class Speed { Speed1 = 4000, Speed2 = 3000, Speed3 = 2000, Speed4 = 1000, Speed5 = 100, Speed6 = 1 };

		std::chrono::time_point<std::chrono::high_resolution_clock> lastPolledTime;
		bool isPaused;
		Speed currentSpeed;
		Date inGameDate;

		//BEGIN BOILERPLATE
		inline static Simulation* _simulation = nullptr;

	protected:
		static void _bind_methods() {
			godot::ClassDB::bind_method(godot::D_METHOD("conductSimulationStep"), &Simulation::conductSimulationStep);
			godot::ClassDB::bind_method(godot::D_METHOD("queryProvinceSize"), &Simulation::queryProvinceSize);
		}

	public:
		inline static Simulation* get_singleton() { return _simulation; }

		inline Simulation() : inGameDate(1836, 1, 1) {
			ERR_FAIL_COND(_simulation != nullptr);
			_simulation = this;
			this->lastPolledTime = std::chrono::high_resolution_clock::now();
			this->isPaused = false;
			this->currentSpeed = Speed::Speed1;
			exampleProvinces.resize(10, 1);
		}
		inline ~Simulation() {
			ERR_FAIL_COND(_simulation != this);
			_simulation = nullptr;
		}
		//END BOILERPLATE

		void togglePauseState();
		bool getPauseState();
		void increaseSimulationSpeed();
		void decreaseSimulationSpeed();
		void setSimulationSpeed(Speed speed);
		int getSimulationSpeed();

		inline void conductSimulationStep() {
			for (uint64_t x = 0; x < exampleProvinces.size(); x++) {
				exampleProvinces[x] += (x + 1);
			}
		}

		inline uint64_t queryProvinceSize(uint64_t provinceID) {
			if (provinceID >= exampleProvinces.size()) {
				return 0;
			}
			return exampleProvinces[provinceID];
		}

		void conditionallyAdvanceSimulation();
	};
}