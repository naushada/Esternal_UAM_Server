typedef enum
{
  Access_Request = 1,
  Access_Accept,
  Access_Reject,
  Accounting_Request,
  Accounting_Response,
  Access_Challenge = 11,
  Status_Server,
  Status_Client,
  Radius_Reserved = 255
}radius_request_type_t;

typedef enum
{
  Login = 1,
  Framed,
  Callback_Login,
  Callback_Framed,
  Outbound,
  Administrative,
  NAS_Prompt,
  Authenticate_Only,
  Callback_NAS_Prompt,
  Call_Check,
  Callback_Administrative,
}radius_service_type_t;



/*
 Radius Attributes:
 0                   1                   2
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
 |     Type      |    Length     |  Value ...
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
*/

typedef enum
{
  User_Name = 1, 
  User_Password = 2,
  CHAP_Password = 3,
  NAS_IP_Address = 4, 
  NAS_Port = 5, 
  Service_Type = 6,
  Framed_Protocol = 7, 
  Framed_IP_Address = 8, 
  Framed_IP_Netmask = 9,
  Framed_Routing, 
  Filter_Id, 
  Framed_MTU, 
  Framed_Compression,
  Login_IP_Host,
  Login_Service, 
  Login_TCP_Port, 
  RADIUS_Reserved_1,
  Reply_Message, 
  Callback_Number, 
  Callback_Id, 
  unassigned,
  Framed_Route, 
  Framed_IPX_Network, 
  State, 
  Class, 
  Vendor_Specific,
  Session_Timeout, 
  Idle_Timeout, 
  Termination_Action, 
  Called_Station_Id,
  Calling_Station_Id, 
  NAS_Identifier, 
  Proxy_State, 
  Login_LAT_Service,
  Login_LAT_Node, 
  Login_LAT_Group, 
  Framed_AppleTalk_Link, 
  Framed_AppleTalk_Network,
  Framed_AppleTalk_Zone,
  /* 40 - 59 is reserved fro accounting */
  CHAP_Challenge = 60, 
  NAS_Port_Type, 
  Port_Limit, 
  Login_LAT_Port
}radius_attr_type_t;



/* Function signature */
int radius_prepare_access_req_attr (int conn_id, 
                                    char *acc_req_byte_buffer);

int radius_encode_tlv (char tag, 
                       short int length, 
                       char *value, 
                       char *encoded_byte_buffer, 
                       int *encoded_offset);

int radius_send_to (char *ip, 
                    int port, 
                    char *data, 
                    int data_len);

int radius_access_req_main (int conn_id, 
                            char *byte_buffer);

