{
    package TextShape;
    require BoxShape;
    @ISA = ("BoxShape");
    
    sub new {
	my ($class, $left, $top, $font, $text) = @_;
	$self= $class->SUPER::new($left, $top, length($text)*5, 20);
	$self->{font} = $font;  
	$self->{text} = $text;
	return $self;
    }
    
    sub print {
	my ($self) = @_;
	print "TextShape (", $self->{left}, ", ", $self->{top}, ", ", $self->{width}, ", ", $self->{height}, ", ", $self->{font}, ", ", $self->{text}, ")\n";
    }
    
    sub setText {
	my ($self, $text) = @_;
	$self->{text} = $text;
	$self->{width} = length($text) * 5;
    }
    
    sub getText {
	my ($self) = @_;
	return $self->{text};
    }  
    1;
}
