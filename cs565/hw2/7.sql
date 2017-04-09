CREATE  VIEW  RoomBookingCount  (hotelNo,  roomNo,  bookingCount)
AS SELECT  b.hotelNo,  r.roomNo,  COUNT(*)
FROM  Room  r,  Booking  b WHERE  r.roomNo  =  b.roomNo  AND  r.hotelNo  =  b.hotelNo
GROUP  BY  b.hotelNo,  r.roomNo;

-- a
SELECT  hotelNo,  roomNo FROM  RoomBookingCount WHERE  hotelNo  =  1;

-- This query is not valid because it is using where to a view created by grouping.

-- b
SELECT  hotelNo,  SUM(bookingCount) FROM  RoomBookingCount GROUP  BY  hotelNo;

-- This query is not valid because it is using aggregate function to a column created in the view by using aggregate function.

-- c
SELECT  * FROM  RoomBookingCount ORDER  BY  bookingCount;  

-- This query is valid.
-- The resolved or mapped query will be as follow:
SELECT  b.hotelNo,  r.roomNo,  COUNT(*) as bookingCount
FROM  Room  r,  Booking  b WHERE  r.roomNo  =  b.roomNo  AND  r.hotelNo  =  b.hotelNo
GROUP  BY  b.hotelNo,  r.roomNo ORDER BY bookingCount;

