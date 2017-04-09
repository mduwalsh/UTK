<html>

<body>
<?php 
$servername = "dbs.eecs.utk.edu";
$username = "mduwalsh";
$password = "iam@UTKnox";
$dbname = "mduwalsh";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

if(isset($_POST['color'])){
  $sql = "update Color set vote = vote + 1 where color = '". strip_tags($_POST['color'])."'";
  $result = $conn->query($sql);
}

$sql = "select color, vote from Color"; 
$result = $conn->query($sql);

$conn->close();
?>
<h3>Vote For Color</h3>

<form action="colorVote.php" method="POST">

<div style="float:left; width:40%">
<h4>Vote now!!</h4>
<input type="radio" name="color" value="red" checked="checked"/> red <br/>
<input type="radio" name="color" value="orange" /> orange <br/>
<input type="radio" name="color" value="yellow" /> yellow <br/>
<input type="radio" name="color" value="green" /> green <br/>
<input type="radio" name="color" value="blue" /> blue <br/>
<input type="radio" name="color" value="violet" /> violet <br/>
<input type="radio" name="color" value="black" /> black <br/>
<input type="radio" name="color" value="white" /> white <br/>
<br/>
</div>
<?php
if ($result->num_rows > 0) {
    echo "<div style=''>";
    echo "<h4>Votes</h4>";
    while($row = $result->fetch_assoc()) {
      echo "<label style='background-color:", $row["color"], "'>", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;", "</label>"," ",$row["vote"], " &nbsp;<br/>";
    }
    echo "</div>";
}
?>
<div style="clear:both" ></div>
<input type="submit" value="Submit" />
</form>
</body>
</html>
