
#include "VEInclude.h"

namespace ve {

	bool stop = false;

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

	void KeyboardListener::initSocket() {

		WSADATA wsaData;
		int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (ret != 0) {
			printf("WSA init failed: %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		// Initialize the socket
		InputSocket = socket(PF_INET, SOCK_DGRAM, 0);
		if (InputSocket < 0) {
			printf("Socket init failed: %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		// Configure address
		InputSocketAddress.sin_family = AF_INET;
		InputSocketAddress.sin_addr.s_addr = UDP_ADDRESS;
		InputSocketAddress.sin_port = htons(UDP_PORT);

		

		// Bind socket
		ret = bind(InputSocket, (const sockaddr*)&InputSocketAddress, sizeof(InputSocketAddress));
		if (ret != 0) {
			printf("Socket bind failed: %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}

	void KeyboardListener::startReceiver() {

		initSocket();
		receiverLoop();
	}

	void KeyboardListener::receiverLoop() {

		printf("K||start receiver loop\n");

		while (!stop) {

			UDPInputPacket packet = nextPacket();
			processKeys(packet);
		}
	}

	byte UDPBuffer[sizeof(UDPInputPacket)];

	UDPInputPacket KeyboardListener::nextPacket() {

		printf("K||receive...\n");
		// Receive next udp packet
		int ret = recvfrom(
			InputSocket,
			(char*)UDPBuffer,
			sizeof(UDPInputPacket),
			0,
			(sockaddr*)&InputSocketAddress,
			&InputSocketAddressLength
		);

		printf("K||received packet\n");

		// Check for error
		if (ret < 0) {
			printf("Receive failure: %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		return *(reinterpret_cast<UDPInputPacket*>(UDPBuffer));
	}

	void KeyboardListener::processKeys(UDPInputPacket inputPacket) {

		for each (int keycode in inputPacket.down) {
			printf("received key: %d\n", keycode);
		}
	}
}