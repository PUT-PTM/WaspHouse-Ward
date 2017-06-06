#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "misc.h"
#include <stdio.h>
#include "STM32F4xx.h"

//dth11
#include "dht11.h"

struct DHT11_Dev dev;
int temparature, humidity;
volatile uint32_t msTicks;                      /* counts 1ms timeTicks       */

// SysTick_Handler
void SysTick_Handler(void) {
  msTicks++;
}

void getTemparatureAndHumidity(int *temp, int *humi);

void LedInit(void);
void Delay_us(volatile uint32_t delay);
void startHotspot(void);
void startWaspWard(void);
void USART3Init(void);
void USART3_IRQHandler(void);
void USART_Send(volatile char *c);


void StartConnectonWithWapsCenter(void);
void CloseConnectonWithWaspCenter(void);
void SendToWaspCenter(volatile char* message);

const int SIZE_WIFI_BUFFOR = 1024;
char tabWifiBuffor[1024];

int read = 0;	//	0-czyta komunikaty	1-nie czyta komunikatów od wifi

//rz¹danie GET
int read_channel = 0;
int channel = -1;
char discovery_channel;
int read_msg_size = 0;
char tab_msg_size[4];
int msg_size = -1;
const SIZE_MSG = 1024;
int read_msg = 0;
char tab_msg[1024];

int new_msg = 0;

//test test test
void printfMagicTable(){

	printf("\nWifi:%s",tabWifiBuffor);

	//clear tab
	for(int k=0; k<512; k++){
		tabWifiBuffor[k]=0;
	}
}

//const DATA
const char* WARD_HOTSPOT_NAME = "Wasp1";
const char* WARD_PASSWORD_NAME = "Wasp1abc";
const int PORT = 80;

//test version
const char* HTML_SITE = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"/><script type=\"text/javascript\">function createLinkWithData(){var str=\"Dane zapisane. Kliknij, by wyslac dane do urzadzenia...\";var secret_password=document.getElementById(\"secret_password\").value;var ward_name=document.getElementById(\"ward_name\").value;var wifi_name = document.getElementById(\"wifi_name\").value;var wifi_password = document.getElementById(\"wifi_password\").value;var server_ip = document.getElementById(\"server_ip\").value;var server_port=document.getElementById(\"server_port\").value;var str2 = \"DATA\";var hash = \"_\";var text = str2.concat(hash,secret_password,hash,ward_name,hash,wifi_name,hash,wifi_password,hash,server_ip,hash,server_port,hash);var result = str.link(text);document.getElementById(\"link\").innerHTML = result; }</script><style type=\"text/css\" ref=\"stylesheet\">#logo{width: 350px;margin-left: auto;margin-right: auto;border-style: outset;border-radius: 50%;}body{background: #eae9c8;font-family:\"Trebuchet MS\",Helvetica,sans-serif;text-align:center;}</style></head><body><div id=\"logo\"><h1>WaspHouse Ward</h1></div><h4>Skonfiguruj ustawienia urzadzenia WaspWard</h4><br/><span>Sekretne haslo:</span></br><input type=\"password\" id=\"secret_password\" value=\"tajnehaslo\" maxlength=\"15\"/></br></br><span>Nazwa urzadzenia:</span></br><input type=\"text\" id=\"ward_name\" value=\"ward1\" maxlength=\"15\"/></br></br></br><b>Dane urzadzenia Center</b><br/></br><span>Identyfikator sieci:</span></br><input type=\"text\" id=\"wifi_name\" value=\"test\" maxlength=\"15\"/></br></br><span>Haslo sieci:</span></br><input type=\"password\" id=\"wifi_password\" value=\"admin123\" maxlength=\"15\"/></br></br><span>IP serwera:</span></br><input type=\"text\" id=\"server_ip\" value=\"192.168.43.129\" maxlength=\"15\"/></br></br><span>Port serwera:</span></br><input type=\"text\" id=\"server_port\" value=\"280\" maxlength=\"15\"/></br></br></br><button type=\"submit\" onclick=\"createLinkWithData()\"/>Zapisz</button></br></br><span id=\"link\" style=\"text-decoration: none;\"></span></body></html>\r\n";

const char* HTML_SITE2 = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"/><style type=\"text/css\" ref=\"stylesheet\">#logo{width: 350px;margin-left: auto;margin-right: auto;border-style: outset;border-radius: 50%;}body{background: #eae9c8;font-family:\"Trebuchet MS\",Helvetica,sans-serif;text-align:center;}</style></head><body><div id=\"logo\"><h1>WaspHouse Ward</h1></div><h4>Ustawienia urzadzenia WaspWard zostana za chwile zmienione.<br/>Urzadzenie zostanie zrestartowane i podlaczy sie z siecia WaspHouse Center.</h4><br/><h2>Dziekujemy za poswiecony czas :)<h2></body></html>\r\n";


const char* NAME= "Wasp1";

const char* NAME_WASPCENTER_AP = "test";
const char* PASSWORD_WASPCENTER_AP = "admin123";
const char* IP_WASPCENTER_SERVER = "192.168.43.129";
const int PORT_WASPCENTER_SERVER = 280;

//
int typeRequest(char* text);
const char* REQUESET1_GET = "GET / HTTP/1.1";
const char* REQUESET2_DATA = "GET /DATA_";

int readDataToRequest(char* request);

int correctSecretePassword(char* pass);
const char SECRETE_PASSWORD[15] = "tajnehaslo";


//przesuwna tablica przechowuj¹ca ostatnie znaki odebrane z wifi
const int SIZE_TABRES = 10;
char tabRes[10];

int main (void) {

	SystemInit();

	LedInit();

	//DHT11
	SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
	if (SysTick_Config(SystemCoreClock / 1000)) { /* SysTick 1 msec interrupts  */
	while (1);                                  /* Capture error              */
	}

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	DHT11_init(&dev, GPIOB, GPIO_Pin_6);
	getTemparatureAndHumidity(&temparature,&humidity);

	//WIFI INIT
	USART3Init();

	//konfiguracja hotspot
	startHotspot();

	//i oczekiwanie na podanie danych waspcenter
	char secret_passwordX[15];
	char ward_nameX[15];
	char wifi_name[15];
	char wifi_password[15];
	char server_ip[15];
	int server_port;

	int endHot = 1;
	do{
		if(new_msg==1){
			new_msg=0;
			int t = typeRequest(tab_msg);

			if(t == 1){
				SendToClient(HTML_SITE,channel);
			}
			else if(t == 2){

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
				while(request[n] != '_'){
					ward_nameX[i] = request[n];
					n++;
					i++;
				}
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

				int n = strlen(SECRETE_PASSWORD);
				endHot = 0;
				for(int i=0; i<n; i++){
					if(secret_passwordX[i]!=SECRETE_PASSWORD[i]){
						endHot=1;
					}
				}
				if(endHot==0){
					SendToClient(HTML_SITE2,channel);
				}

			}

			//clear tab msg
			for(int i=0; i<SIZE_MSG; i++) tab_msg[i] = 0;
			printfMagicTable();
		}
		Delay_us(1000000);
	}while(endHot);

	//inicjalizacja wszystkich czujników i pierwszy odczyt ¿eby nie wysy³aæ zer

	startWaspWard();

	connectWaspCenterNetwork(wifi_name, wifi_password);
	//connectWaspCenterNetwork(NAME_WASPCENTER_AP, PASSWORD_WASPCENTER_AP);

	StartConnectonWithWaspCenter(server_ip, server_port);
	//StartConnectonWithWaspCenter(IP_WASPCENTER_SERVER, PORT_WASPCENTER_SERVER);

	char buffer[512];
	int n = sprintf(buffer,"%s\r\n",ward_nameX);
	//int n = sprintf(buffer,"%s\r\n",NAME);
	SendToWaspCenter(buffer);

	for(int i=0; i<3; i++){
		getTemparatureAndHumidity(&temparature,&humidity);
		char buffer2[512];
		int n = sprintf(buffer2, "{temp: %d; humi: %d}\r\n", temparature, humidity);
		SendToWaspCenter(buffer2);
		Delay_us(10000000);
	}

	SendToWaspCenter("{EXIT}\r\n");
	CloseConnectonWithWaspCenter();
	Delay_us(1000000);
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
	char buffer[512];
	int n = sprintf(buffer,"AT+CWJAP=\"%s\",\"%s\"\r\n",name,password);

	USART_Send(buffer);
	Delay_us(10000000);
}

void startHotspot(void){
	read = 1;
	printf("START RESET ");
	USART_Send("AT+RST\r\n");
	Delay_us(2500000);
	printf(". \n");
	read = 0;

	USART_Send("AT+CWMODE=2\r\n");		// Access Point
	Delay_us(2000000);

	char buffer[512];
	int n = sprintf(buffer,"AT+CWSAP=\"%s\",\"%s\",4,2\r\n",WARD_HOTSPOT_NAME,WARD_PASSWORD_NAME);
	USART_Send(buffer);
	Delay_us(2000000);

	USART_Send("AT+CIPMUX=1\r\n");
	Delay_us(2000000);

	char buffer2[512];
	n = sprintf(buffer2,"AT+CIPSERVER=1,%d\r\n",PORT);
	USART_Send(buffer2);
	Delay_us(2000000);

	GPIO_SetBits(GPIOD,GPIO_Pin_15);
}

void startWaspWard(void)
{
	read = 1;
	printf("START RESET ");
	USART_Send("AT+RST\r\n");
	Delay_us(2500000);
	printf(". \n");
	read = 0;

	USART_Send("AT+CWMODE=1\r\n");
	Delay_us(2000000);
	USART_Send("AT+CIPMUX=0\r\n");
	Delay_us(2000000);
	USART_Send("AT+CIPSERVER=0\r\n");
	Delay_us(2000000);
	USART_Send("AT+CIPMODE=0\r\n");
	Delay_us(2000000);
	USART_Send("AT+CIPSTO?\r\n");
	Delay_us(2000000);
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

    	uint16_t buffer = USART3->DR;
    	char sign = (USART3->DR & 0x1FF);

    	if((read == 0) && ((((sign >= 32) && (sign <= 126)) || (sign == '\n')) || (sign == '\r'))){

    		if(sign == '\n')	sign = '|';
    		if(sign == '\r')	sign = '\\';

    		//test test test
    		int n = strlen(tabWifiBuffor);

			if(n < SIZE_WIFI_BUFFOR){
				tabWifiBuffor[n] = sign;
			}//test test test


			for(int i=0;i<SIZE_TABRES-1;i++){
				tabRes[i]=tabRes[i+1];
			}

			tabRes[SIZE_TABRES-1]=sign;

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
				GPIO_ToggleBits(GPIOD,GPIO_Pin_13);
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

void USART_Send(volatile char *c)
{
	//test test test
	if(read == 0){
		printfMagicTable();
	}

	int length = strlen(c);

	for(int i=0; i<length; i++){
		USART_SendData(USART3, c[i]);
		Delay_us(500);
	}

}

void StartConnectonWithWaspCenter(char* ip, int port){
	Delay_us(2000000);
	char buffer[512];
	int n = sprintf(buffer,"AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",ip,port);

	USART_Send(buffer);
	Delay_us(2000000);
	Delay_us(2000000);

	//test test test
	GPIO_SetBits(GPIOD,GPIO_Pin_12);
	printfMagicTable();
}

void CloseConnectonWithWaspCenter(void){
	USART_Send("AT+CIPCLOSE\r\n");
	Delay_us(2200000);

	//test test test
	GPIO_ResetBits(GPIOD,GPIO_Pin_12);
	printfMagicTable();
}


void SendToClient(volatile char* message, int channel)
{
	int length = strlen(message);

	char buffer[512];
	int n = sprintf(buffer, "AT+CIPSEND=%d,%d\r\n", channel, length);
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

	char buffer[512];
	int n = sprintf(buffer, "AT+CIPSEND=%d\r\n", length);
	USART_Send(buffer);
	Delay_us(2000000);

	USART_Send(message);

	Delay_us(2500000);

	//test test test
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

/*
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
	while(request[n] != '_'){
		ward_nameX[i] = request[n];
		n++;
		i++;
	}
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
}*/
