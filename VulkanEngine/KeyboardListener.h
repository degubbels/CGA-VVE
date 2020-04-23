
#ifndef KEYBOARDLISTENER_H
#define KEYBOARDLISTENER_H

namespace ve {

	class KeyboardListener : public VEEventListener {

	protected:

		virtual bool onKeyboard(veEvent event);

	public:
		///Constructor
		KeyboardListener(std::string name) : VEEventListener(name) { };
		///Destructor
		virtual ~KeyboardListener() {};
	};

}

#endif