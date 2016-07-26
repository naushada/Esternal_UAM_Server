#include <sys/types.h>
#include <stdio.h>
#include <string.h>

/* RFC 2865: Max 128 octets in password */
#define RADIUS_PWSIZE                    128
#define REDIR_MD5LEN                      16

typedef struct
{
  char mime_tag_name[256];
  char mime_value[256];
}http_header_t;

typedef struct
{
  char param_name[256];
  char param_value[256];
}http_qs_arg_t;

typedef struct 
{
  char query_string[512];
  /* Contains Methos, resource and qs */
  char uri[512];
  /* Method GET/POST/PUT/DELETE */
  char method[10];
  /* Query string */
  char qs[1024];
  /* Resource name is the name of html file in the request */
  char resource_name[512];
  int  max_mime_header_num;
  http_header_t mime_header[256];
  http_qs_arg_t param_list[256];
  int  max_qs_param_list_num;
 
}http_ctx_main_t;

int process_http_req(int conn, char *req_data, int req_len);

int http_parse_req(int conn_id, char *http_req, int http_req_len);
