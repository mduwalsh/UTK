create table BookingOld(
       hotelNo int(4),
       guestNo int(7),
       dateFrom date,
       dateTo date,
       roomNo int(3));

insert into BookingOld select * from Booking where dateTo < '2003-01-01';

delete from Booking where dateTo < '2003-01-01';

