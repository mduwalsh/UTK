sub printcount
{
    my $htref = $_[0];
    my @string = sort {if (${$htref}{$a} == ${$htref}{$b}){
	                 return $a cmp $b;
		       }
		       else{
			 return -(${$htref}{$a} <=> ${$htref}{$b});
		       }
    }  keys %{$htref};
    foreach my $str (@string) {
	print $str, " : ", ${$htref}{$str}, "\n";
    }
}

my %count;
foreach my $file (@ARGV){
    open(FILE, "<$file");
    foreach my $line (<FILE>) {
	chomp $line;
	foreach my $str (split (' ', $line)) {
	    $str=~ /(\w+)\W?/;
	    $count{lc($1)}++;
	}
    }
    close FILE;
}
printcount(\%count);
