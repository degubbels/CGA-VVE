
#ifndef CAPTUREFRAMELISTENER_H
#define CAPTUREFRAMELISTENER_H

namespace ve {

	class CaptureFrameListener : public VEEventListener {

	protected:
		virtual void onFrameEnded(veEvent event);
		void convert_frame(uint8_t* dataImage, AVFrame* dst_picture, VkExtent2D extent);
		void encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt, FILE* outfile);
		void encode_frame(AVFrame* frame, VkExtent2D extent);


	public:
		void prepareCapture(std::string filename, uint32_t width, uint32_t height);
		void endCapture();

		///Constructor
		CaptureFrameListener(std::string name) : VEEventListener(name) { };
		///Destructor
		virtual ~CaptureFrameListener() {};
	};

}

#endif