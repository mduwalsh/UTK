create table Room ( roomNo int(3) NULL, hotelNo int(4) NULL, type varchar(9) NULL, price decimal(5,2) NULL, check (roomNo between 1 and 100), check (price between 10 and 100), check (type in ('Single', 'Double', 'Family')) );

create table Booking ( 
hotelNo int(4) NULL, 
roomNo int(3) NULL, 
guestNo int(7) NULL, 
dateFrom date NULL, 
dateTo date NULL, 
constraint  no_doublebooking check (
not exists(select b1.hotelNo, b1.roomNo from Booking b1 where b1.hotelNo = Booking.hotelNo and b1.roomNo = Booking.roomNo and b1.dateFrom <= Booking.dateTo and b1.dateFrom >= Booking.dateFrom)
), 
constraint no_overlap_same_guest check (not exists (select b1.guestNo from Booking b1 where b1.guestNo = Booking.guestNo and b1.dateFrom <= Booking.dateTo and b1.dateFrom >= Booking.dateFrom)
) 
);

create table Guest (guestNo int(7) NULL, guestName varchar(30) NULL, guestAddress varchar(50) NULL );
