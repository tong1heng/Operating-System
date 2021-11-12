hdrs = ipc.h
opts = -g -c

p_src = producer.c ipc.c
p_obj = producer.o ipc.o

tp_src = consumer_tp.c ipc.c
tp_obj = consumer_tp.o ipc.o
tm_src = consumer_tm.c ipc.c
tm_obj = consumer_tm.o ipc.o
pm_src = consumer_pm.c ipc.c
pm_obj = consumer_pm.o ipc.o

all:	producer consumer_tp consumer_tm consumer_pm

producer:	$(p_obj)
	gcc $(p_obj) -o producer
producer.o:	$(p_src) $(hdrs)
	gcc $(opts) $(p_src)

consumer_tp:	$(tp_obj)
	gcc $(tp_obj) -o consumer_tp
consumer_tp.o:	$(tp_src) $(hdrs)
	gcc $(opts) $(tp_src)

consumer_tm:	$(tm_obj)
	gcc $(tm_obj) -o consumer_tm
consumer_tm.o:	$(tm_src) $(hdrs)
	gcc $(opts) $(tm_src)

consumer_pm:	$(pm_obj)
	gcc $(pm_obj) -o consumer_pm
consumer_pm.o:	$(pm_src) $(hdrs)
	gcc $(opts) $(pm_src)

clean:
	rm consumer_tm consumer_tp consumer_pm producer *.o