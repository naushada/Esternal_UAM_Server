MD5_INC=-I../../inc/md5
HTTP_INC=-I../../inc/http
HTML_INC=-I../../inc/html
TCP_SRC=
MD5_SRC=md5.c
CC=gcc
CFLAGS=-Wall -g
OBJ=md5.o
PRINT=


$(OBJ):	$(MD5_SRC)
	$(PRINT)
	$(CC) $(CFLAGS) $(MD5_INC) $(HTTP_INC) $(HTML_INC) -o $@ -c $<



clean:
	rm -fr $(OBJ)
