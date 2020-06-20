#ifndef SCENEGUILISTENER_H
#define SCENEGUILISTENER_H

namespace ve {

	class SceneGUIListener : public VEEventListener {

	private:

		int currtreenodeid = 0;
		VESceneNode* selectedNode;
		bool nodeSelected = false;

		char xbuf[256];
		bool editingX = false;

		virtual void onDrawOverlay(veEvent event);
		virtual void addToTree(VESceneNode* node, nk_context* ctx);
		virtual void addPositionSliders(nk_context* ctx, VESceneNode* selectedNode);
		virtual void addLightColourPicker(nk_context* ctx, VELight* node);
	public:

		///Constructor
		SceneGUIListener(std::string name) : VEEventListener(name) { };
		///Destructor
		virtual ~SceneGUIListener() {};
	};

}
#endif