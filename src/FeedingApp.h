#pragma once

#include <display_interface.h>
#include <loadcell_interface.h>
#include <stepper_interface.h>
#include <time_interface.h>

class FeedingApp
{
public:
    FeedingApp(display_interface display, loadcell_interface loadcell, stepper_interface stepper, time_interface time);
    bool feed(bool state);
    float readFeedAmount();
    bool feedLogs();

private:
    display_interface &_display;
    loadcell_interface &_loadcell;
    stepper_interface &_stepper;
    time_interface &_time;
};