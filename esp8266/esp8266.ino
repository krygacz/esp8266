#include <ESP8266WiFi.h>
//#include <YalerESP8266WiFiServer.h>
#include <PubSubClient.h>
#include <aREST.h>
#include <ESP8266httpUpdate.h>
#include <EEPROM.h>
WiFiClient espClient;
WiFiServer serverc(80);
//YalerESP8266WiFiServer server("try.yaler.io", 80, "gsiot-aj8y-5z4e");
WiFiServer server(80);
IPAddress apIP(192, 168, 4, 1);
aREST rest = aREST();
boolean servermode = false;
boolean printed;
const char* ssid;
const char* password;
String ssid_temp, password_temp, sxc, ipaddr;




String ver = "3.1.3";















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
  String ssid_saved, password_saved;
  for (int i = 0; i < 32; ++i)
  {
    ssid_saved += char(EEPROM.read(i));
  }
  for (int i = 32; i < 96; ++i)
  {
    password_saved += char(EEPROM.read(i));
  }
  if ( ssid_saved == "" ) {
     Serial.println("");
    Serial.println("No saved configuration, switching to server_mode");
    wifi_config();
    return;
  } else {
   Serial.println("Acquired configuration: >" + ssid_saved + "< >" + password_saved + "<");
    ssid = ssid_saved.c_str();
    password = password_saved.c_str();
    normal_setup();
return;
  }
}

void loop() {
    if (servermode == true) {
    WiFiClient cli = serverc.available();  // Check if a client has connected
    printed = 0;
    if (!cli){return;}
    String req = cli.readStringUntil('\r');// Read the first line of the request
    if (req.indexOf("/ssid/") != -1) {
      String reqx = req;
      reqx.replace("GET /ssid/", "");
      reqx.replace(" HTTP/1.1", "");
      ssid_temp = reqx;
      String s = "HTTP/1.1 200 OK\r\n";        // the common header:
      s += "Content-Type: text/html\r\n\r\n";
      s += "<!DOCTYPE HTML>\r\n<html><script>setTimeout(function(){window.location = 'http://192.168.4.1';}, 1000);</script>ok<br>ssid: ";
      s += ssid_temp + "</html>";
      cli.print(s);
      printed = 1;
    }
    if (req.indexOf("/pwd/") != -1) {
      String reqx = req;
      reqx.replace("GET /pwd/", "");
      reqx.replace(" HTTP/1.1", "");
      password_temp = reqx;
      String s = "HTTP/1.1 200 OK\r\n";        // the common header:
      s += "Content-Type: text/html\r\n\r\n";
      s += "<!DOCTYPE HTML>\r\n<html><script>setTimeout(function(){window.location = 'http://192.168.4.1';}, 1000);</script>ok<br>pwd: ";
      s += password_temp + "</html>";
      cli.print(s);
      printed = 1;
    }
    if (req.indexOf("/apply") != -1) {
      Serial.println("conf applied");
      String s = "HTTP/1.1 200 OK\r\n";        // the common header:
      s += "Content-Type: text/html\r\n\r\n";
      s += "<!DOCTYPE HTML>\r\n<html><script>setTimeout(function(){window.location = \"http://192.168.4.1\";}, 2000);alert(\"Configuration saved!\");</script>switching to normal...</html>";
      cli.print(s);
      printed = 1;
      updateconf();
    }
    if (!printed) {
      Serial.println("default page");
      config_display(cli);
    }
    return;
  } else {
 WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);
  }
}
int updater(String params) {
  ESPhttpUpdate.update("http://esp.aplikacjejs.fc.pl/esp.bin");
}
void config_display(WiFiClient clientx) {
  Serial.println("-config_display-");
  clientx.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n");
  sxc = "";
  sxc += "<html>";
  sxc += "<head>";
  sxc += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  sxc += "<title>Configuration</title>";
  sxc += "<style>";
  sxc += "*{font-family:'Roboto', sans-serif;} ";
  sxc += "h3 span{font-size:30px;} ";
  sxc += "h3{font-family:'Roboto', sans-serif;font-size:30px;color:#616161;display:inline-block;position:fixed;right:20px;bottom:10px} ";
  sxc += "h1{font-family:'Roboto', sans-serif;font-size:40px;display:block;} ";
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
  sxc += ".form{background:white;width:300px;max-width:302px;height:210px;position:absolute;left:50%;top:50%;transform:translate(-50%, -50%);bottom:calc(100vh - (30vh + (38vw / 16 * 9) + 280px));}";
  sxc += "</style>";
  sxc += "</head>";
  clientx.println(sxc);
  sxc = "";
  sxc += "<body>";
  sxc += "<br><h1><center>Wi-Fi Config</center></h1>";
  sxc += "<br><br><br>";
  sxc += "<div id='form' class='form'>";
  sxc += "<div class='group'><input id='ssid' onkeyup='chk()' type='text'><span class='bar'></span><label>SSID</label></div>";
  sxc += "<div class='group'><input id='pwd' onkeyup='chk()' type='password'><span class='bar'></span><label>Password</label></div>";
  sxc += "<br>";
  sxc += "<button onclick='save()'>Save</button></div>";
  sxc += "<script>";
  sxc += "function chk(){";
  sxc += "if(document.getElementById('ssid').value != ''){";
  sxc += "document.getElementById('ssid').classList.add('valid');";
  sxc += "}else{";
  sxc += "document.getElementById('ssid').classList.remove('valid');";
  sxc += "}";
  sxc += "if(document.getElementById('pwd').value != ''){";
  sxc += "document.getElementById('pwd').classList.add('valid');";
  sxc += "}else{";
  sxc += "document.getElementById('pwd').classList.remove('valid');";
  sxc += "}";
  sxc += "}";
  sxc += "function save(){";
  sxc += "var ss = document.getElementById('ssid').value;";
  sxc += "var pw = document.getElementById('pwd').value;";
  sxc += "var i = document.createElement('img');i.src = '/ssid/' + ss;";
  sxc += "var j = document.createElement('img');j.src = '/pwd/' + pw;";
  sxc += "setTimeout(function(){window.location = 'http://192.168.4.1/apply'}, 400);";
  sxc += "}";
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
  EEPROM.commit();
  WiFi.mode(WIFI_STA);
  ESP.restart();
}
int reset(String params){
  for (int i = 0; i < 96; ++i) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  return 1;
}
void normal_setup() {
  Serial.println("-normal-setup-");
  WiFi.mode(WIFI_STA);
  servermode = false;
  rest.variable("software_version", &ver);
  rest.function("update", updater);
  rest.function("reset_eeprom", reset);
  rest.variable("ip", &ipaddr);
  rest.set_id("rtx04");
  rest.set_name("esp8266");
  WiFi.begin(ssid, password);
  Serial.println("Connecting to wifi with ssid='" + (String)ssid + "' password='" + (String)password + "'");
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(counter >= 10){
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
  server.begin();
}
