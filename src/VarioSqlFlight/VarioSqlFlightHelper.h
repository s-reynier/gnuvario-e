#ifndef VARIO_SQL_FLIGHT_HELPER_H
#define VARIO_SQL_FLIGHT_HELPER_H

#include <Arduino.h>
#include <RingBuf.h>
#include <VarioSqlFlight.h>

class VarioSqlFlightHelper
{
public:
    void init(int16_t vlimit, int16_t voffset);
    uint16_t readData(uint8_t *buffer, size_t maxLength);

private:
    VarioSqlFlight varioSqlFlight;
    int16_t offset;
    int16_t limit;
    int16_t limitTemp;
    int16_t offsetTemp;
    int16_t limitMax = 10000;
    RingBuf<char, 2048> myRingBuffer;
    boolean firstLine;
    boolean isQueryInit;
    void feedBuffer();
};
#endif