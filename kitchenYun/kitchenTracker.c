//Import the Libraries Required 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <termios.h> 
#include "pubnub_sync.h"
#include "pubnub_helper.h"
#include "include/jsonParse.h"
#include "pubnub_timers.h"

//Pubnub Publish and Subscribe Keys
const char *g_pub_key = "pub-c-913ab39c-d613-44b3-8622-2e56b8f5ea6d";
const char *g_sub_key = "sub-c-8ad89b4e-a95e-11e5-a65d-02ee2ddab7fe";

const char *g_container1 = "001";
const char *g_container2 = "002";

/*Characted String used to form the json data to be sent 
to the app using json_data function */
char g_jsonResponse[26] = "";

int g_uart0_filestream = -1;
int g_firstChar = 0,g_secondChar = 0,g_thirdChar = 0;

int pubnub_publishStatus(char *p_data);
static void generate_uuid(pubnub_t *pbp);
int uartInit(char *port);

int uartInit(char *port)
{
	g_uart0_filestream = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
	if(g_uart0_filestream == -1)
	{
		printf("Error Connecting with the Device \n");
		return -1;
	}
	//Setting the Baud Rate and No. of the Stop Bits for the UART
	struct termios options;
	tcgetattr(g_uart0_filestream,&options);
	//BAUD Rate Initialized to 9600 bps
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(g_uart0_filestream, TCIFLUSH);
	tcsetattr(g_uart0_filestream,TCSANOW,&options);
	return 0;
}

static void generate_uuid(pubnub_t *pbp)
{
    char const *uuid_default = "zeka-peka-iz-jendeka";
    struct Pubnub_UUID uuid;
    static struct Pubnub_UUID_String str_uuid;

    if (0 != pubnub_generate_uuid_v4_random(&uuid)) {
        pubnub_set_uuid(pbp, uuid_default);
    }
    else {
        str_uuid = pubnub_uuid_to_string(&uuid);
        pubnub_set_uuid(pbp, str_uuid.uuid);
        printf("Generated UUID: %s\n", str_uuid.uuid);
    }
}

int pubnub_publishStatus(char *p_data){
	enum pubnub_res res;
    char const *chan = "kitchenDevice-resp";
    pubnub_t *pbp = pubnub_alloc();

    if (NULL == pbp) {
        printf("Failed to allocate Pubnub context!\n");
        return -1;
    }
    pubnub_init(pbp, g_pub_key,g_sub_key);
    pubnub_set_transaction_timeout(pbp, PUBNUB_DEFAULT_NON_SUBSCRIBE_TIMEOUT);
    /* Leave this commented out to use the default - which is
       blocking I/O on most platforms. Uncomment to use non-
       blocking I/O.
    */
	//    pubnub_set_non_blocking_io(pbp);
    generate_uuid(pbp);
    pubnub_set_auth(pbp, "danaske");
    res = pubnub_publish(pbp, chan, p_data);
    if (res != PNR_STARTED) {
        printf("pubnub_publish() returned unexpected: %d\n", res);
        pubnub_free(pbp);
        return -1;
    }
    res = pubnub_await(pbp);
    if (res == PNR_STARTED) {
        printf("pubnub_await() returned unexpected: PNR_STARTED(%d)\n", res);
        pubnub_free(pbp);
        return -1;
    }
    if (PNR_OK == res) {
        //printf("Published! Response from Pubnub: %s\n", pubnub_last_publish_result(pbp));
    }
    else if (PNR_PUBLISH_FAILED == res) {
        printf("Published failed on Pubnub, description: %s\n", pubnub_last_publish_result(pbp));
    }
    else {
        printf("Publishing failed with code: %d\n", res);
    }
    return 0;
}

void prepare_json_data(const char *p_containerId,float p_weight)
{
	char l_buf [8] = "";
	strcat(g_jsonResponse,"{\"containerID\":\"");
	strcat(g_jsonResponse,p_containerId);
	snprintf(l_buf,sizeof(l_buf),"%4.2f",p_weight);
	strcat(g_jsonResponse,"\",\"weight\":");
	strcat(g_jsonResponse,l_buf);
	strcat(g_jsonResponse,"}");
	memset(l_buf, 0, sizeof(l_buf));
}

int main(int argc,char *argv[])
{
	if((uartInit(argv[1])) == 0)
	{
		while(1)
	    {
			if (g_uart0_filestream != -1)
			{
				char l_rxBuffer[50];
				int l_rxLength = read(g_uart0_filestream,(void*)l_rxBuffer, 50);
				if (l_rxLength > 0)
				{
					l_rxBuffer[l_rxLength] = '\0';
				}
				if((l_rxLength > 32) && (l_rxBuffer[l_rxLength-3] == '}') && (l_rxBuffer[0] == '{')){
					struct json_token *l_arr, *l_tok;
					char l_container1_str[10];
					char l_container2_str[10];
					float l_container1_value;
					float l_container2_value;
					float l_container1_prevValue;
					float l_container2_prevValue;
					l_arr = parse_json2(l_rxBuffer, strlen(l_rxBuffer));
					l_tok = find_json_token(l_arr, "g_container1");
  					sprintf(l_container1_str,"%.*s",l_tok->len,l_tok->ptr);
  					l_container1_value = strtof(l_container1_str,NULL);
  					if(l_container1_value != l_container1_prevValue){
  						l_container1_prevValue = l_container1_value;
  						prepare_json_data(g_container1,l_container1_value);
  						pubnub_publishStatus(g_jsonResponse);
  						memset(g_jsonResponse, 0, sizeof(g_jsonResponse));
					}  						
  					l_tok = find_json_token(l_arr, "g_container2");
  					sprintf(l_container2_str,"%.*s",l_tok->len,l_tok->ptr);
  					l_container2_value = strtof(l_container2_str,NULL);
  					if(l_container2_value != l_container2_prevValue){
  						l_container2_prevValue = l_container2_value;
  						prepare_json_data(g_container2,l_container2_value);
  						pubnub_publishStatus(g_jsonResponse);
  						memset(g_jsonResponse, 0, sizeof(g_jsonResponse));
  					}
  					free(l_arr);
				}
			}
        	usleep(500000);
		}
	    close(g_uart0_filestream);
	}
	else
	{
		printf("UART Initialization Failed, Aborting");
		return -1;
	}
	return 0;
}
