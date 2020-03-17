
#include "VEInclude.h"

namespace ve {

	/**
	 *	Contiuously take screenshots
	 */
	void ScreenshotFrameListener::onFrameEnded(veEvent event) {
		
		timeElapsedSinceScreenshot += event.dt;
		
		if (timeElapsedSinceScreenshot >= (1.0 / SCREENSHOTSPERSECOND)) {
			// Take a new screenshot

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

			numScreenshot++;

			std::string name("media/screenshots/screenshot" + std::to_string(numScreenshot - 1) + ".jpg");
			stbi_write_jpg(name.c_str(), extent.width, extent.height, 4, dataImage, 4 * extent.width);
			delete[] dataImage;
		}
	}
}