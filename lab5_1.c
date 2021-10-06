/* ELEC 3040/3050 - Lab 6, Program 1 */
/*====================================================*/
#include "stm3214xx.h"

int timer;
int dummy;
int state;

void setupKeypad() {
	RCC->AHBENR |= 0x02; // Sets up the Keypad
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00005500);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x000000FF);
	GPIOB->PUPDR |= (0x00000055);
	
	NVIC_EnableIRQ (EXTI1_IRQn);
	NVIC_ClearPendingIRQ (EXTI1_IRQn); 
	
	SYSCFG->EXTICR[0] &= 0xFF0F;
	SYSCFG->EXTICR[0] |= 0x0000; 

	EXTI->FTSR |=0x0002;
	EXTI->IMR |=0x0002;

	__enable_irq();
}

void setupTimer() {
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	TIM4->PSC = 0x1FFF;
	TIM4->ARR = 0xFF;
	NVIC_EnableIRQ(TIM4_IRQn);
	TIM4->CR1 |= TIM_CR1_CEN;
	NVIC_ClearPendingIRQ(TIM4_IRQn); 
}

void PinSetup () {
	RCC->AHBENR |= 0x01; // Sets up the Inputs
	GPIOA->MODER &= ~(0x0000000C); // Setups up PA1 as an Input

	RCC->AHBENR |= 0x04; // Sets up the LED Outputs
	GPIOC->MODER &= ~(0x000000FF); 
	GPIOC->MODER |= (0x00000055); 
	
	setupTimer();
	setupKeypad();
}

void delay (double seconds) {
	int i,j,n;
	for (i=0; i<17500*seconds; i++) {
		n = i;
	}
}

int readColumn() {
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00000055);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x0000FF00);
	GPIOB->PUPDR |= (0x00005500);
	dummy = 4;
	while (dummy > 0) {
		dummy--;
	}
	int input = GPIOB->IDR&0xF0;
	switch(input) {
		case 0xE0:
			return 1;
		case 0xD0:
			return 2;
		case 0xB0:
			return 3;
		case 0x70:
			return 4;
		default:
			return -1;
	}
}

int readRow() {
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00005500);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x000000FF);
	GPIOB->PUPDR |= (0x00000055);
	dummy = 4;
	while (dummy > 0) {
		dummy--;
	}
	int input = GPIOB->IDR&0xF;
	switch(input) {
		case 0xE:
			return 1;
		case 0xD:
			return 2;
		case 0xB:
			return 3;
		case 0x7:
			return 4;
		default:
			return -1;
	}
}

int readKeypad() {
	int col = readColumn();
	int row = readRow();
	int key = -1;
	
	switch (row) {
		case 1:
			switch (col) {
				case 1:
					key = 1;
					break;
				case 2:
					key = 2;
					break;
				case 3:
					key = 3;
					break;
				case 4:
					key = 10;
					break;
				default:
					key = -1;
			}
		break;
			case 2:
			switch (col) {
				case 1:
					key = 4;
					break;
				case 2:
					key = 5;
					break;
				case 3:
					key = 6;
					break;
				case 4:
					key = 11;
					break;
				default:
					key = -1;
			}
		break;
			case 3:
			switch (col) {
				case 1:
					key = 7;
					break;
				case 2:
					key = 8;
					break;
				case 3:
					key = 9;
					break;
				case 4:
					key = 12;
					break;
				default:
					key = -1;
			}
		break;
			case 4:
			switch (col) {
				case 1:
					key = 15;
					break;
				case 2:
					key = 0;
					break;
				case 3:
					key = 14;
					break;
				case 4:
					key = 13;
					break;
				default:
					key = -1;
			}
		break;
				default:
					key = -1;
	}
	
	return key;
}

void displayTimer() {
	int decimal = timer%10;
	int seconds = (timer%100-decimal)/10;
	GPIOC->ODR = seconds*32+decimal;
}

void clear() {
	if (state == 0) {
		timer = 0;
		displayTimer();
	}
}

void start() {
	state = !state;
	
	if (state == 0) {
		TIM4->DIER &= ~0x01;
	} else {
		TIM4->DIER |= 0x01;
	}
}

void EXTI1_IRQHandler () {
	int key = readKeypad();
	
	if (key == 0) {
		start();
	} else if (key == 1) {
		clear();
	}
	
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	EXTI->PR |= 0x0002;
}

void TIM4_IRQHandler() {
	if (state == 1) {
		timer++;
		displayTimer();
	}
}

/*------------------------------------------------*/
/* Main program */
/*------------------------------------------------*/
int main(void) {
	
	PinSetup();
	timer = 0;
	state = 0;
	timer=89;
	displayTimer();
	while (1) {
		dummy++;
	}

} 