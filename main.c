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
void startWaspWard(void);
void USART3Init(void);
void USART3_IRQHandler(void);
void USART_Send(volatile char *c);


void StartConnectonWithWapsCenter(void);
void CloseConnectonWithWaspCenter(void);
void SendToWaspCenter(volatile char* message);

const int SIZE_WIFI_BUFFOR = 1024;
char tabWifiBuffor[1024];

int read = 0;	//	0-czyta komunikaty	1-nie czyta komunikat�w od wifi

char mt[5] = "XXXXX";

//test test test
void printfMagicTable(){

	printf("\nWifi:%s",tabWifiBuffor);

	//clear tab
	int k=0;
	for(k=0; k<512; k++){
		tabWifiBuffor[k]=0;
	}
}

//const DATA
const char* NAME= "Wasp1";

const char* NAME_WASPCENTER_AP = "test";
const char* PASSWORD_WASPCENTER_AP = "admin123";
const char* IP_WASPCENTER_SERVER = "192.168.43.129";
const int PORT_WASPCENTER_SERVER = 280;



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

	//tutaj powinna by� konfiguracja hotspot
	//i oczekiwanie na podanie danych waspcenter

	//inicjalizacja wszystkich czujnik�w i pierwszy odczyt �eby nie wysy�a� zer

	startWaspWard();

	connectWaspCenterNetwork(NAME_WASPCENTER_AP, PASSWORD_WASPCENTER_AP);

	StartConnectonWithWaspCenter(IP_WASPCENTER_SERVER, PORT_WASPCENTER_SERVER);

	char buffer[512];
	int n = sprintf(buffer,"%s\r\n",NAME);
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

void startWaspWard(void)
{
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
			}else{
				for(int k=0; k<SIZE_WIFI_BUFFOR; k++){
					tabWifiBuffor[k]=0;
				}
			}

			//wykrywanie wiadomosci
			//tablica mt przechow�je ostatnie 5 znak�w
			/*
			for(int i=0; i<4; i++){
				mk[i] = mk[i+1];
			}
			mk[5] = sign;

			//wykrycie +IPD,
			if(mk[1]=='+' && mk[1]=='I' && mk[1]=='P' && mk[1]=='D' && mk[1]==','){
				GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
			}*/



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
