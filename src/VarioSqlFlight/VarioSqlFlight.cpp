#include <Arduino.h>

#include <HardwareConfig.h>
#include <DebugConfig.h>

#ifdef WIFI_DEBUG
#define ARDUINOTRACE_ENABLE 1
#else
#define ARDUINOTRACE_ENABLE 0
#endif

#define ARDUINOTRACE_SERIAL SerialPort
#include <ArduinoTrace.h>

#ifdef HAVE_SDCARD
#include <sdcardHAL.h>
#endif

#include "VarioSqlFlight.h"
#include <ArduinoJson.h>

VarioSqlFlight::VarioSqlFlight()
{

#ifdef SQL_DEBUG
    SerialPort.println("VarioSqlFlight");
#endif //SQL_DEBUG

    dbPath = "/sd/db/vol.db";
    isOpened = false;

#ifdef SQL_DEBUG
    SerialPort.println("sqlite3_initialize");
#endif //SQL_DEBUG

    sqlite3_initialize();
}

VarioSqlFlight::~VarioSqlFlight()
{
    closeDb();
}

int VarioSqlFlight::callback(void *data, int argc, char **argv, char **azColName)
{
    int i;
    Serial.printf("%s: ", (const char *)data);
    for (i = 0; i < argc; i++)
    {
        Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    Serial.printf("\n");
    return 0;
}

int VarioSqlFlight::openDb(const char *filename, sqlite3 **db)
{
    int rc = sqlite3_open(filename, db);
    if (rc)
    {
        Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
        return rc;
    }
    else
    {
        Serial.printf("Opened database successfully\n");
    }
    isOpened = true;
    return rc;
}

void VarioSqlFlight::closeDb()
{
#ifdef SQL_DEBUG
    Serial.printf("close db");
#endif //SQL_DEBUG
    sqlite3_close(myDb);
    isOpened = false;
}

int VarioSqlFlight::db_exec(sqlite3 *db, const char *sql)
{
    Serial.println(sql);
    long start = micros();
    int rc = sqlite3_exec(db, sql, callback, (void *)data, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        Serial.printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        Serial.printf("Operation done successfully\n");
    }
    Serial.print(F("Time taken:"));
    Serial.println(micros() - start);
    return rc;
}

bool VarioSqlFlight::insertFlight(String data)
{
#ifdef SQL_DEBUG
    SerialPort.println(data);
#endif //SQL_DEBUG

    DynamicJsonDocument doc(4096);
    DeserializationError err = deserializeJson(doc, data);
    if (err)
    {
#ifdef SQL_DEBUG
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
#endif //SQL_DEBUG

        return false;
    }

    int rc;
    sqlite3_stmt *res;
    const char *tail;

#ifdef SQL_DEBUG
    SerialPort.println("openDb");
#endif //SQL_DEBUG

    if (!isOpened)
    {
        if (openDb((char *)dbPath.c_str(), &myDb))
        {
            return false;
        }
    }

    String sql = "INSERT INTO flight (filename, md5, pilot, wing, flight_date, start_flight_time, end_flight_time, start_height, end_height, min_height, max_height) VALUES (?, ?, ? ,?, ?, ?, ?, ?, ?, ?, ?)";

    rc = sqlite3_prepare_v2(myDb, (char *)sql.c_str(), sql.length(), &res, &tail);
    if (rc != SQLITE_OK)
    {
#ifdef SQL_DEBUG
        Serial.printf("ERROR preparing sql: %s\n", sqlite3_errmsg(myDb));
#endif //SQL_DEBUG
        closeDb();
        return false;
    }

#ifdef SQL_DEBUG
    SerialPort.println("Début binding");
#endif //SQL_DEBUG

    sqlite3_bind_text(res, 1, doc["filename"], strlen(doc["filename"]), SQLITE_STATIC);
    sqlite3_bind_text(res, 2, doc["md5"], strlen(doc["md5"]), SQLITE_STATIC);
    sqlite3_bind_text(res, 3, doc["pilot"], strlen(doc["pilot"]), SQLITE_STATIC);
    sqlite3_bind_text(res, 4, doc["wing"], strlen(doc["wing"]), SQLITE_STATIC);
    sqlite3_bind_text(res, 5, doc["flightDate"], strlen(doc["flightDate"]), SQLITE_STATIC);
    sqlite3_bind_text(res, 6, doc["startFlightTime"], strlen(doc["startFlightTime"]), SQLITE_STATIC);
    sqlite3_bind_text(res, 7, doc["endFlightTime"], strlen(doc["endFlightTime"]), SQLITE_STATIC);
    sqlite3_bind_int(res, 8, doc["startHeight"]);
    sqlite3_bind_int(res, 9, doc["endHeight"]);
    sqlite3_bind_int(res, 10, doc["minHeight"]);
    sqlite3_bind_int(res, 11, doc["maxHeight"]);

#ifdef SQL_DEBUG
    Serial.printf("Début step");
#endif //SQL_DEBUG

    if (sqlite3_step(res) != SQLITE_DONE)
    {
#ifdef SQL_DEBUG
        Serial.printf("ERROR executing stmt: %s\n", sqlite3_errmsg(myDb));
#endif //SQL_DEBUG
        closeDb();
        return false;
    }

#ifdef SQL_DEBUG
    Serial.printf("sqlite3_clear_bindings");
#endif //SQL_DEBUG
    sqlite3_clear_bindings(res);
    rc = sqlite3_reset(res);
    if (rc != SQLITE_OK)
    {
#ifdef SQL_DEBUG
        Serial.printf("reset failed");
#endif //SQL_DEBUG
        closeDb();
        return false;
    }
#ifdef SQL_DEBUG
    Serial.printf("sqlite3_finalize");
#endif //SQL_DEBUG
    sqlite3_finalize(res);

    return true;
}

bool VarioSqlFlight::updateFlight(uint8_t id, String data)
{
#ifdef SQL_DEBUG
    SerialPort.println(data);
#endif //SQL_DEBUG

    DynamicJsonDocument doc(4096);
    DeserializationError err = deserializeJson(doc, data);
    if (err)
    {
#ifdef SQL_DEBUG
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
#endif //SQL_DEBUG

        return false;
    }

    int rc;
    sqlite3_stmt *res;
    const char *tail;

#ifdef SQL_DEBUG
    SerialPort.println("openDb");
#endif //SQL_DEBUG

    if (!isOpened)
    {
        if (openDb((char *)dbPath.c_str(), &myDb))
        {
            return false;
        }
    }

    String sql = "UPDATE flight SET site = ?, comment = ? WHERE id = ?";

    rc = sqlite3_prepare_v2(myDb, (char *)sql.c_str(), sql.length(), &res, &tail);
    if (rc != SQLITE_OK)
    {
#ifdef SQL_DEBUG
        Serial.printf("ERROR preparing sql: %s\n", sqlite3_errmsg(myDb));
#endif //SQL_DEBUG
        closeDb();
        return false;
    }

#ifdef SQL_DEBUG
    SerialPort.println("Début binding");
#endif //SQL_DEBUG

    sqlite3_bind_int(res, 1, doc["site"]);
    sqlite3_bind_text(res, 2, doc["comment"], strlen(doc["comment"]), SQLITE_STATIC);
    sqlite3_bind_int(res, 3, id);

#ifdef SQL_DEBUG
    Serial.printf("Début step");
#endif //SQL_DEBUG

    if (sqlite3_step(res) != SQLITE_DONE)
    {
#ifdef SQL_DEBUG
        Serial.printf("ERROR executing stmt: %s\n", sqlite3_errmsg(myDb));
#endif //SQL_DEBUG
        closeDb();
        return false;
    }

#ifdef SQL_DEBUG
    Serial.printf("sqlite3_clear_bindings");
#endif //SQL_DEBUG
    sqlite3_clear_bindings(res);
    rc = sqlite3_reset(res);
    if (rc != SQLITE_OK)
    {
#ifdef SQL_DEBUG
        Serial.printf("reset failed");
#endif //SQL_DEBUG
        closeDb();
        return false;
    }
#ifdef SQL_DEBUG
    Serial.printf("sqlite3_finalize");
#endif //SQL_DEBUG
    sqlite3_finalize(res);

    return true;
}

String VarioSqlFlight::getSites()
{
    String output = "";
    DynamicJsonDocument doc(4096);

    int rc;
    sqlite3_stmt *res;
    const char *tail;

#ifdef SQL_DEBUG
    SerialPort.println("openDb");
#endif //SQL_DEBUG

    if (!isOpened)
    {
        if (openDb((char *)dbPath.c_str(), &myDb))
        {
            return output;
        }
    }

    String sql = "SELECT id, lib, comment, lat, lon FROM site";

    rc = sqlite3_prepare_v2(myDb, sql.c_str(), 1000, &res, &tail);
    if (rc != SQLITE_OK)
    {
        closeDb();
        return output;
    }

    uint8_t i = 1;
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        // rec_count = sqlite3_column_int(res, 0);
        // if (rec_count > 5000)
        // {
        //     sqlite3_finalize(res);
        //     return;
        // }
        doc.add(i);
        JsonObject obj1 = doc.createNestedObject();
        obj1["id"] = sqlite3_column_int(res, 0);
        obj1["lib"] = (const char *)sqlite3_column_text(res, 1);
        obj1["comment"] = (const char *)sqlite3_column_text(res, 2);
        obj1["lat"] = sqlite3_column_double(res, 3);
        obj1["lon"] = sqlite3_column_double(res, 4);
        i++;
        serializeJson(doc, output);
    }
    sqlite3_finalize(res);
    return output;
}

bool VarioSqlFlight::insertSite(String data)
{
#ifdef SQL_DEBUG
    SerialPort.println(data);
#endif //SQL_DEBUG

    DynamicJsonDocument doc(4096);
    DeserializationError err = deserializeJson(doc, data);
    if (err)
    {
#ifdef SQL_DEBUG
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
#endif //SQL_DEBUG

        return false;
    }

    int rc;
    sqlite3_stmt *res;
    const char *tail;

#ifdef SQL_DEBUG
    SerialPort.println("openDb");
#endif //SQL_DEBUG

    if (!isOpened)
    {
        if (openDb((char *)dbPath.c_str(), &myDb))
        {
            return false;
        }
    }

    String sql = "INSERT INTO site (lib, comment, lat, lon) VALUES (?, ?, ? ,?)";

    rc = sqlite3_prepare_v2(myDb, (char *)sql.c_str(), sql.length(), &res, &tail);
    if (rc != SQLITE_OK)
    {
#ifdef SQL_DEBUG
        Serial.printf("ERROR preparing sql: %s\n", sqlite3_errmsg(myDb));
#endif //SQL_DEBUG
        closeDb();
        return false;
    }

#ifdef SQL_DEBUG
    SerialPort.println("Début binding");
#endif //SQL_DEBUG

    sqlite3_bind_text(res, 1, doc["lib"], strlen(doc["lib"]), SQLITE_STATIC);
    sqlite3_bind_text(res, 2, doc["comment"], strlen(doc["comment"]), SQLITE_STATIC);
    sqlite3_bind_double(res, 3, doc["lat"]);
    sqlite3_bind_double(res, 4, doc["lon"]);

#ifdef SQL_DEBUG
    Serial.printf("Début step");
#endif //SQL_DEBUG

    if (sqlite3_step(res) != SQLITE_DONE)
    {
#ifdef SQL_DEBUG
        Serial.printf("ERROR executing stmt: %s\n", sqlite3_errmsg(myDb));
#endif //SQL_DEBUG
        closeDb();
        return false;
    }

#ifdef SQL_DEBUG
    Serial.printf("sqlite3_clear_bindings");
#endif //SQL_DEBUG
    sqlite3_clear_bindings(res);
    rc = sqlite3_reset(res);
    if (rc != SQLITE_OK)
    {
#ifdef SQL_DEBUG
        Serial.printf("reset failed");
#endif //SQL_DEBUG
        closeDb();
        return false;
    }
#ifdef SQL_DEBUG
    Serial.printf("sqlite3_finalize");
#endif //SQL_DEBUG
    sqlite3_finalize(res);

    return true;
}

bool VarioSqlFlight::updateSite(uint8_t id, String data)
{
#ifdef SQL_DEBUG
    SerialPort.println(data);
#endif //SQL_DEBUG

    DynamicJsonDocument doc(4096);
    DeserializationError err = deserializeJson(doc, data);
    if (err)
    {
#ifdef SQL_DEBUG
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
#endif //SQL_DEBUG

        return false;
    }

    int rc;
    sqlite3_stmt *res;
    const char *tail;

#ifdef SQL_DEBUG
    SerialPort.println("openDb");
#endif //SQL_DEBUG

    if (!isOpened)
    {
        if (openDb((char *)dbPath.c_str(), &myDb))
        {
            return false;
        }
    }

    String sql = "UPDATE site SET lib = ?, comment = ?, lat = ?, lon = ? WHERE id = ?";

    rc = sqlite3_prepare_v2(myDb, (char *)sql.c_str(), sql.length(), &res, &tail);
    if (rc != SQLITE_OK)
    {
#ifdef SQL_DEBUG
        Serial.printf("ERROR preparing sql: %s\n", sqlite3_errmsg(myDb));
#endif //SQL_DEBUG
        closeDb();
        return false;
    }

#ifdef SQL_DEBUG
    SerialPort.println("Début binding");
#endif //SQL_DEBUG

    sqlite3_bind_text(res, 1, doc["lib"], strlen(doc["lib"]), SQLITE_STATIC);
    sqlite3_bind_text(res, 2, doc["comment"], strlen(doc["comment"]), SQLITE_STATIC);
    sqlite3_bind_double(res, 3, doc["lat"]);
    sqlite3_bind_double(res, 4, doc["lon"]);
    sqlite3_bind_int(res, 5, id);

#ifdef SQL_DEBUG
    Serial.printf("Début step");
#endif //SQL_DEBUG

    if (sqlite3_step(res) != SQLITE_DONE)
    {
#ifdef SQL_DEBUG
        Serial.printf("ERROR executing stmt: %s\n", sqlite3_errmsg(myDb));
#endif //SQL_DEBUG
        closeDb();
        return false;
    }

#ifdef SQL_DEBUG
    Serial.printf("sqlite3_clear_bindings");
#endif //SQL_DEBUG
    sqlite3_clear_bindings(res);
    rc = sqlite3_reset(res);
    if (rc != SQLITE_OK)
    {
#ifdef SQL_DEBUG
        Serial.printf("reset failed");
#endif //SQL_DEBUG
        closeDb();
        return false;
    }
#ifdef SQL_DEBUG
    Serial.printf("sqlite3_finalize");
#endif //SQL_DEBUG
    sqlite3_finalize(res);

    return true;
}