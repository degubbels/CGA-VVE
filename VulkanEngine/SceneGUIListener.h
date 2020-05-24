#ifndef SCENEGUILISTENER_H
#define SCENEGUILISTENER_H

namespace ve {

	class SceneGUIListener : public VEEventListener {

	private:
		virtual void onDrawOverlay(veEvent event);
	public:

		///Constructor
		SceneGUIListener(std::string name) : VEEventListener(name) { };
		///Destructor
		virtual ~SceneGUIListener() {};
	};

}
#endif