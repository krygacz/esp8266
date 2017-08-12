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
        'timeout' => 2
        )
    )
  );
  @$x = file_get_contents("http://" . $row[1] . "/activate_filemode", 0, $ctx1);
  if(@$json = file_get_contents("http://" . $row[1] . "/", 0, $ctx2)){
    $obj = json_decode($json, true);
    $conn = new mysqli($servername, $username, $password, $dbname);
    $query = "";
    for($i = 1; $i < count($obj["data"]) ;$i++) {
      $time = $obj["data"][$i][0];
      $name = $row[0];
      $port = $obj["data"][0][1];
      $value = $obj["data"][$i][1];
      $query .= "INSERT INTO data (time,value,name,port) VALUES (\"$time\", \"$value\", \"$name\", $port);";
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
$conn->close();
 ?>
