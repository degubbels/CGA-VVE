
#ifndef KEYBOARDLISTENER_H
#define KEYBOARDLISTENER_H

namespace ve {


	struct UDPInputPacket {
		int down[8];
		int up[8];
		bool empty;
	};
	class KeyboardListener : public VEEventListener {

	private:

		// Socket
		u_long UDP_ADDRESS = INADDR_ANY;
		u_short UDP_PORT = 8880;
		SOCKET InputSocket;
		sockaddr_in InputSocketAddress;
		int InputSocketAddressLength = sizeof(InputSocketAddress);;

		// Initialise udp socket
		virtual void initSocket();
		virtual UDPInputPacket KeyboardListener::nextPacket();

		virtual void receiverLoop();
		virtual void updateKeysDown(UDPInputPacket);
		virtual void updateKeysUp(UDPInputPacket);
		virtual void processInput(double dt);

	protected:

		virtual bool onKeyboard(veEvent event);


	public:
		virtual void startReceiver();

		///Constructor
		KeyboardListener(std::string name) : VEEventListener(name) { };
		///Destructor
		virtual ~KeyboardListener() {};
	};

}

#endif