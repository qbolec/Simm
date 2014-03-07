#ifndef TIMER_H
#define TIMER_H

#include <ctime>
#include <iostream>

class Timer
{
    typedef double TimerType;

    clock_t clocksFromBeginning;
    clock_t clocksFromCheckpoint;
    clock_t clockPrev;

    bool paused;

    TimerType convert(clock_t c) const {
        return (TimerType)c/((TimerType)CLOCKS_PER_SEC);
    }

public:
    Timer() { }

    void start() {
        paused = false;
        clocksFromBeginning = 0;
        clocksFromCheckpoint = 0;
        clockPrev = clock();
    }

    void pause() {
        if (paused) {
            std::cerr << "The timer has been paused!\n";
            return ;
        }
        clock_t now = clock();
        clocksFromCheckpoint += now-clockPrev;
        paused = true;
    }
    void unpause() {
        if (!paused) {
            std::cerr << "The timer has not been paused!\n";
            return ;
        }
        paused = false;
        clockPrev = clock();
    }
    void checkpoint() {
        clock_t now = clock();
        // get rid of previous checkpoint
        clocksFromBeginning += clocksFromCheckpoint;
        clocksFromCheckpoint = 0;
        // clockPrev = now
        clocksFromBeginning += now - clockPrev;
        clockPrev = now;
    }
    TimerType sinceBeginning() const {
        clock_t now = clock();
        clock_t total = clocksFromBeginning + clocksFromCheckpoint + now - clockPrev;
        return convert(total);
    }
    TimerType sinceCheckpoint() const {
        clock_t now = clock();
        clock_t total = clocksFromCheckpoint + now - clockPrev;
        return convert(total);
    }
};

#endif // TIMER_H
