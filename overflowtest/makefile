all: of tof

of: mainof.c mybacktrace.h
	gcc -g -I/opt/local/include $< -o $@ -ldl

tof: maintof.c mybacktrace.h
	gcc -g -I/opt/local/include $< -o $@ -lpthread -ldl

.PHONY:clean
clean:
	rm -rf of tof
	rm -rf *.dSYM