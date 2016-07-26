#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<http_main.h>
#include<rad_client.h>

/*
  Radius Packets Format
 0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Code      |  Identifier   |            Length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   |                     Request Authenticator                     |
   |                                                               |
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Attributes ...
   +-+-+-+-+-+-+-+-+-+-+-+-+- 
 */

/* Holds the query string and uri along with conn_id */
extern http_ctx_main_t g_http_ctx[];
extern int http_get_param_value(int conn_id, char *param_name, char *param_value);


int radius_prepare_access_req_attr(int conn_id, char *byte_buffer)
{
  char *param_value;
  int byte_buffer_len = 0;
  param_value = (char *) malloc(256);
  memset(byte_buffer, 0, 1024);
  (void) http_get_param_value(conn_id, "username", param_value);
  fprintf(stderr,"value of user name %s\n",param_value);
  (void) radius_encode_tlv ((char)User_Name, (strlen(param_value)+1), param_value, byte_buffer, &byte_buffer_len);
  
  fprintf(stderr,"value of byte buffer  %s\n",byte_buffer);
  (void) http_get_param_value(conn_id, "password", param_value);

  fprintf(stderr,"value of password %s\n",param_value);
  (void) radius_encode_tlv ((char)User_Password, (strlen(param_value) +1), param_value, (byte_buffer + byte_buffer_len), &byte_buffer_len);
  
  param_value[0] = 1;
  param_value[1] = '\0';
  (void) radius_encode_tlv ((char)Service_Type, 1, param_value, (byte_buffer + byte_buffer_len), &byte_buffer_len);

  (void) http_get_param_value(conn_id, "sessionid", param_value);
  (void) radius_encode_tlv ((char)Calling_Station_Id, strlen(param_value), param_value, (byte_buffer + byte_buffer_len), &byte_buffer_len);

  (void) http_get_param_value(conn_id, "ip", param_value);
  (void) radius_encode_tlv ((char)Framed_IP_Address, strlen(param_value), param_value, (byte_buffer + byte_buffer_len), &byte_buffer_len);

  (void) http_get_param_value(conn_id, "mac", param_value);
  (void) radius_encode_tlv ((char)Calling_Station_Id, strlen(param_value), param_value, (byte_buffer + byte_buffer_len), &byte_buffer_len);

  (void) http_get_param_value(conn_id, "called", param_value);
  (void) radius_encode_tlv ((char)Called_Station_Id, strlen(param_value), param_value, (byte_buffer + byte_buffer_len), &byte_buffer_len);

  (void) http_get_param_value(conn_id, "uamip", param_value);
  (void) radius_encode_tlv ((char)NAS_IP_Address, strlen(param_value), param_value, (byte_buffer + byte_buffer_len), &byte_buffer_len);

#if 0
  (void) http_get_param_value(conn_id, "md", param_value);
  (void) radius_encode(tlv(User_Name, strlen(param_value), param_value, byte_buffer, &byte_buffer_len);
#endif 
  return(byte_buffer_len);

} /* readius_prepare_access_req_attr */



int radius_encode_tlv (char tag, 
                       short int length, 
                       char *value, 
                       char *encoded_byte_buffer, 
                       int *encoded_offset)
{
  int tmp_len = -1;

  if (NULL == encoded_byte_buffer) 
  {
    return(-1);
  }
  encoded_byte_buffer[++tmp_len] = tag;
  encoded_byte_buffer[++tmp_len] = htons(length) & 0xFF00;
  encoded_byte_buffer[++tmp_len] = htons(length) & 0x00FF;
  
  memcpy((void *)&encoded_byte_buffer[++tmp_len], (void *)value, (size_t)length);
  tmp_len += length;
  *encoded_offset = tmp_len + 1;
}/* radius_encode_tlv */

int radius_access_req_main(int conn_id, char *byte_buffer)
{
  int attribute_list_len = -1;
  attribute_list_len = radius_prepare_access_req_attr(conn_id, byte_buffer);
  fprintf(stderr,"radius_access_req_main buffer length %d\n",attribute_list_len);
  fprintf(stderr,"byte_buffer %s\n",byte_buffer);
  radius_prepare_access_request (conn_id, byte_buffer, attribute_list_len);
}

int radius_send_to(char *ip, int port, char *data, int data_len)
{
  int status = -1;
  struct sockaddr_in addr;
  int fd;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);
  memset(addr.sin_zero,'0', sizeof(addr.sin_zero));
  char recv_resp[1024];
  size_t addr_len;

  fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (fd < 0 )
  {
    fprintf(stderr,"socket creation failed\n");
    return (-1);
  }
  
  status = sendto(fd, data, data_len, 0, (struct sockaddr *)&addr,sizeof(addr));  
  if (fd < 0)
  {
    fprintf(stderr,"sendto Failed\n");
    return(-1);
  }
  status = recvfrom(fd,(void *)&recv_resp, (size_t)1024, 0, (struct sockaddr *)&addr, (socklen_t *)&addr_len);
  if (status < 0 )
  {
    return (-1);
  }
  fprintf(stderr,"Response received %s\n", recv_resp);
}/* radius_send_to */


int radius_prepare_access_request (int conn_id, char *byte_buffer, int byte_buffer_len)
{
  char access_req[1024];
  char *access_req_ptr = NULL;
  int index = 0;
  FILE *urandom_fp = NULL;
  int return_status = -1;
  char auth_ip[256];

  urandom_fp = fopen("/dev/urandom", "r");
  if (NULL == urandom_fp)
  {
    return(-1);
  }
  memset ((void *)auth_ip, 0, (size_t)256);
  (void) http_get_param_value(conn_id, "uamip", auth_ip);
 
  access_req_ptr = &access_req;
// (void) radius_encode(tlv(Access_Request, strlen(param_value), param_value, byte_buffer, &byte_buffer_len); 
  access_req[index++] = Access_Request;
  access_req[index++] = 0; /* needs to be changed latter */
  return_status = fread ((access_req + index), 1, 16 /*Authenticator Length */, urandom_fp);
  if (return_status < 0)
  {
    return (-1);
  }
  index += 16;
  /* Length of Access-Request */
  access_req[index++] = htons (byte_buffer_len + 16 + 4) ;
  /* Copying the Radius Attributes */
  memcpy ((access_req + index), byte_buffer, byte_buffer_len);
  fprintf(stderr,"access_req length %d\n",index);
  radius_send_to (auth_ip, 1812 , access_req_ptr, (byte_buffer_len + 16 + 4)); 
   
}/* radius_prepare_access_request */
