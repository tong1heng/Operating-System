for i in `ipcs -s | tail -n +4 | awk {'print $2'}`
do
	echo delete sem = $i
	ipcrm -s $i;
done
for i in `ipcs -m | tail -n +4 | awk {'print $2'}`
do
	echo delete shm = $i
	ipcrm -m $i;
done
for i in `ipcs -q | tail -n +4 | awk {'print $2'}`
do
	echo delete que = $i
	ipcrm -q $i;
done


