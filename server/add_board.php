<?php
$servername = "localhost:3306";
$username = "root";
$password = "";
$dbname = "esp8266";
$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
   die("Connection failed: " . $conn->connect_error);
}
if(isset($_GET['config'])){
  $stmt = $conn->stmt_init();
  $stmt->prepare("SELECT * FROM esp8266 WHERE name = ?");
  if(!$stmt){
    die("fail: " . $conn->error);
  }
  $stmt->bind_param("s", $_GET['name']);
  $stmt->execute();
  $result = $stmt->get_result();
  $stmt->close();
  if ($row = $result->fetch_row()) {
    $stmt = $conn->stmt_init();
    $stmt->prepare("UPDATE esp8266 SET ip_address = ? WHERE name = ?");
    if(!$stmt){
      die("fail: " . $conn->error);
    }
    $stmt->bind_param("ss", $_GET['ip_address'], $_GET['name']);
    $stmt->execute();
    $stmt->close();
  } else {
    $stmt = $conn->stmt_init();
    $stmt->prepare("SELECT * FROM esp8266 WHERE ip_address = ?");
    if(!$stmt){
      die("fail: " . $conn->error);
    }
    $stmt->bind_param("s", $_GET['ip_address']);
    $stmt->execute();
    $result = $stmt->get_result();
    $stmt->close();
    if ($row = $result->fetch_row()) {
      echo "<script>alert('IP address must be unique');</script>";
    } else {
  $stmt = $conn->stmt_init();
  $stmt->prepare("INSERT INTO esp8266 (name,ip_address,version) VALUES (?, ?, '<unknown>')");
  if(!$stmt){
    die("fail: " . $conn->error);
  }
  $stmt->bind_param("ss", $_GET['name'], $_GET['ip_address']);
  $stmt->execute();
  $stmt->close();
}
}
}
$conn->close();
?> 
