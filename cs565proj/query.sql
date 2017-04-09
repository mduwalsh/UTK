-- sample query from project description
select s.fName, s.lName, f.flatNo as apartmentNo, r.roomNo from Student s
inner join Lease l
on s.bannerNo = l.bannerNo
inner join Room r 
on l.placeNo = r.placeNo
inner join Flat f 
on r.flatNo = f.flatNo
where f.address = "1678 Cardiff Rd"; 

-- a
select concat(rs.fName, " ", rs.lName) as manager, h.telNo as telephone_number from ResidenceStaff rs
inner join Hall h 
on rs.staffNo = h.manager;

-- b
(select s.fName, s.lName, l.bannerNo, l.duration, l.placeNo, r.roomNo, f.address, l.dateEnter, l.dateLeave  from Student s
inner join Lease l
on s.bannerNo=l.bannerNo
inner join Room r
on l.placeNo = r.placeNo
inner join Flat f
on r.flatNo = f.flatNo)
union
(select s.fName, s.lName, l.bannerNo, l.duration, l.placeNo, r.roomNo, h.address, l.dateEnter, l.dateLeave  from Student s
inner join Lease l
on s.bannerNo=l.bannerNo
inner join Room r
on l.placeNo = r.placeNo
inner join Hall h
on r.hallNo = h.hallNo);

-- c
(select s.fName, s.lName, l.bannerNo, l.duration, l.placeNo, r.roomNo, f.address, l.dateEnter, l.dateLeave  from Student s
inner join Lease l
on s.bannerNo=l.bannerNo
inner join Room r
on l.placeNo = r.placeNo
inner join Flat f
on r.flatNo = f.flatNo 
where l.duration = 3)
union
(select s.fName, s.lName, l.bannerNo, l.duration, l.placeNo, r.roomNo, h.address, l.dateEnter, l.dateLeave  from Student s
inner join Lease l
on s.bannerNo=l.bannerNo
inner join Room r
on l.placeNo = r.placeNo
inner join Hall h
on r.hallNo = h.hallNo
where l.duration = 3);

-- d


-- this for the details of all rent paid by student with individual payment of invoice
set @bannerNo = 1;
set @lastName = "Duwal";
(select i.invoiceNo, i.leaseNo, i.semester, i.paymentdue, concat(s.fName, " ", s.lName) as name, s.bannerNo, l.placeNo, r.roomNo, f.address, i.reminderDate1, i.reminderDate2, i.datePaid, i.paymentMethod from Invoice i
inner join Lease l
on i.leaseNo = l.leaseNo 
inner join Student s  
on l.bannerNo = s.bannerNo
inner join Room r 
on l.placeNo = r.placeNo 
inner join Flat f 
on r.flatNo = f.flatNo 
where i.datePaid is not null and (s.bannerNo = @bannerNo or s.lName = @lastName) )
union
(select i.invoiceNo, i.leaseNo, i.semester, i.paymentdue, concat(s.fName, " ", s.lName) as name, s.bannerNo, l.placeNo, r.roomNo, h.address, i.reminderDate1, i.reminderDate2, i.datePaid, i.paymentMethod from Invoice i
inner join Lease l
on i.leaseNo = l.leaseNo 
inner join Student s  
on l.bannerNo = s.bannerNo
inner join Room r 
on l.placeNo = r.placeNo 
inner join Hall h 
on r.hallNo = h.hallNo 
where i.datePaid is not null and (s.bannerNo = @bannerNo or s.lName = @lastName) );

-- this is for total rent paid by a student
set @bannerNo = 1;
set @lastName = "Duwal";
select sum(i.paymentdue) as total_rent_paid, concat(s.fName, " ", s.lName) as name, s.bannerNo from Invoice i
inner join Lease l
on i.leaseNo = l.leaseNo 
inner join Student s  
on l.bannerNo = s.bannerNo
where i.datePaid is not null and (s.bannerNo = @bannerNo or s.lName = @lastName);

-- e
set @given_date = "2015-09-10";
(select i.invoiceNo, i.leaseNo, i.semester, i.paymentdue, concat(s.fName, " ", s.lName) as name, s.bannerNo, l.placeNo, r.roomNo, f.address, i.reminderDate1, i.reminderDate2, i.datePaid, i.paymentMethod from Invoice i
inner join Lease l
on i.leaseNo = l.leaseNo 
inner join Student s  
on l.bannerNo = s.bannerNo
inner join Room r 
on l.placeNo = r.placeNo 
inner join Flat f 
on r.flatNo = f.flatNo 
where (i.datePaid is null or i.datePaid > @given_date))
union
(select i.invoiceNo, i.leaseNo, i.semester, i.paymentdue, concat(s.fName, " ", s.lName) as name, s.bannerNo, l.placeNo, r.roomNo, h.address, i.reminderDate1, i.reminderDate2, i.datePaid, i.paymentMethod from Invoice i
inner join Lease l
on i.leaseNo = l.leaseNo 
inner join Student s  
on l.bannerNo = s.bannerNo
inner join Room r 
on l.placeNo = r.placeNo 
inner join Hall h 
on r.hallNo = h.hallNo 
where (i.datePaid is null or i.datePaid > @given_date));

-- f
select fi.flatNo, f.address, concat(rs.fName, " ", rs.lName) as inspected_by, fi.inspectionDate, fi.conditionSatisfactory, fi.comments 
from FlatInspection fi
inner join Flat f
on fi.flatNo = f.flatNo
inner join ResidenceStaff rs 
on fi.inspectStaffNo = rs.staffNo 
where fi.conditionSatisfactory = "no";

-- g
select concat(s.fName, " ", s.lName) as name, s.bannerNo, r.roomNo, r.placeNo from Student s
inner join Lease l
on l.bannerNo = s.bannerNo 
inner join Room r 
on l.placeNo = r.placeNo 
inner join Hall h
on r.hallNo = h.hallNo
where h.name = "Stark hall";

-- h
select s.bannerNo, concat(s.fName, " ", s.lName) as name, concat(s.street, ", ", s.city, ", ", s.postcode) as address, s.phone as phoneNo, s.email, s.DOB, s.gender, s.category, s.nationality, s.specialNeeds, s.comments, s.status as current_status, s.major, s.minor, a.name as advisor  from Student s 
inner join Staff a
on s.advisor = a.staffNo
where s.status = "waiting";

-- i
select s.category, count(*) as no_of_students from Student s group by s.category;

-- j
select concat(s.fName, " ", s.lName) as name, s.bannerNo as banner_number from Student s 
left join NOK n 
on s.bannerNo = n.bannerNo
where n.bannerNo is null;

-- k
set @bannerNo = 1;
set @lastName = "Duwal";
select a.name as advisor_name, a.telNo as internal_telephone from Staff a 
inner join Student s
on a.staffNo = s.advisor 
where s.bannerNo = @bannerNo or s.lName = @lastName;


-- l
select MIN(r.rentRate) as minimum_rent, MAX(r.rentRate) as maximum_rent, AVG(r.rentRate) as average_rent from Room r
inner join Hall h 
on r.hallNo = h.hallNo; 

-- m
select h.name, count(r.placeNo) from Room r inner join Hall h on r.hallNo = h.hallNo 
group by h.name;

-- n
select rs.staffNo as staff_number, concat(rs.fName, " ", rs.lName) as name, cast((datediff(curdate(),rs.DOB)/365) as decimal(5,2)) as age , rs.location from ResidenceStaff rs where (datediff(curdate(),rs.DOB)/365) > 60;





















