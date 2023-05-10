#include "digitwester.h"

int main() {
    auto twester = new DIGITWESTER::Digitwester();
    twester->tryConnectingToAgila();
    twester->tryDetectingTheProbe();
    twester->startService();
}
