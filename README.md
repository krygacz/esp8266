# ESP8266 Smart Home

**[WORK IN PROGRESS]**
This project is a DIY budget Smart Home solution. It gathers data from different sensors placed across your house and notifies you about important events. In order make your own one, you're gonna need:
* sensors, eg. temperature sensors, motion detectors, etc.
* esp8266 boards (like ESP-12), which will get data from sensors and pass them to the main server
* a PC or laptop, acting as a server

## Setting up: ESP8266 boards and Arduino IDE
You'll need an esp8266 board with at least 1MB of flash, a USB to TTL adapter and [properly configured Arduino IDE](https://github.com/esp8266/Arduino). Find more information about flashing ESP8266 [here](https://gist.github.com/stonehippo/3d9eb100d4f545015515). To proceed, download and install following libraries:
* [aREST](https://github.com/marcoschwartz/aREST)
* [TimeLib](https://github.com/PaulStoffregen/Time)
* [NtpClientLib](https://github.com/gmag11/NtpClient)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

After properly installing all the necessary software, open 'esp8266/esp8266.ino' in Arduino IDE and upload the sketch to your board. Make sure the board settings (such as flash size) are correct and correspond to your board.
## Setting up: server
If you don't have any server software, install something like [XAMPP](https://www.apachefriends.org/). Then, set document root to *repository location*/server. In order to set create necessary database and tables, just run the following code on your SQL server:

    CREATE DATABASE `esp8266`;
    CREATE TABLE `data` (
      `id` int(11) NOT NULL,
      `name` varchar(50) NOT NULL,
      `time` varchar(20) NOT NULL,
      `port` int(11) NOT NULL,
      `value` int(11) NOT NULL
    );
    CREATE TABLE `esp8266` (
      `name` varchar(50) NOT NULL,
      `ip_address` varchar(15) NOT NULL,
      `version` text NOT NULL,
      `is_running` tinyint(1) NOT NULL,
      `value` varchar(200) NOT NULL,
      `last_online` text NOT NULL
    );
    ALTER TABLE `data`
      ADD KEY `id` (`id`);
    ALTER TABLE `esp8266`
      ADD UNIQUE KEY `name` (`name`),
      ADD UNIQUE KEY `ip_address` (`ip_address`);
    ALTER TABLE `data`
     MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;

## Powering on for the first time
* Start HTTP & SQL servers
* Power on your ESP8266 board. It should automatically create a hotspot called *config_wifi*. Connect with it and go to *192.168.4.1* in your browser in order to set up the board. After filling in all the textboxes click *Save* and wait for the board to reboot.
* Go to the main control panel by opening your server's address in a web browser. The board you've just configured should appear in the 'Boards' list. You're all set up now!

## Interface
### Main panel: Boards
* The first column indicates whether a board is currently connected or not
* `Board name` displays the name of the board you've set through configuration
* `Ver` displays the current software version of the board
* `IP address` displays current IP address of the board
* `Last sync` column indicates the last time the board was active
* `Value` displays all sensor values in the format `| *port* *value* |`, eg. |**12** 0|. Critical sensors are marked red
* `Actions` column allows you to perform different actions on the board (currently only reboot is supported).
You can also update or remove boards by selecting them from the list and clicking `update` or `remove`
### Sensor configuration panel
This panel allows you to determine which ports are connected to sensors. In the first field type the name of the board you're trying to configure. In the next one, type the pin numbers connected to sensors separated with ';', eg. `12;14;15`. The last field allows you to configure 'critical' sensors. Every time the value of a critical sensor changes, you receive a push notification. Just type the port numbers separated by ';', like in the previous field. Click on `configure` to send sensor configuration to the board.
## Available commands
As for now, the only commands acceptable by the board are ones from [aREST library](https://github.com/marcoschwartz/aREST)
