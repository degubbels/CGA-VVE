#ifndef SCENEGUILISTENER_H
#define SCENEGUILISTENER_H

namespace ve {

	class SceneGUIListener : public VEEventListener {

	private:

		int currtreenodeid = 0;
		VESceneNode* selectedNode;
		bool nodeSelected = false;

		virtual void onDrawOverlay(veEvent event);
		virtual void addToTree(VESceneNode* node, nk_context* ctx);
	public:

		///Constructor
		SceneGUIListener(std::string name) : VEEventListener(name) { };
		///Destructor
		virtual ~SceneGUIListener() {};
	};

}
#endif