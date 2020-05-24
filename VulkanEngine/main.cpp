/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/


#include "VEInclude.h"



namespace ve {

	float CAT_Y_OFFSET = 0.5;

	int NUM_CUBES = 60;

	float COURSE_LENGTH = 60.0;

	float CUBES_X_MIN = 2.0;
	float CUBES_X_MAX = COURSE_LENGTH;
	float CUBES_Z_MIN = -12.0;
	float CUBES_Z_MAX = 12.0;

	double g_initialTime = 30.0;

	uint32_t g_score = 0;				//derzeitiger Punktestand
	double g_time = g_initialTime;				//zeit die noch übrig ist

	bool g_gameWon = false;
	bool g_gameLost = false;			//true... das Spiel wurde verloren
	bool g_restart = false;				//true...das Spiel soll neu gestartet werden

	CaptureFrameListener* captureFrameListener;

	//
	//Zeichne das GUI
	//
	class EventListenerGUI : public VEEventListener {
	protected:
		
		virtual void onDrawOverlay(veEvent event) {
			VESubrenderFW_Nuklear * pSubrender = (VESubrenderFW_Nuklear*)getRendererPointer()->getOverlay();
			if (pSubrender == nullptr) return;

			struct nk_context * ctx = pSubrender->getContext();

			if (!g_gameLost && !g_gameWon) {
				if (nk_begin(ctx, "", nk_rect(0, 0, 400, 300), NK_WINDOW_BORDER )) {
					char outbuffer[100];

					nk_layout_row_dynamic(ctx, 45, 1);
					sprintf(outbuffer, "Get to the big cube ASAP");
					nk_label(ctx, outbuffer, NK_TEXT_LEFT);

					nk_layout_row_dynamic(ctx, 45, 1);
					sprintf(outbuffer, "Avoid the small cubes");
					nk_label(ctx, outbuffer, NK_TEXT_LEFT);

					nk_layout_row_dynamic(ctx, 45, 1);
					sprintf(outbuffer, "Move forward with [W]");
					nk_label(ctx, outbuffer, NK_TEXT_LEFT);

					nk_layout_row_dynamic(ctx, 45, 1);
					sprintf(outbuffer, "Turn with[A], [D]");
					nk_label(ctx, outbuffer, NK_TEXT_LEFT);

					nk_layout_row_dynamic(ctx, 45, 1);
					sprintf(outbuffer, "Move camera with [I][J][k][L]");
					nk_label(ctx, outbuffer, NK_TEXT_LEFT);					

					nk_layout_row_dynamic(ctx, 45, 1);
					sprintf(outbuffer, "Time: %004.1lf", g_time);
					nk_label(ctx, outbuffer, NK_TEXT_LEFT);
				}
			}
			else if (g_gameWon) {
				
				if (nk_begin(ctx, "", nk_rect(500, 500, 200, 170), NK_WINDOW_BORDER)) {
					char outbuffer[100];

					nk_layout_row_dynamic(ctx, 45, 1);
					nk_label(ctx, "You Won!", NK_TEXT_LEFT);

					nk_layout_row_dynamic(ctx, 45, 1);
					sprintf(outbuffer, "Score: %03d", g_score);
					nk_label(ctx, outbuffer, NK_TEXT_LEFT);

					if (nk_button_label(ctx, "Restart")) {
						g_restart = true;
					}
				}
			
			} else {
				if (nk_begin(ctx, "", nk_rect(500, 500, 200, 170), NK_WINDOW_BORDER )) {
					nk_layout_row_dynamic(ctx, 45, 1);
					nk_label(ctx, "You Lost", NK_TEXT_LEFT);
					if (nk_button_label(ctx, "Restart")) {
						g_restart = true;
					}
				}

			};

			nk_end(ctx);
		}

	public:
		///Constructor of class EventListenerGUI
		EventListenerGUI(std::string name) : VEEventListener(name) { };

		///Destructor of class EventListenerGUI
		virtual ~EventListenerGUI() {};
	};


	static std::default_random_engine e{ 12345 };					//Für Zufallszahlen
	static std::uniform_real_distribution<> d{ -10.0f, 10.0f };		//Für Zufallszahlen

	//
	// Überprüfen, ob die Kamera die Kiste berührt
	//
	class EventListenerCollision : public VEEventListener {
	protected:
		virtual void onFrameStarted(veEvent event) {
			static uint32_t cubeid = 0;

			// Reset game
			if (g_restart) {
				g_gameWon = false;
				g_gameLost = false;
				g_restart = false;
				g_time = g_initialTime;
				g_score = 0;
				getSceneManagerPointer()->getSceneNode("catP")->setPosition(glm::vec3(0, 1.0f-CAT_Y_OFFSET, 0));

				VESceneNode* pCameraParent = getSceneManagerPointer()->getCamera()->getParent();
				pCameraParent->setPosition(glm::vec3(-10.0f, 10.0f, 0.0f));
				getEnginePointer()->m_irrklangEngine->play2D("media/sounds/ophelia.mp3", true);
				return;
			}

			// Do nothing if the game is over
			if (g_gameLost || g_gameWon) return;

			glm::vec3 catPos = getSceneManagerPointer()->getSceneNode("catP")->getPosition();

			glm::vec3 positionFinish   = getSceneManagerPointer()->getSceneNode("finish")->getPosition();
			glm::vec3 positionCamera = getSceneManagerPointer()->getSceneNode("StandardCameraParent")->getPosition();

			float finishDistance = glm::length(positionFinish - catPos);

			// Player won, End game
			if (finishDistance < 5.0f) {
				g_gameWon = true;
				printf("won\n");
				g_score = 100 * g_time;

				getEnginePointer()->m_irrklangEngine->removeAllSoundSources();
				getEnginePointer()->m_irrklangEngine->play2D("media/sounds/bell.wav", false);
			}

			for (int i = 0; i < NUM_CUBES; i++) {
				glm::vec3 cubePos = getSceneManagerPointer()->getSceneNode("cube-" + i)->getPosition();
				float cubeDistance = glm::length(cubePos - catPos);

				if (cubeDistance < 1.0f) {
					// Player lost, End game
					g_gameLost = true;
					getEnginePointer()->m_irrklangEngine->removeAllSoundSources();
					getEnginePointer()->m_irrklangEngine->play2D("media/sounds/gameover.wav", false);
				}
			}

			g_time -= event.dt;
					

			// Player lost, End game
			if (g_time <= 0) {
				g_gameLost = true;
				getEnginePointer()->m_irrklangEngine->removeAllSoundSources();
				getEnginePointer()->m_irrklangEngine->play2D("media/sounds/gameover.wav", false);
			}
		};

	public:
		///Constructor of class EventListenerCollision
		EventListenerCollision(std::string name) : VEEventListener(name) { };

		///Destructor of class EventListenerCollision
		virtual ~EventListenerCollision() {};
	};

	

	///user defined manager class, derived from VEEngine
	class MyVulkanEngine : public VEEngine {
	public:

		MyVulkanEngine( bool debug=false) : VEEngine(debug) {};
		~MyVulkanEngine() {};


		///Register an event listener to interact with the user
		
		virtual void registerEventListeners() {

			// Register keyboard event listener for movement
			registerEventListener(new KeyboardListener("movement"), { veEvent::VE_EVENT_KEYBOARD });

			VEEngine::registerEventListeners();

			registerEventListener(new EventListenerCollision("Collision"), { veEvent::VE_EVENT_FRAME_STARTED });
			registerEventListener(new EventListenerGUI("GUI"), { veEvent::VE_EVENT_DRAW_OVERLAY});
			//registerEventListener(new CaptureFrameListener("captureListener"), { veEvent::VE_EVENT_FRAME_ENDED });
			registerEventListener(new SceneGUIListener("SceneUI"), { veEvent::VE_EVENT_DRAW_OVERLAY });
		};
		

		///Load the first level into the game engine
		///The engine uses Y-UP, Left-handed
		virtual void loadLevel( uint32_t numLevel=1) {

			VEEngine::loadLevel(numLevel );			//create standard cameras and lights

			// Move camera up
			VESceneNode* pCamera = getSceneManagerPointer()->getCamera();
			VESceneNode* pCameraParent = pCamera->getParent();
			pCameraParent->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 10.0f, 0.0f)));
			pCamera->multiplyTransform(glm::rotate(glm::mat4(1.0), glm::half_pi<float>(), glm::vec3(0.0, 1.0, 0.0)));
			pCamera->multiplyTransform(glm::rotate(glm::mat4(1.0), -glm::pi<float>() / 6, glm::vec3(0.0, 0.0, 1.0)));

			VESceneNode *pScene;
			VECHECKPOINTER( pScene = getSceneManagerPointer()->createSceneNode("Level 1", getRoot()) );
	
			//scene models

			VESceneNode *sp1;
			VECHECKPOINTER( sp1 = getSceneManagerPointer()->createSkybox("The Sky", "media/models/test/sky/cloudy",
										{	"bluecloud_ft.jpg", "bluecloud_bk.jpg", "bluecloud_up.jpg", 
											"bluecloud_dn.jpg", "bluecloud_rt.jpg", "bluecloud_lf.jpg" }, pScene)  );

			VESceneNode *e4;
			VECHECKPOINTER( e4 = getSceneManagerPointer()->loadModel("The Plane", "media/models/test", "plane_t_n_s.obj",0, pScene) );
			e4->setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f, 1.0f, 1000.0f)));

			VEEntity *pE4;
			VECHECKPOINTER( pE4 = (VEEntity*)getSceneManagerPointer()->getSceneNode("The Plane/plane_t_n_s.obj/plane/Entity_0") );
			pE4->setParam( glm::vec4(1000.0f, 1000.0f, 0.0f, 0.0f) );

			VESceneNode *e1,*eParent;
			eParent = getSceneManagerPointer()->createSceneNode("The Cube Parent", pScene, glm::mat4(1.0));

			spawnCubes(eParent, NUM_CUBES, CUBES_X_MIN, CUBES_X_MAX, CUBES_Z_MIN, CUBES_Z_MAX);
			

			// Load cat model
			// Position should only be changed for the catParent ("catP")
			VESceneNode* cat, *catParent;
			catParent = getSceneManagerPointer()->createSceneNode("catP", pScene, glm::mat4(1.0));
			VECHECKPOINTER(cat= getSceneManagerPointer()->loadModel("cat", "media/models/test/cat", "12221_Cat_v1_l3.obj", aiProcess_FlipWindingOrder | aiProcess_FlipUVs, catParent));
			cat->setTransform(glm::scale(glm::mat4(0.1f), glm::vec3(0.02f, 0.02f, 0.02f)));

			// Rotate 90 deg around x-axis and y-axis
			cat->multiplyTransform(glm::rotate( glm::mat4(1.0), -glm::half_pi<float>(), glm::vec3(1.0, 0.0, 0.0) ));

			// Move up to ground level
			catParent->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f - CAT_Y_OFFSET, 0.0f)));
			catParent->multiplyTransform(glm::rotate(glm::mat4(1.0), glm::half_pi<float>(), glm::vec3(0.0, 1.0, 0.0)));

			
			// Load Gate
			VESceneNode* finish;
			VECHECKPOINTER(finish = getSceneManagerPointer()->loadModel("finish", "media/models/test/crate0", "cube.obj", 0, pScene));
			finish->setTransform(glm::scale(glm::mat4(1.0), glm::vec3(10.0, 10.0, 10.0)));
			finish->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(COURSE_LENGTH, 1.0f, 0.0f)));
			


			m_irrklangEngine->play2D("media/sounds/ophelia.mp3", true);
		};

		// spawn num cubes with the given bounds
		void spawnCubes(VESceneNode* parent, int num, float x_min, float x_max, float z_min, float z_max) {

			for (int i = 0; i < num; i++) {
				VESceneNode *cube;

				VECHECKPOINTER(cube = getSceneManagerPointer()->loadModel("cube-"+i, "media/models/test/crate0", "cube.obj", 0, parent));

				float xpos = (rand() % (int)(x_max - x_min)) + x_min;
				float zpos = (rand() % (int)(z_max - z_min)) + z_min;

				cube->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(xpos, 1.0f, zpos)));
			}
		}
	};


}

using namespace ve;

int main() {

	bool debug = true;

	MyVulkanEngine mve(debug);	//enable or disable debugging (=callback, validation layers)

	mve.initEngine();
	mve.loadLevel(1);
	mve.run();

	// Close video capture
	captureFrameListener->endCapture();

	return 0;
}

