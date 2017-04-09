<?php
// Start the session
session_start();
?>
<!DOCTYPE html>
<html>
  <head>
  <title>Confirmation</title>
  </head>
  <body>
    <?php
      // Set session variables
      $_SESSION["creditcardType"] = $_POST["creditcardType"];
      $_SESSION["creditcardNo"] = $_POST["creditcardNo"];
      $_SESSION["securityCode"] = $_POST["securityCode"];
      $_SESSION["expireMonth"] = $_POST["expireMonth"];
      $_SESSION["expireYear"] = $_POST["expireYear"];
      $_SESSION["cardholderName"] = strip_tags($_POST["cardholderName"]);
      $_SESSION["streetAddress"] = strip_tags($_POST["streetAddress"]);
      $_SESSION["city"] = $_POST["city"];
      $_SESSION["state"] = $_POST["state"];
      $_SESSION["zipcode"] = $_POST["zipcode"];
      
      $state = array(AK=>"Alaska", Al=>"Alabama", FL=>"Florida", GA=>"Georgia", KY=>"Kentucky", MS=>"Mississippi", NC=>"North Carolina", SC=>"South Carolina", OH=>"Ohio", TN=>"Tennessee");
      $month = array(1=>"Jan", 2=>"Feb", 3=>"Mar", 4=>"Apr", 5=>"May", 6=>"Jun", 7=>"Jul", 8=>"Aug", 9=>"Sep", 10=>"Oct", 11=>"Nov", 12=>"Dec");
      $rentalClass = array("Economy", "Compact", "Luxury", "Van", "Sport utility", "Truck");      
    ?>
    <h3> Confirmation Page</h3>
    <b> Rental Information </b><br/>
    <?php 
    $test = 0;
    echo "Airport code/ city: ", $_SESSION["airportCodeCity"], "<br/>";
    echo "Pick up date: ", $month[$_SESSION["pickupMonth"]], ", ", $_SESSION["pickupDay"], ", during ", $_SESSION["pickupTime"], "<br/>";
    if((int)$_SESSION["dropoffMonth"] < (int)$_SESSION["pickupMonth"] || ((int)$_SESSION["dropoffMonth"] == (int)$_SESSION["pickupMonth"] && 
(int)$_SESSION["dropoffDay"] < (int)$_SESSION["pickupDay"])){ $test = 1;}
    if($test == 1){echo "<b> ";} 
    echo "Drop-off date: "; 
    echo $month[$_SESSION["dropoffMonth"]], ", ", $_SESSION["dropoffDay"], ", during ", $_SESSION["dropoffTime"]; if($test == 1){echo "</b> ";}
    if($test == 1){ echo " &emsp;[Drop off date should be after pick up date]";}  echo "<br/>";
    if(in_array($_SESSION["rentalClass"], $rentalClass)){
      $test = 0;
    }
    else{
      $test = 1;
    }
    if($test == 1){
      echo "<b>";
    }
    echo "Rental class: ";     
    echo $_SESSION["rentalClass"];
    if($test == 1){
      echo "</b> &emsp;[Select valid rental class]";
    }
    echo "<br/>";
    echo "Equipment options: ";
    if($_SESSION["incarNavigation"] == "yes"){ echo "[In-car navigation system] ";}
    if($_SESSION["satelliteRadio"] == "yes"){ echo "[Satellite radio] ";}
    if($_SESSION["childSeats"] == "yes"){ echo "[Child seats] ";}
    if($_SESSION["nonSmoking"] == "yes"){ echo "[Non-smoking] <br/>";}
    ?>
    <br/> <br/>
    <b> Credit card Information </b><br/>
    <?php 
    if(preg_match("/^\d{4}-\d{4}-\d{4}-\d{4}$/", $_SESSION["creditcardNo"])){
    echo "Credit card type: ", $_SESSION["creditcardNo"], "<br/>";
    }
    else{
    echo "<b>", "Credit card type: ", $_SESSION["creditcardNo"], "</b>", " &emsp; [Enter credit card numbers as xxxx-xxxx-xxxx-xxxx] <br/>";
    }
    if(preg_match("/^\d{3,4}$/", $_SESSION["securityCode"])){
      echo "Security code: ", $_SESSION["securityCode"], "<br/>";
    }
    else{
      echo "<b>Security code: ", $_SESSION["securityCode"], "</b>", " &emsp; [Enter valid security code]<br/>";
    }
    echo "Expiration date: ", $_SESSION["expireMonth"], " / ", $_SESSION["expireYear"], "<br/>";
    echo "Card holder's name: ", $_SESSION["cardholderName"], "<br/>";
    echo "<br/>Billing Address<br/>";
    echo "Street Address: ", $_SESSION["streetAddress"], "<br/>";
    echo "City: ", $_SESSION["city"], "<br/>";
    if(array_key_exists($_SESSION["state"],$state)){
      echo "State: ", $state[$_SESSION["state"]], "<br/>";
    }
    else{
      echo "<b>State: ", $_SESSION["state"], "</b> &emsp;[Select a valid state]<br/>";
    }
    if(preg_match("/^\d{5}$/", $_SESSION["zipcode"])){
      echo "Zip code: ", $_SESSION["zipcode"], "<br/>";
    }
    else{
      echo "<b>Zip code: ", $_SESSION["zipcode"], "</b> &emsp;[Enter a 5 digit numeric zip code]<br/>";
    }
    ?>
    
    <br/>
    <a href="reservations.html">Go to home page</a>
  </body>
<html>




