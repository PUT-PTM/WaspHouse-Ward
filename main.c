#include <stdio.h>
#include "STM32F4xx.h"
#include "dht11.h"

// PATTERN: https://github.com/sapher/stm32-dht11_driver

struct DHT11_Dev dev;

volatile uint32_t msTicks;

void SysTick_Handler(void) {
  msTicks++;
}

void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}

int temparature = 0;
int humidity = 0;

int main (void) {

	SystemInit();

	SystemCoreClockUpdate();
	if (SysTick_Config(SystemCoreClock / 1000)) {
	while (1);
	}

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	DHT11_init(&dev, GPIOB, GPIO_Pin_6);

	while(1) {
		int res = DHT11_read(&dev);
		if(res == DHT11_ERROR_CHECKSUM) {
			//temporary - wolniak
			temparature = 5500;
			humidity = 200;
		}
		else if(res == DHT11_SUCCESS) {
			temparature = dev.temparature; //0-50, +- 2 deegres
			humidity = dev.humidity; //20-95%, +-RH
		}
		else {
			//temporary - wolniak
			temparature = -273;
			humidity = -100;
		}
	}
}
