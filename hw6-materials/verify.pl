$lineno = 0;
while (my $line = <>) {
    chomp $line;
    $lineno++;
    if($lineno == 1){                 # validate room type
	if(!(($line =~ /^double$/) || ($line =~ /^queen$/) || ($line =~ /^king$/) || ($line =~ /^suite$/))){
	    print "Error!! Room type not valid: ", $line, ". Room type can only be double, queen, king or suite.\n";
	}
    }
    elsif($lineno == 2){
	if(!($line =~ /^\d{1,2}-\d{1,2}-(\d{2})\d{2}$/)){
	    print "Error!! Incorrect date format: ", $line, ". Date format should be mm-dd-yyyy.\n"
	}
	else{
	    if($1 != 20){
		print "Error!! Year in the date should start with 20.\n"
	    }
	}
    }
    elsif($lineno == 3){
	if(!($line =~ /^[a-zA-Z0-9 _]+$/)){
	    print "Error!! Guest name must be alphanumeric characters.\n"
	}
	elsif(!($line =~ /^[a-zA-Z0-9 _]{1,30}$/)){
	    print "Error!! Guest name exceeds maximum character length 30.\n";
	}
    }
    elsif($lineno == 4){
	if(!($line =~ /^(pl|gl)-?[1-9][0-9]{3}$/)){
	    print "Error!! Member identifier ", $line, " does not match.\n"
	}
    }
    elsif($lineno == 5){
	if(!($line =~ /^\(?011\)?[ -]\d{1,3}[ -]\d{2}[ -]\d{3}-\d{4}$/)){
	    print "Error!! Phone number ", $line, " does not match pattern.\n";
	}
    }
    
}
