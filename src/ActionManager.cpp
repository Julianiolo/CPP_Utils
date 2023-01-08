#include "ActionManager.h"

void ActionManager::Action::addKey(int id) {
	parts.push_back({ Part::Type_Key, id });
}
void ActionManager::Action::addMouseButton(int id) {
	parts.push_back({ Part::Type_MouseButton, id });
}

ActionManager::Action& ActionManager::addAction(size_t id) {
	return actions[id] = Action();
}
ActionManager::Action& ActionManager::getAction(size_t id) {
	return actions[id];
}

bool ActionManager::isActionActive(size_t id, ActivationState activationState) {
	const Action& action = getAction(id);

	ActivationState testState = (activationState == ActivationState_Down || activationState == ActivationState_Pressed) ? ActivationState_Down : ActivationState_Up;
	for (size_t i = 0; i < action.parts.size(); i++) {
		if (!testCallB(action.parts[i].type, action.parts[i].id, testState)) {
			return false;
		}
	}

	if (activationState == ActivationState_Pressed || activationState == ActivationState_Released) {
		for (size_t i = 0; i < action.parts.size(); i++) {
			if (testCallB(action.parts[i].type, action.parts[i].id, activationState)) {
				return true;
			}
		}
		return false;
	}
	return true;
}

void ActionManager::setTestCallB(const TestCallB& callB) {
	testCallB = callB;
}