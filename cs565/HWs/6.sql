select sum(price) from Hotel h  
inner join Room r
    on h.hotelNo = r.hotelNo  
inner join Booking b
    on r.hotelNo = b.hotelNo
    and r.roomNo = b.roomNo  
where h.hotelName = 'Grosvenor Hotel'
    and  curdate() between b.dateFrom and b.dateTo;
