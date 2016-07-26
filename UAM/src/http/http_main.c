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
#include <stdlib.h>
#include <string.h>

#include <tcp_main.h>
#include <http_main.h>
#include <html_main.h>
#include <md5.h>

/* extern declarations */

extern void tcp_select (void);

extern void tcp_instance_create (char *ip_addr,
                                 int tcp_port,
                                 tcp_cb_t tcp_cb_ind);

extern int tcp_write (int conn_id,
                      char *data,
                      int data_len);


extern void MD5Init (struct MD5Context *context);
extern void MD5Update (struct MD5Context *context,
                       unsigned char const *buf,
                       size_t len);

extern void MD5Final (unsigned char digest[16],
                      struct MD5Context *context);

extern int html_main (int conn_id,
                      char *data);

/* Instantiation of HTTP Structure */

http_ctx_main_t g_http_ctx[256];


/**
 * This function converts hex string into character string.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     hex string.
 * @param     character string into which result will be stored.
 * @param     length of source string.
 * @return    0 upon success & -1 upon failure.
 */
static int hextochar ( char *src, unsigned char * dst, int len )
{
  char x[3];
  int n;
  int y;

  for (n=0; n < len; n++)
  {
    x[0] = src[n*2+0];
    x[1] = src[n*2+1];
    x[2] = 0;

    if (sscanf(x, "%2x", &y) != 1)
      return -1;

    dst[n] = (unsigned char) y;
  }
  return 0;
}/* hextochat */


/**
 * This function initializes the tcp context global instance to 0 except for
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
static int chartohex ( unsigned char *src, char *dst, int len )
{
  char x[3];
  int n;

  for (n=0; n < len; n++)
  {
    sprintf(x, "%.2x", src[n]);
    dst[n*2+0] = x[0];
    dst[n*2+1] = x[1];
  }
  dst[len*2] = 0;
  return 0;
}/* chartohex */


/**
 * This function initializes the tcp context global instance to 0 except for
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
char *http_encode_pap_password (char *password,
                                char *uam_secret,
                                char *challenge)
{
  int            n,m;
  unsigned char  user_password[RADIUS_PWSIZE + 1];
  unsigned char  p[RADIUS_PWSIZE + 1];
  int            plen = -1; 
  char *buffer;
  MD5_CTX context;
  
  /* challenge  */
  buffer = (char*) malloc(512);
  memset (buffer, 0, sizeof(buffer));
  strncpy ((char *)buffer, (const char *)challenge, strlen(challenge));
  
  /* converting from hex to character string */
  hextochar (buffer, challenge, MD5LEN);
  
  /* uamsecret */
  MD5Init(&context);
  MD5Update(&context, challenge, MD5LEN);
  MD5Update(&context, (unsigned char*)uam_secret, strlen(uam_secret));
  MD5Final(challenge, &context);
  
  /* password */
  
  memset(p, 0, sizeof(p));
  
  strncpy((char *)p, (const char *)password, (size_t)strlen(password));
  plen = strlen(password);

  for (m=0; m < plen;)
  {
    for (n=0; n < REDIR_MD5LEN; m++, n++)
    {
      user_password[m] = p[m] ^ challenge[n];
    }
  }
  
  chartohex(user_password, buffer, plen);
  return (buffer);
}/* http_encode_pap_password */


/**
 * This function retrieves the MIME HEADER FIELD VALUE from its stored global.
 *
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    length of the MIME FIELD VALUE
 */
int http_get_mime_param_value(int conn_id, char *mime_param_name, char *mime_param_value)
{
  int mime_idx             = 0;
  int mime_param_value_len = 0;


  for(mime_idx = 0; g_http_ctx[conn_id].max_mime_header_num; mime_idx++)
  {
    if (!strcmp ( g_http_ctx[conn_id].mime_header[mime_idx].mime_tag_name, mime_param_name ))
    {
      strcpy(mime_param_value, g_http_ctx[conn_id].mime_header[mime_idx].mime_value);
      mime_param_value_len = strlen ( mime_param_value );
      break;
    }
  }
  return ( mime_param_value_len );
}/* http_get_mime_param_value */


/**
 * This function initializes the tcp context global instance to 0 except for
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
int http_process_http_response (int conn_id, char *data, int data_len)
{
  char http_rsp[1<<20];
  char *http_rsp_ptr;
  char *html_page_ptr = NULL;
  char *image_content_ptr;
  int http_len = 0;
  char qs_param[20][256];
  char *uam_secret = "change-me";
  char file_name[30];
  char extension[10];
  int html_len = 0;
  char html_page[1<<10];
  char *md5_password = NULL;
  
  /* The asterisks suppress the assignemt */
  memset (file_name, 0, sizeof(file_name));
  memset (extension, 0, sizeof(extension));
  memset (html_page, 0, sizeof(html_page));
  
  sscanf (g_http_ctx[conn_id].resource_name, "%[^.].%s",
          file_name,
          extension);

  if (0 == strcmp(g_http_ctx[conn_id].resource_name, "/login_response.html"))
  {
    (void) http_get_param_value(conn_id, "username",  qs_param[0]);
    (void) http_get_param_value(conn_id, "password",  qs_param[1]);
    (void) http_get_param_value(conn_id, "userurl",   qs_param[2]);
    (void) http_get_param_value(conn_id, "challenge", qs_param[3]);
    
    md5_password = http_encode_pap_password (qs_param[1],
                                                   uam_secret,
                                                   qs_param[3]);
    html_len = snprintf ( html_page,
    		              sizeof(html_page),
						  "%s%s%s%s%s",
                          "<html><title></title><head></head>"
                          "<body><a href = http://192.168.3.1:3990/login?username=",
						  qs_param[0],
						  "&password=",
						  md5_password,
						  "><h1>Clink Here</h1></a></body></html>");

    http_len += snprintf ( http_rsp,
    		               sizeof(http_rsp),
						   "%s%s%s%s%s%s%s%s%s%s"
                           "%s%s%s%s%s%d%s%s",
                           "HTTP/1.1 302 Moved Temporarily\r\n"
                           "Connection: Keep-Alive\r\n",
                           "Location: ",
                           "http://192.168.3.1:3990/login?username=",
                     qs_param[0],
                     "&password=",
                     md5_password,
                     "&challenge=",
                     qs_param[3],
                     "&userurl=",
                     qs_param[2],
                     "\r\n",
                     "Content-Type: text/html\r\n",
                     "Accept-Language: en-US,en;q=0.5\r\n",
                     "Accept: text/*;q=0.3, text/html;q=0.7, text/html;level=1,"
                     "text/html;level=2;q=0.4, */*;q=0.5\r\n",
                     "Content-Length:",
                     html_len,
                     "\r\n\r\n\r\n",
                     html_page);
    free ( md5_password );
    http_len += html_len;
    (void) tcp_write ( conn_id, http_rsp, http_len );
  }
  else if (!strcmp ( g_http_ctx[conn_id].resource_name, "/prelogin.html" ))
  {
	 (void) http_get_param_value(conn_id, "res", qs_param[0]);
	 (void) http_get_param_value(conn_id, "userurl", qs_param[1]);
	 if (!strcmp(qs_param[0], "notyet"))
	 {
       html_len = html_main ( conn_id, (char **) &html_page_ptr );
       http_len = snprintf ( http_rsp,
    	   	                 sizeof(http_rsp),
                            "%s%s%d%s%s%s%s%s%s%s%s",
                            "HTTP/1.1 200 OK\r\n",
                            "Content-Length:",
                            html_len,
                            "\r\n",
                            "Connection: Keep-Alive",
                            "\r\n",
                            "Content-Type: text/html\r\n",
                            "Accept-Language: en-US,en;q=0.5\r\n",
                            "Accept: text/*;q=0.3, text/html;q=0.7, text/html;level=1,"
                            "text/html;level=2;q=0.4, */*;q=0.5",
                            /* separate between http header and its body */
                            "\r\n\r\n\r\n",
                            html_page_ptr );
	 }
	 else if (!strcmp(qs_param[0], "success"))
	 {
	   html_len = snprintf ( html_page,
		     		         sizeof(html_page),
		 					"%s%s%s",
		                    "<html><title></title><head></head>"
		                    "<body><a href=",
							qs_param[1],
							"><h1>being Redirected</h1></a></body></html>");

	   http_len = snprintf ( http_rsp,
		     	   	         sizeof(http_rsp),
		                     "%s%s%d%s%s%s%s%s%s%s%s%s%s",
		                     "HTTP/1.1 302 Move Temporarily\r\n",
		                     "Content-Length:",
		                     html_len,
		                     "\r\n",
		                     "Connection: Keep-Alive",
		                     "\r\n",
		                     "Content-Type: text/html\r\n",
		                     "Accept-Language: en-US,en;q=0.5\r\n",
		                     "Accept: text/*;q=0.3, text/html;q=0.7, text/html;level=1,"
		                     "text/html;level=2;q=0.4, */*;q=0.5\r\n",
							 "Location: ",
							 /*userurl i.e. the original URL*/
							 qs_param[1],
		                     /* separate between http header and its body */
		                     "\r\n\r\n\r\n",
		                     html_page_ptr );
	 }
    (void) tcp_write ( conn_id, http_rsp, http_len );
    free ( html_page_ptr );
  } 
  else if ( (!strcmp ( extension, "jpg" )) ||
            (!strcmp ( extension, "gif" )) ||
            (!strcmp ( extension, "jpeg" )) ||
            (!strcmp ( extension, "png" )) )
  {
   
    html_len = html_main ( conn_id, (char **) &html_page_ptr );
    http_len = snprintf ( http_rsp,
    		              sizeof(http_rsp),
                         "%s%s%d%s%s%s%s%s%s",
                         "HTTP/1.1 200 OK\r\n",
                         "Content-Length:",
                         html_len,
                         "\r\n",
                         "Connection: Keep-Alive\r\n",
                         "Content-Type: image/gif;image/jpeg;image/png;image/jpg;\r\n",
                         /*"Content-Type: image/gif\r\n",*/
                         "Accept-Language: en-US,en;q=0.5\r\n",
                         "Accept: image/*",
                         /* separate between http header and its body */
                         "\r\n\r\n\r\n");

    (void) tcp_write ( conn_id, http_rsp, http_len );
    (void) tcp_write ( conn_id, html_page_ptr, html_len );
    free ( html_page_ptr );
  }
  else
  {
    html_len = html_main ( conn_id, (char **) &html_page_ptr );
    http_len = snprintf ( http_rsp,
    		              sizeof(http_rsp),
                          "%s%s%d%s%s%s%s%s%s%s%s",
                          "HTTP/1.1 200 OK\r\n",
                          "Content-Length:",
                          html_len,
                          "\r\n",
                          "Connection: Keep-Alive",
                          "\r\n",
                          "Content-Type: text/html\r\n",
                          "Accept-Language: en-gb;q=0.8, en;q=0.7\r\n",
                          "Accept: text/*;q=0.3, text/html;q=0.7, text/html;level=1,"
                          "text/html;level=2;q=0.4, */*;q=0.5",
                          /* separate between http header and its body */
                          "\r\n\r\n\r\n",
                          html_page_ptr );
    (void) tcp_write ( conn_id, http_rsp, http_len );
    free ( html_page_ptr );
  }
  return ( http_len );
} /* http_process_http_response */


/**
 * This function initializes the tcp context global instance to 0 except for
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
int http_get_param_value(int conn_id, char *param_name, char *param_value)
{
  int idx             = 0;
  int param_value_len = 0;
    
  for(idx = 0; g_http_ctx[conn_id].param_list[idx].param_name; idx++)
  {
    if (0 == strcmp ( g_http_ctx[conn_id].param_list[idx].param_name, param_name ))
    {
      strcpy(param_value, g_http_ctx[conn_id].param_list[idx].param_value);
      param_value_len = strlen ( param_value );
      break;
    }
  }
  return ( param_value_len );
}/* http_get_param_value */

int http_decode_url(int conn_id)
{
  int idx = 0;
  int idx_base64 = 0;
  char *base64_decoded = NULL;

  base64_decoded = (char*) malloc ( strlen( g_http_ctx[conn_id].qs ) );

  if ( NULL == base64_decoded )
  {
    fprintf(stderr,"MALLOC FAILED\n");
    return(-1);
  }
  while ('\0' != g_http_ctx[conn_id].qs[idx])
  {
    if ('%' != g_http_ctx[conn_id].qs[idx])
    {
      base64_decoded[idx_base64++] = g_http_ctx[conn_id].qs[idx++];
    }
    else
    {
     if (!strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%3a",3) ||
         !strncmp((char *)&g_http_ctx[conn_id].qs[idx],"%3A",3))
     {
       base64_decoded[idx_base64++] = ':';
       idx+= 3;    
     } 
     else if (!strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%2f",3) ||
             !strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%2F",3))
     {
       base64_decoded[idx_base64++] = '/';    
       idx+= 3;    
     }
     else if (!strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%3f",3) ||
              !strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%3F",3) )
     {
       base64_decoded[idx_base64++] = '?';    
       idx+= 3;    
     }
     else if (!strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%3d",3) ||
              !strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%3D",3))
     {
       base64_decoded[idx_base64++] = '=';    
       idx+= 3;    
     }
     else if (!strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%26",3))
     {
       base64_decoded[idx_base64++] = '&';    
       idx+= 3;    
     }
     else if (!strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%253a",5))
     {
       base64_decoded[idx_base64++] = ':';    
       idx+= 5;    
     }
     else if (!strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%252f",5))
     {
       base64_decoded[idx_base64++] = '/';    
       idx+= 5;    
     }
     else if (!strncmp((char*)&g_http_ctx[conn_id].qs[idx],"%253f",5))
     {
       base64_decoded[idx_base64++] = '?';    
       idx+= 5;    
     }
     else
     {
       fprintf(stderr,"QS %s\n",g_http_ctx[conn_id].qs);
       fprintf(stderr,"Nothing Matched\n");
     }
    }
  }
  
  base64_decoded[idx_base64] = '\0';
  strcpy ( g_http_ctx[conn_id].qs, base64_decoded );
  free ( base64_decoded );
}

/*
"loginurl=http://192.168.3.1:8989/prelogin.html?res=notyet&uamip=192.168.3.1&uamport=3990&challenge=3b12e0d4be4270ab4554c9a96d91efbe&called=44-37-E6-37-D3-61&mac=54-4E-90-A1-FB-37&ip=192.168.3.3&nasid=nas01&sessionid=574f032100000002&userurl=http%3a%2f%2fcaptive.apple.com%2fhotspot-detect.html&md=5074BAE26CC5522A709A8143E8D4AC51"
*/

/*
loginurl=http%3a%2f%2f192.168.3.1%3a8989%2fprelogin.html%3fres%3dnotyet%26uamip%3d192.168.3.1%26uamport%3d3990%26challenge%3d3b12e0d4be4270ab4554c9a96d91efbe%26called%3d44-37-E6-37-D3-61%26mac%3d54-4E-90-A1-FB-37%26ip%3d192.168.3.3%26nasid%3dnas01%26sessionid%3d574f032100000002%26userurl%3dhttp%253a%252f%252fcaptive.apple.com%252fhotspot-detect.html%26md%3d5074BAE26CC5522A709A8143E8D4AC51
*/
int http_parse_qs(int conn_id)
{
  
  char login_uri[1024];
  char *login_uri_ptr = NULL;
  char qs_string[2048];
  char remaining_str[2048]; 
  int qs_param_idx = 0;
  int ret_value = -1;

  http_decode_url(conn_id);

  ret_value = sscanf ( g_http_ctx[conn_id].qs,
		               "%[^\?]\?%s",
					   login_uri,
					   qs_string );
 
  if ( ret_value > 1 )
  {
    sscanf ( login_uri,
    		"%[^=]=%s",
           g_http_ctx[conn_id].param_list[qs_param_idx].param_name,
           g_http_ctx[conn_id].param_list[qs_param_idx].param_value);
  }
  else 
  {
    strcpy(qs_string, g_http_ctx[conn_id].qs);
  }

  login_uri_ptr = strtok(qs_string,"&");
  do
  {
    qs_param_idx++;
    sscanf ( login_uri_ptr,
	  	   "%[^=]=%s",
           g_http_ctx[conn_id].param_list[qs_param_idx].param_name,
           g_http_ctx[conn_id].param_list[qs_param_idx].param_value);

  }while (NULL != (login_uri_ptr = strtok(NULL,"&")));
  g_http_ctx[conn_id].max_qs_param_list_num = qs_param_idx;
  return ( 0 );
}/* http_parse_qs */


/**
 * This function initializes the tcp context global instance to 0 except for
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
int http_parse_uri ( int conn_id )
{
  char proto[10];
  char tmp_res_name[256];
  int ret_status;

  /* sscanf stops when it encounters the white space */
  ret_status = sscanf ( g_http_ctx[conn_id].uri,
		                "%s %s %s",
                        g_http_ctx[conn_id].method,
                        g_http_ctx[conn_id].resource_name,
                        proto );

  /* Resource name shall be having file name and query string */
  ret_status = sscanf ( g_http_ctx[conn_id].resource_name,
                        "%[^?]?%s",
                        tmp_res_name,
                        g_http_ctx[conn_id].qs );

  strcpy ( g_http_ctx[conn_id].resource_name,
		  tmp_res_name );
  return ( ret_status );
}/* http_parse_uri */


/**
 * This function initializes the tcp context global instance to 0 except for
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
int http_parse_req ( int conn_id, char *http_req, int http_req_len )
{
  char *http_header_string;
  int mime_idx = 0;

  if (NULL != (http_header_string = strtok(http_req,"\r\n")))
  {
    strcpy(g_http_ctx[conn_id].uri, http_header_string);
  }
  while (NULL != (http_header_string = strtok(NULL,"\r\n")))
  {
    mime_idx++;
    /* starts the scanning until : is encountered & store them into global */
    sscanf(http_header_string,"%[^:]:%s",
           g_http_ctx[conn_id].mime_header[mime_idx].mime_tag_name,
           g_http_ctx[conn_id].mime_header[mime_idx].mime_value);
  }
  g_http_ctx[conn_id].max_mime_header_num = mime_idx;
  http_parse_uri(conn_id);
  return ( 0 );
}/* http_parse_req */


/**
 * This function initializes the tcp context global instance to 0 except for
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
int http_print_mime_header ( int conn_id )
{
  int loop_idx;
  for (loop_idx = 0;
       loop_idx <= g_http_ctx[conn_id].max_mime_header_num;
       loop_idx++)
  {
    fprintf(stderr,"\nMIME HEADER NAME %s \t MIME HEADER VALUE %s\n",
            g_http_ctx[conn_id].mime_header[loop_idx].mime_tag_name,
            g_http_ctx[conn_id].mime_header[loop_idx].mime_value);
    
  }
  fprintf(stderr,"\nURI[%d] ==>%s\n",conn_id,
          g_http_ctx[conn_id].uri);
  
  fprintf(stderr,"\nMETHOD[%d] %s RESOURCE %s QS %s\n",
          conn_id,
          g_http_ctx[conn_id].method,
          g_http_ctx[conn_id].resource_name,
          g_http_ctx[conn_id].qs);
  return ( 0 );
}/* http_print_mime_header */


/**
 * This function initializes the tcp context global instance to 0 except for
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
int http_process_http_req ( int conn_id, char *http_req, int http_req_len )
{
  char http_header[1024];
  char http_body[2048];
  int http_header_len;
  
  memset((void*)&http_header, 0, sizeof(http_header));
  memset((void*)&http_body, 0, sizeof(http_body));
  http_parse_req(conn_id, http_req, http_req_len);

  if (!strcmp(g_http_ctx[conn_id].resource_name, "/prelogin.html") ||
      !strcmp(g_http_ctx[conn_id].resource_name, "/login_response.html"))
  {
    http_parse_qs(conn_id);
  }
  
  return (0);
}/* http_process_http_req */


/**
 * This function initializes the tcp context global instance to 0 except for
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
int tcp_process_request_cb ( int fd, int msg_type, char *data, int data_len )
{
  switch ((msg_req_type_t)msg_type)
  {
    case MSG_NEW_CONNECTION_REQ:
      (void) tcp_new_connection ( fd );
      break;
      
    case MSG_DATA_RECEIVED_REQ:
      (void) tcp_clr_read_fd ( fd );
      http_process_http_req ( fd, data, data_len );
      (void) tcp_set_write_fd ( fd );
      break;
      
    case MSG_CLOSE_CONNECTION_REQ:
      (void) tcp_close_connection ( fd );
      break;
      
    case MSG_DATA_SENT_REQ:
      (void) tcp_clr_write_fd ( fd );
      http_process_http_response ( fd, data, data_len );
      (void) tcp_set_read_fd ( fd );
      break;
    default:
      //log_dbg("NO CASE IS MATCHED");
      break;
  }
  return ( 0 );
}/* tcp_process_request_cb */


/**
 * This function initializes the tcp context global instance to 0 except for
 * file descriptor descriptor for stdin, stdout and stderr.
 *
 * @author    Mohd Naushad Ahmed
 * @version   1.0
 * @param     none
 * @return    none
 */
int main (int argc, char *argv[])
{
  
  tcp_instance_create("192.168.3.1",8989,tcp_process_request_cb);
  tcp_select();
}
