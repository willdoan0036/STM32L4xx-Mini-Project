/* PWM duty cycle controlled by keypab buttons. */
/* Default period of 1ms */
/*====================================================*/

#include "stm32l4xx.h" /* Microcontroller information */

/* Define global variables */

struct {
	int row;
	int column;
	unsigned char event;
	const unsigned char row1[4];
  const unsigned char row2[4];
  const unsigned char row3[4];
  const unsigned char row4[4];
  const unsigned char* keys[];
} typedef keypad;


keypad keypad1 = {
  .row = ~0,  
  .column = ~0, 
  .event = 0,
  .row1 = {1, 2, 3, 0xA},
  .row2 = {4, 5, 6, 0xB},
  .row3 = {7, 8, 9, 0xC},
  .row4 = {0xE, 0, 0xF, 0xD},
  .keys = {keypad1.row1, keypad1.row2, keypad1.row3, keypad1.row4},
};

unsigned char pressed;
unsigned char count;
/*---------------------------------------------------*/
/* Initialize GPIO pins used in the program */
/* PA1 triggers interrupt EXTI1 */
/* PC[7:0] = counter output */
/* PB[3:0] = keypad rows */
/* PB[7:4] = keypad columns */
/*---------------------------------------------------*/
void PinSetup() {
	/* Configure PA1 as input */
	RCC->AHBENR |= 0x01; // Enable GPIOA clock (bit 0)
	GPIOA->MODER &= ~(0x0000000C); // General purpose input mode
	
	/* Configure PB7-0 as inputs */
	RCC->AHBENR |= 0x02; // Enable GPIOB clock (bit 1)
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00005500);
	GPIOB->ODR = 0;
	
	GPIOB->PUPDR &= ~(0x000000FF);
	GPIOB->PUPDR |= (0x00000055);
	
	/* Configure PC[7:0] as output pins to disply value of counter */
	RCC->AHBENR |= 0x04; // Enable GPIOC clock (bit 2)
	GPIOC->MODER &= ~(0x0000FFFF); // Clear PC3-PC0 mode bits
	GPIOC->MODER |= (0x00005555); // General purpose output mode
	
	/* Set PA6 to altrnative function mode */
  MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODER6, 0x00002000);
  
  /* Set alternative function to be the CC*/
  MODIFY_REG(GPIOA->AFR[0], GPIO_AFRL_AFRL6, 0x03000000);
}

void InterruptSetup() {
	
	SYSCFG->EXTICR[0] &= 0xFF0F; // Clear EXTI1
	SYSCFG->EXTICR[0] |= 0x0010; // Configure EXTI1 to be triggered by PA1
	
	EXTI->FTSR |= 0x0002; // Set EXTI1 to be rising edge triggered
	EXTI->IMR |= 0x0002; // Set interrupt masks for EXTI1
	EXTI->PR |= 0x0002; // Set pending register for EXTI1

	NVIC_EnableIRQ(EXTI1_IRQn); // Enable EXTI1
	
	NVIC_ClearPendingIRQ(EXTI1_IRQn); // Clear pending register for EXTI1
}

void TimerSetup() {
	RCC->CR |= RCC_CR_HSION; // Turn on 16MHz HSI oscillator
  while ((RCC->CR & RCC_CR_HSIRDY) == 0); // Wait until HSI ready
	RCC->CFGR |= RCC_CFGR_SW_HSI; // Select HSI as system clock
  
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM10EN); //enable clock source
  TIM10->ARR = 99; //set auto reload. 
  TIM10->PSC = 159; //set prescale.
  TIM10->CCR1 = 10; //Set compare value
  TIM10->CNT = 0;
  MODIFY_REG(TIM10->CCMR1, TIM_CCMR1_CC1S, 0x0000); // Capture compair select
  MODIFY_REG(TIM10->CCMR1, TIM_CCMR1_OC1M, 0x0060); // Active to inactive
  SET_BIT(TIM10->CCER, TIM_CCER_CC1E); // drive output pin 
  SET_BIT(TIM10->CR1, TIM_CR1_CEN); //enable counting
}

/*----------------------------------------------------------*/
/* SmallDelay function - do nothing for a short period */
/*----------------------------------------------------------*/
void smallDelay() {
  int i, j;
  for (i=0; i<4; i++) { //outer loop
    i = j;; //dummy operation for single-step test
  }
}

/*------------------------------------------------*/
/* Determine row number of pressed key */
/*------------------------------------------------*/
int readRow() {
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00005500);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x000000FF);
	GPIOB->PUPDR |= (0x00000055);
	
	smallDelay();
	
	int input = GPIOB->IDR&0xF;
	switch(input) {
		case 0xE:
			return 0;
		case 0xD:
			return 1;
		case 0xB:
			return 2;
		case 0x7:
			return 3;
		default:
			return -1;
	}
}

/*------------------------------------------------*/
/* Determine column number of pressed key */
/*------------------------------------------------*/
int readColumn() {
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00000055);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x0000FF00);
	GPIOB->PUPDR |= (0x00005500);

	smallDelay();
	
	int input = GPIOB->IDR&0xF0;
	switch(input) {
		case 0xE0:
			return 0;
		case 0xD0:
			return 1;
		case 0xB0:
			return 2;
		case 0x70:
			return 3;
		default:
			return -1;
	}
}

/*----------------------------------------------------------*/
/* EXTI1_IRQHandler function - performs operations when EXTI1 is triggered */
/*----------------------------------------------------------*/
void EXTI1_IRQHandler() {
	EXTI->PR |= 0x0002; //Set pending register for EXTI1
	
	keypad1.row = readRow();
	keypad1.column = readColumn();
			
			if ((keypad1.row != -1) && (keypad1.column != -1)) {
				keypad1.event = keypad1.keys[keypad1.row][keypad1.column];
				GPIOC->ODR = keypad1.event;
				pressed = 1;
			}
	
	RCC->AHBENR |= 0x02; // Enable GPIOB clock (bit 1)
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->PUPDR &= ~(0x0000FF00);
	GPIOB->PUPDR |= (0x00005500);
			
	GPIOB->MODER &= ~(0x0000FFFF);
	GPIOB->MODER |= (0x00005500);
	GPIOB->ODR = 0;
	GPIOB->PUPDR &= ~(0x000000FF);
	GPIOB->PUPDR |= (0x00000055);		
	NVIC_ClearPendingIRQ(EXTI1_IRQn); //Reset pending register for EXTI1
	EXTI->PR |= 0x0002;
}

/*------------------------------------------------*/
/* Main program */
/*------------------------------------------------*/
int main(void) {
	pressed = 0;
	PinSetup(); // Configure GPIO 
	InterruptSetup(); // Configure interrupts
	TimerSetup();
	
	__enable_irq(); //Enable interrupts to occur
	
	/* Endless loop */
	while (1) {
		
		if (keypad1.event < 11 & pressed == 1) {
			TIM10->CCR1 = keypad1.event * (TIM10->ARR + 1) / 10;
			pressed = 0;
		}
		
	} /* repeat forever */
}