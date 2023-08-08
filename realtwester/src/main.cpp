#include "RealTwester.h"

int main() {
    auto twester = new REALTWESTER::RealTwester();
//    twester->tryToConnectToMQTTAPI();
    twester->tryDetectingTheProbe();
    twester->startService();
}
