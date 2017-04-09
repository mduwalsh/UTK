<html>
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

$sql = "Drop table if exists Color";
$result = $conn->query($sql);
$sql = "create table Color (color varchar(20) not null, vote int null, primary key (color))";
$result = $conn->query($sql);
$sql = "insert into Color values ('red', 0), ('orange', 0), ('yellow', 0), ('green', 0), ('blue', 0), ('violet', 0), ('white', 0), ('black', 0)"; 
$result = false;
$result = $conn->query($sql);
if ($result == true) {
    echo "Relation color is successfully created!!";
} else {
    echo "Relation color could not be created!!";
}
$conn->close();
?>
</html>
