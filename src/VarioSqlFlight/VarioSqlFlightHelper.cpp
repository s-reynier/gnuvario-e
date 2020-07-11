#include <Arduino.h>
#include <VarioSqlFlightHelper.h>

void VarioSqlFlightHelper::init(int16_t vlimit, int16_t voffset)
{
    limit = vlimit;
    offset = voffset;

    //ruse pour contourner le pb de mémoire sur sqlite
    limitTemp = limit;
    offsetTemp = offset;
    firstLine = true;
    isQueryInit = false;
}

void VarioSqlFlightHelper::feedBuffer()
{
    String unvol = "";
    if (limit > 0)
    {
        if (!isQueryInit)
        {
            if (limit > limitMax)
            {
                limitTemp = limitMax;
            }
            else
            {
                limitTemp = limit;
            }

            if (!varioSqlFlight.initGetFlightsQuery(limitTemp, offsetTemp))
            {
                //erreur requete
                limit = 0;
                return;
            }
            isQueryInit = true;
            unvol = varioSqlFlight.getNextFlight();
            if (unvol == "")
            {
                //pas la peine de continuer, on est a la fin
                limit = 0;
                return;
            }

            isQueryInit = true;
        }
        else
        {
            unvol = varioSqlFlight.getNextFlight();
        }

        if (unvol != "")
        {
            if (firstLine)
            {
                myRingBuffer.push('[');
                firstLine = false;
            }
            else
            {
                myRingBuffer.push(',');
            }

            for (int i = 0; i < unvol.length(); i++)
            {
                myRingBuffer.push(unvol[i]);
            }
        }
        else
        {
            isQueryInit = false;
            offsetTemp = offsetTemp + limitMax;
            limit = limit - limitMax;
            feedBuffer();
        }
    }
}

uint8_t VarioSqlFlightHelper::readData(uint8_t *buffer, size_t maxLength)
{
    size_t bufLength;
    uint8_t nb = 0;

    if (limit <= 0)
    {
        return 0;
    }

    //si le buffer est vide, on tente de le remplir
    if (myRingBuffer.isEmpty())
    {
        feedBuffer();
    }

    //si le buffer est toujours vide, on renvoie 0 pour terminer
    if (myRingBuffer.isEmpty())
    {
        return 0;
    }

    size_t max = (ESP.getFreeHeap() / 3) & 0xFFE0;

    if (myRingBuffer.size() > maxLength)
    {
        bufLength = maxLength;
    }
    else
    {
        bufLength = myRingBuffer.size();
    }
    if (bufLength > max)
    {
        bufLength = max;
    }

    Serial.print("Buffer length: ");
    Serial.println(bufLength);

    char tBuf[bufLength];

    // si le buffer contient des données, on en envoie jusqu'a maxLength
    while (nb < bufLength && (!myRingBuffer.isEmpty()))
    {
        myRingBuffer.pop(tBuf[nb]);
        nb++;
    }

    if (nb > 0)
    {
        memcpy(buffer, tBuf, nb);
    }

    return nb;
}