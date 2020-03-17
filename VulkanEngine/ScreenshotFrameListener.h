
#ifndef SCREENSHOTFRAMELISTENER_H
#define SCREENSHOTFRAMELISTENER_H

namespace ve {

	class ScreenshotFrameListener : public VEEventListener {

	protected:
		virtual void onFrameEnded(veEvent event);

	public:
		///Constructor
		ScreenshotFrameListener(std::string name) : VEEventListener(name) { };
		///Destructor
		virtual ~ScreenshotFrameListener() {};
	};

}

#endif