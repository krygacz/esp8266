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
$stmt->prepare("SELECT name, ip_address FROM esp8266");
if(!$stmt){
  die("fail: " . $conn->error);
}
$stmt->execute();
$result = $stmt->get_result();
$stmt->close();
$conn->close();
$counter = 0;
$boards = null;
while ($row = $result->fetch_row()) {
  $conn = new mysqli($servername, $username, $password, $dbname);
  $boards[$counter][0] = $row[0];
  $ctx = stream_context_create(array(
    'http' => array(
        'timeout' => 1
        )
    )
  );
  if(@$json = file_get_contents("http://" . $row[1] . "/", 0, $ctx)){
    $obj = json_decode($json, true);
    if($obj["connected"]) {
      $boards[$counter][1] = 1;
      $boards[$counter][2] = $obj["variables"]["software_version"];

      $stmt = $conn->stmt_init();
      $stmt->prepare("UPDATE esp8266 SET version = ? WHERE name = ?");
      if(!$stmt){
        die("fail: " . $conn->error);
      }
      $stmt->bind_param("ss", $obj["variables"]["software_version"], $row[0]);
      $stmt->execute();
      $stmt->close();
      $stmt = $conn->stmt_init();
      $stmt->prepare("UPDATE esp8266 SET is_running = 1 WHERE name = ?");
      if(!$stmt){
        die("fail: " . $conn->error);
      }
      $stmt->bind_param("s", $row[0]);
      $stmt->execute();
      $stmt->close();
      $stmt = $conn->stmt_init();
      $stmt->prepare("UPDATE esp8266 SET value = ? WHERE name = ?");
      if(!$stmt){
        die("fail: " . $conn->error);
      }
      $stmt->bind_param("ss", $obj["variables"]["value"], $row[0]);
      $stmt->execute();
      $stmt->close();
      $stmt = $conn->stmt_init();
      $stmt->prepare("UPDATE esp8266 SET last_online = ? WHERE name = ?");
      if(!$stmt){
        die("fail: " . $conn->error);
      }
      $stmt->bind_param("ss", $obj["variables"]["time"], $row[0]);
      $stmt->execute();
      $stmt->close();
    } else {
      $boards[$counter][1] = 0;
      $stmt = $conn->stmt_init();
      $stmt->prepare("UPDATE esp8266 SET is_running = 0 WHERE name = ?");
      if(!$stmt){
        die("fail: " . $conn->error);
      }
      $stmt->bind_param("s", $row[0]);
      $stmt->execute();
      $stmt->close();
    }
  } else {
    $boards[$counter][1] = 0;
    $stmt = $conn->stmt_init();
    $stmt->prepare("UPDATE esp8266 SET is_running = 0 WHERE name = ?");
    if(!$stmt){
      die("fail: " . $conn->error);
    }
    $stmt->bind_param("s", $row[0]);
    $stmt->execute();
    $stmt->close();
  }
  $counter = $counter + 1;
}
 ?>
