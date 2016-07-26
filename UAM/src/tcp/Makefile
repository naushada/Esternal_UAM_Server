TCP_INC=-I../../inc/tcp
HTTP_INC=-I../../inc/http
SYS_INC=-I../../inc/sys
HTML_INC=-I../../inc/html
TCP_SRC=tcp_main.c
HTTP_SRC=
CC=gcc
CFLAGS=-Wall -g
OBJ=tcp_main.o
SRC=tcp_main.c


$(OBJ):	$(SRC)
	$(CC) $(CFLAGS) $(TCP_INC) $(HTTP_INC) $(HTML_INC) -c $(TCP_SRC)


clean:
	rm -fr $(OBJ)
