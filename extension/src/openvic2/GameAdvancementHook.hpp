#pragma once

#include <chrono>
#include <functional>

namespace OpenVic2 {
	//Value of different game speeds are the minimum number of miliseconds before the simulation advances
	enum class GameSpeed {
		Speed1 = 4000,
		Speed2 = 3000,
		Speed3 = 2000,
		Speed4 = 1000,
		Speed5 = 100,
		Speed6 = 1
	};

	//Conditionally advances game with provided behaviour
	//Class governs game speed and pause state
	class GameAdvancementHook {
		public:
		using AdvancementFunction = std::function<void()>;

		private:
		std::chrono::time_point<std::chrono::high_resolution_clock> lastPolledTime;
		//A function pointer that advances the simulation, intended to be a capturing lambda or something similar. May need to be reworked later
		AdvancementFunction triggerFunction;

		public:
		bool isPaused;
		GameSpeed currentSpeed;

		GameAdvancementHook(AdvancementFunction function = nullptr, bool startPaused = false, GameSpeed startingSpeed = GameSpeed::Speed1);

		void increaseSimulationSpeed();
		void decreaseSimulationSpeed();
		GameAdvancementHook operator++(int);
		GameAdvancementHook operator--(int);
		void conditionallyAdvanceGame();
	};
}