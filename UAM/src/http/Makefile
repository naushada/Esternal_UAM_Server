TCP_INC=-I../../inc/tcp
MD5_INC=-I../../inc/md5
HTTP_INC=-I../../inc/http
HTML_INC=-I../../inc/html
TCP_SRC=
HTTP_SRC=http_main.c
CC=gcc
CFLAGS=-Wall -g
OBJ=http_main.o
PRINT=


$(OBJ):	$(HTTP_SRC)
	$(PRINT)
	$(CC) $(CFLAGS) $(MD5_INC) $(TCP_INC) $(HTTP_INC) $(HTML_INC) -o $@ -c $<



clean:
	rm -fr $(OBJ)
