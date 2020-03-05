/*
   esp32 firmware OTA
   Date: December 2018
   Author: Chris Joyce <https://chrisjoyce911/esp32FOTA>
   Purpose: Perform an OTA update from a bin located on a webserver (HTTP & HTTPS)
*/

/* 
 *********************************************************************************
 *                                                                               *
 *                           esp32fota2                                          *
 *                                                                               *
 *  version    Date     Description                                              *
 *    1.0    18/01/20                                                            *
 *    1.0.1  19/01/20   Ajout execHTTPexist et execHTTPSexist					 					 *
 *    1.0.2  06/02/20   Ajout downloadWwwFiles()                                 *         
 *    1.0.3             Ajout downloadWwwFiles                                   *
 *    1.0.4  10/02/20   Ajout UpdateWwwDirectory                                 *
 *                                                                               *
 *********************************************************************************/

#include <DebugConfig.h>
#include "esp_log.h"

#include <HardwareConfig.h>

#ifdef WIFI_DEBUG
#define ARDUINOTRACE_ENABLE 1
#else
#define ARDUINOTRACE_ENABLE 0
#endif

#define ARDUINOTRACE_SERIAL SerialPort
#include <ArduinoTrace.h>

#include "esp32fota2.h"
#include "Arduino.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Update.h>
#include "ArduinoJson.h"
#include <WiFiClientSecure.h>

#include <varioscreenGxEPD.h>

#ifdef HAVE_SDCARD
#include <sdcardHAL.h>
#endif

//************************************
esp32FOTA2::esp32FOTA2(String firwmareType, int firwmareVersion, int firwmareSubVersion, int firwmareBetaVersion)
//************************************
{
    _firwmareType = firwmareType;
    DUMP(_firwmareType);
    _firwmareVersion = firwmareVersion;
    _firwmareSubVersion = firwmareSubVersion;
    _firwmareBetaVersion = firwmareBetaVersion;
    useDeviceID = false;
}

// Utility to extract header value from headers
//************************************
String esp32FOTA2::getHeaderValue(String header, String headerName)
//************************************
{
    return header.substring(strlen(headerName.c_str()));
}

// OTA Logic
//************************************
void esp32FOTA2::execOTA()
//************************************
{

    TRACE();

    screen.ScreenViewReboot("Upgrade");

    WiFiClient client;
    int contentLength = 0;
    bool isValidContentType = false;

    SerialPort.println("Connecting to: " + String(_host));
    // Connect to Webserver
    //    if (client.connect(_host.c_str(), _port))

#ifdef HAVE_SDCARD
    if (NB_WWW_FILES > 0)
    {
        downloadWwwFiles();
    }
#endif

    if (client.connect(_host.c_str(), _port))
    {
        // Connection Succeed.
        // Fecthing the bin
#ifdef WIFI_DEBUG
        SerialPort.println("Fetching Bin: " + String(_bin));
#endif
        // Get the contents of the bin file
        client.print(String("GET ") + _bin + " HTTP/1.1\r\n" +
                     "Host: " + _host + "\r\n" +
                     "Cache-Control: no-cache\r\n" +
                     "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0)
        {
            if (millis() - timeout > 5000)
            {
#ifdef WIFI_DEBUG
                SerialPort.println("Client Timeout !");
#endif
                client.stop();
                return;
            }
        }

        while (client.available())
        {
            // read line till /n
            String line = client.readStringUntil('\n');
            // remove space, to check if the line is end of headers
            line.trim();

            if (!line.length())
            {
                //headers ended
                break; // and get the OTA started
            }

            // Check if the HTTP Response is 200
            // else break and Exit Update
            if (line.startsWith("HTTP/1.1"))
            {
#ifdef WIFI_DEBUG
                SerialPort.println("HTTP LINE");
                SerialPort.println(line);
#endif
                if (line.indexOf("200") < 0)
                {
#ifdef WIFI_DEBUG
                    SerialPort.println("Got a non 200 status code from server. Exiting OTA Update.");
#endif
                    break;
                }
            }

            // extract headers here
            // Start with content length
            if (line.startsWith("Content-Length: "))
            {
                contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
#ifdef WIFI_DEBUG
                SerialPort.println("Got " + String(contentLength) + " bytes from server");
#endif
            }

            // Next, the content type
            if (line.startsWith("Content-Type: "))
            {
                String contentType = getHeaderValue(line, "Content-Type: ");
#ifdef WIFI_DEBUG
                SerialPort.println("Got " + contentType + " payload.");
#endif
                if (contentType == "application/octet-stream")
                {
                    isValidContentType = true;
                }
            }
        }
    }
    else
    {
        // Connect to webserver failed
        // May be try?
        // Probably a choppy network?
#ifdef WIFI_DEBUG
        SerialPort.println("Connection to " + String(_host) + " failed. Please check your setup");
#endif
        // retry??
        // execOTA();
    }

    // Check what is the contentLength and if content type is `application/octet-stream`
#ifdef WIFI_DEBUG
    SerialPort.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));
#endif

    // check contentLength and content type
    if (contentLength && isValidContentType)
    {
        // Check if there is enough to OTA Update
        bool canBegin = Update.begin(contentLength);

        // If yes, begin
        if (canBegin)
        {
#ifdef WIFI_DEBUG
            SerialPort.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
#endif
            // No activity would appear on the Serial monitor
            // So be patient. This may take 2 - 5mins to complete
            size_t written = Update.writeStream(client);

            if (written == contentLength)
            {
#ifdef WIFI_DEBUG
                SerialPort.println("Written : " + String(written) + " successfully");
#endif
            }
            else
            {
#ifdef WIFI_DEBUG
                SerialPort.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
#endif
                // retry??
                // execOTA();
            }

            if (Update.end())
            {
#ifdef WIFI_DEBUG
                SerialPort.println("OTA done!");
#endif
                if (Update.isFinished())
                {
#ifdef WIFI_DEBUG
                    SerialPort.println("Update successfully completed. Rebooting.");
#endif
                    screen.ScreenViewReboot();
                    ESP.restart();
                }
                else
                {
#ifdef WIFI_DEBUG
                    SerialPort.println("Update not finished? Something went wrong!");
#endif
                }
            }
            else
            {
#ifdef WIFI_DEBUG
                SerialPort.println("Error Occurred. Error #: " + String(Update.getError()));
#endif
            }
        }
        else
        {
            // not enough space to begin OTA
            // Understand the partitions and
            // space availability
#ifdef WIFI_DEBUG
            SerialPort.println("Not enough space to begin OTA");
#endif
            client.flush();
        }
    }
    else
    {
#ifdef WIFI_DEBUG
        SerialPort.println("There was no content in the response");
#endif
        client.flush();
    }
}

//************************************
bool esp32FOTA2::execHTTPexist()
//************************************
{
    TRACE();
    String useURL;

    // if (useDeviceID)
    // {
    //     // String deviceID = getDeviceID() ;
    //     useURL = checkURL + "/" + getDeviceID();
    // }
    // else
    // {
    //     useURL = checkURL;
    // }

    useURL = checkURL + "/" + getDeviceID();

    WiFiClient client;
    _port = 80;

#ifdef WIFI_DEBUG
    SerialPort.println("Getting HTTP");
    SerialPort.println(useURL);
    SerialPort.println("------");
#endif
    if ((WiFi.status() == WL_CONNECTED))
    { //Check the current connection status

        HTTPClient http;

        http.begin(useURL);        //Specify the URL
        int httpCode = http.GET(); //Make the request

        if (httpCode > 0)
        { //Check is a file was returned

#ifdef WIFI_DEBUG
            SerialPort.printf("[HTTP] GET... code: %d\n", httpCode);
#endif

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                String payload = http.getString();
#ifdef WIFI_DEBUG
                SerialPort.println(payload);
#endif
            }

            http.end(); //Free the resources
            return true;
        }
        else
        {
#ifdef WIFI_DEBUG
            SerialPort.println("Error on HTTP request");
            SerialPort.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
#endif
            http.end(); //Free the resources
            return false;
        }
    }
    return false;
}

//************************************
uint8_t esp32FOTA2::execHTTPcheck(bool betaVersion)
//************************************
{
    TRACE();
    String useURL;

    // if (useDeviceID)
    // {
    //     // String deviceID = getDeviceID() ;
    //     useURL = checkURL + "?id=" + getDeviceID();
    // }
    // else
    // {
    //     useURL = checkURL;
    // }

    useURL = checkURL + "/" + getDeviceID();

    WiFiClient client;
    _port = 80;

#ifdef WIFI_DEBUG
    SerialPort.println("Getting HTTP");
    SerialPort.println(useURL);
    SerialPort.println("------");
#endif
    if ((WiFi.status() == WL_CONNECTED))
    { //Check the current connection status

        HTTPClient http;

        http.begin(useURL);        //Specify the URL
        int httpCode = http.GET(); //Make the request

        if (httpCode == 200)
        { //Check is a file was returned

            String payload = http.getString();

            int str_len = payload.length() + 1;
            char JSONMessage[str_len];
            payload.toCharArray(JSONMessage, str_len);

#ifdef WIFI_DEBUG
            SerialPort.print("Parsing size : ");
            SerialPort.println(str_len);
            //SerialPort.println(JSONMessage);
#endif

            StaticJsonDocument<1300> JSONDocument; //Memory pool
            DeserializationError err = deserializeJson(JSONDocument, JSONMessage);

            if (err)
            { //Check for errors in parsing
#ifdef WIFI_DEBUG
                SerialPort.println("Parsing failed");
#endif
                delay(5000);
                return MAJ_ERROR;
            }

            String tmp;
            tmp = _firwmareType;
            if (betaVersion)
                tmp = tmp + "b";
#ifdef WIFI_DEBUG
            SerialPort.print("Update Firmware : ");
            SerialPort.println(tmp);
#endif

            if (JSONDocument.containsKey(tmp))
            {
                JsonObject JSONDocumentUpdate = JSONDocument[tmp];

                //const char *pltype = JSONDocument["type"];
                int plversion = JSONDocumentUpdate["version"];
                UpdateVersion = plversion;

                int plsubversion = JSONDocumentUpdate["subversion"];
                UpdateSubVersion = plsubversion;

                int plbetaversion = JSONDocumentUpdate["betaversion"];
                UpdateBetaVersion = plbetaversion;

                const char *plhost = JSONDocumentUpdate["host"];
                _port = JSONDocumentUpdate["port"];

                NB_WWW_FILES = 0;

                if (JSONDocumentUpdate.containsKey("www"))
                {

#ifdef WIFI_DEBUG
                    SerialPort.println("la section du fichier json contient la clé www");
#endif
                    JsonArray myArray = JSONDocumentUpdate["www"].as<JsonArray>();

                    for (JsonVariant myValue : myArray)
                    {
#ifdef WIFI_DEBUG
                        Serial.println(myValue.as<char *>());
#endif
                        _wwwfiles[NB_WWW_FILES] = myValue.as<char *>();
                        NB_WWW_FILES++;
                    }
                }

#ifdef WIFI_DEBUG
                SerialPort.print("Version : ");
                SerialPort.println(plversion);

                SerialPort.print("Sub Version : ");
                SerialPort.println(plsubversion);

                SerialPort.print("Beta Version : ");
                SerialPort.println(plbetaversion);

                SerialPort.print("Port : ");
                SerialPort.println(_port);
#endif

                const char *plbin = JSONDocumentUpdate["bin"];

                String jshost(plhost);
                String jsbin(plbin);

                _host = jshost;
                _bin = jsbin;

#ifdef WIFI_DEBUG
                SerialPort.print("Host : ");
                SerialPort.println(_host);

                SerialPort.print("Bin : ");
                SerialPort.println(_bin);

                SerialPort.print("Firmware Version : ");
                SerialPort.println(_firwmareVersion);

                SerialPort.print("Firmware Sub Version : ");
                SerialPort.println(_firwmareSubVersion);

                SerialPort.print("Firmware Beta Version : ");
                SerialPort.println(_firwmareBetaVersion);
#endif

                if (plversion > _firwmareVersion)
                {
                    http.end(); //Free the resources
                    return MAJ_AVAILABLE;
                }
                else if ((plversion == _firwmareVersion) && (plsubversion > _firwmareSubVersion))
                {
                    http.end(); //Free the resources
                    return MAJ_AVAILABLE;
                }
                else if ((plversion == _firwmareVersion) && (plsubversion == _firwmareSubVersion) && (plbetaversion > _firwmareBetaVersion))
                {
                    //ce cas ne match que pour les beta
                    http.end(); //Free the resources
                    return MAJ_AVAILABLE;
                }

                else
                {
#ifdef WIFI_DEBUG
                    SerialPort.println("MAJ MAJ_NOTAVAILABLE");
#endif
                    http.end(); //Free the resources
                    return MAJ_NOTAVAILABLE;
                }
            }
            else
            {
#ifdef WIFI_DEBUG
                if (betaVersion)
                {
                    SerialPort.println("Erreur info firmware non trouvé dans la beta");
                }
                else
                {
                    SerialPort.println("Erreur info firmware non trouvé dans la non beta");
                }
#endif
                http.end(); //Free the resources
                return MAJ_ERROR;
            }
        }

        else
        {
#ifdef WIFI_DEBUG
            SerialPort.println("Error on HTTP request");
#endif
            http.end(); //Free the resources
            return MAJ_ERROR;
        }

        http.end(); //Free the resources
        return MAJ_ERROR;
    }
    return MAJ_ERROR;
}

//************************************
String esp32FOTA2::getDeviceID()
//************************************
{
    char deviceid[21];
    uint64_t chipid;
    chipid = ESP.getEfuseMac();
    sprintf(deviceid, "%" PRIu64, chipid);
    String thisID(deviceid);
    return thisID;
}

// Force a firmware update regartless on current version
//************************************
bool esp32FOTA2::forceUpdate(String firwmareHost, int firwmarePort, String firwmarePath)
//************************************
{
    _host = firwmareHost;
    _bin = firwmarePath;
    _port = firwmarePort;
    execOTA();

    return true;
}

//************************************
String esp32FOTA2::getHTTPVersion()
//************************************
{
    TRACE();
    String output;

    output = "{\n";

    output += "\"Firmware\":{\n";
    output += "\"type\": " + String(VARIOSCREEN_SIZE) + ",\n";
    output += "\"version\": " + String(_firwmareVersion) + ",\n";
    output += "\"subversion\": " + String(_firwmareSubVersion) + ",\n";
    output += "\"betaversion\": " + String(_firwmareBetaVersion) + "\n}";

    int8_t updatedNeeded = execHTTPcheck(false);

    if (updatedNeeded > 0)
    {
#ifdef WIFI_DEBUG
        SerialPort.println("************** Version Stable : Mise à jour disponible *****************");
        SerialPort.println(updatedNeeded);
        SerialPort.print("Version : ");
        SerialPort.print(UpdateVersion);
        SerialPort.print(".");
        SerialPort.println(UpdateSubVersion);
#endif

        output += ",\n\"" + _firwmareType + "\":{\n";
        output += "\"type\": " + String(VARIOSCREEN_SIZE) + ",\n";
        output += "\"version\": " + String(UpdateVersion) + ",\n";
        output += "\"subversion\": " + String(UpdateSubVersion) + ",\n";
        output += "\"betaversion\": 0\n}";
    }

    updatedNeeded = execHTTPcheck(true);
    if (updatedNeeded > 0)
    {
#ifdef WIFI_DEBUG
        SerialPort.println("***************** Version Beta : Mise à jour disponible *********************");
        SerialPort.print("Version : ");
        SerialPort.print(UpdateVersion);
        SerialPort.print(".");
        SerialPort.print(UpdateSubVersion);
        SerialPort.print("b");
        SerialPort.println(UpdateBetaVersion);
#endif

        output += ",\n\"" + _firwmareType + "b\":{\n";
        output += "\"type\": " + String(VARIOSCREEN_SIZE) + ",\n";
        output += "\"version\": " + String(UpdateVersion) + ",\n";
        output += "\"subversion\": " + String(UpdateSubVersion) + ",\n";
        output += "\"betaversion\": " + String(UpdateBetaVersion) + "\n}";
    }

    output += "\n}";

    return output;
}

//************************************
void esp32FOTA2::downloadWwwFiles()
//************************************
{
    TRACE();

#ifdef WIFI_DEBUG
    SerialPort.println("[HTTP] Debut méthode downloadWwwFiles");
#endif
    // File system object.
    // Directory file.
    SdFile root;

    String newPath = "wwwnew";
    if (!SDHAL_SD.mkdir(newPath.c_str()))
    {
#ifdef WIFI_DEBUG
        SerialPort.println("[HTTP] Impossible de créer le répertoire wwwnew");
#endif
        return;
    }

    uint8_t i;

    for (i = 0; i < NB_WWW_FILES; i++)
    {
        //telechargement des fichiers 1 par un
        HTTPClient http;
        String myfilename = newPath + "/" + _wwwfiles[i].substring(_wwwfiles[i].lastIndexOf("/") + 1);
        http.begin(_host.c_str(), _port, _wwwfiles[i]);

        // start connection and send HTTP header
        int httpCode = http.GET();
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
#ifdef WIFI_DEBUG
            SerialPort.println("[HTTP] GET... code: " + String(httpCode));
#endif

            // file found at server
            if (httpCode == HTTP_CODE_OK)
            {

                SdFile myFile;
                boolean tmpReturn = false;
                tmpReturn = myFile.open(myfilename.c_str(), O_WRONLY | O_CREAT);

                if (!tmpReturn)
                {
#ifdef WIFI_DEBUG
                    SerialPort.print("Impossible de créer le fichier : ");
                    SerialPort.println(myfilename);
#endif
                    return;
                }

#ifdef WIFI_DEBUG
                SerialPort.print("[HTTP] Début écriture");
                SerialPort.println(myfilename);
#endif
                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = http.getSize();

                // create buffer for read
                uint8_t buff[128] = {0};

                // get tcp stream
                WiFiClient *stream = http.getStreamPtr();

                // read all data from server
                while (http.connected() && (len > 0 || len == -1))
                {
                    // get available data size
                    size_t size = stream->available();

                    if (size)
                    {
                        // read up to 128 byte
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // ICI l'ecriture dans le fichier
                        //USE_SERIAL.write(buff, c);
                        myFile.write(buff, c);
                        myFile.flush();

                        if (len > 0)
                        {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                myFile.close();
#ifdef WIFI_DEBUG
                SerialPort.print("[HTTP] Fin écriture");
                SerialPort.println(myfilename);
#endif

#ifdef WIFI_DEBUG
                SerialPort.println("[HTTP] connection closed or file end.");
#endif
            }
        }
        else
        {
#ifdef WIFI_DEBUG
            SerialPort.println("[HTTP] GET... failed, error:" + String(http.errorToString(httpCode).c_str()));
#endif
        }

        http.end();
    }
}

//************************************
bool esp32FOTA2::UpdateWwwDirectory()
//************************************
{

    TRACE();

#ifdef WIFI_DEBUG
    SerialPort.println("[HTTP] Debut méthode updateWwwDirectory");
#endif
    // File system object.
    // Directory file.
    SdFile root;

    String newPath = "wwwnew";
    //		String oldPath = "";
    DUMP(newPath.c_str());
    if (SDHAL_SD.exists(newPath.c_str()))
    {
#ifdef WIFI_DEBUG
        SerialPort.println("[HTTP] le dossier wwwnew existe");
#endif

        // Traitement du fichier wwwnew

        newPath = "wwwold";
        if (SDHAL_SD.exists(newPath.c_str()))
        {
#ifdef WIFI_DEBUG
            SerialPort.println("[HTTP] le dossier wwwold existe");
#endif

            SdFile root;
            SdFile file;

            if (!root.open("wwwold"))
            {
#ifdef WIFI_DEBUG
                SerialPort.println("[HTTP] impossible d'ouvrir le dossier wwwold");
#endif
                return false;
            }
            // Open next file in root.
            // Warning, openNext starts at the current directory position
            // so a rewind of the directory may be required.
            while (file.openNext(&root, O_RDONLY))
            {

                char fBuffer[32];
                file.getName(fBuffer, 30);
                String SBuffer = "wwwold/";
                SBuffer += String(fBuffer);

#ifdef WIFI_DEBUG
                SerialPort.print("[HTTP] suppression du fichier : ");
                SerialPort.println(SBuffer);
#endif

                file.close();

                if (!SDHAL_SD.remove(SBuffer.c_str()))
                {
#ifdef WIFI_DEBUG
                    SerialPort.println("[HTTP] le fichier n'a pas pu être supprimé");
#endif
                }
            }

            //Effacement du repertoire wwwold

            if (!SDHAL_SD.rmdir(newPath.c_str()))
            {
#ifdef WIFI_DEBUG
                SerialPort.println("[HTTP] le dossier wwwold n'a pas pu être supprimé");
#endif
                return false; //Pas de mise à jour
            }
        }

        // rename "www" into "wwwold"
        if (!SDHAL_SD.rename("www", "wwwold"))
        {
#ifdef WIFI_DEBUG
            SerialPort.println("[HTTP] le dossier www ne peut être renomé en wwwold");
#endif
            return false; //Pas de mise à jour
        }

        // rename "www" into "wwwold"
        if (!SDHAL_SD.rename("wwwnew", "www"))
        {
#ifdef WIFI_DEBUG
            SerialPort.println("[HTTP] le dossier wwwnew ne peut être renomé en www");
#endif
            return false; //Pas de mise à jour
        }

        return true; // Mise à jour site web OK
    }
    else
    {
#ifdef WIFI_DEBUG
        SerialPort.println("[HTTP] le dossier wwwnew n'existe pas");
#endif
        return false; //Pas de mise à jour
    }
    return false; //Pas de mise à jour
}

//************************************
String esp32FOTA2::getHTTPSVersion()
//************************************
{
    String output;
    return output;
}
