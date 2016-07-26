TCP_INC=-I../../inc/tcp
HTTP_INC=-I../../inc/http
RADIUS_CLIENT_INC=-I../../inc/rad_client
SYS_INC=-I../../inc/sys
HTML_INC=-I../../inc/html
TCP_SRC=
HTTP_SRC=
HTML_SRC=html_main.c
CC=gcc
CFLAGS=-Wall -g
OBJ=html_main.o
SRC=


$(OBJ):	$(SRC)
	$(CC) $(CFLAGS) $(TCP_INC) $(HTTP_INC) $(HTML_INC) $(RADIUS_CLIENT_INC) -c $(HTML_SRC)


clean:
	rm -fr $(OBJ)
