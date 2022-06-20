
#ifndef MYVULKANENGINE_H
#define MYVULKANENGINE_H

namespace ve {

	class MVE : public VEEngine {
	public:
		//static MVE I;	// instance

		static uint32_t g_score;				//derzeitiger Punktestand
		static double g_initialTime;
		static double g_time;				//zeit die noch übrig ist

		static bool g_preStart;
		static bool g_gameWon;
		static bool g_gameLost;			//true... das Spiel wurde verloren
		static bool g_restart;				//true...das Spiel soll neu gestartet werden

		virtual void registerEventListeners();
		virtual void loadLevel(uint32_t numLevel = 1);
		virtual void spawnCubes(VESceneNode* parent, int num, float x_min, float x_max, float z_min, float z_max);

		MVE(bool debug = false) : VEEngine(debug) {};
		~MVE() {};
	};
}

#endif