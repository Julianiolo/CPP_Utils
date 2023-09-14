#ifndef __CPPUTILS_ACTIONMANAGER_H__
#define __CPPUTILS_ACTIONMANAGER_H__

#include <cstdint>
#include <vector>
#include <map>
#include <functional>
#include <iterator>
#include <string>

class ActionManager {
public:
	struct Action {
		struct Part {
			enum {
				Type_MouseButton=0,
				Type_Key
			};
			uint8_t type;
			int id;

			bool operator==(const Part& other);
		};
		
		std::string title;
		size_t id;
		std::vector<Part> parts;
		std::vector<Part> defParts;

		Action();
		Action(const char* title);

		void clear();

		Action& addKey(int keyId);
		Action& addMouseButton(int mouseId);

		void setAsDefault();
		void resetToDefault();
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
	std::vector<size_t> keysOrdered;
	TestCallB testCallB;
public:

	class Iterator {
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = Action;
		using pointer = value_type*;
		using reference = value_type&;

	private:
		ActionManager& am;
		size_t ind;
	public:

		Iterator(ActionManager& am, size_t ind);

		reference operator*() const;
		pointer operator->();

		Iterator& operator++();
		Iterator operator++(int);

		bool operator== (const Iterator& b);
		bool operator!= (const Iterator& b); 
	};
	
	Action& addAction(const char* title, size_t id);
	Action& getAction(size_t id);

	bool isActionActive(size_t id, ActivationState activationState);
	
	void setTestCallB(const TestCallB& callB);
	
	Iterator begin();
	Iterator end();
};

#endif