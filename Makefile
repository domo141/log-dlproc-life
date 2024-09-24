
ALL = ldpreload-log-dlproc-life-973.so
ALL+= ldpreload-log-statopen-974.so
ALL+= ldpreload-log-execs-975.so

all: $(ALL)

ldpreload-log-dlproc-life-973.so: ldpreload-log-dlproc-life.c
	sh $<

ldpreload-log-statopen-974.so: ldpreload-log-statopen.c
	sh $<

ldpreload-log-execs-975.so: ldpreload-log-execs.c
	sh $<

clean:
	rm -vf ldpreload-log-*.so
