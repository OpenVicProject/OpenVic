#include "Simulation.hpp"

namespace OpenVic2 {
	void Simulation::togglePauseState() {
		this->isPaused = !isPaused;
	}

	bool Simulation::getPauseState() {
		return this->isPaused;
	}

	void Simulation::increaseSimulationSpeed() {
		switch (this->currentSpeed) {
		case(Speed::Speed1):
			this->currentSpeed = Speed::Speed2;
			break;
		case(Speed::Speed2):
			this->currentSpeed = Speed::Speed3;
			break;
		case(Speed::Speed3):
			this->currentSpeed = Speed::Speed4;
			break;
		case(Speed::Speed4):
			this->currentSpeed = Speed::Speed5;
			break;
		}
	}

	void Simulation::decreaseSimulationSpeed() {
		switch (this->currentSpeed) {
		case(Speed::Speed2):
			this->currentSpeed = Speed::Speed1;
			break;
		case(Speed::Speed3):
			this->currentSpeed = Speed::Speed2;
			break;
		case(Speed::Speed4):
			this->currentSpeed = Speed::Speed3;
			break;
		case(Speed::Speed5):
			this->currentSpeed = Speed::Speed4;
			break;
		}
	}

	void Simulation::setSimulationSpeed(Speed speed) {
			this->currentSpeed = speed;
	}

	int Simulation::getSimulationSpeed() {
		return static_cast<int>(this->currentSpeed);
	}

	void Simulation::conditionallyAdvanceSimulation() {
		if (!(this->isPaused)) {
			std::chrono::time_point<std::chrono::high_resolution_clock> previousTime = this->lastPolledTime;
			std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
			if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - previousTime).count() >= this->getSimulationSpeed()) {
				this->lastPolledTime = currentTime;
				this->inGameDate++;
			}
		}
	}
}
