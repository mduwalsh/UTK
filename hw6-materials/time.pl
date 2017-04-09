my $input;
print("Enter military time (hh:mm:ss) or enter exit to quit.\n>");      
chomp($input = <>);
while ($input ne "exit") {
    print $input, "\n";
    @data = split(':', $input);
    print ("standard time format " ,(($data[0]+11) % 12) + 1, ":", $data[1], ":", $data[2], ($data[0] < 12)? "am":"pm", "\n");
    #print (($data[0]+11)%12 + 1, ":", $data[1], ":", $data[2], "\n");
    print(">");  
    chomp($input = <>);
};
