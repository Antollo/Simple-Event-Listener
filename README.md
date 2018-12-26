# Simple Event Listener

[![https://ci.appveyor.com/api/projects/status/github/Antollo/simple-event-listener?svg=true]( https://ci.appveyor.com/api/projects/status/github/Antollo/simple-event-listener?svg=true)](https://ci.appveyor.com/project/Antollo/simple-event-listener)

## Introduction

Simple Event Listener is a tiny library providing event listener functionality in C++ (C++ 14 required). Pros:

- 2 files only (one header and one source file)
- no external dependencies (only standard C++)
- uses templates

## 1. Predicate based `eventListener`

Convenient way to wrap old-fashioned `bool` returning functions to use them as event emitters.

```cpp
template<typename P, typename H>
class eventListener : public eventListenerBase
```

It's constructor is taking:

- convertible to `bool` returning taking no parameters `predicate` invocable
- taking no parameters `handle` invocable

but I recommend you to use `makeEventListener` function (taking same parameters as constructor) and `auto` keyword because they will deduce `P` and `H` even if `P` and `H` are lambdas (lambdas are highly recommended):

```cpp
template<typename P, typename H>
eventListener<P, H> makeEventListener(P predicate, H handle);
```

When value of `predicate()` is true `handle()` is called.

#### Example:

_(Windows specific key press detection, paste `std::this_thread::sleep_for(std::chrono::seconds(5));` after following snippet when you are testing it to give your programm some time for event listening.)_

```cpp
auto listenerOfKeyA = makeEventListener([]{
    return GetKeyState('A') & 0x8000;
}, []{
    std::cout << "A was pressed\n";
});
```

## 2. Emitter based `eventListener`

This specialization of `eventListener` give you a little bit higher level of abstraction. At first you have to construct `eventEmitter`:

```cpp
template <typename E>
class eventEmitter : public eventListenerBase
```

It constructor has no parameters. Then you could attach `eventListener` to emitter, I recommend you to use `makeEventListener` function (taking reference to emitter and handle) and `auto` keyword because they will deduce `E` and `H` even if `H` is lambda function:

```cpp
template<typename E, typename H>
eventListener<eventEmitter<E>, H> makeEventListener(eventEmitter<M>& emmiter, H handle);
```

Then you could use `eventEmitter`'s `void emit(const E& event)` member function to invoke `handle(event)` (`event` is copy of `event` passed to `emit`) in all attached listeners.

#### Example:

```cpp
eventEmitter<char> m;
auto b = makeEventListener(m, [](char ev){std::cout << ev << "b\n";});
auto c = makeEventListener(m, [](char ev){std::cout << ev << "c\n";});
auto d = makeEventListener(m, [](char ev){std::cout << ev << "d\n";});
auto e = makeEventListener(m, [](char ev){std::cout << ev << "d\n";});
auto f = makeEventListener(m, [](char ev){std::cout << ev << "f\n";});
std::this_thread::sleep_for(std::chrono::seconds(1));
m.emit('x');
std::this_thread::sleep_for(std::chrono::seconds(1));
m.emit('y');
std::this_thread::sleep_for(std::chrono::seconds(1));
m.emit('z');
std::this_thread::sleep_for(std::chrono::seconds(5));
```

## Safeness

Only one `handle` or `predicate` of all instances of `eventListener` is executed at the moment (that means predicates, emitters and event handlers are thread-safe with each other, but they are not thread-safe with main thread, that's why it's more convenient to move all tasks to event handlers). Every call of `emit` will wait until previous `emit` event is consumed by all listening to this `eventEmitter` listeners. 

## Big example:

Suppose we have to refresh something once a second or when user press `R` key.

```cpp
#include <iostream>
#include <string>
#include <cstdlib>
#include "eventListener.h"
#include "Windows.h"

int main()
{
    eventEmitter<std::string> refreshEmitter;

    auto refreshKeyListener = makeEventListener([](){ 
        return GetKeyState('R') & 0x8000; 
    }, [&refreshEmitter](){
        refreshEmitter.emit("key");
    });

    auto exitKeyListener = makeEventListener([](){ 
        return GetKeyState('E') & 0x8000; 
    }, [](){
        std::exit(0);
    });

    auto refreshListener = makeEventListener(refreshEmitter, [](std::string ev){
        std::cout << "Refresh: " << ev << std::endl;
    });

    while (true)
    {
    	std::this_thread::sleep_for(std::chrono::seconds(1));
    	refreshEmitter.emit("time");
    }
    return 0;
}
```

## How to include in your project

1. Copy `eventListener.h` and `eventListener.cpp` to your project directory.
2. Add `#include "eventListener.h"`.
