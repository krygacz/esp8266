#include "lwip/tcp_impl.h" // Library needed for tcpCleanup() method
#include <FS.h> // For SPIFFS - accessing flash memory
#include <ESP8266WiFi.h> // Standard library for WiFi
#include <aREST.h> // For REST API
#include <ESP8266httpUpdate.h> // For OTA updates
#include <EEPROM.h> // For reading / saving configuration to EEPROM
#include <TimeLib.h> // For NtpClientLib.h
#include <NtpClientLib.h> // For synchronization with NTP (time server)
#include <ArduinoJson.h> // For encoding / decoding JSON stuff
WiFiClient espClient;
WiFiServer serverc(80);
WiFiServer server(80);
IPAddress apIP(192, 168, 4, 1);
aREST rest = aREST();
boolean printed, syncEventTriggered, configmode;
String ssid, password, device_name, server_ip, sxc, ipaddr, realTime;
NTPSyncEvent_t ntpEvent;
int previousMillis, filemode, value;
int global_sensor_port = 13;
int log_interval = 10000;


String ver = "4.1.5";

void setup() {
  Serial.begin(74880);
  if(eeprom_read()){
    wifi_init();
    rest_init();
    ntp_init();
    spiffs_init();
    send_config_request();
    server.begin();
    previousMillis = millis();
    Serial.println("Initialization done\n\nready");
  } else {
    activate_config_mode();
  }
}
void loop() {
  if (configmode) {
    config_mode();
  } else {
    handle_server();
  }
}








/*
######## Spaghetti ########
*/



// Config & init methods

void wifi_init(){
  // Connects to WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.println("Connecting to wifi with ssid='" + (String)ssid + "' password='" + (String)password + "'");
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(counter >= 20){
      reset("none");
      Serial.println("Couldn't connect with WiFi. Switching to configuration mode");
      ESP.restart();
    }
    delay(1000);
    Serial.println((String)(20 - counter) + " seconds left");
    counter++;
  }
  ipaddr = WiFi.localIP().toString();
  Serial.println("");
  Serial.println("WiFi connected with IP " + ipaddr);
}
void ntp_init(){
  // Connects to NTP server
  NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
    ntpEvent = event;
    syncEventTriggered = true;
  });
  NTP.begin("tempus2.gum.gov.pl", 1, true);
  NTP.setInterval(600);
}
void spiffs_init(){
  // Initializes SPIFFS and creates text file to store sensor data
  SPIFFS.begin();
  File new_file = SPIFFS.open("/" + (String)global_sensor_port, "w");
  new_file.print("{'" + (String)global_sensor_port + "'");
  new_file.close();
}
void rest_init(){
  // Initializes and configures aREST
  rest.variable("software_version", &ver);
  rest.function("update", updater);
  rest.function("reset_eeprom", reset);
  rest.function("restart", restart);
  rest.variable("ip", &ipaddr);
  rest.variable("time", &realTime);
  rest.variable("value", &value);
  rest.function("activate_filemode", activate_filemode);
  rest.set_id("rtx04");
  rest.set_name("esp8266");
}
void send_config_request(){
  // Sends GET request to server with name, software version & ip address information
  tcpCleanup();
  WiFiClient sync;
  if (!sync.connect(server_ip.c_str(), 80)) {
    Serial.println("connection failed with " + (String)server_ip.c_str());
  } else {
    tcpCleanup();
    char str1[150];
    strcpy(str1, "GET /?config&name=");
    strcat(str1, device_name.c_str());
    strcat(str1, "&version=");
    strcat(str1, ver.c_str());
    strcat(str1, "&ip_address=");
    strcat(str1, ipaddr.c_str());
    strcat(str1, " HTTP/1.1\r\nHost: ");
    strcat(str1, server_ip.c_str());
    strcat(str1, "\r\n\r\n");
    sync.print(str1);
    String line = sync.readStringUntil('\r');
    if(line.indexOf("200 OK") > -1){
      Serial.println("GET request successful");
    }
    sync.stop();
  }
}






// Server methods

void handle_server(){
  realTime = NTP.getTimeDateString();
  value = digitalRead(global_sensor_port);
  if(millis() - previousMillis > log_interval) {
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







// Config mode methods

void activate_config_mode() {
  // Initializes local configuration server
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("config_wifi");
  serverc.begin();
  configmode = true;
  Serial.println("######### Configuration mode active #########");
  Serial.println("Connect with 'config_wifi' to configure board");
}
void config_mode() {
  // Handles local configuration server
  WiFiClient cli = serverc.available();
  printed = 0;
  if (!cli){return;}
  String req = cli.readString();
  if(req.indexOf("POST") != -1){
    const int datalen = req.length() + 1;
    char data[datalen];
    // Processes data from POST request (eg. changes '%20' to ' ')
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
    ssid = req2;
    password = req2;
    device_name = req2;
    server_ip = req2;
    ssid = ssid.substring(req2.indexOf("ssid=") + 5,req2.indexOf("&password=")); 
    password = password.substring(req2.indexOf("&password=") + 10,req2.indexOf("&name="));
    device_name = device_name.substring(req2.indexOf("&name=") + 6,req2.indexOf("&ip_address="));
    server_ip = server_ip.substring(req2.indexOf("&ip_address=") + 12);
    Serial.print("ssid: >" + ssid + "< password: >" + password + "< name: >" + device_name + "< server_ip: >" + server_ip + "<\n");
    String s = "HTTP/1.1 200 OK\r\n";
    s += "Content-Type: text/html\r\n\r\n";
    s += "<!DOCTYPE HTML>\r\n<html><script>setTimeout(function(){window.location = \"http://192.168.4.1\";}, 2000);alert(\"Configuration saved!\");</script><h1>Rebooting...</h1><br><h2>You can now close this page</h2></html>";
    cli.print(s);
    printed = 1;
    eeprom_write();
  } else {
    config_display(cli);
  }
  return;
}
void config_display(WiFiClient clientx) {
  // Displays configuration page
  Serial.println("-config_display-");
  clientx.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n");
  String sxc = "";
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







//EEPROM methods

int eeprom_read(){
  //Reads information stored in EEPROM: WiFi ssid & password, board name and server's IP address
  EEPROM.begin(512);
  delay(10);
  for (int i = 0; i < 32; ++i) {
    ssid += char(EEPROM.read(i));
  }
  for (int i = 32; i < 96; ++i) {
    password += char(EEPROM.read(i));
  }
  for (int i = 96; i < 128; ++i) {
    device_name += char(EEPROM.read(i));
  }
  for (int i = 128; i < 144; ++i) {
    server_ip += char(EEPROM.read(i));
  }
  if (ssid == "" || password == "" || device_name == "" || server_ip == "") {
    Serial.println("");
    Serial.println("No saved configuration or configuration incomplete, switching to server_mode");
    return 0;
  } else {
    Serial.println();
    return 1;
  }
}
void eeprom_write() {
  //Writes configuration to EEPROM: WiFi ssid & password, board name and server's IP address
  for (int i = 0; i < 144; ++i) {
    EEPROM.write(i, 0);
  }
  for (int i = 0; i < ssid.length(); ++i) {
    EEPROM.write(i, ssid[i]);
    Serial.print(ssid[i]);
  }
  for (int i = 0; i < password.length(); ++i) {
    EEPROM.write(32 + i, password[i]);
    Serial.print(password[i]);
  }
  for (int i = 0; i < device_name.length(); ++i) {
    EEPROM.write(96 + i, device_name[i]);
    Serial.print(device_name[i]);
  }
  for (int i = 0; i < server_ip.length(); ++i) {
    EEPROM.write(128 + i, server_ip[i]);
    Serial.print(server_ip[i]);
  }
  Serial.println();
  Serial.println();
  EEPROM.commit();
  WiFi.mode(WIFI_STA);
  ESP.restart();
}
int reset(String params){
  //Clears all information stored in EEPROM
  for (int i = 0; i < 144; ++i) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  return 1;
}







//FLASH SAVE methods

int endFile(int sensor_pin){
  // Adds } to the end of text file so that it is correcctly interpreted by jSON
  File fx = SPIFFS.open("/" + (String)sensor_pin, "a");
  fx.print("}");
  fx.close();
  return 0;
}
int refreshFile(int sensor_pin){
  // Clears all the information stored in text file
  File fx = SPIFFS.open("/" + (String)sensor_pin, "w");
  fx.print("{'" + (String)sensor_pin + "'");
  fx.close();
  return 0;
}
int activate_filemode(String params){
  // Activates 'filemode', which sends information stored in text file to server (see sendFile())
  filemode = 1;
  return 0;
}
int sendFile(WiFiClient client, int sensor_pin){
  // Sends information stored in text file to server
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

int saveData(String sensor_pin, String sensor_value){
  //Saves current time & GPIO value to a text file
  File fx = SPIFFS.open("/" + (String)sensor_pin, "a");
  fx.print(",\"" + NTP.getTimeDateString() + "\":\"" + sensor_value + "\"");
  fx.close();
  return 0;
}







//MISC methods

int updater(String params) {
  // Updates software to newest version
  ESPhttpUpdate.update("http://esp.aplikacjejs.fc.pl/esp.bin");
}
int restart(String params){
  // Restarts the board
  ESP.restart();
  return 1;
}
void processSyncEvent(NTPSyncEvent_t ntpEvent) {
  // Processes NTP synchronization event
  if (ntpEvent) {
    Serial.print("Time Sync error: ");
    if (ntpEvent == noResponse)
      Serial.println("NTP server not reachable");
    else if (ntpEvent == invalidAddress)
      Serial.println("Invalid NTP server address");
  }
}
void tcpCleanup() {
  // Fixes heap issues
  while(tcp_tw_pcbs!=NULL)
  {
    tcp_abort(tcp_tw_pcbs);
  }
}














//OLD_METHODS

/*
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
int updater(String params) {
  // Updates software to newest version
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
*/

