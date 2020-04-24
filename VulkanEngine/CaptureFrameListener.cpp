
#include "VEInclude.h"

namespace ve {

	/**
	 *	Record every frame
	 */
	void CaptureFrameListener::onFrameEnded(veEvent event) {
		
		VkExtent2D extent = getWindowPointer()->getExtent();
		uint32_t imageSize = extent.width * extent.height * 4;
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
		printf("start frame encoding\n");
		encode_frame(dst_picture, extent);

		delete[] dataImage;
	}

	// Convert dataImage and put into dst_picture
	void CaptureFrameListener::convert_frame(uint8_t* dataImage, AVFrame* dst_picture, VkExtent2D extent) {
		if (!dst_picture) {
			fprintf(stderr, "video frame not allocated\n");
			exit(5);
		}
		
		// Set destination frame parameters
		dst_picture->width = extent.width;
		dst_picture->height = extent.height;
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
			AV_PIX_FMT_RGBA, extent.width, extent.height,
			AV_PIX_FMT_YUV420P, 0, 0, 0, 0);

		uint8_t* inData[1] = { dataImage }; // RGB24 have one plane
		int inLinesize[1] = { 4 * extent.width }; // RGB stride
		sws_scale(ctx, inData, inLinesize, 0, extent.height, dst_picture->data, dst_picture->linesize);
	}

	// Encode single frame to file
	void CaptureFrameListener::encode_frame(AVFrame* frame, VkExtent2D extent) {
		uint8_t endcode[] = { 0, 0, 1, 0xb7 };

		const char* filename = "out/frame.mpg";

		avcodec_register_all();

		const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
		if (!codec) {
			fprintf(stderr, "codec not found\n");
			exit(1);
		}

		AVCodecContext* c = avcodec_alloc_context3(codec);
		AVFrame* picture = frame;

		AVPacket* pkt = av_packet_alloc();
		if (!pkt) {
			fprintf(stderr, "Cannot alloc packet\n");
			exit(1);
		}

		c->bit_rate = 400000;

		// resolution must be a multiple of two
		c->width = extent.width;
		c->height = extent.height;
		// frames per second
		c->time_base.num = 1;
		c->time_base.den = 25;
		c->framerate.num = 25;
		c->framerate.den = 1;

		c->gop_size = 10; // emit one intra frame every ten frames
		c->max_b_frames = 1;
		c->pix_fmt = AV_PIX_FMT_YUV420P;

		// open it
		if (avcodec_open2(c, codec, NULL) < 0) {
			fprintf(stderr, "could not open codec\n");
			exit(1);
		}

		FILE* f = fopen(filename, "wb");
		if (!f) {
			fprintf(stderr, "could not open %s\n", filename);
			exit(1);
		}

		// Encode frame
		encode(c,
			picture,
			pkt,
			f);


		// flush the encoder
		encode(c, NULL, pkt, f);

		// add sequence end code to have a real MPEG file
		fwrite(endcode, 1, sizeof(endcode), f);
		fclose(f);

		avcodec_free_context(&c);
		av_frame_free(&picture);
		av_packet_free(&pkt);

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

