#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "misc.h"
#include <stdio.h>
#include "stm32f4xx.h"

//protocol
#include "protocol.h"
#include<string.h>
#include<stdlib.h>
//HC-SR04
#include "defines.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_disco.h"
#include "tm_stm32f4_hcsr04.h"
TM_HCSR04_t HCSR04;

//photoresistor
#include "photoresistor.h"w

//dth11
#include "dht11.h"

struct DHT11_Dev dev;
volatile uint32_t msTicks;                      /* counts 1ms timeTicks       */

// SysTick_Handlern
void SysTick_HandlerX(void) {
  msTicks++;
}

void getTemparatureAndHumidity(int *temp, int *humi);

//wiatrak
void relayInit(void);

void LedInit(void);
void Delay_us(volatile uint32_t delay);
void startHotspot(void);
void startWaspWard(void);
void USART3Init(void);
void USART3_IRQHandler(void);
void USART_Send(volatile char *c);

void StartConnectonWithWapsCenter(void);
void SendToWaspCenter(volatile char* message);

const int SIZE_WIFI_BUFFOR = 1024;
char tabWifiBuffor[1024];

int read = 0;	//	0-czyta komunikaty	1-nie czyta komunikatÃ³w od wifi

//rzÂ¹danie GET
int read_channel = 0;
int channel = -1;
char discovery_channel;
int read_msg_size = 0;
char tab_msg_size[4];
int msg_size = -1;
const SIZE_MSG = 1024;
int read_msg = 0;
char tab_msg[1024];
int i = 0;

int new_msg = 0;

//szymon
char tabWifiResponse[10]={' '};
void checkWifiResponseOKOrERROR();

//test test test
void printfMagicTable(){

	int len = strlen(tabWifiBuffor);
	printf("\nWifi(%d):%s",len,tabWifiBuffor);
	Delay_us(10000);
	//clear tab
	for(int k=0; k<512; k++){
		tabWifiBuffor[k]=0;
	}

	//printf("Wifi Response: %s", tabWifiResponse);
}

//const DATA
const char* WARD_HOTSPOT_NAME = "Wasp1";
const char* WARD_PASSWORD_NAME = "Wasp1abc";
const int PORT = 80;

//test version
const char* HTML_SITE = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"/><script type=\"text/javascript\">function createLinkWithData(){var str=\"Dane zapisane. Kliknij, by wyslac dane do urzadzenia...\";var secret_password=document.getElementById(\"secret_password\").value;var wifi_name=document.getElementById(\"wifi_name\").value;var room_id = document.getElementById(\"room_id\").value;var wifi_password=document.getElementById(\"wifi_password\").value;var server_ip=document.getElementById(\"server_ip\").value;var server_port=document.getElementById(\"server_port\").value;var str2=\"DATA\";var hash=\"_\";var text=str2.concat(hash,secret_password,hash,room_id,hash,wifi_name,hash,wifi_password,hash,server_ip,hash,server_port,hash);var result=str.link(text);document.getElementById(\"link\").innerHTML=result; }</script><style type=\"text/css\" ref=\"stylesheet\">#logo{width: 350px;margin-left: auto;margin-right: auto;border-style: outset;border-radius: 50%;}body{background: #eae9c8;font-family:\"Trebuchet MS\",Helvetica,sans-serif;text-align:center;}</style></head><body><div id=\"logo\"><h1>WaspHouse Ward</h1></div><h4>Skonfiguruj ustawienia urzadzenia WaspWard</h4><br/><span>Sekretne haslo:</span></br><input type=\"password\" id=\"secret_password\" value=\"tajnehaslo\" maxlength=\"15\"/></br></br><span>Identyfikator pokoju:</span></br><input type=\"text\" id=\"room_id\" value=\"101\" maxlength=\"15\"/></br></br></br><b>Dane urzadzenia Center</b><br/></br><span>Identyfikator sieci:</span></br><input type=\"text\" id=\"wifi_name\" value=\"test\" maxlength=\"15\"/></br></br><span>Haslo sieci:</span></br><input type=\"password\" id=\"wifi_password\" value=\"admin123\" maxlength=\"15\"/></br></br><span>IP serwera:</span></br><input type=\"text\" id=\"server_ip\" value=\"192.168.43.129\" maxlength=\"15\"/></br></br><span>Port serwera:</span></br><input type=\"text\" id=\"server_port\" value=\"280\" maxlength=\"15\"/></br></br></br><button type=\"submit\" onclick=\"createLinkWithData()\"/>Zapisz</button></br></br><span id=\"link\" style=\"text-decoration: none;\"></span></body></html>\r\n";
//const char * HTML_SITE = "<h1>WORK!</h1>\r\n";
const char* HTML_SITE2 = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"/><style type=\"text/css\" ref=\"stylesheet\">#logo{width: 350px;margin-left: auto;margin-right: auto;border-style: outset;border-radius: 50%;}body{background: #eae9c8;font-family:\"Trebuchet MS\",Helvetica,sans-serif;text-align:center;}</style></head><body><div id=\"logo\"><h1>WaspHouse Ward</h1></div><h4>Ustawienia urzadzenia WaspWard zostana za chwile zmienione.<br/>Urzadzenie zostanie zrestartowane i podlaczy sie z siecia WaspHouse Center.</h4><br/><h2>Dziekujemy za poswiecony czas :)<h2></body></html>\r\n";


const char* NAME= "Wasp1";

const char* NAME_WASPCENTER_AP = "CENTER";
const char* PASSWORD_WASPCENTER_AP = "";
const char* IP_WASPCENTER_SERVER = "192.168.4.1";
const int PORT_WASPCENTER_SERVER = 80;

//
int typeRequest(char* text);
const char* REQUESET1_GET = "GET / HTTP/1.1";
const char* REQUESET2_DATA = "GET /DATA_";

int readDataToRequest(char* request);

int correctSecretePassword(char* pass);
const char SECRETE_PASSWORD[15] = "tajnehaslo";


//przesuwna tablica przechowujÄ…ca ostatnie znaki odebrane z wifi
const int SIZE_TABRES = 10;
char tabRes[10];

char secret_passwordX[15];
int room_id;
char wifi_name[15];
char wifi_password[15];
char server_ip[15];
int server_port;

// protocol
int systemID = 101;
int roomID = 1;

JSON device;
void sendJSON(JSON json);
void JSONToCharArray2(JSON structure);
void redefineJSON(JSON *json);
void doOrder(JSON json);
void updatePortPin(JSON json, GPIO_TypeDef *port, uint16_t pin);

char tabJSON[55];
char buffer[55];
int receivedJSON = 0;

//VERY IMPORTANT
int hotspot_or_ward = 0;

int main (void) {

	/*
	printf("\ncharArrayToJSON: ");
	JSON newJSON;
	charArrayToJSON(&newJSON, tabJSON);
	printf("\nJSON: {order:%d,systemID:%d,roomID:%d,value:%d}",newJSON.order,newJSON.systemID,newJSON.roomID,newJSON.value);
	*/

	SystemInit();

	device.order = 800;
	device.roomID = 101;
	device.systemID = 100;
	device.value = 100;

	LedInit();

	//DHT11
	SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
	if (SysTick_Config(SystemCoreClock / 1000)) { /* SysTick 1 msec interrupts  */
	while (1);                                  /* Capture error              */
	}

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	DHT11_init(&dev, GPIOB, GPIO_Pin_6);
	int temparature, humidity;
	getTemparatureAndHumidity(&temparature,&humidity);

	//WIFI INIT
	USART3Init();

	//photorestor init
	photoresistorInit();

	//HCSR04
	TM_DELAY_Init();
	/* Initialize distance sensor1 on pins; ECHO: PD0, TRIGGER: PC1 */
	if (!TM_HCSR04_Init(&HCSR04, GPIOD, GPIO_PIN_0, GPIOC, GPIO_PIN_1)) {
		printf("\nHCSR04 missing");
		while(1){}
	}

	//wiatrak
	relayInit();

	/*
	//konfiguracja hotspot
	startHotspot();

	hotspot_or_ward = 1;

	//i oczekiwanie na podanie danych waspcenter


	int endHot = 1;
	do{
		if(new_msg==1){
			new_msg=0;
			int t = typeRequest(tab_msg);

			if(t == 1){
				//read=1;
				SendToClient(HTML_SITE,channel);
				//read=0;
			}
			else if(t == 2){

				int x = readDataToRequest(tab_msg);

				int n = strlen(SECRETE_PASSWORD);
				endHot = 0;
				for(int i=0; i<n; i++){
					if(secret_passwordX[i]!=SECRETE_PASSWORD[i]){
						endHot=1;
					}
				}
				if(endHot==0){
					read=1;
					SendToClient(HTML_SITE2,channel);
					read=0;
				}

			}

			//clear tab msg
			for(int i=0; i<SIZE_MSG; i++) tab_msg[i] = 0;
			printfMagicTable();
		}
		//Delay_us(1000000);
		//printfMagicTable();
	}while(endHot);

	*/

	hotspot_or_ward = 2;
	startWaspWard();

	//connectWaspCenterNetwork(wifi_name, wifi_password);
	connectWaspCenterNetwork(NAME_WASPCENTER_AP, PASSWORD_WASPCENTER_AP);

	//StartConnectonWithWaspCenter(server_ip, server_port);
	StartConnectonWithWaspCenter(IP_WASPCENTER_SERVER, PORT_WASPCENTER_SERVER);

	while(1){}
}

void LedInit(void){
	/* GPIOD Periph clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef  GPIO_InitStructure;
	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void Delay_us(volatile uint32_t delay)
{
	delay*=24;
	while(delay--);
}

void connectWaspCenterNetwork(char* name, char* password)
{
	char buffer[64];
	int n = sprintf(buffer,"AT+CWJAP=\"%s\",\"%s\"\r\n",name,password);

	USART_Send(buffer);
	checkWifiResponseOKOrERROR();
	//Delay_us(20000000);

}

void startHotspot(void){
	read = 1;
	printf("\nSTART RESET ");
	USART_Send("AT+RST\r\n");
	Delay_us(2500000);
	printf(". \n");
	read = 0;

	USART_Send("AT+CWMODE=2\r\n");		// Access Point
	Delay_us(2000000);

	char buffer[64];
	int n = sprintf(buffer,"AT+CWSAP=\"%s\",\"%s\",4,2\r\n",WARD_HOTSPOT_NAME,WARD_PASSWORD_NAME);
	USART_Send(buffer);
	Delay_us(2000000);

	USART_Send("AT+CIPMUX=1\r\n");
	Delay_us(2000000);

	char buffer2[64];
	n = sprintf(buffer2,"AT+CIPSERVER=1,%d\r\n",PORT);
	USART_Send(buffer2);
	Delay_us(2000000);

	GPIO_SetBits(GPIOD,GPIO_Pin_15);
}

void startWaspWard(void)
{
	read = 1;
	printf("\nSTART RESET ");
	USART_Send("AT+RST\r\n");
	Delay_us(2500000);
	printf(". \n");
	read = 0;

	USART_Send("AT+CWMODE=1\r\n");
	//Delay_us(2000000);
	checkWifiResponseOKOrERROR();
	//USART_Send("AT+CIPMODE=2\r\n");
	//Delay_us(2000000);
	//checkWifiResponseOKOrERROR();
	//USART_Send("AT+CIPMUX=0\r\n");
	//Delay_us(2000000);
//	checkWifiResponseOKOrERROR();
//	USART_Send("AT+CIPMODE=0\r\n");
//	Delay_us(2000000);
	//checkWifiResponseOKOrERROR();
}

void USART3Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(USART3_IRQn);

	USART_Cmd(USART3, ENABLE);
}

void USART3_IRQHandler(void)
{
    if((USART_GetITStatus(USART3, USART_IT_RXNE) != RESET))
    {
    	char sign = USART3->DR;

    	if((read == 0) && ((((sign >= 32) && (sign <= 126)) || (sign == '\n')) || (sign == '\r'))){

    		if(sign == '\n')	sign = '|';
    		if(sign == '\r')	sign = '\\';

    		//test test test
    		int n = strlen(tabWifiBuffor);

			if(n < SIZE_WIFI_BUFFOR){
				tabWifiBuffor[n] = sign;
			}//test test test

			if(hotspot_or_ward == 2){

				if(sign == '{') {
					receivedJSON++;
					i = 0;
				}

				if(receivedJSON > 0) {
					buffer[i] = sign;
					i++;
				}

				if(sign == '}') {
					i = 0;
					receivedJSON = 0;
					printf("\njson: %s", buffer);
					printf("  %s  ", "kk");
					JSON json;
					charArrayToJSON(&json, &buffer);
					doOrder(json);
				}
			}

			//szymon
			for(int i=1;i<10;i++)
				tabWifiResponse[i-1]=tabWifiResponse[i];
			tabWifiResponse[9]=sign;


			for(int i=0;i<SIZE_TABRES-1;i++){
				tabRes[i]=tabRes[i+1];
			}

			tabRes[SIZE_TABRES-1]=sign;

			if(hotspot_or_ward == 1){

				if(read_msg==1 && msg_size==1){
					int n = strlen(tab_msg);
					tab_msg[n]=sign;
					msg_size--;
					read_msg = 0;
					GPIO_ToggleBits(GPIOD,GPIO_Pin_12);
					new_msg = 1;
				}
				if(read_msg==1 && msg_size>1){
					int n = strlen(tab_msg);
					tab_msg[n]=sign;
					msg_size--;
				}
				if(read_msg_size==1 && sign==':'){
					msg_size = atoi(tab_msg_size);
					read_msg_size = 0;
					read_msg = 1;
					//clear tab msg size
					for(int i=0; i<4; i++) tab_msg_size[i]=0;
				}
				if(read_msg_size==1 && sign!=':'){
					int n = strlen(tab_msg_size);
					tab_msg_size[n]=sign;
					//GPIO_ToggleBits(GPIOD,GPIO_Pin_13);
				}
				if(read_channel==1 && sign==','){
					read_channel = 0;
					read_msg_size = 1;

				}
				if(read_channel==1) {
					channel = sign - '0';
					discovery_channel = sign;

				}
				if(tabRes[SIZE_TABRES-5]=='+' && tabRes[SIZE_TABRES-4]=='I' && tabRes[SIZE_TABRES-3]=='P' && tabRes[SIZE_TABRES-2]=='D' && tabRes[SIZE_TABRES-1]==','){
					read_channel = 1;
					GPIO_ToggleBits(GPIOD,GPIO_Pin_14);
				}

			}

    	}
    }
}

void USART_Send(volatile char *c) {
	if(read == 0){
		printfMagicTable();
	}

	int length = strlen(c);
	for(int i=0; i<length; i++) {
		USART_SendData(USART3, c[i]);
		Delay_us(500);
	}
}

void StartConnectonWithWaspCenter(char* ip, int port){
	char buffer[64];
	int n = sprintf(buffer,"AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",ip,port);
	USART_Send(buffer);
	//Delay_us(2000000);
	checkWifiResponseOKOrERROR();

	JSONToCharArray2(device);
	SendToWaspCenter(tabJSON);

	//test test test
	//GPIO_SetBits(GPIOD,GPIO_Pin_15);
	printfMagicTable();
}

void SendToClient(volatile char* message, int channel)
{
	int length = strlen(message);

	char buffer[64];
	//int n = sprintf(buffer, "AT+CIPSEND=%d,%d\r\n", channel, length);
	int n = sprintf(buffer, "AT+CIPSEND=%d,%d\r\n",channel,length);
	USART_Send(buffer);
	Delay_us(2000000);

	USART_Send(message);

	Delay_us(2500000);

	//test test test
	printfMagicTable();
}

//send to waspcenter
void SendToWaspCenter(volatile char* message)
{
	int length = strlen(message);
	char buffer[32];
	int n = sprintf(buffer, "AT+CIPSEND=%d\r\n", length);
	USART_Send(buffer);
	checkWifiResponseOKOrERROR();
	USART_Send(message);
	checkWifiResponseOKOrERROR();
	printfMagicTable();
}

void getTemparatureAndHumidity(int *temp, int *humi)
{
	int res = DHT11_read(&dev);
	if(res == DHT11_ERROR_CHECKSUM) {
		*temp = 0;
		*humi = 0;
	}
	else if(res == DHT11_SUCCESS) {
		*temp = dev.temparature;
		*humi = dev.humidity;
	}
	else {
		*temp = 0;
		*humi = 0;
	}
}

int typeRequest(char* text){
	int lenght_request,n;

	//GET / HTTP/1.1
	n = 0;
	lenght_request = strlen(REQUESET1_GET);
	for(int i=0; i<lenght_request; i++){
		if(text[i] == REQUESET1_GET[i])	n++;
	}

	if(n == lenght_request)	return 1;

	//GET /DATA_
	n = 0;
	lenght_request = strlen(REQUESET2_DATA);
	for(int i=0; i<lenght_request; i++){
		if(text[i] == REQUESET2_DATA[i]) n++;
	}

	if(n == lenght_request)	return 2;

	return 0;
}


int readDataToRequest(char* request){
	int n = 0;
	int i = 0;
	while(request[n] != '_')	n++;
	n++;
	i=0;
	while(request[n] != '_'){
		secret_passwordX[i] = request[n];
		n++;
		i++;
	}
	n++;
	i=0;
	char number_room[3];
	while(request[n] != '_'){
		number_room[i] = request[n];
		n++;
		i++;
	}
	roomID = atoi(number_room);
	n++;
	i=0;
	while(request[n] != '_'){
		wifi_name[i] = request[n];
		n++;
		i++;
	}
	n++;
	i=0;
	while(request[n] != '_'){
		wifi_password[i] = request[n];
		n++;
		i++;
	}
	n++;
	i=0;
	while(request[n] != '_'){
		server_ip[i] = request[n];
		n++;
		i++;
	}
	n++;
	i=0;
	char number_port[3];
	while(request[n] != '_'){
		number_port[i] = request[n];
		n++;
		i++;
	}
	server_port = atoi(number_port);
	return 1;
}

void checkWifiResponseOKOrERROR(){
	int flag=1;
	while(flag){
		for(int i=0;i<9;i++){
			if(tabWifiResponse[i]=='O'&&tabWifiResponse[i+1]=='K')
				flag=0;
		}
		for(int i=0;i<6;i++){
			if(tabWifiResponse[i]=='E'&&tabWifiResponse[i+1]=='R'&&tabWifiResponse[i+2]=='R'&&tabWifiResponse[i+3]=='O'&&tabWifiResponse[i+4]=='R')
				flag=0;
		}
	}
	for(int i=0;i<10;i++){
		tabWifiResponse[i]=' ';
	}
}

void relayInit(void){//dioda albo wiatrak
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA, GPIO_Pin_7);
}

void updatePortPin(JSON json, GPIO_TypeDef *port, uint16_t pin) {
	switch(json.value) {
	case 100: GPIO_SetBits(port, pin); break;
	case 101: GPIO_ResetBits(port, pin); break;
	case 102: GPIO_ToggleBits(port, pin); break;
	case 103:
		json.systemID = device.systemID;
		json.roomID = device.roomID;
		json.value = (int)GPIO_ReadInputDataBit(port, pin) + 100;
		sendJSON(json);
	break;
	}
}

void doOrder(JSON json) {
	int temparature, humidity;
	switch(json.order) {
	case 301: // niebieska dioda
		updatePortPin(json, GPIOD, GPIO_Pin_15);
		break;

	case 302: // czerwona dioda
		updatePortPin(json, GPIOD, GPIO_Pin_14);
		break;

	case 303: // pomarañczowa dioda
		updatePortPin(json, GPIOD, GPIO_Pin_13);
		break;

	case 304: // zielona dioda
		updatePortPin(json, GPIOD, GPIO_Pin_12);
		break;

	case 800: // przypisanie nowego ID
		device.systemID = json.value;
		break;

	case 900: // zwrot temperatury
		redefineJSON(&json);
		getTemparatureAndHumidity(&temparature,&humidity);
		json.value = 500 + humidity;
		sendJSON(json);
	break;

	case 901: // zwrot wilgotnoœci
		redefineJSON(&json);
		getTemparatureAndHumidity(&temparature,&humidity);
		json.value = 500 + humidity;
		sendJSON(json);
	break;

	case 902: // zwrot œwiat³a
		printf(" %s ", "wszedlem;");
		GPIO_SetBits(GPIOD,GPIO_Pin_13);
		redefineJSON(&json);
		json.value = 500 + getLighting();
		sendJSON(json);
	break;

	case 903: // zwrot dystansu
		redefineJSON(&json);
		TM_HCSR04_Read(&HCSR04);
		float distanceF = HCSR04.Distance;
		int distance = distanceF;
		json.value = (distance / 10) + 100;
		sendJSON(json);
	break;

	case 904: // zakrêæ wiatrakiem
		//updatePortPin(json, GPIOA, GPIO_Pin_7);
		printf(" %s", "jestem  ");
		switch(json.value) {
		case 101: GPIO_SetBits(GPIOA, GPIO_Pin_7); break;
		case 100: {
			printf(" %s", "jestem");
			GPIO_SetBits(GPIOD,GPIO_Pin_12);
			GPIO_ResetBits(GPIOA, GPIO_Pin_7);
			break;
		}
		case 102: GPIO_ToggleBits(GPIOA, GPIO_Pin_7); break;
		case 103:
			json.systemID = device.systemID;
			json.roomID = device.roomID;
			json.value = (int)GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) + 100;
			sendJSON(json);
		break;
		}
		printf(" %s", "jestem");
	break;
	}
}

/**
 * void sendDataFromSensor(int nrOrder){

	JSON json;

	json.systemID = systemID;
	json.roomID = roomID;
	json.order = nrOrder+100;

	int temparature, humidity;

	switch(nrOrder){
	case 201://temparature
		getTemparatureAndHumidity(&temparature,&humidity);
		json.value = 500 + temparature;
		break;
	case 202://humidity
		getTemparatureAndHumidity(&temparature,&humidity);
		json.value = 500 + humidity;
		break;
	case 203://light
		json.value = 500 + getLighting();
		break;
	case 204://distance
		TM_HCSR04_Read(&HCSR04);
		float distanceF = HCSR04.Distance;
		int distance = distanceF;
		json.value = (distance + 100) / 10;
		break;
	}

	JSONToCharArray(json);
	SendToWaspCenter(tabJSON);
}
 */

void redefineJSON(JSON *json) {
	json->systemID = device.systemID;
	json->roomID = device.roomID;
}

void JSONToCharArray2(JSON structure) {
	for(int i=0;i<55;i++) tabJSON[i]=0;
    int len = sprintf(tabJSON, "{\"order\":%d,\"systemID\":%d,\"roomID\":%d,\"value\":%d}\r\n", structure.order, structure.systemID, structure.roomID, structure.value);
    //printf("\nJSON: %s.",tabJSON);
}

void sendJSON(JSON json) {
	JSONToCharArray2(json);
	int length = strlen(tabJSON);
	char buffer[32];
	int n = sprintf(buffer, "AT+CIPSEND=%d\r\n", length);
	USART_Send(buffer);
	USART_Send(tabJSON);

	//special !!!!!!!!!!!!
	printfMagicTable();
}
