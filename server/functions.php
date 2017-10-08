<?php
$servername = "localhost:3306";
$username = "root";
$password = "";
$dbname = "esp8266";
$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
   die("Connection failed: " . $conn->connect_error);
}
$stmt = $conn->stmt_init();
$stmt->prepare("SELECT name, ip_address FROM esp8266 WHERE is_running = 1");
if(!$stmt){
  die("fail: " . $conn->error);
}
$stmt->execute();
$result = $stmt->get_result();
$stmt->close();

$counter = 0;
while ($row = $result->fetch_row()) {

  if(isset($_GET['get_sensor_data'])) {
  $counter = $counter + 1;
  $ctx1 = stream_context_create(array(
    'http' => array(
        'timeout' => 1
        )
    )
  );
  $ctx2 = stream_context_create(array(
    'http' => array(
        'timeout' => 5
        )
    )
  );
  @$x = file_get_contents("http://" . $row[1] . "/activate_filemode", 0, $ctx1);
  if(@$json = file_get_contents("http://" . $row[1] . "/", 0, $ctx2)){
    $obj = json_decode($json, true);
    $conn = new mysqli($servername, $username, $password, $dbname);
    $query = "";

      foreach($obj["data"] as $datax){
        foreach($datax['ports'] as $portx){
          $time = $datax['time'];
          $name = $row[0];
          $port = $portx[0];
          $value = $portx[1];
          $query .= "INSERT INTO data (time,value,name,port) VALUES (\"$time\", \"$value\", \"$name\", $port);";
        }
      }


    $multi_result = mysqli_multi_query($conn, $query);
  } else {
    die("Couldn't get data");
  }
}
  $counter = $counter + 1;
}
if(isset($_GET['remove'])){
  $names = $_GET['name'];
  for($c = 0;$c < count($names);$c++){
    $name = $names[$c];
    $stmt = $conn->stmt_init();
    $stmt->prepare("DELETE FROM esp8266 WHERE name = ?");
    if(!$stmt){
      die("fail: " . $conn->error);
    }
    $stmt->bind_param("s", $name);
    $stmt->execute();
    $stmt->close();
  }
}
if(isset($_GET['update'])){
  $names = $_GET['name'];
  for($c = 0;$c < count($names);$c++){
    $name = $names[$c];
    $stmt = $conn->stmt_init();
    $stmt->prepare("SELECT ip_address FROM esp8266 WHERE name = ?");
    if(!$stmt){
      die("fail: " . $conn->error);
    }
    $stmt->bind_param("s", $name);
    $stmt->execute();
    $result = $stmt->get_result();
    $stmt->close();
    $ctx1 = stream_context_create(array(
      'http' => array(
          'timeout' => 1
          )
      )
    );
    while ($row = $result->fetch_row()) {
      @$y = file_get_contents("http://" . $row[0] . "/update", 0, $ctx1);
    }

  }
}
if(isset($_GET['reload'])){
  $names = $_GET['name'];
  for($c = 0;$c < count($names);$c++){
    $name = $names[$c];
    $stmt = $conn->stmt_init();
    $stmt->prepare("SELECT ip_address FROM esp8266 WHERE name = ?");
    if(!$stmt){
      die("fail: " . $conn->error);
    }
    $stmt->bind_param("s", $name);
    $stmt->execute();
    $result = $stmt->get_result();
    $stmt->close();
    $ctx1 = stream_context_create(array(
      'http' => array(
          'timeout' => 1
          )
      )
    );
    while ($row = $result->fetch_row()) {
      @$y = file_get_contents("http://" . $row[0] . "/restart", 0, $ctx1);
    }

  }
}
if(isset($_GET['sensor_config'])){
  $name = $_GET['name'];
    $stmt = $conn->stmt_init();
    $stmt->prepare("SELECT ip_address FROM esp8266 WHERE name = ?");
    if(!$stmt){
      die("fail: " . $conn->error);
    }
    $stmt->bind_param("s", $name);
    $stmt->execute();
    $result = $stmt->get_result();
    $stmt->close();
    $ctx1 = stream_context_create(array(
      'http' => array(
          'timeout' => 1
          )
      )
    );
    while ($row = $result->fetch_row()) {
      echo "proc";
      $y = file_get_contents("http://" . $row[0] . "/sensor_config?params=" . stripslashes(file_get_contents("php://input")), 0, $ctx1);
      echo "http://" . $row[0] . "/sensor_config?params=" . stripslashes(file_get_contents("php://input"));
    }


}
if(isset($_GET['notify']) && isset($_GET['ip']) && isset($_GET['port']) && isset($_GET['value'])){
  $board_name = "";
  $board_port = $_GET['port'];
  $board_value = $_GET['value'];
  $stmt = $conn->stmt_init();
  $stmt->prepare("SELECT name FROM esp8266 WHERE ip_address = ?");
  if(!$stmt){
    die("fail: " . $conn->error);
  }
  $stmt->bind_param("s", $_GET['ip']);
  $stmt->execute();
  $result = $stmt->get_result();
  $stmt->close();
  $ctx1 = stream_context_create(array(
    'http' => array(
        'timeout' => 1
        )
    )
  );
  while ($row = $result->fetch_row()) {
    $board_name = $row[0];
  }
  $notification_title = urlencode("$board_name: Critical sensor value changed");
  $notification_body = urlencode("Sensor on port $board_port outputs $board_value");
  $notify_url = "http://esp.aplikacjejs.fc.pl/?notify&title=$notification_title&content=$notification_body";
  @$y = file_get_contents($notify_url, 0, $ctx1);
}
$conn->close();
 ?>
