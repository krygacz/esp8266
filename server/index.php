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

if(isset($_GET['notify'])){
  echo "<script>alert('" . "functions.php?" . http_build_query($_GET) . "');</script>";
    @$y = file_get_contents("http://localhost/functions.php?" . http_build_query($_GET), 0, $ctx1);
}
?>

<!DOCTYPE html>
<html>
<head>
  <link rel="stylesheet" href="https://fonts.googleapis.com/icon?family=Material+Icons">
  <link rel="stylesheet" href="https://code.getmdl.io/1.3.0/material.blue-red.min.css" />
  <script src="https://code.jquery.com/jquery-3.2.1.min.js" integrity="sha256-hwg4gsxgFZhOsEEamdOYGBf13FyQuiTwlAQgxVSNgt4=" crossorigin="anonymous"></script>
  <link rel="stylesheet" href="styles.css" />
  <meta charset="utf-8" />
  <title>ESP8266 Home Server</title>
</head>
<body class="main-body mdl-color--grey-100 mdl-color-text--grey-700 mdl-base">
    <div class="mdl-layout mdl-js-layout mdl-layout--fixed-header">
      <header class="mdl-layout__header mdl-layout__header--scroll mdl-color--primary">
        <div class="mdl-layout--large-screen-only mdl-layout__header-row">
        </div>
        <div class="mdl-layout--large-screen-only mdl-layout__header-row">
          <h3>ESP8266 Home Server</h3>
        </div>
        <div class="mdl-layout--large-screen-only mdl-layout__header-row">
        </div>
        <div class="mdl-layout__tab-bar mdl-js-ripple-effect mdl-color--primary-dark">
          <a href="#boards" class="mdl-layout__tab is-active">Boards</a>
          <a href="#config" class="mdl-layout__tab">Configure sensors</a>
          <a href="#notifications" class="mdl-layout__tab">Notifications</a>
        </div>
      </header>
      <main class="mdl-layout__content">
        <div class="mdl-layout__tab-panel is-active" id="boards">
          <section id="info-container" class="section--center mdl-grid mdl-grid--no-spacing" style="display: inline-block;left: 50%;position: relative;transform: translate(-50%,0);">
            <div id="p2" style="left:50%;transform:translate(-50%,5px);z-index:9999;width:100%" class="mdl-progress mdl-js-progress"></div>
          </section>
        </div>
    <div class="mdl-layout__tab-panel" id="config">
      <section id="config-container" class="section--center mdl-grid mdl-grid--no-spacing" style="display: inline-block;left: 50%;position: relative;transform: translate(-50%,0);">
        <div class="demo-card-square mdl-card mdl-shadow--2dp">
          <div class="mdl-card__title mdl-card--expand">
            <h2 class="mdl-card__title-text">Configure sensors</h2>
          </div>
          <div class="mdl-card__supporting-text" style="padding-left:30px;padding-right:30px;padding-top:25px;">
            <form method="POST" id="config-form" action="#">
              <div class="mdl-textfield mdl-js-textfield mdl-textfield--floating-label">
                <input class="mdl-textfield__input" type="text" id="config-name">
                  <label class="mdl-textfield__label" for="config-name">Board name</label>
              </div>
              <br>
              <div class="mdl-textfield mdl-js-textfield mdl-textfield--floating-label">
                <input class="mdl-textfield__input" type="text" id="config-ports">
                  <label class="mdl-textfield__label" for="config-ports">Sensor pins</label>
              </div>
              <br>
              <div class="mdl-textfield mdl-js-textfield mdl-textfield--floating-label">
                <input class="mdl-textfield__input" type="text" id="config-ports-critical">
                  <label class="mdl-textfield__label" for="config-ports-critical">Critical sensor pins</label>
              </div>
            </form>
          </div>
          <div class="mdl-card__actions mdl-card--border">
            <a onclick="encode()" class="mdl-button mdl-button--colored mdl-js-button mdl-js-ripple-effect">Configure</a>
          </div>
        </div>
      </section>

    </div>
    <div class="mdl-layout__tab-panel" id="notifications">
      <section id="config-container" class="section--center mdl-grid mdl-grid--no-spacing" style="display: inline-block;left: 50%;position: relative;transform: translate(-50%,0);">
      <div class="demo-card-square notifications-info mdl-card mdl-shadow--2dp">
        <div class="mdl-card__title mdl-card--expand">
          <h2 class="mdl-card__title-text">Receive notifications</h2>
        </div>
        <div class="mdl-card__supporting-text">
          To get notifications, click on the link below and allow sending notifications.<br>Currently works only in Chrome
        </div>
        <div class="mdl-card__actions mdl-card--border">
          <a class="mdl-button mdl-button--colored mdl-js-button mdl-js-ripple-effect" href="http://esp.aplikacjejs.fc.pl/">
            Get notifications
          </a>
        </div>
      </div>
    </section>
    </div>
        <footer class="mdl-mini-footer">
          <div class="mdl-mini-footer__left-section">
            <div class="mdl-logo">More</div>
            <ul class="mdl-mini-footer__link-list">
              <li><a href="https://github.com/rtx04/esp8266">View on GitHub</a></li>
            </ul>
          </div>
        </footer>
      </main>
    </div>
    <div id="snackbar" class="mdl-js-snackbar mdl-snackbar">
  <div class="mdl-snackbar__text">Refreshing stopped. Unselect elements to continue</div>
  <button class="mdl-snackbar__action" type="button"></button>
</div>
  </body>
  <script defer src="https://code.getmdl.io/1.3.0/material.min.js"></script>
  <script src="script.js"></script>
</html>
