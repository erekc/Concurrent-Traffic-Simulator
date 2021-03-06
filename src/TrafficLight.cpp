#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

/* */
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); });
    T message = std::move(_queue.back());
    _queue.pop_back();
    return message;
}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> guard(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}


void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    // while (true){
    //     TrafficLightPhase phase = _messageQueue.receive();
    //     //std::cout << "Traffic Light #" << _id << " phase: " << phases[phase] << std::endl;
    //     if (phase == TrafficLightPhase::green){
    //         //long timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    //         //std::cout << "Traffic Light #" << _id << " green after " << timeDifference << " milliseconds." << std::endl;
    //         break;
    //     }
    // }

    // std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    // std::cout << "Traffic Light #" << _id << " red and waiting for green." << std::endl;
    while (_messageQueue.receive() == TrafficLightPhase::red){}
    // long timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    // std::cout << "Traffic Light #" << _id << " green after " << timeDifference << " milliseconds." << std::endl;
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_int_distribution<int> distribution(4000,6000);

    int duration = distribution(generator);
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    while (true){
        int timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
        if (timeDifference > duration){
            if (_currentPhase == TrafficLightPhase::green){
                _currentPhase = TrafficLightPhase::red;
            }
            else {
                _currentPhase = TrafficLightPhase::green;
            }
            start = std::chrono::system_clock::now();
            duration = distribution(generator);
        }
        _messageQueue.send(std::move(_currentPhase));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
