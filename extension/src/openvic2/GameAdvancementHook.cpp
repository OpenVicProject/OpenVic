#include "GameAdvancementHook.hpp"

namespace OpenVic2 {
	GameAdvancementHook::GameAdvancementHook(AdvancementFunction function, bool startPaused, GameSpeed startingSpeed) {
		triggerFunction = function;
		lastPolledTime = std::chrono::high_resolution_clock::now();
		isPaused = startPaused;
		currentSpeed = startingSpeed;
	}

	void GameAdvancementHook::increaseSimulationSpeed() {
		switch (currentSpeed) {
		case(GameSpeed::Speed1):
			currentSpeed = GameSpeed::Speed2;
			break;
		case(GameSpeed::Speed2):
			currentSpeed = GameSpeed::Speed3;
			break;
		case(GameSpeed::Speed3):
			currentSpeed = GameSpeed::Speed4;
			break;
		case(GameSpeed::Speed4):
			currentSpeed = GameSpeed::Speed5;
			break;
		}
	}

	void GameAdvancementHook::decreaseSimulationSpeed() {
		switch (currentSpeed) {
		case(GameSpeed::Speed2):
			currentSpeed = GameSpeed::Speed1;
			break;
		case(GameSpeed::Speed3):
			currentSpeed = GameSpeed::Speed2;
			break;
		case(GameSpeed::Speed4):
			currentSpeed = GameSpeed::Speed3;
			break;
		case(GameSpeed::Speed5):
			currentSpeed = GameSpeed::Speed4;
			break;
		}
	}

	GameAdvancementHook GameAdvancementHook::operator++(int) {
		GameAdvancementHook oldCopy = *this;
		increaseSimulationSpeed();
		return oldCopy;
	};

	GameAdvancementHook GameAdvancementHook::operator--(int) {
		GameAdvancementHook oldCopy = *this;
		decreaseSimulationSpeed();
		return oldCopy;
	};

	void GameAdvancementHook::conditionallyAdvanceGame() {
		if (!isPaused) {
			std::chrono::time_point<std::chrono::high_resolution_clock> previousTime = lastPolledTime;
			std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
			if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - previousTime).count() >= static_cast<int64_t>(currentSpeed)) {
				lastPolledTime = currentTime;
				if (triggerFunction) {
					triggerFunction();
				}
			}
		}
	}
}