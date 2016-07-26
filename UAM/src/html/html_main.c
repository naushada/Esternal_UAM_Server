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
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 

#include<tcp_main.h>
#include<http_main.h>
#include<rad_client.h>
#include<html_main.h>


#define IMAGE_FILE_PATH "../src/html/"
#define get_image_path(image_name)      ("../src/html/img/" ## image_name)

/* Defined in the html folder */
extern http_ctx_main_t g_http_ctx[256];

int http_get_param_value(int conn_id, char *param_name, char *param_value);

html_ctx_main_t g_html_ctx_main[] =
{
  {HTML_LOGIN_PAGE,    html_prelogin_page,         "/prelogin.html"},
  {HTML_LOGIN_PAGE ,   html_login_page,             "/login.html"},
  {HTML_LOGIN_PAGE ,   html_wait_page,              "/wait.gif"},
  {HTML_LOGIN_PAGE ,   html_logo_page,              "/coova.png"},
  {HTML_LOGIN_PAGE ,   html_login_page_response,    "/login_response.html"},
  {HTML_REGISTER_PAGE, html_register_page,          "/register.html"},
  {HTML_REGISTER_PAGE, html_register_page_response, "/register_response.html"},
  
  /* This line shall be the last line */
  {HTML_END_PAGE,      NULL,                        NULL},
};


int html_login_page_response(int conn_id, char **html_page, int *html_page_len)
{
  *html_page = (char *) malloc(1024);
  *html_page_len = snprintf(*html_page,1024,"%s%s",
           "<html><head><title>",
           "OpenCoovaChilli"
           "</title></head><body><h1> SUCCESS</h1>"
           "</body></html>");

  return(*html_page_len);
}


int html_register_page_response(int conn_id, char **html_page, int *html_page_len)
{
 return(*html_page_len) ;
}


int html_login_page(int conn_id, char **html_page, int *html_page_len)
{
  char param_value[10][512];
  *html_page_len = 0;
   *html_page = (char *) malloc(1024);
  
  (void) http_get_param_value(conn_id, "userurl",   param_value[0]);
  (void) http_get_param_value(conn_id, "sessionid", param_value[1]);
  (void) http_get_param_value(conn_id, "ip",        param_value[2]);
  (void) http_get_param_value(conn_id, "mac",       param_value[3]);
  (void) http_get_param_value(conn_id, "called",    param_value[4]);
  (void) http_get_param_value(conn_id, "uamip",     param_value[5]);
  (void) http_get_param_value(conn_id, "challenge", param_value[6]);
  (void) http_get_param_value(conn_id, "res",       param_value[7]);
    
  if (0 == strcmp(param_value[7], "success"))
  {
    return (html_login_page_response (conn_id, html_page, html_page_len));
  }
  *html_page_len = snprintf(*html_page, 1024,
           "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
           "<html><head><title>",
           "OpenCoovaChilli",
           "</title></head><body>"
           "<form method=\"GET\" action=\"/login_response.html\">"
           "<div id=\"login-form\">"
           "<table><tr> <td>Username:</td>"
           "<td><INPUT NAME=\"username\" VALUE=\"\"></td>"
           "</tr><tr> <td>Password:</td><br>"
           "<td><INPUT NAME=\"password\" VALUE=\"\" TYPE=\"password\"></td>"
           "</tr><tr><td><INPUT NAME=\"userurl\" VALUE=\"",
           param_value[0],
           "\" TYPE=\"hidden\"></td>"
           "</tr><tr><td><INPUT NAME=\"sessionid\" VALUE=\"",
           param_value[1],
           "\" TYPE=\"hidden\"></td>"
           "</tr><tr><td><INPUT NAME=\"ip\" VALUE=\"",
           param_value[2],
           "\" TYPE=\"hidden\"></td>"
           "</tr><tr><td><INPUT NAME=\"mac\" VALUE=\"",
           param_value[3],
           "\" TYPE=\"hidden\"></td>"
           "</tr><tr><td><INPUT NAME=\"called\" VALUE=\"",
           param_value[4],
           "\" TYPE=\"hidden\"></td>"
           "</tr><tr><td><INPUT NAME=\"uamip\" VALUE=\"",
           param_value[5],
           "\" TYPE=\"hidden\"></td>"
           "</tr><tr><td><INPUT NAME=\"challenge\" VALUE=\"",
           param_value[6],
           "\" TYPE=\"hidden\"></td>"
           "</tr><tr><td><input type=\"submit\" name=\"button\" value=\"OpenCaptivePortal\">"
           "</td></tr></table></div></form></body></html>");
  return(*html_page_len);
}/* html_login_page */


int html_register_page(int conn_id, char **html_page_ptr, int *html_page_len)
{
  return(1);
}


int html_prelogin_page(int conn_id, char **html_page_ptr, int *html_page_len)
{
   *html_page_ptr = (char *) malloc(1024);
   if (NULL == html_page_ptr)
   {
     return (-1);
   } 
  *html_page_len = snprintf (*html_page_ptr, 1024,"%s%s%s%s%s%s%s%s%s%s%s%s%s",
    "<html><head><title>OpenCoova</title>",
    "<meta http-equiv=\"refresh\" content=\"7; URL=/login.html\">",
    "</head>",
    "<body style=\"margin: 0pt auto; height:100%;\">",
    "<div style=\"width:100%;height:80%;position:fixed;display:table;\">",
    "<p style=\"display: table-cell; line-height: 2.5em;",
    "vertical-align:middle;text-align:center;color:grey;\">",
    "<img src=\"coova.png\" alt=\"\" border=\"0\" height=\"39\" width=\"123\"></img>",
    "<small><img src=\"wait.gif\">Redirecting...</img></small></p>",
    "<br><br>",
    "</div>",
    "</body>",
    "</html>");
    return(*html_page_len);

}/* html_prelogin_page */

int html_read_image_file (char *file_name, char **contents, int *contents_length)
{
  int fd        = -1;
  char *data    = NULL;
  int file_size = -1;
  struct stat stat_buff;
  FILE *fp      = NULL;
  char abs_file_name[256];
  sprintf(abs_file_name, "/home/naushada/hotspot/http_server/img/%s",file_name);
  
  fp = fopen (abs_file_name, "rb");
  if (NULL == fd)
  {
    fprintf(stderr, "\nOpening of file %s failed\n", abs_file_name);
  }
   
  //fd = open (abs_file_name, O_RDONLY);
  fd = fileno (fp);
    
  if ( fd < 0 )
  {
    fprintf (stderr,"Opening of wait.gif Failed\n");
    return ( fd );
  }
    
  if ( fstat(fd, &stat_buff) < 0 )
  {
    fprintf (stderr, "stats of file wait.");
    close (fd);
    return(-1);
  }
    
  *contents = (char *) malloc(stat_buff.st_size);
  if ( NULL == contents )
  {
    fprintf(stderr,"Allocation failed for %d\n",stat_buff.st_size);
    close (fd);
    return (-1);
  }
  memset(*contents, 0, stat_buff.st_size);

  file_size = fread(*contents, stat_buff.st_size, 1, fp);
  if ( file_size <= 0 )
  {
    fprintf(stderr,"read Failed for size %d\n",stat_buff.st_size);
    return (-1);
  }
  /* Closing the FIle Now */
  close (fd);
  close(fp);
  *contents_length = stat_buff.st_size;
  return (*contents_length);
}/* html_read_image_file */


int html_wait_page(int conn_id, char **html_page, int *html_page_len)
{
  char *file_name = "wait.gif";
  return (html_read_image_file (file_name, html_page, html_page_len));
    
}/* html_wait_page */


int html_logo_page(int conn_id, char **html_page, int *html_page_len)
{
  char *file_name = "coova.png";
  return (html_read_image_file (file_name, html_page, html_page_len));
    
}/* html_wait_page */


int html_main(int conn_id, char **html_page_ptr)
{
 int idx           = 0;
 int html_page_len = 0;

 for (idx = 0; idx < HTML_END_PAGE; idx++)
 {
   if (!strcmp(g_html_ctx_main[idx].resource_name, g_http_ctx[conn_id].resource_name)) 
   {
      return (g_html_ctx_main[idx].rsp_cb (conn_id, html_page_ptr, &html_page_len));
     
   }
 } 
}/* html_main */
