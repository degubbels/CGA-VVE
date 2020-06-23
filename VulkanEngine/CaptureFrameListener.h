
#ifndef CAPTUREFRAMELISTENER_H
#define CAPTUREFRAMELISTENER_H

namespace ve {

	class CaptureFrameListener : public VEEventListener {

	protected:
		const static int PACKET_SIZE = 1400;

		void prepareCapture(uint32_t width, uint32_t height);
		void prepareSocket();

		virtual void onFrameEnded(veEvent event);

		void convert_frame(uint8_t* dataImage, AVFrame* dst_picture, VkExtent2D extent);
		void encode_frame(AVFrame* frame, VkExtent2D extent);
		void encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt);
		void prepare_send(int frame, AVPacket* pkt);

		void udp_send(int frame, int frag, char pkt[1400], int fragsize, int nfrags, int framesize);

		struct UDPHeader {
			uint32_t nframe;
			uint32_t nfrag;
			uint32_t nfrags;
			uint32_t framesize;
		};

		struct UDPGameInfo {
			float currentTime;
			bool prestart;
			bool won;
			bool lost;
		};

		struct UDPPacket {
			UDPHeader header;
			UDPGameInfo gameinfo;
			char packet[PACKET_SIZE] = { 0 };
		};

	public:
		void endCapture();

		///Constructor
		CaptureFrameListener(std::string name) : VEEventListener(name) { };
		///Destructor
		virtual ~CaptureFrameListener() {};
	};

}

#endif