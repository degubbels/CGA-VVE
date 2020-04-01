
#include "VEInclude.h"

namespace ve {

	// Movement speed
	float speed = 160.0f;
	float rotSpeed = 2.0;


	/**
	 *	Movement
	 */
	bool KeyboardListener::onKeyboard(veEvent event) {

		VESceneNode* catP = getSceneManagerPointer()->getSceneNode("catP");
		VESceneNode* cat = getSceneManagerPointer()->getSceneNode("cat");

		glm::vec4 translate = glm::vec4(0.0, 0.0, 0.0, 1.0);	//total translation
		glm::vec4 rot4 = glm::vec4(1.0);
		float angle = 0.0;

		switch (event.idata1) {
		case GLFW_KEY_W:
			translate = glm::translate(cat->getTransform(), glm::vec3(10.0f, 0.0f, 0.0f)) * glm::vec4(0.0, 0.0, 1.0, 1.0); //forward
			translate.y = 0.0f;
			break;
		case GLFW_KEY_A:
			angle = rotSpeed * (float)event.dt * -1.0f;
			rot4 = glm::vec4(0.0, 1.0, 0.0, 1.0);
			break;
		case GLFW_KEY_D:
			angle = rotSpeed * (float)event.dt * 1.0f;
			rot4 = glm::vec4(0.0, 1.0, 0.0, 1.0);
			break;
		default:
			break;
		}

		// Movement
		glm::vec3 trans = speed * glm::vec3(translate.x, translate.y, translate.z);
		catP->multiplyTransform(glm::translate( glm::mat4(1.0f), (float)event.dt * trans) );

		// Rotation
		glm::vec3  rot3 = glm::vec3(rot4.x, rot4.y, rot4.z);
		glm::mat4  rotate = glm::rotate(glm::mat4(1.0), angle, rot3);
		cat->multiplyTransform(rotate);

		return false;
	}
}