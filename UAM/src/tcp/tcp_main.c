/* mode: c; c-basic-offset: 2
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
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <tcp_main.h>
#include <http_main.h>

/* Instantiation of tcp context */
tcp_ctx_main_t g_tcp_ctx_main;

/**
 * This function initializes the tcp context global instance to 0 except for 
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
void tcp_init_ctx (void)
{
  memset((void *)&g_tcp_ctx_main.ctx,
         0,
         (sizeof(g_tcp_ctx_main.ctx) * TCP_MAX_CTX));

  /* 0 for STDIN, 1 for STDOUT and 2 for STDERR */
  g_tcp_ctx_main.ctx[0].fd_state = FD_STATE_RESERVED;
  g_tcp_ctx_main.ctx[1].fd_state = FD_STATE_RESERVED;
  g_tcp_ctx_main.ctx[2].fd_state = FD_STATE_RESERVED;
}/* tcp_init_ctx */


/**
 * This function is used to create the TCP socket of INTERNET TYPE
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    Newly created file descriptor upon success or an error upon
 *            failure.
 */
int tcp_socket (void)
{
  int sock_fd = -1;

  sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock_fd > 0)
  {
    g_tcp_ctx_main.ctx[sock_fd].fd_state = FD_STATE_CREATED;
  }
  return(sock_fd);
}/* tcp_socket */


/**
 * This function makes the file descriptor addressable by binding
 * file descriptor to IP address and port.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     IP Address to be bind and this shall be IPv4.
 * @param     TCP port on which IP Address to associated with.
 * @param     TCP file descriptor.
 * @return    return code of bind function.
 */
int tcp_bind (char *ip_address, int ip_port, int sock_fd)
{
  int rc = -1;
  struct sockaddr_in self_addr;

  memset((void*)&self_addr,0,sizeof(self_addr));

  self_addr.sin_family        = AF_INET;
  self_addr.sin_addr.s_addr   = inet_addr(ip_address);
  self_addr.sin_port          = htons(ip_port);

  memset((void *)&self_addr.sin_zero,
         0,
         (size_t)sizeof(self_addr.sin_zero));

  rc =  bind((int)sock_fd,
             (struct sockaddr *)&self_addr,
             (size_t)sizeof(self_addr));

  if (rc < 0 )
  {
    fprintf(stderr,"Bind Failed %d\n",errno);
    return(rc);
  }
  strncpy((char *)g_tcp_ctx_main.ctx[sock_fd].ip_addr,
          (const char*)ip_address,
          (size_t)15);
  
  g_tcp_ctx_main.ctx[sock_fd].listen_port = ip_port;
  g_tcp_ctx_main.ctx[sock_fd].addr        = self_addr;
  g_tcp_ctx_main.ctx[sock_fd].fd_state    = FD_STATE_LISTEN;
  g_tcp_ctx_main.ctx[sock_fd].listen_fd   = sock_fd;

  if (g_tcp_ctx_main.fd_max < sock_fd)
  {
    g_tcp_ctx_main.fd_max = sock_fd;
  }
  return(rc);
}/* tcp_bind */


/**
 * This function sets the back log of simultaneous connection of the adderssed
 * file descriptor.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     file descriptor
 * @param     TCP connection back log (queue size).
 * @return    return code of listen function.
 */
int tcp_listen (int sock_fd, int back_log)
{
  int rc = -1;
  rc = listen (sock_fd,back_log);
  
  if (rc < 0)
  {
    fprintf(stderr,"TCP listen failed [%d]\n",errno);
    return (rc);
  }
  g_tcp_ctx_main.ctx[sock_fd].back_log = back_log;
  return(rc);
}/* tcp_listen */


/**
 * This function is used to accept a new cllient connection and updates the
 * max_fd. Also stores the IP Address of the TCP client and marks the fd_state 
 * as FD_STATE_CONNECTED.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     listen file descriptor on which TCP client can connect.
 * @return    returns the newly connected file descriptor.
 */
int tcp_accept (int listen_fd)
{
  int       sock_fd = -1;
  struct    sockaddr_in addr;
  socklen_t addr_len;
  
  sock_fd =  accept (listen_fd,
                     (struct sockaddr *)&addr,
                     (socklen_t *)&addr_len);
  
  if (sock_fd < 0)
  {
    fprintf (stderr,"Accept Failed with reason %d\n",errno);
    return (sock_fd);
  }
  
  g_tcp_ctx_main.ctx[sock_fd].addr     = addr;
  g_tcp_ctx_main.ctx[sock_fd].aux_fd   = sock_fd;
  g_tcp_ctx_main.ctx[sock_fd].fd_state = FD_STATE_CONNECTED;
    
  if (g_tcp_ctx_main.fd_max < sock_fd)
  {
    g_tcp_ctx_main.fd_max = sock_fd;
  }
  return(sock_fd);
}/* tcp_accept */


/**
 *  This function is used to read the incoming data from TCP client for given 
 *  file descriptor.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     file descriptor on which data has arrived.
 * @param     pointer to char data buffer in which read data to be stored.
 * @param     maximum data buffer length.
 * @return    actual number of bytes read.
 */
int tcp_read (int sock_fd, char *buffer, int buffer_len)
{
  return (recv (sock_fd, (void *)buffer, buffer_len, (int)0) );
} /* tcp_read */


/**
 * This function is used to send data on TCP connection for given 
 * file descriptor.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     file descriptor on which data to be writen.
 * @param     data buffer to be sent.
 * @param     actual data length to be sent over TCP connection.
 * @return    bytes sent to TCP client.
 */
int tcp_write (int sock_fd, char * data_buffer, int data_len)
{
  int sent_data = 0;
  int offset    = 0;
  char *data_buff_ptr = NULL;

  /* First remove sock_fd from write fd set */
  FD_CLR(sock_fd,&g_tcp_ctx_main.writefd_set);
  data_buff_ptr = (char *) malloc(data_len);
  memcpy(data_buff_ptr, data_buffer, data_len); 
  while (sent_data != data_len)
  {
    fprintf(stderr,"Before Send\n");
    sent_data = send(sock_fd, (data_buff_ptr + offset), data_len, (int)0);
    fprintf(stderr,"sending Data [%d] sent_data [%d] data_len [%d]",sock_fd,sent_data,data_len);
    if (sent_data < 0)
    {
      fprintf(stderr,"Send Failed with reason %s\n",strerror(errno));
      return (sent_data);
    }
    else if (sent_data < data_len)
    {
      fprintf (stderr, "\nsent_data %d data_len %d\n",sent_data,data_len); 
      /* calculate the offset of the remaining data to be sent. */
      data_len -= sent_data;
      offset   += sent_data;
      sent_data = 0; 
    }
  }
  free(data_buff_ptr);
}/* tcp_write */


/**
 * This function computes fd_max and initializes readfd_set with listen fd.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     select timeout for request from client/peer.
 * @return    fd_max is returned to caller.
 */
int tcp_iomux_init (int time_wait)
{
  int rc       = -1;
  int loop_idx = -1;
  int fd_max   = -1;

  /* Initializing fdset to zero */
  FD_ZERO(&g_tcp_ctx_main.readfd_set);
  FD_ZERO(&g_tcp_ctx_main.writefd_set);
  FD_ZERO(&g_tcp_ctx_main.excepfd_set);


  g_tcp_ctx_main.to.tv_sec  = time_wait;
  /* milli seconds */
  g_tcp_ctx_main.to.tv_usec = 500000;

  for (loop_idx = 0; loop_idx < TCP_MAX_CTX ;loop_idx++)
  {
    if ( (FD_STATE_CREATED == g_tcp_ctx_main.ctx[loop_idx].fd_state) ||
         (FD_STATE_LISTEN  == g_tcp_ctx_main.ctx[loop_idx].fd_state))
    {
      fd_max = (fd_max > g_tcp_ctx_main.ctx[loop_idx].listen_fd) ?
               fd_max :
               g_tcp_ctx_main.ctx[loop_idx].listen_fd;
      
      g_tcp_ctx_main.fd_max = fd_max;

      FD_SET(g_tcp_ctx_main.ctx[loop_idx].listen_fd,
             &g_tcp_ctx_main.readfd_set);
    }
  }
  return (g_tcp_ctx_main.fd_max);
}/* tcp_iomux */


/**
 * This function is to process client/peer request. The client/peer request 
 * could be - New Request, Data to be sent and Data to be received and it 
 * invokes the registered callback to deliver the response to upper layer which 
 * is HTTP layer.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     active_connections is represents the number of file descriptors 
 *            are ready for READ/WRITE or CLOSE.
 * @param     read fd set
 * @param     write fd set
 * @param     exception fd set
 * @return    return 0 always
 */
int tcp_process_request (int active_connections,
                         fd_set rd,
                         fd_set wr,
                         fd_set ex)
{
  int fd_idx     = 0;
  int aux_fd     = 0;
  int bytes_read = 0;

  char data_buf[2448];
  int data_buf_len = sizeof(data_buf);
  
  /* fd_idx 0,1 and 2 is for stdin, stdout and stderr respectively. */
  for (fd_idx = 3; fd_idx <= g_tcp_ctx_main.fd_max; fd_idx++)
  {
    if (FD_ISSET(fd_idx, &rd))
    {
      if ( (FD_STATE_LISTEN == g_tcp_ctx_main.ctx[fd_idx].fd_state) &&
           (fd_idx == g_tcp_ctx_main.ctx[fd_idx].listen_fd) )
      {
        aux_fd = tcp_accept(fd_idx);
        g_tcp_ctx_main.ctx[fd_idx].tcp_cb_ind(aux_fd,
                                              MSG_NEW_CONNECTION_REQ,
                                              NULL,
                                              0);
        
        g_tcp_ctx_main.ctx[aux_fd].tcp_cb_ind = g_tcp_ctx_main.ctx[fd_idx].tcp_cb_ind;
      }
      else if ( (FD_STATE_CONNECTED == g_tcp_ctx_main.ctx[fd_idx].fd_state) &&
                (fd_idx == g_tcp_ctx_main.ctx[fd_idx].aux_fd) )
      {
        memset(data_buf, 0, data_buf_len);
        bytes_read = tcp_read (fd_idx, (char *)data_buf, data_buf_len);

        if (0 == bytes_read)
        {
          g_tcp_ctx_main.ctx[fd_idx].tcp_cb_ind(fd_idx,
                                                MSG_CLOSE_CONNECTION_REQ,
                                                NULL,
                                                0);
        }
        else
        {
          fprintf(stderr,"\nTCP DATA RECVEID<<====>>>[CONN_ID=%d]%*s\n",
                  fd_idx,bytes_read,data_buf);
          g_tcp_ctx_main.ctx[fd_idx].tcp_cb_ind(fd_idx,
                                                MSG_DATA_RECEIVED_REQ,
                                                (char *)&data_buf,
                                                bytes_read);
        }
      }
    }
    else if (FD_ISSET(fd_idx, &wr))
    {
      g_tcp_ctx_main.ctx[fd_idx].tcp_cb_ind(fd_idx,
                                            MSG_DATA_SENT_REQ,
                                            NULL,
                                            0);
    }
  }
  return (0);
}/*tcp_process_request*/


/**
 *  This function is used to wait for sent/receive data to/from client/peer 
 *  and delivers received data to be processed.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none.
 */
void tcp_select (void)
{
  int     rc = -1;
  fd_set  orig_readfd_set;
  fd_set  orig_writefd_set;
  fd_set  orig_excepfd_set;
  struct  timeval to;
  
  for(;;)
  {
    /* copy the original one because select clears the fd's upon its return */
    FD_ZERO(&orig_readfd_set);
    FD_ZERO(&orig_writefd_set);
    FD_ZERO(&orig_excepfd_set);
    
    orig_readfd_set  = g_tcp_ctx_main.readfd_set;
    orig_writefd_set = g_tcp_ctx_main.writefd_set;
    orig_excepfd_set = g_tcp_ctx_main.excepfd_set;
    
    /* Response timeout */
    to = g_tcp_ctx_main.to;

    rc = select((g_tcp_ctx_main.fd_max + 1),
                 (fd_set *)&orig_readfd_set,
                 (fd_set *)&orig_writefd_set,
                 (fd_set*)&orig_excepfd_set,
                 &to);

    if (rc > 0)
    {
       tcp_process_request (rc,
                            orig_readfd_set,
                            orig_writefd_set,
                            orig_excepfd_set);
    }
    else if (0 == rc)
    {
      /* timeout has happened */
      continue;
    }
    else
    {
      fprintf(stderr,"rc %d errorno %d\n", rc, errno);
    }
  }
}/* tcp_select */


/**
 * This is the main entry function for TCP layer functionality. The caller has 
 * to provide the call back function so that caller can get the notification for
 * 1. New Connection
 * 2. Data received
 * 3. Data sent
 * 4. Client disconnected.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     ip_address to be bind the TCP Server.
 * @param     TCP port to be listen for Request.
 * @param     HTTP callback through which received Data will be sent out to
 *            HTTP layer.
 * @return    none.
 */
void tcp_instance_create (char *ip_addr, int tcp_port, tcp_cb_t tcp_cb_ind)
{
  int sock_fd   = -1;
  int rc        = -1;
  
  /* Unit is in seconds */
  int time_wait = 2;
  
  /* at max 20 connections are accepted and beyond this connection is refused */
  int tcp_back_log = 20;
  
  sock_fd = tcp_socket ();
  
  rc = (int) tcp_bind (ip_addr, tcp_port, sock_fd);
  
  rc = (int) tcp_listen (sock_fd, tcp_back_log);
  
  g_tcp_ctx_main.ctx[sock_fd].tcp_cb_ind = tcp_cb_ind;
  
  (void)tcp_iomux_init (time_wait);
}/* tcp_instance_create */


void tcp_close_connection(int conn_id)
{
  FD_CLR(conn_id,&g_tcp_ctx_main.readfd_set);
  g_tcp_ctx_main.ctx[conn_id].fd_state = FD_STATE_RELEASED;
  (void) close(conn_id);
}


void tcp_new_connection(int conn_id)
{
  FD_SET(conn_id, &g_tcp_ctx_main.readfd_set);
}


void tcp_set_write_fd (int conn_id)
{
  FD_SET(conn_id, &g_tcp_ctx_main.writefd_set);
}


void tcp_set_read_fd (int conn_id)
{
  FD_SET(conn_id, &g_tcp_ctx_main.readfd_set);
}

void tcp_clr_read_fd (int conn_id)
{
  FD_CLR(conn_id, &g_tcp_ctx_main.readfd_set);

}

void tcp_clr_write_fd (int conn_id)
{
  FD_CLR(conn_id, &g_tcp_ctx_main.writefd_set);
}
