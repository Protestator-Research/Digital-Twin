#include <iostream>
#include "digitwester.h"

#include <curl/curl.h>


void digitwester(){
    CURL* curl = curl_easy_init();
    if(curl) {
        CURLcode red;
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/agila-server");
        red = curl_easy_perform(curl);
        std::cout<<red<<std::endl;
        if(red == CURLE_OK)
        {
            std::cout<<"Connection Successfully established"<<std::endl;
        }
        curl_easy_cleanup(curl);
    }
}

DIGITWESTER::Digitwester::Digitwester() {

}

void DIGITWESTER::Digitwester::tryDetectingTheProbe() {

}

void DIGITWESTER::Digitwester::tryConnectingToAgila() {

}

void DIGITWESTER::Digitwester::startService() {

}
