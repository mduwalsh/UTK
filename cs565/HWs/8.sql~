select h.hotelName, count(*) as count from Hotel h 
inner join Room r 
  on h.hotelNo = r.hotelNo 
where h.city = 'London' 
group by h.hotelName;
