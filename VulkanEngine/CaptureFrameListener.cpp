
#include "VEInclude.h"

namespace ve {

	bool prepared = false;

	AVCodec* codec;
	AVCodecContext* codecContext;

	VkExtent2D outputExtent;

	FILE* outFile;

	int frames_since_last_capture = 0;
	const int frames_between_captures = 3;

	const AVCodecID CODEC_ID = AV_CODEC_ID_VP9;
	const uint32_t BITRATE = 300'000;
	const std::string name = "VP9_300";

	/**
	 *	Record every frame
	 */
	void CaptureFrameListener::onFrameEnded(veEvent event) {

		if (frames_since_last_capture < frames_between_captures)
		{
			// Skip frame
			frames_since_last_capture++;
			return;
		}

		// Prepare frame capture
		VkExtent2D extent = getWindowPointer()->getExtent();
		uint32_t imageSize = extent.width * extent.height * 4;

		if (!prepared)
		{
			prepareCapture("out/"+name+".mpg", extent.width, extent.height);
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
	void CaptureFrameListener::prepareCapture(std::string filename, uint32_t width, uint32_t height) {
		prepared = true;

		outputExtent = {
			width,
			height
		};

		avcodec_register_all();

		codec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
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

		// Open file
		outFile = fopen(filename.c_str(), "wb");
		if (!outFile) {
			fprintf(stderr, "could not open %s\n", filename);
			exit(1);
		}
	}

	void CaptureFrameListener::endCapture() {

		AVPacket* pkt = av_packet_alloc();
		if (!pkt) {
			fprintf(stderr, "Cannot alloc packet\n");
			exit(1);
		}

		// flush the encoder
		encode(codecContext, NULL, pkt, outFile);

		// add sequence end code to have a real MPEG file
		uint8_t endcode[] = { 0, 0, 1, 0xb7 };
		fwrite(endcode, 1, sizeof(endcode), outFile);
		fclose(outFile);

		avcodec_free_context(&codecContext);
	}

	// Convert dataImage and put into dst_picture
	void CaptureFrameListener::convert_frame(uint8_t* dataImage, AVFrame* dst_picture, VkExtent2D extent) {
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
			pkt,
			outFile);
		
		av_packet_free(&pkt);
		av_frame_free(&picture);
	}

	void CaptureFrameListener::encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt, FILE* outfile)
	{
		int ret;

		// send the frame to the encoder */
		ret = avcodec_send_frame(enc_ctx, frame);
		if (ret < 0) {
			fprintf(stderr, "error sending a frame for encoding\n");
			exit(1);
		}

		while (ret >= 0) {
			int ret = avcodec_receive_packet(enc_ctx, pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0) {
				fprintf(stderr, "error during encoding\n");
				exit(1);
			}

			printf("encoded frame %lld (size=%5d)\n", pkt->pts, pkt->size);
			fwrite(pkt->data, 1, pkt->size, outfile);
			av_packet_unref(pkt);
		}
	}	
}

