#include "RealTwester.h"

int main() {
    auto twester = new REALTWESTER::RealTwester();
    twester->tryConnectingToAgila();
    twester->tryDetectingTheProbe();
    twester->startService();
}
