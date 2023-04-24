#pragma once

#include <chrono>
#include <functional>
#include <vector>

namespace OpenVic2 {
	// Conditionally advances game with provided behaviour
	// Class governs game speed and pause state
	class GameAdvancementHook {
	public:
		using AdvancementFunction = std::function<void()>;
		using RefreshFunction = std::function<void()>;
		using speed_t = int8_t;

		// Minimum number of miliseconds before the simulation advances
		static const std::vector<std::chrono::milliseconds> GAME_SPEEDS;

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> lastPolledTime;
		// A function pointer that advances the simulation, intended to be a capturing lambda or something similar. May need to be reworked later
		AdvancementFunction triggerFunction;
		RefreshFunction refreshFunction;
		speed_t currentSpeed;

	public:
		bool isPaused;

		GameAdvancementHook(AdvancementFunction tickFunction, RefreshFunction updateFunction, bool startPaused = true, speed_t startingSpeed = 0);

		void setSimulationSpeed(speed_t speed);
		speed_t getSimulationSpeed() const;
		void increaseSimulationSpeed();
		void decreaseSimulationSpeed();
		bool canIncreaseSimulationSpeed() const;
		bool canDecreaseSimulationSpeed() const;
		GameAdvancementHook& operator++();
		GameAdvancementHook& operator--();
		void conditionallyAdvanceGame();
		void reset();
	};
}