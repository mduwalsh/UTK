select r.roomNo, h.hotelNo, r.type, r.price, b.guestName from Room r 
inner join Hotel h 
on r.hotelNo = h.hotelNo 
left join (select bk.hotelNo, bk.roomNo, bk.dateFrom, bk.dateTo, g.guestName from Booking bk inner join Guest g on bk.guestNo = g.guestNo) b 
on r.hotelNo = b.hotelNo and r.roomNo = b.roomNo 
where h.hotelName = 'Grosvenor Hotel' and curdate() between b.dateFrom and b.dateTo;
