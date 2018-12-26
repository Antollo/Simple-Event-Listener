#include <iostream>
#include <string>
#include <cstdlib>
#include "eventListener.h"

std::atomic_int i(0);

int main()
{
    eventEmitter<std::string> refreshEmitter;

	auto exitListener = makeEventListener([](){ 
        return i == 10;
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
        i++;
    }
    return 0;
}