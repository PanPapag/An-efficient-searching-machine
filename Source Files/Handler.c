#include <stdio.h>
#include <signal.h>

#include "../Headers/Handler.h"

extern volatile sig_atomic_t signal_received;

void sig_handler()
{
    signal_received++;
}
