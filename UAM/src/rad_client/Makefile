TCP_INC=-I../../inc/tcp
RAD_CLIENT_INC=-I../../inc/rad_client
HTTP_INC=-I../../inc/http
SYS_INC=-I../../inc/sys
HTML_INC=-I../../inc/html
HTTP_SRC=
CC=gcc
CFLAGS=-Wall -g
OBJ=rad_client.o
SRC=rad_client.c


$(OBJ):	$(SRC)
	$(CC) $(CFLAGS) $(RAD_CLIENT_INC) $(TCP_INC) $(HTTP_INC) $(HTML_INC) -c $(SRC)


clean:
	rm -fr $(OBJ)
