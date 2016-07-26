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

#include<stdio.h>
#include<sys/types.h>


typedef int (*html_response_page_cb)(int conn_id,
                                     char *html_page,
                                     int *html_page_len);

typedef enum
{
  HTML_LOGIN_PAGE = 0x00000000,
  HTML_LOGIN_PAGE_RESPONSE,
  HTML_REGISTER_PAGE,
  HTML_REGISTER_PAGE_RESPONSE,
  HTML_WAIT_PAGE,
  /* Add enumeration just above it */
  HTML_END_PAGE
}html_page_type_t;

typedef struct
{
    html_page_type_t page_type;
    html_response_page_cb rsp_cb;
    char resource_name[512];
}html_ctx_main_t;


int html_login_page (int conn_id,
                     char *html_page,
                     int *html_page_len);

int html_login_page_response (int conn_id,
                              char *html_page,
                              int *html_page_len);

int html_register_page_response (int conn_id,
                                 char *html_page,
                                 int *html_page_len);

int html_register_page (int conn_id,
                        char *html_page,
                        int *html_page_len);

int html_wait_page (int conn_id,
                    char *html_page,
                    int *html_page_len);

int html_prelogin_page (int conn_id,
                        char *html_page_ptr,
                        int *html_page_len);

int html_read_image_file (char *file_name,
                          char *contents,
                          int *contents_length);

int html_logo_page (int conn_id,
                    char *html_page,
                    int *html_page_len);



