
#ifndef CAPTUREFRAMELISTENER_H
#define CAPTUREFRAMELISTENER_H

namespace ve {

	class CaptureFrameListener : public VEEventListener {

	protected:
		const static int PACKET_SIZE = 1400;

		virtual void onFrameEnded(veEvent event);
		void convert_frame(uint8_t* dataImage, AVFrame* dst_picture, VkExtent2D extent);
		void encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt, FILE* outfile);
		void encode_send(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt);
		void encode_frame(AVFrame* frame, VkExtent2D extent);
		void prepare_send(int frame, AVPacket* pkt);
		void udp_send(int frame, int frag, char pkt[1400], int fragsize, int nfrags);

		struct UDPHeader {
			uint32_t nframe;
			uint32_t nfrag;
			uint32_t nfrags;
		};

		struct UDPPacket {
			UDPHeader header;
			char packet[PACKET_SIZE] = { 0 };
		};

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