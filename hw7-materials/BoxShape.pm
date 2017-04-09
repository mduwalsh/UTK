{
    package BoxShape;
    
    sub new {
	my ($class, $left, $top, $width, $height) = @_;  
	my $self = {};           # reference to the hash table
	bless($self, $class);    # associate the reference with the package
	$self->{left} = $left;
	$self->{top} = $top;
	$self->{width} = $width;
	$self->{height} = $height;
	return $self;
    }
    
    sub print {
	my ($self) = @_;
	print "BoxShape (", $self->{left}, ", ", $self->{top}, ", ", $self->{width}, ", ", $self->{height}, ")\n";
    }
    
    sub containsPt {
	my ($self, $x, $y) = @_;
	if( ($x > $self->{left}) && ($x < ($self->{left}+ $self->{width})) && ($y > $self->{top}) && ($y < ($self->{top}+$self->{height})) ){
	    return 1;
	}
	else{
	    return undef;
	}
    }
    
    sub getBounds {
	my ($self) = @_;
	return ($self->{left}, $self->{top}, $self->{width}, $self->{height});
    }
    
    sub setBounds {
	my ($self, $left, $top, $width, $height) = @_;
	$self->{left} = $left;
	$self->{top} = $top;
	$self->{width} = $width;
	$self->{height} = $height;
    }
    1;
}
