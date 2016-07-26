/* mode: h; c-basic-offset: 2
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
================================================================================
Date        Author                        Remarks
--------------------------------------------------------------------------------
05/15/2016  naushad.dln@gmail.com         Inital Draft
 
------------------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define TCP_MAX_CTX  255

typedef int (*tcp_cb_t)(int conn_id, int msg_type, char* data, int len);

typedef enum
{
  MSG_UNKNOWN_REQ = 0x00000000,
  MSG_NEW_CONNECTION_REQ,
  MSG_CLOSE_CONNECTION_REQ,
  MSG_DATA_RECEIVED_REQ,
  MSG_DATA_SENT_REQ
}msg_req_type_t;

typedef enum
{
  FD_STATE_ERR      = 0x00000000,
  FD_STATE_RESERVED,
  FD_STATE_CREATED,
  FD_STATE_CONNECTED,
  FD_STATE_RELEASED,
  FD_STATE_LISTEN
}listen_fd_state_t;


typedef struct 
{
  int  listen_fd;
  int  aux_fd;
  char ip_addr[15];
  int  listen_port;
  tcp_cb_t tcp_cb_ind;
  struct sockaddr_in addr;
  int back_log; 
  listen_fd_state_t fd_state;
}tcp_ctx_t;

typedef struct
{
  int max_listen_num;
  fd_set readfd_set;
  fd_set writefd_set;
  fd_set excepfd_set;
  struct timeval to;
  int fd_max;
  tcp_ctx_t ctx[TCP_MAX_CTX];
}tcp_ctx_main_t;


/*----------------------------------------------------------------------------*/

/* Function Prototype */

void tcp_init_ctx (void);

int tcp_socket (void);

int tcp_bind (char *ip_address, int ip_port, int sock_fd);

int tcp_listen (int sock_fd, int back_log);

int tcp_accept (int listen_fd);

int tcp_read (int sock_fd, char *buffer, int buffer_len);

int tcp_write (int sock_fd, char * data_buffer, int data_len);

int tcp_iomux_init (int time_wait);

int tcp_process_request (int active_connections,
                         fd_set rd,
                         fd_set wr,
                         fd_set ex);

void tcp_select (void);

void tcp_instance_create (char *ip_addr, int tcp_port, tcp_cb_t tcp_cb_ind);

void tcp_close_connection(int conn_id);




