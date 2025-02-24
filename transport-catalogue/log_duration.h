#pragma once

#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X ## Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)


class LogDuration {
public:
    // заменим имя типа std::chrono::steady_clock
    // с помощью using для удобства
    using Clock = std::chrono::steady_clock;

    LogDuration() {
    }
    
    explicit LogDuration(std::string text)
    :announcement(text)
    {
        
    }

    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        if(announcement.empty()){
            std::cerr << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
        }
        else{
            std::cerr << announcement << ": " << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
        }
    }

private:
    const Clock::time_point start_time_ = Clock::now();
    std::string announcement;
};