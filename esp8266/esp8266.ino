#include <FS.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <aREST.h>
#include <ESP8266httpUpdate.h>
#include <EEPROM.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ArduinoJson.h>
WiFiClient espClient;
WiFiServer serverc(80);
WiFiServer server(80);
IPAddress apIP(192, 168, 4, 1);
aREST rest = aREST();
boolean servermode = false;
boolean printed;
const char* ssid;
const char* password;
const char* server_ip;
const char* device_name;
String ssid_temp, password_temp, server_ip_temp, sxc, ipaddr, realTime, device_name_temp;
boolean syncEventTriggered = false;
NTPSyncEvent_t ntpEvent;
int previousMillis;
int global_sensor_port = 13;
String ver = "4.0.5";
int filemode = 0;

void setup() {
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  ESP.eraseConfig();
  Serial.begin(74880);
  Serial.println("");
  Serial.println("-setup-");
  EEPROM.begin(512);
  //reset("none");
  delay(10);
  String ssid_saved, password_saved, server_ip_saved, device_name_saved;
  for (int i = 0; i < 32; ++i)
  {
    ssid_saved += char(EEPROM.read(i));
  }
  for (int i = 32; i < 96; ++i)
  {
    password_saved += char(EEPROM.read(i));
  }
  for (int i = 96; i < 128; ++i)
  {
    device_name_saved += char(EEPROM.read(i));
  }
  for (int i = 128; i < 144; ++i)
  {
    server_ip_saved += char(EEPROM.read(i));
  }
  if ( ssid_saved == "" || password_saved == "" || device_name == "" || server_ip_saved == "") {
    Serial.println("");
    Serial.println("No saved configuration or configuration incomplete, switching to server_mode");
    wifi_config();
    return;
  } else {
    ssid = ssid_saved.c_str();
    password = password_saved.c_str();
    server_ip = server_ip_saved.c_str();
    device_name = device_name_saved.c_str();
    normal_setup();
return;
  }
}

void loop() {
  
    if (servermode == true) {
    WiFiClient cli = serverc.available();
    printed = 0;
    if (!cli){return;}
    String req = cli.readString();
    if(req.indexOf("POST") != -1){
      const int datalen = req.length() + 1;
      char data[datalen];
      req.toCharArray(data, datalen);
      
      char *leader = data;
      char *follower = leader;
      while (*leader) {
          if (*leader == '%') {
              leader++;
              char high = *leader;
              leader++;
              char low = *leader;
              if (high > 0x39) high -= 7;
              high &= 0x0f;
              if (low > 0x39) low -= 7;
              low &= 0x0f;
              *follower = (high << 4) | low;
          } else {
              *follower = *leader;
          }
          leader++;
          follower++;
      }
      *follower = 0;
      
      String req2(data);
      ssid_temp = req2;
      password_temp = req2;
      device_name_temp = req2;
      server_ip_temp = req2;
      ssid_temp = ssid_temp.substring(req2.indexOf("ssid=") + 5,req2.indexOf("&password=")); 
      password_temp = password_temp.substring(req2.indexOf("&password=") + 10,req2.indexOf("&name="));
      device_name_temp = device_name_temp.substring(req2.indexOf("&name=") + 6,req2.indexOf("&ip_address="));
      server_ip_temp = server_ip_temp.substring(req2.indexOf("&ip_address=") + 12);
      Serial.print("ssid: >" + ssid_temp + "< password: >" + password_temp + "< name: >" + device_name_temp + "< server_ip: >" + server_ip_temp + "<\n");
      String s = "HTTP/1.1 200 OK\r\n";
      s += "Content-Type: text/html\r\n\r\n";
      s += "<!DOCTYPE HTML>\r\n<html><script>setTimeout(function(){window.location = \"http://192.168.4.1\";}, 2000);alert(\"Configuration saved!\");</script><h1>Rebooting...</h1><br><h2>You can now close this page</h2></html>";
      cli.print(s);
      printed = 1;
      updateconf();
    } else {
          config_display(cli);
    }
    return;
  } else {
    
    realTime = NTP.getTimeDateString();
    if(millis() - previousMillis > 10000) {
    //Serial.println("triggered");
    previousMillis = millis();
    saveData((String)global_sensor_port, (String)digitalRead(global_sensor_port));
    
  }
    WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
    
  }
  if(filemode){
    Serial.println("FILEMODE!");
    endFile(global_sensor_port);
    sendFile(client, global_sensor_port);
    refreshFile(global_sensor_port);
    filemode = 0;
  } else {
    rest.handle(client);
  }
  }
  
}
int updater(String params) {
  //ESPhttpUpdate.update("http://esp.aplikacjejs.fc.pl/esp.bin");
  WiFiClient update_client;
  const char* update_host = "raw.githubusercontent.com";
  if(!update_client.connect(update_host, 80)){
    Serial.println("Couldn't connect");
    return 1;
  }
  auto ret = ESPhttpUpdate.update("http://raw.githubusercontent.com/rtx04/esp8266/master/esp8266/esp8266.ino.generic.bin");
  Serial.println("Update failed: " + ((int)ret));
  return 1;
}
void config_display(WiFiClient clientx) {
  Serial.println("-config_display-");
  clientx.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n");
  sxc = "";
  String sxc;
  sxc += "<!DOCTYPE html />";
  sxc += "<html>";
  sxc += "<head>";
  sxc += "<meta charset='UTF-8'> ";
  sxc += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  sxc += "<title>Configuration</title>";
  sxc += "<style>";
  sxc += "*{font-family:'Roboto', sans-serif;} ";
  sxc += "h3 span{font-size:30px;} ";
  sxc += "h3{font-family:'Roboto', sans-serif;font-size:30px;color:#616161;display:inline-block;position:fixed;right:20px;bottom:10px} ";
  sxc += "h1{font-family:'Roboto', sans-serif;font-size:40px;display:block;transition: all 120ms;} ";
  sxc += "a{display:block;font-size:35px;font-family:'Roboto', sans-serif;font-weight:100;background:white;width:100vw;position:relative;left:0;color:#008aff;text-decoration:none;}";
  sxc += "#container {position: fixed;top: 50%;left: 50%;transform:translate(-100%, -50%);margin-left: -34px;background: #45453f;}";
  sxc += "* { box-sizing:border-box; outline:none}";
  sxc += ".group{ position:relative; margin-bottom:45px; text-align:center;}";
  sxc += "input{font-size:18px;padding:10px 10px 10px 5px;display:block;width:300px;border:none;border-bottom:1px solid #757575;transition:all 200ms;}";
  sxc += "input:focus{outline:none;box-shadow:none;!important} ";
  sxc += "label{color:#999; font-family:'Roboto', sans-serif;font-size:18px;font-weight:300;position:absolute;pointer-events:none;left:5px;top:10px;transition:0.2s ease all;}";
  sxc += "input:focus ~ label, .group .valid ~ label{top:-20px;font-size:14px;color:#008aff;}";
  sxc += "input:focus, .group .valid{border-color:transparent;}";
  sxc += ".bar{position:relative;display:block; width:300px; }";
  sxc += ".bar:before, .bar:after{content:'';height:2px; width:0;bottom:1px; position:absolute;background:#008aff; transition:0.2s ease all;}";
  sxc += ".bar:before{left:50%;}.bar:after{right:50%; }";
  sxc += "input:focus ~ .bar:before, input:focus ~ .bar:after{width:50%;}";
  sxc += ".form button{background:white;border:1px solid #e0e0e0;padding:10px 100px;display:block;border-radius:6px;transition: all 300ms;position:relative;border-color:#00a8ff;color:#666666;text-align:center;left:50%;transform:translate(-50%, 0);cursor:pointer;} ";
  sxc += ".form button:hover{background: #2196f3;color:white;} ";
  sxc += ".form{background:white;width:300px;max-width:302px;position:absolute;left:50%;top:50%;transform:translate(-50%, -50%);}";
  sxc += "@media screen and (max-height:720px){.form{position:relative;top:100%;transform:translate(-50%, 0);}h3{position:relative}h1{font-size:30px;}}";
  sxc += "</style>";
  sxc += "</head>";
  clientx.println(sxc);
  sxc = "";
  sxc += "<body>";
  sxc += "<br><h1><center>Wi-Fi Config</center></h1>";
  sxc += "<br><br><br>";
  sxc += "<form id='form' method='POST' class='form'>";
  sxc += "<div class='group'><input id='ssid' onkeyup='chk()' name='ssid' type='text'><span class='bar'></span><label>SSID</label></div>";
  sxc += "<div class='group'><input id='pwd' onkeyup='chk()' name='password' type='password'><span class='bar'></span><label>Password</label></div>";
  sxc += "<div class='group'><input id='name' onkeyup='chk()' name='name' type='text'><span class='bar'></span><label>Device name</label></div>";
  sxc += "<div class='group'><input id='ip' onkeyup='chk()' name='ip_address' type='text'><span class='bar'></span><label>Server IP address</label></div>";
  sxc += "<br>";
  sxc += "<button type='submit'>Save</button></form>";
  sxc += "<script>";
  sxc += "function chk(){";
  sxc += "if(document.getElementById('ssid').value != ''){";
  sxc += "document.getElementById('ssid').classList.add('valid');";
  sxc += "}else{";
  sxc += "document.getElementById('ssid').classList.remove('valid');}";
  sxc += "if(document.getElementById('pwd').value != ''){";
  sxc += "document.getElementById('pwd').classList.add('valid');";
  sxc += "}else{";
  sxc += "document.getElementById('pwd').classList.remove('valid');}";
  sxc += "if(document.getElementById('name').value != ''){";
  sxc += "document.getElementById('name').classList.add('valid');";
  sxc += "}else{";
  sxc += "document.getElementById('name').classList.remove('valid');}";
  sxc += "if(document.getElementById('ip').value != ''){";
  sxc += "document.getElementById('ip').classList.add('valid');";
  sxc += "}else{";
  sxc += "document.getElementById('ip').classList.remove('valid');}}";
  sxc += "</script>";
  sxc += "</body>";
  sxc += "</html>";
  clientx.println(sxc);
}

void wifi_config() {
  Serial.println("-wifi_config-");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("config_wifi");
  serverc.begin();
  servermode = true;
  Serial.println("server mode");
}
void updateconf() {
  Serial.println("-updateconf-");
  Serial.println("UPDATE_CONF");
  for (int i = 0; i < 96; ++i) {
    EEPROM.write(i, 0);
  }
  for (int i = 0; i < ssid_temp.length(); ++i)
  {
    EEPROM.write(i, ssid_temp[i]);
  }
  for (int i = 0; i < password_temp.length(); ++i)
  {
    EEPROM.write(32 + i, password_temp[i]);
  }
  for (int i = 0; i < device_name_temp.length(); ++i)
  {
    EEPROM.write(96 + i, device_name_temp[i]);
  }
  for (int i = 0; i < server_ip_temp.length(); ++i)
  {
    EEPROM.write(128 + i, server_ip_temp[i]);
  }
  EEPROM.commit();
  WiFi.mode(WIFI_STA);
  ESP.restart();
}
int reset(String params){
  for (int i = 0; i < 144; ++i) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  return 1;
}
int restart(String params){
  ESP.restart();
  return 1;
}
void processSyncEvent(NTPSyncEvent_t ntpEvent) {
  if (ntpEvent) {
    Serial.print("Time Sync error: ");
    if (ntpEvent == noResponse)
      Serial.println("NTP server not reachable");
    else if (ntpEvent == invalidAddress)
      Serial.println("Invalid NTP server address");
  }
}
void normal_setup() {
  Serial.println("-normal-setup-");
  WiFi.mode(WIFI_STA);
  servermode = false;
  rest.variable("software_version", &ver);
  rest.function("update", updater);
  rest.function("reset_eeprom", reset);
  rest.function("restart", restart);
  rest.variable("ip", &ipaddr);
  rest.variable("time", &realTime);
  rest.function("activate_filemode", activate_filemode);
  rest.set_id("rtx04");
  rest.set_name("esp8266");
  WiFi.begin(ssid, password);
  Serial.println("Connecting to wifi with ssid='" + (String)ssid + "' password='" + (String)password + "'");
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(counter >= 25){
      reset("none");
      Serial.println("Couldn't connect with WiFi. Switching to configuration mode");
      ESP.restart();
    }
    delay(500);
    Serial.print(".");
    counter++;
  }
  ipaddr = WiFi.localIP().toString();
  Serial.println("");
  Serial.println("WiFi connected with IP " + ipaddr);
  NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
    ntpEvent = event;
    syncEventTriggered = true;
  });
  NTP.begin("tempus2.gum.gov.pl", 1, true);
  NTP.setInterval(600);
  server.begin();
  spiffs_init();
  WiFiClient sync;
  if (!sync.connect(server_ip, 80)) {
    Serial.println("connection failed");
  } else {
    sync.print("GET /?config&name=" + String(device_name) + "&version=" + ver + "&ip_address=" + ipaddr + " HTTP/1.1\r\nHost: " + String(server_ip) + "\r\n\r\n");
    String line = sync.readStringUntil('\r');
    Serial.println(line);
    if(line.indexOf("200 OK") > -1){
      Serial.println("ok");
    }
    Serial.println("closing connection");
    sync.stop();
  }
  
  previousMillis = millis();
}
void spiffs_init(){
  SPIFFS.begin();
  //if(SPIFFS.exists("/sensor_data.txt")){
  //File fr = SPIFFS.open("/sensor_data.txt", "r");
  //Serial.println("contents of /sensor_data.txt before clearing:\n");
  //while(fr.available()){
  //  Serial.print((char)fr.read());
  // }
  //fr.close();
  //SPIFFS.remove("/sensor_data.txt");
  //} 
  //File new_file = SPIFFS.open("/sensor_data.txt", "w");
  //StaticJsonBuffer<200> jsonBuffer;
  //JsonObject& root = jsonBuffer.createObject();
  //JsonArray& data = root.createNestedArray((String)global_sensor_port);
  //data.add("test");
  //root.printTo(new_file);
  //new_file.close();
  //previousMillis = millis();
  File new_file = SPIFFS.open("/" + (String)global_sensor_port, "w");
  new_file.print("{['test'");
  new_file.close();
}
int endFile(int sensor_pin){
  File fx = SPIFFS.open("/" + (String)sensor_pin, "a");
  fx.print("}");
  fx.close();
  return 0;
}
int refreshFile(int sensor_pin){
  File fx = SPIFFS.open("/" + (String)sensor_pin, "w");
  fx.print("{['test'");
  fx.close();
  return 0;
}
int activate_filemode(String params){
  filemode = 1;
  return 0;
}
int sendFile(WiFiClient client, int sensor_pin){
  File fr = SPIFFS.open("/" + (String)sensor_pin, "r");
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println((String)fr.size());
  client.println();
  while(fr.available()) {
    char a = (char)fr.read();
    client.print((String)a);
    Serial.print((char)a);
  }
  fr.close();
  client.stop();
}
int saveDataDeprecated(String sensor_pin, String sensor_value){
  File fr2 = SPIFFS.open("/sensor_data.txt", "r");
  size_t size = fr2.size();
  if (size > 2048) {
    Serial.println("Config file size is too large");
    return 1;
  }
  std::unique_ptr<char[]> buf(new char[size]);
  fr2.readBytes(buf.get(), size);
  fr2.close();
  StaticJsonBuffer<2048> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  File fx = SPIFFS.open("/sensor_data.txt", "w");
  JsonArray& data = json[sensor_pin];
  JsonArray& details = data.createNestedArray();
  details.add(NTP.getTimeDateString());
  details.add(sensor_value);
  json.printTo(fx);
  //json.printTo(Serial);
  //fx.close();
  //File fr = SPIFFS.open("/sensor_data.txt", "r");
  Serial.println("size: " + (String)size);
  //while(fr.available()){
  //  Serial.print((char)fr.read());
  //}
  //fr.close();
  return 0;
}
int saveData(String sensor_pin, String sensor_value){
  File fx = SPIFFS.open("/" + (String)sensor_pin, "a");
  fx.print(",[\"" + NTP.getTimeDateString() + "\",\"" + sensor_value + "\"]");
  fx.close();
  return 0;
}
