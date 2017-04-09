use BoxShape;
use TextShape;

sub pointCheck {
    my ($obj, $x, $y) = @_;
    my @bounds = $obj->getBounds();
    if ($obj->containsPt($x,$y)) {
	print "($x, $y) is in (@bounds)\n";
    }
    else {
	print "($x, $y) is not in (@bounds)\n";
    }
}

print "**** BoxShape Checks ****\n";

# test the BoxShape methods
$box = new BoxShape(10, 20, 30, 40);
$box->print();
($left, $top, $width, $height) = $box->getBounds();
print ("Box Bounds($left, $top, $width, $height)\n");
$box->setBounds(100, 100, 50, 50);
$box->print();

#contains this point
pointCheck($box, 125, 125);

#does not contain these points
pointCheck($box, 80, 125);
pointCheck($box, 125, 80);
pointCheck($box, 200, 200);

# test the TextShape methods
print "\n**** TextShape Checks ****\n";
$text = new TextShape(40, 60, "times", "brad vander zanden");
$text->print();
($left, $top, $width, $height) = $text->getBounds();
print ("Text Bounds($left, $top, $width, $height)\n");

# note--normally TextShape would redefine setBounds so that you
# could not change the width or height, but I wanted you to
# inherit one of BoxShape's methods 
$text->setBounds(60, 60, 30, 30);
$text->print();

$text->setText("smiley");
$newtext = $text->getText();
print "new text = $newtext\n";

#contains this point
pointCheck($text, 70, 80);

#does not contain these points
pointCheck($text, 70, 120);
pointCheck($text, 125, 80);
pointCheck($text, 200, 200);

