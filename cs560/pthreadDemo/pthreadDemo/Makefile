all: multi_thr cond1 sem1
multi_thr:multi_thr.c
	cc -o multi_thr -pthread multi_thr.c
cond1:cond1.c
	cc -o cond1 -pthread cond1.c
sem1:sem1.c
	cc -o sem1 -pthread sem1.c
clean:
	rm -rf *.o multi_thr cond1 sem1
test1:clean multi_thr
	./multi_thr
test2: clean sem1
	./sem1
test3: clean cond1
	./cond1
release: clean
	cd ..; tar cvfz pthreadDemo.tgz   pthreadDemo; mv pthreadDemo.tgz pthreadDemo/
