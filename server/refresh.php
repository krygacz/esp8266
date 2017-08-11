<?php
$servername = "localhost:3306";
$username = "root";
$password = "";
$dbname = "esp8266";
$conn = new mysqli($servername, $username, $password, $dbname);
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

<thead>
  <tr>

    <th></th>
    <th class="mdl-data-table__cell--non-numeric">Board name</th>
    <th>Ver</th>
    <th>IP address</th>
    <th>Last sync</th>
    <th>Value</th>
    <th>Actions</th>
  </tr>
</thead>
<tbody id="table_body">
  <?php
  for($x = 0;$x < $counter; $x++){
    $name = $boards[$x][0];
    $ip_address = $boards[$x][1];
    $version = $boards[$x][2];
    $is_running = $boards[$x][3];
    $value = $boards[$x][4];
    $last_online = $boards[$x][5];
    ?>
  <tr data-name="<?php echo "$name"; ?>">
    <td><center><i class="material-icons md-36"><?php if($is_running == "1"){echo "network_wifi";}else{echo "signal_wifi_off";} ?></i></td>
    <td class="mdl-data-table__cell--non-numeric"><?php echo "$name"; ?></td>
    <td><?php echo "$version"; ?></td>
    <td><?php echo "$ip_address"; ?></td>
    <td class="last-online mdl-data-table__cell--non-numeric" data-time="<?php echo "$last_online"; ?>"><?php echo "$last_online"; ?></td>
    <td><?php echo "$value"; ?></td>
    <td><button onclick="reload('<?php echo "$name"; ?>')" class="mdl-button mdl-js-button mdl-button--icon">
  <i class="material-icons">refresh</i>
</button></td>
  </tr>
  <?php } ?>

</tbody>
