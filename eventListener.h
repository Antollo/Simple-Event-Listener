#include <thread>
#include <mutex>
#include <atomic>

class eventListenerBase
{
protected:
	static std::mutex eventMutex;
};

template<class P, class H>
class eventListener : public eventListenerBase
{
public:
	eventListener(P predicate, H handle) : p(predicate), h(handle), stop(false),th([this]() -> void
	{
		bool last = false, now = false;
		while (true)
		{
			if (stop) return;
			eventListenerBase::eventMutex.lock();
			now = p();
			eventListenerBase::eventMutex.unlock();
			if (!last && now)
			{
				eventListenerBase::eventMutex.lock();
				h();
				eventListenerBase::eventMutex.unlock();
			}
			last = now;
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}
	})
    { }
	eventListener(eventListener&& another)
		: p(another.p), h(another.h), stop(false)
	{
		std::swap(th, another.th);
	}
	~eventListener()
	{
		stop = true;
		if (th.joinable()) th.join();
	}
private:
	P p;
	H h;
	std::atomic_bool stop;
	std::thread th;
};

template <class E>
class eventEmitter : public eventListenerBase
{
public:
    eventEmitter() : listenerCounter(0), actualListenerCounter(0), cnd(false), stop(false) {}
    template <class A, class B>
    friend class eventListener;
    void emit(const E& event)
    {
		if (consumed.joinable()) consumed.join();
        e = event;
        cnd = true;
		consumed = std::thread([this]() -> void 
		{
			while (listenerCounter != actualListenerCounter && !stop);
			actualListenerCounter = 0;
			cnd = false;
		});
    }
	~eventEmitter()
	{
		stop = true;
		if (consumed.joinable()) consumed.join();
		std::this_thread::sleep_for(std::chrono::nanoseconds(1));
	}
private:
    E e;
    std::atomic_bool cnd;
	std::atomic_bool stop;
    std::atomic_int listenerCounter, actualListenerCounter;
	std::thread consumed;
};

template<class E, class H>
class eventListener<eventEmitter<E>, H> : public eventListenerBase
{
public:
	eventListener(eventEmitter<E>& emmiter, H handle) : m(emmiter), h(handle), stop(false),th([this]() -> void
	{
		while (true)
		{
    		if (m.cnd)
			{
				eventMutex.lock();
				h(m.e);
				eventMutex.unlock();
				m.actualListenerCounter++;
				while (m.cnd);
			}
			if (stop || m.stop) return;
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}
	})
    {
		m.listenerCounter++;
	}
	eventListener(eventListener&& another)
		: m(another.m), h(another.h), stop(false)
	{
		std::swap(th, another.th);
	}
	~eventListener()
	{
		stop = true;
		if (th.joinable()) th.join();
		m.listenerCounter--;
	}
private:
	H h;
	eventEmitter<E>& m;
	std::atomic_bool stop;
	std::thread th;

};

template<class P, class H>
eventListener<P, H> makeEventListener(P predicate, H handle)
{
	return {predicate, handle};
}

template<class E, class H>
eventListener<eventEmitter<E>, H> makeEventListener(eventEmitter<E>& emmiter, H handle)
{
	return {emmiter, handle};
}