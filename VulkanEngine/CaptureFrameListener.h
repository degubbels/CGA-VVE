
#ifndef CAPTUREFRAMELISTENER_H
#define CAPTUREFRAMELISTENER_H

namespace ve {

	class CaptureFrameListener : public VEEventListener {

	protected:
		virtual void onFrameEnded(veEvent event);

	public:
		///Constructor
		CaptureFrameListener(std::string name) : VEEventListener(name) { };
		///Destructor
		virtual ~CaptureFrameListener() {};
	};

}

#endif