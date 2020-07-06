#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "SD.h"
#include <Update.h>
#include "VarioWebHandler.h"


File uploadFile;


AsyncResponseStream *VarioWebHandler::handleListFlights(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");

    String path = "/vols";

    File dir;
    dir = SD.open((char *)path.c_str(), FILE_READ);

    if (!dir.isDirectory())
    {
        dir.close();
    }
    dir.rewindDirectory();

    response->print("[");
    for (int cnt = 0; true; ++cnt)
    {
        File entry;
        entry = dir.openNextFile(FILE_READ);
        if (!entry)
        {
            break;
        }

        String output;
        if (cnt > 0)
        {
            output = ',';
        }

        String fsize = "";

        if (!entry.isDirectory())
        {
            int bytes = entry.size();
            fsize = getFileSizeStringFromBytes(bytes);

            output += "{\"type\":\"";
            output += (entry.isDirectory()) ? "dir" : "file";
            output += "\",\"name\":\"";
            output += entry.name();
            output += "\",\"size\":\"";
            output += fsize;
            output += "\"";
            output += "}";

            response->print(output);
        }

        entry.close();
    }
    response->print("]");

    //correction bug chunk transfer webserver
    // server.sendContent("");
    dir.close();

    return response;
}

AsyncResponseStream *VarioWebHandler::handlePrintDirectory(AsyncWebServerRequest *request)
{
    /***********************************/
#ifdef WIFI_DEBUG
    SerialPort.println("handlePrintDirectory");
#endif

    AsyncResponseStream *response = request->beginResponseStream("application/json");

    String path;
    boolean isRecursive = true;

    if (request->hasParam("dir"))
    {
        AsyncWebParameter *p = request->getParam("dir");
        path = p->value().c_str();
    }
    else
    {
        path = "/";
    }

    if (request->hasParam("dir"))
    {
        isRecursive = false;
    }

#ifdef WIFI_DEBUG
    SerialPort.print("dir : ");
    SerialPort.println((char *)path.c_str());
#endif

    File dir;
    dir = SD.open((char *)path.c_str(), FILE_READ);
    dir.rewindDirectory();

    if (!dir.isDirectory())
    {
#ifdef WIFI_DEBUG
        SerialPort.println("Not directory");
#endif

        dir.close();
        //response->setCode(404);
        request->send(500, "text/plain", "NOT DIR");

        return response;
    }

#ifdef WIFI_DEBUG
    SerialPort.println("[");
#endif

    response->print("[");

    String output;
    output += "{\"type\":\"";
    output += "dir";
    output += "\",\"name\":\"";
    output += path;
    output += "\",\"size\":\"";
    output += "na";
    output += "\"";

    output += ",\"contents\" :[";
#ifdef WIFI_DEBUG
    //SerialPort.println(output);
#endif
    response->print(output);
    printDirectoryRecurse(response, path, isRecursive);
    output = "]";

    output += "}";
    response->print(output);
    // printDirectoryRecurse(path);

    response->print("]");

    //correction bug chunk transfer webserver
    // server.sendContent("");
    // server.client().stop();
#ifdef WIFI_DEBUG
    SerialPort.println("]");
#endif
    dir.close();

    return response;
}

/***********************************/
void VarioWebHandler::printDirectoryRecurse(AsyncResponseStream *response, String path, boolean isRecursive)
/***********************************/
{

    File dir;
    dir = SD.open((char *)path.c_str(), FILE_READ);
    dir.rewindDirectory();

    int tmpcnt = 0;

    for (int cnt = 0; true; ++cnt)
    {
        File entry;
        if (!(entry = dir.openNextFile(FILE_READ)))
        {
            // TRACE();
            break;
        }
        String tmpName = entry.name();

        if (tmpName.equalsIgnoreCase("SYSTEM~1") || tmpName.startsWith(".")) //equalsIgnoreCase(".TRASH~1"))
        {
            continue;
        }

        String output;
        if (tmpcnt > 0)
        {
            output = ',';
        }

        String fsize = "";

        if (!entry.isDirectory())
        {
            int bytes = entry.size();
            fsize = getFileSizeStringFromBytes(bytes);
        }
        else
        {
            fsize = "na";
        }

        output += "{\"type\":\"";

        output += (entry.isDirectory()) ? "dir" : "file";

        output += "\",\"name\":\"";
        String tmpFullName = entry.name();
        output += tmpFullName.substring(tmpFullName.lastIndexOf("/") + 1);

        output += "\",\"size\":\"";
        output += fsize;
        output += "\"";

        if (entry.isDirectory())
        {
            output += ",\"contents\" :[";
#ifdef WIFI_DEBUG
            // SerialPort.println(output);
#endif
            response->print(output);

            if (isRecursive)
            {
                printDirectoryRecurse(response, entry.name(), isRecursive);
            }
            output = "]";
        }
        output += "}";

#ifdef WIFI_DEBUG
        //SerialPort.println(output);
#endif

        response->print(output);
        entry.close();
        tmpcnt++;
    }
}

AsyncWebServerResponse *VarioWebHandler::handleParams(AsyncWebServerRequest *request)
{
#ifdef WIFI_DEBUG
    SerialPort.println("handleParams");
#endif

    String path = "/params.jso";

    AsyncWebServerResponse *response = request->beginResponse(SD, "/params.jso", "application/json");

    return response;
}

void VarioWebHandler::handleSaveParams(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
#ifdef WIFI_DEBUG
    SerialPort.println("handleSaveParams");
#endif

    String path = "/params.jso";
    String pathBak = "/params.bak";
    File dataFile;

    backupFile(path, pathBak);

    if (!(dataFile = SD.open(path.c_str(), FILE_WRITE)))
    {
        request->send(500, "text/plain", "NO FILE");
        return;
    }

    for (size_t i = 0; i < len; i++)
    {
        Serial.write(data[i]);
        dataFile.write(data[i]);
    }

    dataFile.close();
    request->send(200);
    return;
}

void VarioWebHandler::handleOtaUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

    if (!index)
    {
        Serial.printf("UploadStart: %s\n", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
        { //start with max available size
            Update.printError(Serial);
            request->send(500, "text/plain", "UPDATE FAIL");
            return;
        }
    }

    if (Update.write(data, len) != len)
    {
        Update.printError(Serial);
        request->send(500, "text/plain", "UPDATE FAIL");
        return;
    }

    if (final)
    {
        Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);

        if (Update.end(true))
        { //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", index + len);
            request->send(200);
            Serial.println("RESTART ESP32");
            Serial.flush();

            ESP_LOGI("GnuVario-E", "RESTART ESP32");
#ifdef ENABLE_DISPLAY_WEBSERVER
            screen.ScreenViewReboot();
#endif
            ESP.restart();
        }
        else
        {
            Update.printError(Serial);
            request->send(500, "text/plain", "UPDATE FAIL");
            return;
        }
    }
}

//telechargement d'un fichier dont le nom complet avec chemin se trouve en param
AsyncWebServerResponse *VarioWebHandler::handleFileDownload(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response;
    String path;
    if (request->hasParam("path"))
    {
        AsyncWebParameter *p = request->getParam("path");
        path = p->value().c_str();
    }
    else
    {
        response = request->beginResponse(500, "text/plain", "BAD ARGS");
        return response;
    }

    if (!SD.exists((char *)path.c_str()))
    {
        response = request->beginResponse(500, "text/plain", "NO FILE");
        return response;
    }
    response = request->beginResponse(SD, path, "application/octet-stream", true);

    return response;
}

//suppression d'un fichier dont le nom complet avec chemin se trouve en param
AsyncWebServerResponse *VarioWebHandler::handleFileDelete(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response;
    String path;
    if (request->hasParam("path"))
    {
        AsyncWebParameter *p = request->getParam("path");
        path = p->value().c_str();
    }
    else
    {
        response = request->beginResponse(500, "text/plain", "BAD ARGS");
        return response;
    }

    if (!SD.exists((char *)path.c_str()))
    {
        response = request->beginResponse(500, "text/plain", "NO FILE");
        return response;
    }

    deleteRecursive(path);
    response = request->beginResponse(200, "text/plain", "OK");
    return response;
}

// upload d'un fichier, le chemin de destination se trouve dans le nom du fichier posté
void VarioWebHandler::handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

    if (!index)
    {
        if (SD.exists(filename))
        {
            SD.remove(filename);
        }

        Serial.printf("UploadStart: %s\n", filename.c_str());
        uploadFile = SD.open(filename, FILE_WRITE);
        if (!uploadFile)
        {
            //return error
            request->send(500, "text/plain", "UPLOAD FAIL");
            return;
        }
    }

    if (len && uploadFile)
    {
        if (uploadFile.write(data, len) != len)
        {
            //return error
            Serial.printf("Upload fail");
            request->send(500, "text/plain", "UPLOAD FAIL");
            return;
        }
    }

    if (final && uploadFile)
    {
        Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
        uploadFile.close();
        request->send(200);
    }
}

void VarioWebHandler::handleFileCreate(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    String path;

    if (request->hasParam("path"))
    {
        AsyncWebParameter *p = request->getParam("path");
        path = p->value().c_str();
    }
    else
    {
        request->send(500, "text/plain", "BAD ARGS");

        return;
    }

    if (path == "/" || SD.exists((char *)path.c_str()))
    {
        request->send(500, "text/plain", "BAD PATH");
        return;
    }

    if (path.indexOf('.') > 0)
    {
        File fileSD = SD.open((char *)path.c_str(), FILE_WRITE);
        if (fileSD)
        {
            fileSD.print("\0");
            fileSD.close();
        }
    }
    else
    {
        SD.mkdir((char *)path.c_str());
    }
    request->send(200, "text/plain", "OK");
    return;
}

//récupération du contenu du fichier wifi
AsyncWebServerResponse *VarioWebHandler::handleWifi(AsyncWebServerRequest *request)
{
#ifdef WIFI_DEBUG
    SerialPort.println("handleParams");
#endif

    AsyncWebServerResponse *response = request->beginResponse(SD, "/wifi.cfg", "text/plain");

    return response;
}

// sauvegarde des parametres
void VarioWebHandler::handleSaveWifi(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{

#ifdef WIFI_DEBUG
    SerialPort.println("handleSaveParams");
#endif

    String path = "/wifi.cfg";
    String pathBak = "/wifi.bak";
    File dataFile;

    backupFile(path, pathBak);

    if (!(dataFile = SD.open(path.c_str(), FILE_WRITE)))
    {
        request->send(500, "text/plain", "NO FILE");
        return;
    }

    dataFile.write(data, len);

    dataFile.close();
    request->send(200);
    return;
}

AsyncWebServerResponse *VarioWebHandler::handleWebConfig(AsyncWebServerRequest *request)
{
#ifdef WIFI_DEBUG
    SerialPort.println("handleParams");
#endif

    AsyncWebServerResponse *response = request->beginResponse(SD, "/prefs.jso", "application/json");

    return response;
}

// sauvegarde du contenu du fichier preference
void VarioWebHandler::handleSaveWebConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{

#ifdef WIFI_DEBUG
    SerialPort.println("handleSaveParams");
#endif

    String path = "/prefs.jso";
    String pathBak = "/prefs.bak";
    File dataFile;

    backupFile(path, pathBak);

    if (!(dataFile = SD.open(path.c_str(), FILE_WRITE)))
    {
        request->send(500, "text/plain", "NO FILE");
        return;
    }

    dataFile.write(data, len);

    dataFile.close();
    request->send(200);
    return;
}

// parsage d'un fichier IGC
AsyncWebServerResponse *VarioWebHandler::handleParseIgc(AsyncWebServerRequest *request)
{
#ifdef WIFI_DEBUG
    SerialPort.println("handleParseIgc");
#endif
    AsyncWebServerResponse *response;

    String path;

    if (request->hasParam("path"))
    {
        AsyncWebParameter *p = request->getParam("path");
        path = p->value().c_str();
    }
    else
    {
        response = request->beginResponse(500, "text/plain", "BAD ARGS");
        return response;
    }

    File dataFile;

    //test présence fichier
    if (dataFile = SD.open(path, FILE_READ))
    {
        //parsage du fichier IGC
        VarioIgcParser varioIgcParser;
        varioIgcParser.parseFile(path);

        VarioSqlFlight varioSqlFlight;
        varioSqlFlight.insertFlight(varioIgcParser.getJson());

        String tmpFullName = dataFile.name();
        dataFile.close();

        String filename = tmpFullName.substring(tmpFullName.lastIndexOf("/") + 1);
        if (!SD.exists("/vols/parsed"))
        {
            SD.mkdir("/vols/parsed");
        }
        SD.rename(path, "/vols/parsed/" + filename);

        response = request->beginResponse(200, "text/plain", "OK");
        return response;
    }
    response = request->beginResponse(500, "text/plain", "BAD ARGS");
    return response;
}

String VarioWebHandler::getFileSizeStringFromBytes(int bytes)
{
    // conversion taille
    /***********************************/
    String fsize = "";

    if (bytes < 1024)
        fsize = String(bytes) + " B";
    else if (bytes < (1024 * 1024))
        fsize = String(bytes / 1024.0, 3) + " KB";
    else if (bytes < (1024 * 1024 * 1024))
        fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
    else
        fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";

    return fsize;
}

void VarioWebHandler::deleteRecursive(String path)
{
    /***********************************/

    File fileSD;

    fileSD = SD.open((char *)path.c_str(), FILE_READ);
    if (!fileSD.isDirectory())
    {
        fileSD.close();
        SD.remove((char *)path.c_str());
        return;
    }

    fileSD.rewindDirectory();

    while (true)
    {
        File entry;
        if (!(entry = fileSD.openNextFile(FILE_READ)))

        {
            break;
        }

        String entryPath = entry.name();
        if (entry.isDirectory())
        {
            entry.close();
            deleteRecursive(entryPath);
        }
        else
        {
            entry.close();
            SD.remove((char *)entryPath.c_str());
        }
    }

    SD.rmdir((char *)path.c_str());
    fileSD.close();
}

void VarioWebHandler::backupFile(String pathOrig, String pathBack)
{
    if (SD.exists((char *)pathBack.c_str()))
    {
        SD.remove((char *)pathBack.c_str());
    }
    size_t n;
    uint8_t buf[64];

    File dataFile;
    dataFile = SD.open(pathOrig.c_str(), FILE_READ);
    File dataFile2;
    dataFile2 = SD.open(pathBack.c_str(), FILE_WRITE);

    while ((n = dataFile.read(buf, sizeof(buf))) > 0)
    {
        dataFile2.write(buf, n);
    }
    dataFile.close();
    dataFile2.close();

    SD.remove((char *)pathOrig.c_str());
}