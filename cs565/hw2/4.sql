create View CurrentGuestCount (hotelNo, guestCount) as select hotelNo, count(*) from Booking where curdate() between dateFrom and dateTo group by hotelNo;

