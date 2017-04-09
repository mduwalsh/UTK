set @staffcount = (select count(*) from Staff);
set @branchcount = (select count(*) from Branch);
set @avg_staff = @staffcount/@branchcount;
select branchNo, (count(*) - @avg_staff) as staffDiff from Staff group by branchNo having staffDiff > 0;

