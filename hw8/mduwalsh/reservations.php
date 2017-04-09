<?php
// Start the session
session_start();
?>
<!DOCTYPE html>
<html>
  <body>
    <?php
      // Set session variables
      $_SESSION["airportCodeCity"] = $_POST["airportCodeCity"];
      $_SESSION["pickupMonth"] = $_POST["pickupMonth"];
      $_SESSION["pickupDay"] = $_POST["pickupDay"];
      $_SESSION["pickupTime"] = $_POST["pickupTime"];
      $_SESSION["dropoffMonth"] = $_POST["dropoffMonth"];
      $_SESSION["dropoffDay"] = $_POST["dropoffDay"];
      $_SESSION["dropoffTime"] = $_POST["dropoffTime"];
      $_SESSION["rentalClass"] = $_POST["rentalClass"];
      $_SESSION["incarNavigation"] = $_POST["incarNavigation"];
      $_SESSION["satelliteRadio"] = $_POST["satelliteRadio"];
      $_SESSION["childSeats"] = $_POST["childSeats"];
      $_SESSION["nonSmoking"] = $_POST["nonSmoking"];
    ?>
    <form action="creditcard.php" method="POST">
    <b>Credit card information </b>    
    <table cellspacing="5px">
    <tr>
    <td>
     Credit card type:
    </td>
    <td>
    <input type="radio" name="creditcardType" value="Visa" checked="checked"/> Visa
    <input type="radio" name="creditcardType" value="MasterCard" /> MasterCard
    <input type="radio" name="creditcardType" value="American Express" /> American Express
    </td>
    </tr>
    <tr>
    <td>
    Credit card number:
    </td>
    <td>
    <input type="text" name="creditcardNo" value="" maxlength="19" />
    </td>
    </tr>
    <tr>
    <td>
    Security code:
    </td>
    <td>
    <input type="text" name="securityCode" value="" maxlength="4" size="4" />
    </td>
    </tr>
    <tr>
    <td>
    Expiration date:
    </td>
    <td>
    <select name="expireMonth">
	   <option> - Month - </option>
	   <option value="1">1</option>
	   <option value="2">2</option>
	   <option value="3">3</option>
	   <option value="4">4</option>
	   <option value="5">5</option>
	   <option value="6">6</option>
	   <option value="7">7</option>
	   <option value="8">8</option>
	   <option value="9">9</option>
	   <option value="10">10</option>
	   <option value="11">11</option>
	   <option value="12">12</option>	   
	 </select>
    <select name="expireYear">
	   <option> - Year - </option>
	   <option value="1">12</option>
	   <option value="2">13</option>
	   <option value="3">14</option>
	   <option value="4">15</option>	   	   
	 </select>
    </td>
    </tr>
    <tr>
    <td>
    Card holder's name:
    </td>
    <td>
    <input type="text" name="cardholderName" value="" />
    </td>
    </tr>
    <tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr>
    <tr>
    <td>
    Street address:
    </td>
    <td>
    <input type="text" name="streetAddress" value="" />
    </td>
    </tr>
    <tr>
    <td>
    City:
    </td>
    <td>
    <input type="text" name="city" value="" />
    </td>
    </tr>
    <tr>
    <td>
    State:
    </td>
    <td>
    <select name="state">
	   <option value=""> - State - </option>
	   <option value="AK">Alaska</option>
	   <option value="AL">Alabama</option>
	   <option value="FL">Florida</option>
	   <option value="GA">Georgia</option>
	   <option value="KY">Kentucky</option>
	   <option value="MS">Mississippi</option>
	   <option value="NC">North Carolina</option>
	   <option value="OH">Ohio</option>
	   <option value="SC">South Carolina</option>
	   <option value="TN">Tennessee</option>	   
	 </select>
    </td>
    </tr>
    <tr>
    <td>
    Zip code:
    </td>
    <td>
    <input type="text" name="zipcode" maxlength="5" size="5" value="" />
    </td>
    </tr>
    </table>
    <br/>
    <input type="submit" value="Submit" />
    
    </form>
  </body>
</html>











