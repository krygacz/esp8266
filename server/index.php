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
    echo "<script>alert('Name must be unique');</script>";
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
  $stmt->prepare("INSERT INTO esp8266 (name,version,ip_address) VALUES (?, ?, ?)");
  if(!$stmt){
    die("fail: " . $conn->error);
  }
  $stmt->bind_param("sss", $_GET['name'], $_GET['version'], $_GET['ip_address']);
  $stmt->execute();

  $stmt->close();
}
}
}
$stmt = $conn->stmt_init();
$stmt->prepare("SELECT * FROM esp8266");
if(!$stmt){
  die("fail: " . $conn->error);
}
$stmt->execute();
$result = $stmt->get_result();
$counter = 0;
$boards = null;
while ($row = $result->fetch_row()) {
  $boards[$counter] = $row;
  $counter = $counter + 1;
}
$stmt->close();
$conn->close();
?>

<!DOCTYPE html>

<html>
<head>
  <meta charset="utf-8" />
  <title>Smart Home</title>
</head>
<body>
  <h1>test</h1>
<?php
for($x = 0;$x < $counter; $x++){
  $name = $boards[$x][0];
  $version = $boards[$x][1];
  $ip_address = $boards[$x][2];
  $is_running = $boards[$x][3];
  $value = $boards[$x][4];
  echo "name: $name version: $version ip address: $ip_address is_on: $is_running value: $value <br><br>";
}
 ?>
</body>
</html>
