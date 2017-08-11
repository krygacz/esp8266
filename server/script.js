function get_data(){
  var req = new XMLHttpRequest();
  req.onload = function(){
    alert("Success")
  }
  req.open("GET", "functions.php", true);
  req.send(null);
}
function update(){
  var req = new XMLHttpRequest();
  req.onload = function(){
    alert("Success")
  }
  req.open("GET", "functions.php?update", true);
  req.send(null);
}
function refresh(){
  document.getElementById('p2').classList.add("mdl-progress__indeterminate");
  document.getElementById('snackbar').style = "";
  var check = document.getElementsByClassName('is-selected');
  if(check.length > 0){
    document.getElementById('snackbar').classList.add("mdl-snackbar--active");
    $('#p2').fadeTo(500,0);
    $('.table-action').fadeTo(500,1);
    document.getElementById('update').addEventListener("click", process_board);
    document.getElementById('remove').addEventListener("click", process_board);
    return;
  }
  try{document.getElementById('snackbar').classList.remove("mdl-snackbar--active");} catch(e){}
  $('.table-action').fadeTo(500,0);
  $('#p2').fadeTo(250,1);
  var req = new XMLHttpRequest();
  req.onload = function(){
    if(check.length > 0){
      return;
    }
    rf();
    $('#p2').fadeTo(500,0);
  }
  req.open("GET", "boards.php", true);
  req.send(null);
}
var refreshing = setInterval(refresh, 3000);
function timeSince(date) {
  if(date == "not_in_sync"){
    return "waiting for NTC...";
  }
  if(date == "err"){
    return "error";
  }
  var seconds = Math.floor((new Date() - date) / 1000);
  var interval = Math.floor(seconds / 31536000);
  if (interval >= 1) {
    return interval + " y ago";
  }
  interval = Math.floor(seconds / 2592000);
  if (interval >= 1) {
    return interval + " m ago";
  }
  interval = Math.floor(seconds / 86400);
  if (interval >= 1) {
    return interval + " d ago";
  }
  interval = Math.floor(seconds / 3600);
  if (interval >= 1) {
    return interval + " h ago";
  }
  interval = Math.floor(seconds / 60);
  if (interval >= 1) {
    return interval + " min ago";
  }
  if(seconds < 4) {
    return "now";
  }
  return Math.floor(seconds) + " s ago";
}
function convert(datex){
  if(!datex.split('/') || !datex.split(' ') || !datex.split(':')){
    return "err";
  }
  try{
    var date1 = datex.split(' ');
    var date2 = date1[1].split('/');
    var date3 = date1[0].split(':');
  }catch(e){
    return "err";
  }
  if(date2[2] == "1970"){
    return "not_in_sync";
  }
  return new Date(date2[2], date2[1]-1, date2[0], date3[0], date3[1], date3[2]);
}
function time_refresh(){
  var gh = document.getElementsByClassName('last-online');
  for(var i = 0;i < gh.length;i++){
    gh[i].innerHTML = timeSince(convert(gh[i].dataset.time));
  }
}
function rf(){
  $.ajax({
  url: 'refresh.php',
  method: 'get',
  success: function(response)
  {
      setInterval(time_refresh,1000);
      var table = document.createElement('table');
      table.innerHTML = response;
      var gh = table.getElementsByClassName('last-online');
      for(var i = 0;i < gh.length;i++){
        var dat = gh[i].innerHTML;
        gh[i].innerHTML = timeSince(convert(dat));
      }
      table.className = "mdl-data-table mdl-js-data-table mdl-data-table--selectable mdl-shadow--2dp";
      table.id = "table";
      componentHandler.upgradeElement(table);
      try{document.getElementById('info-container').removeChild(document.getElementById('table'));} catch(e){}
      var button = document.createElement('button');
      button.innerHTML = "Update";
      button.className = "mdl-button mdl-js-button mdl-button--raised mdl-js-ripple-effect mdl-button--primary table-action";
      button.id = "update";
      componentHandler.upgradeElement(button);
      try{document.getElementById('info-container').removeChild(document.getElementById('update'));} catch(e){}
      try{document.getElementById('update').removeEventListener("click", process_board('update'));}catch(e){}
      var button2 = document.createElement('button');
      button2.innerHTML = "Remove";
      button2.className = "mdl-button mdl-js-button mdl-button--raised mdl-js-ripple-effect mdl-button--accent table-action";
      button2.id = "remove";
      componentHandler.upgradeElement(button2);
      try{document.getElementById('info-container').removeChild(document.getElementById('remove'));} catch(e){}
      try{document.getElementById('remove').removeEventListener("click", process_board('remove'));}catch(e){}
      document.getElementById('info-container').appendChild(table);
      document.getElementById('info-container').appendChild(button);
      document.getElementById('info-container').appendChild(button2);
  }
});
}

document.getElementById('p2').addEventListener('mdl-componentupgraded', function() {
    this.MaterialProgress.setProgress(0);
    this.MaterialProgress.setBuffer(100);
  });

function process_board(){
  var what = this.id;
  if(what == 'remove' || what =='update'){
  } else {
    alert("wrong action: " + what);
    return;
  }
  var check = document.getElementsByClassName('is-selected');
    var theUrl = "functions.php?" + what;
    var ix = 0;
    for(ix = 0;ix < check.length;ix++){
      theUrl += "&name[]=" + check[ix].dataset.name;
    }
    for(ix = 0;ix <= check.length;ix++){
      var s = document.getElementsByClassName('is-selected');
      for (t = 0; t < s.length; t++) i = s[t].querySelector("td").querySelector(".mdl-checkbox"), i.MaterialCheckbox.uncheck(), s[t].classList.remove('is-selected');
    }
    var xmlHttp = new XMLHttpRequest();
  xmlHttp.open( "GET", theUrl, true );
  xmlHttp.send( null );
  refresh();
}
function reload(addr){
  var theUrl = "functions.php?reload&name[]=" + addr;
  var xmlHttp = new XMLHttpRequest();
xmlHttp.open( "GET", theUrl, true );
xmlHttp.send( null );
}
refresh();
