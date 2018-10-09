#include "EasyWifi-Globals.h"

void initSerial()
{
    Serial.begin(115200);
    delay(1000);
}
bool initFS()
{
    LOG("\n\nMounting FS...");
    
    for(u8 i= 0; !SPIFFS.begin() && i < 50; i++)
    {
        LOG("Failed to mount file system,wait 1000 ms retry...");
        delay(1000);
        if (i == 49)
        {
            return false;
        }
    }

    return true;
}

bool initConfig(Config& config)
{
    // 检测config文件是否存在
    if (!SPIFFS.exists(CONFIG_FILE)) {
        LOG("config file not exists,so use the init config params");
        // 不存在说明是第一次启动
        config.eState = Ap_state;
        config.strDeviceName = "DevName";
        config.strApName = "EasyWifi";
        config.strConnectSsid = "";
        config.strConnectPwd = "";
        config.strMqttServiceAddress = "lot-xu.top";
        config.strPubTopic = "out";
        config.strSubTopic = "in";
        saveConfig(config);
    }
    else
    {
        LOG("load config file...");
        loadConfig(config);
    }
}

bool loadConfig(Config& config)
{

    LOG("===================== loadConfig Start=========================");
    File configFile = SPIFFS.open(CONFIG_FILE, "r");
    if (!configFile) {
        LOG("Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    if (size > 1024) {
        LOG("Config file size is too large");
        return false;
    }

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    configFile.readBytes(buf.get(), size);
    // Serial.println(buf.get());
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(buf.get());

    if (!json.success()) {
        LOG("Failed to parse config file");
        return false;
    }
    
    config.eState = (State)json[DEV_STATE].as<int>();
    config.strDeviceName = json[DEV_NAME].asString();
    config.strApName = json[DEV_AP_SSID].asString();
    config.strConnectSsid = json[DEV_CONNECT_SSID].asString();
    config.strConnectPwd = json[DEV_CONNECT_PWD].asString();
    config.strMqttServiceAddress = json[MQTT_SERVICE_ADDRESS].asString();
    config.strPubTopic = json[PUB_TOPIC].asString();
    config.strSubTopic = json[SUB_TOPIC].asString();

    json.prettyPrintTo(Serial);

    LOG("\n===================== loadConfig End=========================");

    return true;
}

bool saveConfig(Config& config)
{
  LOG("===================== saveConfig Start=========================");  
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json[DEV_STATE] = int(config.eState);
  json[DEV_NAME] = config.strDeviceName;
  json[DEV_AP_SSID] = config.strApName;
  json[DEV_CONNECT_SSID] = config.strConnectSsid;
  json[DEV_CONNECT_PWD]= config.strConnectPwd;
  json[MQTT_SERVICE_ADDRESS] = config.strMqttServiceAddress;
  json[PUB_TOPIC] = config.strPubTopic;
  json[SUB_TOPIC] = config.strSubTopic;

  File configFile = SPIFFS.open(CONFIG_FILE, "w");
  if (!configFile) {
    LOG("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  json.prettyPrintTo(Serial);
  LOG("\n===================== saveConfig End=========================");  
  return true;
}


