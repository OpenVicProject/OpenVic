#include "openvic2/GameAdvancementHook.hpp"

using namespace OpenVic2;

const std::vector<std::chrono::milliseconds> GameAdvancementHook::GAME_SPEEDS = {
	std::chrono::milliseconds{ 4000 },
	std::chrono::milliseconds{ 3000 },
	std::chrono::milliseconds{ 2000 },
	std::chrono::milliseconds{ 1000 },
	std::chrono::milliseconds{ 100 },
	std::chrono::milliseconds{ 1 } };

GameAdvancementHook::GameAdvancementHook(AdvancementFunction tickFunction, RefreshFunction updateFunction, bool startPaused, speed_t startingSpeed)
	: triggerFunction{ tickFunction }, refreshFunction{ updateFunction }, isPaused{ startPaused } {
	lastPolledTime = std::chrono::high_resolution_clock::now();
	setSimulationSpeed(startingSpeed);
}

void GameAdvancementHook::setSimulationSpeed(speed_t speed) {
	if (speed < 0)
		currentSpeed = 0;
	else if (speed >= GAME_SPEEDS.size())
		currentSpeed = GAME_SPEEDS.size() - 1;
	else
		currentSpeed = speed;
}

GameAdvancementHook::speed_t GameAdvancementHook::getSimulationSpeed() const {
	return currentSpeed;
}

void GameAdvancementHook::increaseSimulationSpeed() {
	setSimulationSpeed(currentSpeed + 1);
}

void GameAdvancementHook::decreaseSimulationSpeed() {
	setSimulationSpeed(currentSpeed - 1);
}

bool GameAdvancementHook::canIncreaseSimulationSpeed() const {
	return currentSpeed + 1 < GAME_SPEEDS.size();
}

bool GameAdvancementHook::canDecreaseSimulationSpeed() const {
	return currentSpeed > 0;
}

GameAdvancementHook& GameAdvancementHook::operator++() {
	increaseSimulationSpeed();
	return *this;
};

GameAdvancementHook& GameAdvancementHook::operator--() {
	decreaseSimulationSpeed();
	return *this;
};

void GameAdvancementHook::conditionallyAdvanceGame() {
	if (!isPaused) {
		std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastPolledTime) >= GAME_SPEEDS[currentSpeed]) {
			lastPolledTime = currentTime;
			if (triggerFunction) triggerFunction();
		}
	}
	if (refreshFunction) refreshFunction();
}
