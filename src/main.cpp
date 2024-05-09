#include <Arduino.h>
#include <WiFi.h>
#include <asyncHTTPrequest.h>
#include <ArduinoJson.h>
#include <mbedtls/aes.h>
#include <base64.h>
//Library for OTA
#include <HTTPClient.h>
#include <HTTPUpdate.h>
uint32_t LasterVersion = 0;

const char *ssid = "Long Tran";
const char *password = "a12345678";

asyncHTTPrequest request;
enum readyStates
{
  readyStateUnsent = 0,    // Client created, open not yet called
  readyStateOpened = 1,    // open() has been called, connected
  readyStateHdrsRecvd = 2, // send() called, response headers available
  readyStateLoading = 3,   // receiving, partial data available
  readyStateDone = 4       // Request complete, all data available.
} _readyState;

String Encrypt(char *input)
{
  const char *key = "92eU5ePqsct6QwGF";
  size_t inputSize = strlen(input);
  uint8_t rs[inputSize]; //char *rs = (char *)malloc(sizeof(inputSize));
  size_t keySize = strlen(key);
  for (size_t i = 0; i < inputSize; i++)
  {
    rs[i] = (input[i] ^ key[i % keySize]);
  }
  return base64::encode(rs, inputSize);
}

//Register new tank device
void RegisterNewTank(char *szMAC, char *szTankName)
{
  if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone)
  {
    request.open("POST", "http://npe.cotbomxang.com/api/TankAPI/Register");
    request.setReqHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(1024);
    doc["MAC"] = szMAC;
    doc["Name"] = szTankName;
    char checkSumParams[128];
    sprintf(checkSumParams, "%s%s", szMAC, szTankName);
    doc["CheckSum"] = Encrypt(checkSumParams);

    String body;
    serializeJson(doc, body);
    request.send(body);
  }
}

//Register new tank device
void CheckFWUpdate()
{
  if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone)
  {
    request.open("GET", "http://npe.cotbomxang.com/api/TankAPI/CheckVersion");
    request.send();
  }
}

void OtaUpdate(uint32_t latestVersion)
{
  WiFiClient wf;
  char url[128];
  sprintf(url, OTAUpdateUrlC, latestVersion);

  log_i("%s", url);

  t_httpUpdate_return ret = httpUpdate.update(wf, url);
  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    log_e("HTTP_UPDATE_FAILED Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    log_e("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    log_i("HTTP_UPDATE_OK");
    break;
  }
}

//Push tank status
void PushLog(char *szMAC, uint32_t iHeaderNum, char *szHeaderName, uint32_t FuelType, uint32_t HeightMax, uint32_t HeightCurrent, uint32_t VolumeMax,
             uint32_t VolumeCurrent, uint32_t Temperature_C, char *szLocalTime)
{
  if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone)
  {
    request.open("POST", "http://npe.cotbomxang.com/api/TankAPI/PushLog");
    request.setReqHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(1024);
    doc["MAC"] = szMAC;
    doc["HeaderNum"] = iHeaderNum;
    doc["HeaderName"] = szHeaderName;
    doc["GasType"] = FuelType;
    doc["HeightMax"] = HeightMax;
    doc["HeightCurrent"] = HeightCurrent;
    doc["VolumeMax"] = VolumeMax;
    doc["VolumeCurrent"] = VolumeCurrent;
    doc["Temperature"] = Temperature_C;
    doc["LocalTime"] = szLocalTime;

    char checkSumParams[128];
    sprintf(checkSumParams, "%s%s", szMAC, szLocalTime);
    log_i("%s", checkSumParams);
    doc["CheckSum"] = Encrypt(checkSumParams);

    String body;
    serializeJson(doc, body);
    request.send(body);
  }
}

void requestCallback(void *optParm, asyncHTTPrequest *request, int readyState)
{
  if (readyState == readyStateDone)
  {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, request->responseText());
    // Test if parsing succeeds.
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    uint32_t ErrorCode = 0;
    if (doc.containsKey("ErrorCode"))
    {
      uint32_t ErrorCode = doc["ErrorCode"];
    }

    if (doc.containsKey("LatestVersion"))
    {
      LasterVersion = doc["LatestVersion"];
    }
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  log_i("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    log_i(".");
  }

  Serial.printf("App Current Version: %u", FIRMWARE_VERSION);
  //disable debug message
  request.setDebug(false);
  //setup callback method for non-blocking process
  request.onReadyStateChange(requestCallback);

  //Register New Tank
  RegisterNewTank("74:dd:f5:57:ff:ff", "MonitorDevice 1");

  //make sure http instance is free before next call
  while (request.readyState() != readyStateDone)
    yield();
  PushLog("74:dd:f5:57:ff:ff", 1, "G12", 1, 6000, 4500, 3123123, 123123, 2231, "2020/09/30 23:59:59");

  //check for new update
  while (request.readyState() != readyStateDone)
    yield();
  CheckFWUpdate();
}

void loop()
{
  //handle ota
  if (LasterVersion && LasterVersion > FIRMWARE_VERSION)
  {
    OtaUpdate(LasterVersion);
  }
}