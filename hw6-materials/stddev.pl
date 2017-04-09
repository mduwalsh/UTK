my $total = 0;
my $count =0;
my @data = ();
my $avg = 0;
my $var = 0;
while (my $line = <>) {
    @values = split(' ', $line);
    foreach $value (@values){
	push(@data, $value);
	$total +=$value;
	$count = $count + 1; 
    }
}
$avg = $total / $count;
foreach my $d (@data){
    $var += ($d - $avg)**2;
}
$var = $var/($count-1);
print "average =", $avg, "\n";
print "sample variance = ", $var, "\n";
print "sample standard deviation = ", sqrt($var), "\n";

