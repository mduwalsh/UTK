<html>
<head>
<style>
p.widget { text-indent: 1cm }
</style>
</head>
<body>
<center>
<h2>Account Management Form</h2>
</center>
<hr>
<p>
<?php

if(isset($_POST['action'])){
  $servername = "dbs.eecs.utk.edu";
  $username = "mduwalsh";
  $password = "iam@UTKnox";
  $dbname = "mduwalsh";

  // Create connection
  $conn = new mysqli($servername, $username, $password, $dbname);
  if($_POST['action'] == "login"){
    $sql = "select 1 from Accounts where userid = '" . strip_tags($_POST['userid']) . "'";
    $result = $conn->query($sql);
    if($result->num_rows != 1){
      echo "<b>Invalid userid: ".htmlentities($_POST['userid'])."</b>";
    }
    else{
      $sql = "select userid, name from Accounts where userid = '" . strip_tags($_POST['userid']) . "' and password='".strip_tags($_POST['password'])."'";
      $result = $conn->query($sql);      
      if($result->num_rows != 1){
        echo "<b>Invalid password</b>";
      }
      else{
        $row = $result->fetch_assoc();
        echo "<b>Welcome ".$row['name']."!</b>";
      }      
    }
  }
  else if($_POST['action']=="add"){
    if($_POST['newuserid'] ){
      $sql = "select 1 from Accounts where userid = '" . strip_tags($_POST['newuserid']) . "'";
      $result = $conn->query($sql);
      if($result->num_rows > 0){
        echo "<b>".htmlentities($_POST['newuserid'])." aleady exists --please select another user id</b>";
      }
      else{
        $sql = "insert into Accounts(userid, name, password) values('".strip_tags($_POST['newuserid'])."', '".strip_tags($_POST['name'])."', '".strip_tags($_POST['newpassword'])."')";
        $result = $conn->query($sql);
        if($result == true){
          echo "<b>Account ".strip_tags($_POST['newuserid'])." created--Welecome ".strip_tags($_POST['name'])."!</b>";
        }
      }
    }
  }
  else if($_POST['action'] == "delete"){
    $sql = "select name from Accounts where userid = '" . strip_tags($_POST['deluserid']) . "'";
    $result = $conn->query($sql);
    if($result->num_rows !=1){
      echo "<b>".htmlentities($_POST['deluserid'])." not found</b>";
    }
    else{
      $row = $result->fetch_assoc();
      $sql = "delete from Accounts where userid = '" . strip_tags($_POST['deluserid']) . "'";
      $result = $conn->query($sql);
      if($result == true){
        echo "<b>".$row['name']." successfully deleted</b>";
      }
    }
  }
$conn->close();
}
else{
echo "<form action='account.php' method='post'>";
echo "<input type=\"radio\" name=\"action\" value=\"login\" />Login";
echo "<p class=\"widget\">user id: <input type=\"text\" name=\"userid\" maxlength=\"8\" />";
echo "<p class=\"widget\">password: <input type=\"password\" name=\"password\" maxlength=\"8\" />";
echo "<p><input type=\"radio\" name=\"action\" value=\"add\" />Add Account";
echo "<p class=\"widget\">name: <input type=\"text\" name=\"name\" maxlength=\"20\" />";
echo "<p class=\"widget\">user id: <input type=\"text\" name=\"newuserid\" maxlength=\"8\" />";
echo "<p class=\"widget\">password: <input type=\"password\" name=\"newpassword\" maxlength=\"8\" />";
echo "<p><input type=\"radio\" name=\"action\" value=\"delete\" />Delete Account";
echo "<p class=\"widget\">user id: <input type=\"text\" name=\"deluserid\" maxlength=\"8\" />";
echo "<p>
<input type=\"submit\" />";
echo "</form>";
}
?>
</body>
</html>
