#if!defined FINITESTATEMACHINE_HPP
#define FINITESTATEMACHINE_HPP
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <functional>
#include <memory>

class FiniteStateMachine
{
public:
	class AState
	{
	protected:
		explicit AState(std::string&& name) :
			m_stateName(name)
		{
		}

	public:
		virtual ~AState() = default;

		std::string const& getName()const
		{
			return m_stateName;
		}

		virtual void beginState() = 0;
		virtual void stepState() = 0;
		virtual void endState() = 0;
	private:
		std::string const m_stateName;
	};

	using StatePointer = std::unique_ptr<AState>;
	using StateCreator = std::function<std::unique_ptr<AState>()>;

	template <class State>
	void registerStateType(unsigned int key)
	{
		m_creators.emplace(key, []() { return std::unique_ptr<State>{new State}; });
	}

	void registerStateType(unsigned int key, StateCreator&& creator)
	{
		m_creators.emplace(key, std::move(creator));
	}

	AState& currentState()const
	{
		return *(m_stack.top());
	}

	bool hasState()const
	{
		return !m_stack.empty();
	}

	void push(unsigned int key);
	void change(unsigned int key);
	void pop();
	void clear();
	/*! Should be called at each step. */
	void step();
private:
	using Event = std::function<void()>;

	StatePointer createState(unsigned int key)const;
	void processEvents();
	void pushEvent(Event&& event);
private:
	std::map<unsigned int, StateCreator> m_creators;
	std::stack<StatePointer> m_stack;
	std::queue<Event> m_eventQueue;
};

#endif
