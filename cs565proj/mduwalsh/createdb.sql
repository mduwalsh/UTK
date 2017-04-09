
-- create Staff table
drop table if exists Staff;
create table Staff (
staffNo int not null, 
name varchar(255) not null, 
position varchar(50) not null, 
department varchar(255) not null, 
telNo varchar(16) not null, 
email varchar(100) not null, 
roomNo int not null,
primary key (staffNo),
unique (email)
);

-- create table ResidenceStaff
drop table if exists ResidenceStaff;
create table ResidenceStaff (
staffNo int not null, 
fName varchar(30) not null, 
lName varchar(30) not null, 
DOB date not null, 
email varchar(100) not null, 
street varchar(40) not null, 
city varchar(30) not null, 
postcode varchar(5) not null, 
gender varchar(1) not null, 
position varchar(50) not null, 
location varchar(30) not null,
primary key (staffNo),
unique (email),
constraint check chk_gender(gender in ('M', 'F'))
);

-- create Hall table
drop table if exists Hall;
create table Hall (
hallNo int not null,
name varchar(255) not null, 
address varchar(255) not null, 
telNo varchar(16) not null, 
manager int null,
primary key (hallNo),
unique (name),
foreign key (manager) references ResidenceStaff(staffNo) on delete set null
);

-- create Flat table
drop table if exists Flat;
create table Flat (
flatNo int not null, 
address varchar(255) not null, 
noOfRooms int not null,
primary key (flatNo)
);

-- create Room table
drop table if exists Room;
create table Room (
placeNo int not null, 
roomNo int not null, 
rentRate int not null, 
hallNo int, 
flatNo int,
primary key (placeNo),
foreign key (hallNo) references Hall(hallNo) on delete cascade on update cascade,
foreign key (flatNo) references Flat(flatNo) on delete cascade on update cascade,
constraint  check chk_Room ((hallNo is not null or flatNo is not null) and not (hallNo is not null and flatNo is not null))
);


-- create Student table
drop table if exists Student;
create table Student (
bannerNo int not null, 
fName varchar(30) not null, 
lName varchar(30) not null, 
street varchar(40) not null, 
city varchar(30) not null,
postcode varchar(5) not null, 
phone varchar(16) not null, 
email varchar(100) not null, 
DOB date not null, 
gender varchar(1) not null, 
nationality varchar(50) not null, 
category varchar(50) not null, 
status varchar(20) not null, 
major varchar(100) not null, 
minor varchar(100) null, 
advisor int null, 
placeNo int null, 
specialNeeds varchar(255), 
comments text null,
primary key (bannerNo),
foreign key (placeNo) references Room(placeNo) on delete set null,
foreign key (advisor) references Staff(staffNo) on delete set null,
unique (email),
unique (phone),
constraint check chk_gender(gender in ('M', 'F')),
constraint check chk_status(status in ('placed', 'waiting'))
);

-- create table Lease
drop table if exists Lease;
create table Lease (
leaseNo int not null, 
bannerNo int not null, 
placeNo int not null, 
duration int not null, 
dateEnter date not null, 
dateLeave date null,
primary key (leaseNo),
foreign key (bannerNo) references Student(bannerNo) on delete no action,
foreign key (placeNo) references Room(placeNo) on delete no action,
constraint check chk_duration(duration in (1, 2, 3))
);


-- create table Invoice
drop table if exists Invoice;
create table Invoice (
invoiceNo int not null, 
leaseNo int not null, 
semester int not null, 
paymentDue decimal(19, 2) not null, 
reminderDate1 date null, 
reminderDate2 date null, 
datePaid date null, 
paymentMethod varchar(30) null,
primary key (invoiceNo),
foreign key (leaseNo) references Room(leaseNo),
constraint check chk_semester(semester in (1,2,3)),
constraint check chk_reminderdate(reminderDate2 > reminderDate1)
);

-- create table FlatInspection
drop table if exists FlatInspection;
create table FlatInspection ( 
flatNo int not null, 
inspectionDate date not null, 
inspectStaffNo int not null, 
conditionSatisfactory varchar(3) not null, 
comments text null,
primary key (flatNo, inspectionDate),
foreign key (flatNo) references Flat(flatNo),
foreign key (inspectStaffNo) references ResidenceStaff(staffNo),
constraint check chk_condition(conditionSatisfactory in ('yes', 'no'))
);

-- create table NOK
drop table if exists NOK;
create table NOK (
bannerNo int not null, 
name varchar(70) not null, 
relationship varchar(30) not null, 
street varchar(40) not null, 
city varchar(30) not null, 
postcode varchar(5) not null, 
telNo varchar(16) not null,
primary key (bannerNo),
foreign key (bannerNo) references Student(bannerNo) on delete cascade
);





