//
// Created by moritzh on 19.05.23.
//

#ifndef ENERGYPROBEDRIVER_DEFINITIONS_H
#define ENERGYPROBEDRIVER_DEFINITIONS_H

#define MAX_EPROBE_CHANNELS     3
#define MAX_DAQ_CHANNELS        40  // A caiman 'channel' includes V+I

#define MAX_CHANNELS MAX_DAQ_CHANNELS
#define MAX_FIELDS_PER_CHANNEL 3
#define MAX_FIELDS MAX_CHANNELS * MAX_FIELDS_PER_CHANNEL
#define MAX_COUNTERS MAX_FIELDS * 2 // one for peak, one for average
#define MAX_STRING_LEN 80
#define MAX_DESCRIPTION_LEN 400

#define CAIMAN_VERSION 850

#ifdef WIN32
#include <stdio.h>
#define PATH_SEPARATOR '\\'
#define CAIMAN_PATH_MAX MAX_PATH
#if !defined(_MSC_VER) || _MSC_VER < 1900
#define snprintf _snprintf
#endif
#else
#include <limits.h>
#define PATH_SEPARATOR '/'
#define CAIMAN_PATH_MAX PATH_MAX
#endif

// Fields
static const char * const field_title_names[] = { "", "Power", "Voltage", "", "Current" };
enum
{
    POWER = 1,
    VOLTAGE = 2,
    CURRENT = 4
};


#endif //ENERGYPROBEDRIVER_DEFINITIONS_H
