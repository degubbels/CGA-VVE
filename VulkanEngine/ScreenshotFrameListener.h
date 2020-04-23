
#ifndef SCREENSHOTFRAMELISTENER_H
#define SCREENSHOTFRAMELISTENER_H

namespace ve {

	class ScreenshotFrameListener : public VEEventListener {

	protected:
		int SCREENSHOTSPERSECOND = 5;

		double timeElapsedSinceScreenshot = 0;
		uint32_t numScreenshot = 0;	

		virtual void onFrameEnded(veEvent event);

	public:
		///Constructor
		ScreenshotFrameListener(std::string name) : VEEventListener(name) { };
		///Destructor
		virtual ~ScreenshotFrameListener() {};
	};

}

#endif