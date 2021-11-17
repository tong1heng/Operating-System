hdrs = ipc.h
c_src = customer.c ipc.c
c_obj = customer.o ipc.o
b_src = barber.c ipc.c
b_obj = barber.o ipc.o
t_src = tbarber.c ipc.c
t_obj = tbarber.o ipc.o
opts = -g -c
all: customer barber tbarber
customer: $(c_obj)
	gcc $(c_obj) -o customer
customer.o: $(c_src) $(hdrs)
	gcc $(opts) $(c_src)
barber: $(b_obj)
	gcc $(b_obj) -o barber
barber.o: $(b_src) $(hdrs)
	gcc $(opts) $(b_src)
tbarber: $(t_obj)
	gcc $(t_obj) -o tbarber
tbarber.o: $(t_src) $(hdrs)
	gcc $(opts) $(t_src)
clean:
	rm customer barber tbarber *.o
