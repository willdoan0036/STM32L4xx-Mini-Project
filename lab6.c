/* ELEC 3040/3050 - Lab 6 */
/* Stopwatch controlled by timer interrupt. */
/* Keypad button 0 controls start/stop  */
/* Keypad button 1 controls reset */
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

unsigned char count1;
unsigned char count2;
/*---------------------------------------------------*/
/* Initialize GPIO pins used in the program */
/* PA1 triggers interrupt EXTI1 */
/* PB[7:0] = counter output */
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
}

void InterruptSetup() {
	
	SYSCFG->EXTICR[0] &= 0xFF0F; // Clear EXTI1
	SYSCFG->EXTICR[0] |= 0x0010; // Configure EXTI1 to be triggered by PA1
	
	EXTI->FTSR |= 0x0002; // Set EXTI1 to be rising edge triggered
	EXTI->IMR |= 0x0002; // Set interrupt masks for EXTI1
	EXTI->PR |= 0x0002; // Set pending register for EXTI1

	NVIC_EnableIRQ(EXTI1_IRQn); // Enable EXTI1
	NVIC_EnableIRQ(TIM10_IRQn);
	
	NVIC_ClearPendingIRQ(EXTI1_IRQn); // Clear pending register for EXTI1
	NVIC_ClearPendingIRQ(TIM10_IRQn);
}

void TimerSetup() {
	
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM10EN); //enable clock source
  TIM10->ARR = 0x333; //set auto reload. assumes 2MHz
  TIM10->PSC=0xFF; //set prescale. assumes 2MHz
  SET_BIT(TIM10->DIER, TIM_DIER_UIE); //enable interupts
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

void TIM10_IRQHandler() {
	CLEAR_BIT(TIM10->SR, TIM_SR_UIF);
  
  count2 = (count2 + 1) % 10;
	GPIOC->BSRR = 0xF0000;
  SET_BIT(GPIOC->BSRR, count2 & 0xF); //turn on decimal
 
  if (count2 == 0) {
		count1 = (count1 + 1) % 10;
		GPIOC->BSRR = 0xF00000;
		SET_BIT(GPIOC->BSRR, count1 << 4);
	} 
 
  NVIC_ClearPendingIRQ(TIM10_IRQn);
}

/*------------------------------------------------*/
/* Main program */
/*------------------------------------------------*/
int main(void) {
	
	PinSetup(); // Configure GPIO 
	InterruptSetup(); // Configure interrupts
	TimerSetup();
	
	__enable_irq(); //Enable interrupts to occur
	
	/* Endless loop */
	while (1) {
		
		unsigned char running = READ_BIT(TIM10->CR1, TIM_CR1_CEN);
		
    if (keypad1.event == 0 && running) {
      CLEAR_BIT(TIM10->CR1, TIM_CR1_CEN); //toggle counting
			keypad1.event = ~0;
		} 
		else if (keypad1.event == 0 && !running) {
			SET_BIT(TIM10->CR1, TIM_CR1_CEN); //toggle counting
			keypad1.event = ~0;
    } 
		else if (!running && keypad1.event == 1) {
      count1 = 0;
			count2 = 0;
			GPIOC->BSRR = 0xFF0000;
			keypad1.event = ~0;
    }		
		
	} /* repeat forever */
}