#ifndef __CPPUTILS_ACTIONMANAGER_H__
#define __CPPUTILS_ACTIONMANAGER_H__

#include <cstdint>
#include <vector>
#include <map>
#include <functional>

class ActionManager {
	struct Action {
		struct Part {
			enum {
				Type_MouseButton=0,
				Type_Key
			};
			uint8_t type;
			int id;
		};

		std::vector<Part> parts;

		void addKey(int id);
		void addMouseButton(int id);
	};

	enum {
		ActivationState_Down=0,
		ActivationState_Up,
		ActivationState_Pressed,
		ActivationState_Released,
	};
	typedef uint8_t ActivationState;

	typedef std::function<bool(uint8_t type, int id, ActivationState activationState)> TestCallB;

private:
	std::map<size_t, Action> actions;
	TestCallB testCallB;
public:
	
	Action& addAction(size_t id);
	Action& getAction(size_t id);

	bool isActionActive(size_t id, ActivationState activationState);
	
	void setTestCallB(const TestCallB& callB);
	
};

#endif