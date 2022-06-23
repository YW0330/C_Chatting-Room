CFLAG = -fexec-charset=BIG5 -Os -Wall
compile:
	gcc $(CFLAG) server.c -lwsock32 -o server.exe
	gcc $(CFLAG) client.c -lwsock32 -o client.exe

.PHONY: clean
clean:
	@rm *.exe
	