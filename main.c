/* SENSOR...
PATTERN: https://github.com/sapher/stm32-dht11_driver

TEMPERATURE: 0-50, +- 2 degrees
HUMIDITY: 20-95%, +-RH

+ -> 3/5v
MIDDLE -> PB6
- -> GND
*/

/* WIFI...
WIFI ->
PINS:
https://cdn.instructables.com/FR0/YPQQ/IJX7FOP5/FR0YPQQIJX7FOP5.LARGE.jpg

CONNECT:
GND->GND
PC11->Tx
PC10->Rx
3V->VCC
3V->CH_PD

COMMANDS:
http://www.pridopia.co.uk/pi-doc/ESP8266ATCommandsSet.pdf
https://www.espressif.com/sites/default/files/documentation/4a-esp8266_at_instruction_set_en.pdf
https://www.itead.cc/wiki/ESP8266_Serial_WIFI_Module
http://www.sunduino.pl/wordpress/esp8266-czesc-2-komendy-at/
https://room-15.github.io/blog/2015/03/26/esp8266-at-command-reference/


COMMUNICATION BOARD - USB MODULE - HERKULES:
PA0->TxD
PA1->RxD
GND->GND

PATTERN: https://github.com/PUT-PTM/WiFiAutomatics/blob/master/ESP8266/main.c
*/

#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "STM32F4xx.h"
#include "misc.h"
#include "dht11.h"
#include <stdio.h>

int temperature = 0;
int humidity = 0;
uint16_t buffer;
struct DHT11_Dev dev;
volatile uint32_t msTicks;

void GPIOInit(void);
void relayInit(void);
void SENSORInit(void);
void SysTick_Handler(void);
void Delay (uint32_t dlyTicks);
void Delay_us(volatile uint32_t delay);
void connect(void);
void USART3Init(void);
void USART3_IRQHandler(void);
void USART_Send(volatile char *c);

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();
	GPIOInit();
	USART3Init();
	SENSORInit();
	relayInit();
	connect();
	void USART3_IRQHandler(void);
	while(1) {
	}
}

void relayInit(void){
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

// INIT FOR SENSOR + TIMER - EVERY 5 SECONDS SEND DATA
void SENSORInit(void){

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 8399;
	TIM_TimeBaseStructure.TIM_Prescaler = 1999;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM3, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

		DHT11_init(&dev, GPIOB, GPIO_Pin_6);
}

// IRQ HANDLER FOR SENSOR
void TIM3_IRQHandler(void)
{
         	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
         	{
         		int res = DHT11_read(&dev);
         				if(res == DHT11_ERROR_CHECKSUM) {
         					/* USART_Send("AT+CIPSEND=5\r\n");
         					   Delay_us(5000000);
         					   USART_Send("ERROR\r\n\r\n");
         					   Delay_us(5000000);
         					   USART_Send("AT+CIPCLOSE\r\n");
         				    */
         				}
         				if(res == DHT11_SUCCESS) {
         					temperature = dev.temparature;
         					humidity = dev.humidity;
         					if(temperature<10){
             					/* USART_Send("AT+CIPSEND=2\r\n");
             					   Delay_us(5000000);
             					   USART_Send("T");
             					   Delay_us(5000000);
             					   USART_Send(temperature);
             					   Delay_us(5000000);
             					   USART_Send(\r\n\r\n");
             					   Delay_us(5000000);
             					   USART_Send("AT+CIPCLOSE\r\n");
             				    */
         					}
         					else{
             					/* USART_Send("AT+CIPSEND=3\r\n");
             					   Delay_us(5000000);
             					   USART_Send("T");
             					   Delay_us(5000000);
             					   USART_Send(temperature);
             					   Delay_us(5000000);
             					   USART_Send(\r\n\r\n");
             					   Delay_us(5000000);
             					   USART_Send("AT+CIPCLOSE\r\n");
             				    */
         					}
         					if(humidity<10){
             					/* USART_Send("AT+CIPSEND=2\r\n");
             					   Delay_us(5000000);
             					   USART_Send("H");
             					   Delay_us(5000000);
             					   USART_Send(humidity);
             					   Delay_us(5000000);
             					   USART_Send(\r\n\r\n");
             					   Delay_us(5000000);
             					   USART_Send("AT+CIPCLOSE\r\n");
             				    */
         					}
         					else{
         						if(humidity>20){
         							GPIO_ResetBits(GPIOA, GPIO_Pin_7);
         						}
         						else{
         							GPIO_SetBits(GPIOA, GPIO_Pin_7);
         						}
             					/* USART_Send("AT+CIPSEND=3\r\n");
             					   Delay_us(5000000);
             					   USART_Send("H");
             					   Delay_us(5000000);
             					   USART_Send(humidity);
             					   Delay_us(5000000);
             					   USART_Send(\r\n\r\n");
             					   Delay_us(5000000);
             					   USART_Send("AT+CIPCLOSE\r\n");
             				    */
         					}
         				}
                	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
         	}
}

// INIT FOR WIFI
void USART3Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructureW;
	GPIO_InitStructureW.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructureW.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructureW.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructureW.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructureW.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOC, &GPIO_InitStructureW);

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

// INIT FOR DIODS => ADDITIONAL!
void GPIOInit(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructureD;

	GPIO_InitStructureD.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructureD.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructureD.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructureD.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructureD.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOD, &GPIO_InitStructureD);
}

// DELAY FOR STM
void Delay_us(volatile uint32_t delay)
{
	delay*=24;
	while(delay--);
}
// DELAY FOR SENSOR
void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;
  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}

// TICKS FROM SYSTEM
void SysTick_Handler(void) {
  msTicks++;
}

// CHECK AND CONNECT WIFI
void connect(void)
{
	Delay_us(5000000);
	USART_Send("AT+RST\r\n");
	GPIO_SetBits(GPIOD, GPIO_Pin_12);
	Delay_us(5000000);
	USART_Send("AT+CWMODE=1\r\n");
	Delay_us(5000000);
	USART_Send("AT+CWJAP=\"S5\",\"kupseneta\"\r\n");
	Delay_us(5000000);
	USART_Send("AT+CIFSR\r\n");
	Delay_us(5000000);
	USART_Send("AT+CIPSTART=\"TCP\",\"192.168.43.223\",80\r\n");
}

// READ WHAT WE RECIEVE FROM WIFI
void USART3_IRQHandler(void)
{

}

// FUNCTION - SEND MORE THAN ONE SIGN THROUGH WIFI
void USART_Send(volatile char *c)
{
	while(*c)
	{
		while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
		USART_SendData(USART3, *c);
		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
		Delay_us(500);
		*c++;
	}
