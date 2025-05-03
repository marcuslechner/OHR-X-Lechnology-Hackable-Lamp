#ifndef APP_TIMER_HPP
#define APP_TIMER_HPP

class Timer
{
public:
    Timer(unsigned long intervalMs = 1000, bool startNow = true);

    void start();
    void stop();
    void reset();
    bool expired();
    void setInterval(unsigned long intervalMs);
    bool isRunning() const;

private:
    unsigned long _interval;
    unsigned long _lastTime;
    bool _enabled;
};


#endif // APP_TIMER_HPP