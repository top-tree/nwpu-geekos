#include "libuser.h"

int main(int argc, char **argv)
{
    int gate = Create_Semaphore("short_start", 0);

    assert(gate >= 0);
    assert(P(gate) == 0);
    V(gate);
    Destroy_Semaphore(gate);
    return 0;
}
