BASE=/Users/naushada/hotspot/uam/http_server
TCP_SRC_DIR=src/tcp
HTTP_SRC_DIR=src/http
HTML_SRC_DIR=src/html
RADIUS_SRC_DIR=src/rad_client
MD5_SRC_DIR=src/md5
CC=gcc

.PHONY: all

all:	UAM_OBJ UAM

UAM_OBJ:
	$(MAKE) -C $(BASE)/$(MD5_SRC_DIR)
	$(MAKE) -C $(BASE)/$(HTTP_SRC_DIR)
	$(MAKE) -C $(BASE)/$(TCP_SRC_DIR)
	$(MAKE) -C $(BASE)/$(RADIUS_SRC_DIR)
	$(MAKE) -C $(BASE)/$(HTML_SRC_DIR)

OBJ=$(BASE)/$(MD5_SRC_DIR)/md5.o	\
	$(BASE)/$(HTTP_SRC_DIR)/http_main.o	\
	$(BASE)/$(TCP_SRC_DIR)/tcp_main.o \
	$(BASE)/$(RADIUS_SRC_DIR)/rad_client.o \
	$(BASE)/$(HTML_SRC_DIR)/html_main.o

UAM: $(OBJ)
	$(CC) -o $@ $^


clean:
	rm -fr UAM
	rm -fr $(TCP_SRC_DIR)/*.o
	rm -fr $(HTTP_SRC_DIR)/*.o
	rm -fr $(HTML_SRC_DIR)/*.o
	rm -fr $(RADIUS_SRC_DIR)/*.o
	rm -fr $(MD5_SRC_DIR)/*.o
