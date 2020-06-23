
#include "VEInclude.h"

namespace ve {

	bool stop = false;

	// Movement speed
	float speed = 160.0f;
	float rotSpeed = 2.0;

	// Keys currently pressed
	std::set<int> keysDown;

	std::chrono::time_point<std::chrono::steady_clock> lastPacketTime;

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

	// Initialise input receiver socket
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

		while (!stop) {

			UDPInputPacket packet = nextPacket();

			// update dt
			auto currentPacketTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> dt = currentPacketTime - lastPacketTime;
			lastPacketTime = currentPacketTime;

			if (!packet.empty) {
				updateKeysDown(packet);
				processInput(dt.count());
				updateKeysUp(packet);
			}
		}
	}

	byte UDPBuffer[sizeof(UDPInputPacket)];

	UDPInputPacket KeyboardListener::nextPacket() {

		// Receive next udp packet
		int ret = recvfrom(
			InputSocket,
			(char*)UDPBuffer,
			sizeof(UDPInputPacket),
			0,
			(sockaddr*)&InputSocketAddress,
			&InputSocketAddressLength
		);

		// Check for error
		if (ret < 0) {
			printf("Receive failure: %d\n", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		return *(reinterpret_cast<UDPInputPacket*>(UDPBuffer));
	}

	void KeyboardListener::updateKeysDown(UDPInputPacket inputPacket) {

		// Update keysDown set
		for each (int keycode in inputPacket.down) {

			if (keycode > 0) {
				keysDown.insert(keycode);
			}
		}
	}

	void KeyboardListener::updateKeysUp(UDPInputPacket inputPacket) {
		for each (int keycode in inputPacket.up) {

			if (keycode > 0) {
				keysDown.erase(keycode);
			}
		}
	}

	void KeyboardListener::processInput(double dt) {

		VESceneNode* catP = getSceneManagerPointer()->getSceneNode("catP");
		VESceneNode* cat = getSceneManagerPointer()->getSceneNode("cat");

		glm::vec4 translate = glm::vec4(0.0, 0.0, 0.0, 1.0);	//total translation
		glm::vec4 rot4 = glm::vec4(1.0);
		float angle = 0.0;

		for each (int keycode in keysDown) {

			if (keycode > 0) {
				//printf("received key: %d\n", keycode);

				switch (keycode) {
				case 119: // W
					if (!(MVE::g_preStart || MVE::g_gameWon || MVE::g_gameLost)) {
						translate = glm::translate(cat->getTransform(), glm::vec3(10.0f, 0.0f, 0.0f)) * glm::vec4(0.0, 0.0, 1.0, 1.0); //forward
						translate.y = 0.0f;
					}
					break;
				case 97: // A
					if (!(MVE::g_preStart || MVE::g_gameWon || MVE::g_gameLost)) {
						angle = rotSpeed * (float)dt * -1.0f;
						rot4 = glm::vec4(0.0, 1.0, 0.0, 1.0);
					}
					break;
				case 100:
					if (!(MVE::g_preStart || MVE::g_gameWon || MVE::g_gameLost)) {
						angle = rotSpeed * (float)dt * 1.0f;
						rot4 = glm::vec4(0.0, 1.0, 0.0, 1.0);
					}
					break;
				case 13: // ENTER

					// (re)start game if applicable
					if (MVE::g_preStart || MVE::g_gameWon || MVE::g_gameLost) {
						MVE::g_restart = true;
					}
					break;
				default:
					printf("key: %d\n", keycode);
					break;
				}
			}
		}

		// Movement
		glm::vec3 trans = speed * glm::vec3(translate.x, translate.y, translate.z);
		catP->multiplyTransform(glm::translate(glm::mat4(1.0f), (float)dt * trans));

		// Rotation
		glm::vec3  rot3 = glm::vec3(rot4.x, rot4.y, rot4.z);
		glm::mat4  rotate = glm::rotate(glm::mat4(1.0), angle, rot3);
		cat->multiplyTransform(rotate);
	}
}