
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

		vh::vhBufCopySwapChainImageToHost(getRendererPointer()->getDevice(),
			getRendererPointer()->getVmaAllocator(),
			getRendererPointer()->getGraphicsQueue(),
			getRendererPointer()->getCommandPool(),
			image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			dataImage, extent.width, extent.height, imageSize);

		// TODO: Export frame

		delete[] dataImage;
	}
}