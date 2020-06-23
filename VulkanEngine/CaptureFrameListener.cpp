
#include "VEInclude.h"

namespace ve {

	// Have the codec and socket been initialized
	bool prepared = false;

	AVCodec* codec;
	AVCodecContext* codecContext;

	VkExtent2D outputExtent;

	FILE* outFile;

	// Socket data
	struct sockaddr_in addr;
	WSADATA wsaData;
	SOCKET sock;

	int frames_since_last_capture = 0;
	const int frames_between_captures = 3;

	const AVCodecID CODEC_ID = AV_CODEC_ID_MPEG4;
	const uint32_t BITRATE = 500'000;

	/**
	 *	Record every frame
	 */
	void CaptureFrameListener::onFrameEnded(veEvent event) {
		
		//if (frames_since_last_capture < frames_between_captures)
		//{
		//	// Skip frame
		//	frames_since_last_capture++;
		//	return;
		//}

		// Prepare frame capture
		VkExtent2D extent = getWindowPointer()->getExtent();
		uint32_t imageSize = extent.width * extent.height * 4;

		if (!prepared)
		{
			prepareCapture(1280, 720);
			prepareSocket();
		}

		VkImage image = getRendererPointer()->getSwapChainImage();

		uint8_t* dataImage = new uint8_t[imageSize];

		// Get frame
		vh::vhBufCopySwapChainImageToHost(getRendererPointer()->getDevice(),
			getRendererPointer()->getVmaAllocator(),
			getRendererPointer()->getGraphicsQueue(),
			getRendererPointer()->getCommandPool(),
			image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			dataImage, extent.width, extent.height, imageSize);


		// Convert frame for export
		AVFrame* dst_picture = av_frame_alloc();
		convert_frame(dataImage, dst_picture, extent);
		
		// Start encoding process
		encode_frame(dst_picture, extent);

		delete[] dataImage;
	}

	// Prepare capturing video to given filename
	void CaptureFrameListener::prepareCapture(uint32_t width, uint32_t height) {
		prepared = true;

		outputExtent = {
			width,
			height
		};

		avcodec_register_all();

		codec = avcodec_find_encoder(CODEC_ID);
		if (!codec) {
			fprintf(stderr, "codec not found\n");
			exit(1);
		}

		codecContext = avcodec_alloc_context3(codec);

		codecContext->bit_rate = BITRATE;

		// resolution must be a multiple of two
		codecContext->width = outputExtent.width;
		codecContext->height = outputExtent.height;
		// frames per second
		codecContext->time_base.num = frames_between_captures+1;
		codecContext->time_base.den = 30;
		codecContext->framerate.num = 30;
		codecContext->framerate.den = frames_between_captures+1;

		codecContext->gop_size = 10; // emit one intra frame every ten frames
		codecContext->max_b_frames = 1;
		codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	}

	void CaptureFrameListener::prepareSocket() {

		// Initialise winsock
		printf("Initialising Winsock...");
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			printf("WSA init failed: %d\n", WSAGetLastError());
			return;
		}
		printf("Initialised.\n");

		// Socket definition
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(8888);

		// Initialize the socket
		sock = socket(PF_INET, SOCK_DGRAM, 0);
		if (sock < 0) {
			printf("Socket init failed: %d\n", WSAGetLastError());
			return;
		}
		printf("Socket created.\n");
	}

	// Flush encoder and release codec
	void CaptureFrameListener::endCapture() {

		// Create empty packet for flushing
		AVPacket* pkt = av_packet_alloc();
		if (!pkt) {
			fprintf(stderr, "Cannot alloc packet\n");
			exit(1);
		}

		// flush the encoder
		encode(codecContext, NULL, pkt);

		avcodec_free_context(&codecContext);
	}

	// Convert dataImage and put into dst_picture
	void CaptureFrameListener::convert_frame(uint8_t* dataImage, AVFrame* dst_picture, VkExtent2D extent) {

		// Chekc frame exists
		if (!dst_picture) {
			fprintf(stderr, "video frame not allocated\n");
			exit(5);
		}
		
		// Set destination frame parameters
		dst_picture->width = outputExtent.width;
		dst_picture->height = outputExtent.height;
		dst_picture->format = AV_PIX_FMT_YUV420P;

		// Allocate destination frame
		int ret = av_frame_get_buffer(dst_picture, 32);
		if (ret < 0) {
			fprintf(stderr, "could not alloc the frame data\n");
			exit(1);
		}

		// Convert to AV_PIX_FMT_YUV420P
		// From: https://stackoverflow.com/questions/16667687/how-to-convert-rgb-from-yuv420p-for-ffmpeg-encoder
		SwsContext* ctx = sws_getContext(extent.width, extent.height,
			AV_PIX_FMT_RGBA, outputExtent.width, outputExtent.height,
			AV_PIX_FMT_YUV420P, 0, 0, 0, 0);

		uint8_t* inData[1] = { dataImage }; // RGB24 have one plane
		int inLinesize[1] = { 4 * extent.width }; // RGB stride
		sws_scale(ctx, inData, inLinesize, 0, extent.height, dst_picture->data, dst_picture->linesize);
	}

	// Encode single frame to file
	void CaptureFrameListener::encode_frame(AVFrame* frame, VkExtent2D extent) {
		
		// Create av-packet
		AVPacket* pkt = av_packet_alloc();
		if (!pkt) {
			fprintf(stderr, "Cannot alloc packet\n");
			exit(1);
		}
		
		AVFrame* picture = frame;

		// open codec
		if (avcodec_open2(codecContext, codec, NULL) < 0) {
			fprintf(stderr, "could not open codec\n");
			exit(1);
		}
		
		// Encode frame
		encode(codecContext,
			picture,
			pkt);
		
		// Release
		av_packet_free(&pkt);
		av_frame_free(&picture);
	}

	// Encode and send to udp
	void CaptureFrameListener::encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt) {
		int ret;

		// send the frame to the encoder
		ret = avcodec_send_frame(enc_ctx, frame);
		if (ret < 0) {
			fprintf(stderr, "error sending a frame for encoding\n");
			exit(1);
		}

		// Get av-packets from frame
		while (ret >= 0) {
			int ret = avcodec_receive_packet(enc_ctx, pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0) {
				fprintf(stderr, "error during encoding\n");
				exit(1);
			}

			//printf("encoded frame %lld (size=%5d)\n", pkt->pts, pkt->size);

			// Send av-packet
			prepare_send(pkt->pts, pkt);
			av_packet_unref(pkt);
		}
	}

	// Divide send av-packets as multiple udp-packets (frags)
	void CaptureFrameListener::prepare_send(int frame, AVPacket* pkt) {

		int nfrags = (pkt->size / PACKET_SIZE) + 1;	

		// Divide frame into packets and send them
		for (size_t i = 0; i < (nfrags-1); i++) {
			
			// Send fragment of packet starting at i*packet_size
			udp_send(
				frame, i, 
				reinterpret_cast<char*>(&pkt->data[i * PACKET_SIZE]), 
				PACKET_SIZE, 
				nfrags, pkt->size
				);
		}

		// Last frag has different size
		udp_send(
			frame, nfrags-1,
			reinterpret_cast<char*>(&pkt->data[(nfrags-1) * PACKET_SIZE]),
			pkt->size - (nfrags-1)*PACKET_SIZE,
			nfrags, pkt->size
			);
	}

	// Send data fragment
	void CaptureFrameListener::udp_send(int frame, int frag, char* pkt, int fragsize, int nfrags, int framesize) {
		
		// Create packet object
		UDPPacket packet;
		packet.header.nframe = frame;
		packet.header.nfrag = frag;
		packet.header.nfrags = nfrags;
		packet.header.framesize = framesize;
		memcpy_s(&packet.packet, PACKET_SIZE, pkt, fragsize);

		packet.gameinfo.currentTime = MVE::g_time;
		packet.gameinfo.prestart = MVE::g_preStart;
		packet.gameinfo.lost = MVE::g_gameLost;
		packet.gameinfo.won = MVE::g_gameWon;

		// Send packet
		int ret = sendto(
			sock, 
			reinterpret_cast<char*>(&packet),
			sizeof(packet),
			0, 
			(const struct sockaddr*) & addr,
			sizeof(addr)
			);

		if (ret < 0) {
			printf("Send failure: %d\n", WSAGetLastError());
			return;
		}
	}
}
