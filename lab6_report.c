#include "stm32l4xx.h"
//Lab 6 - Using Timers to create a Pulse Width Modulation
//Define Global Variables

unsigned int static cntVal;
unsigned int static buttonVal;
unsigned int static dispButton;
unsigned int static row, column;
unsigned int rowCheck;
unsigned int writeColumns;
unsigned int rowInput;

void pinSetup(void);
void interruptSetup(void);
void smallDelay()
void delay(void);
void count(void);
void display(void);
void scan(void);
int main(void);

/*---------------------------------------------*/
/*	Function to setup GPIO Pins	*/
/*---------------------------------------------*/
void pinSetup() {
	// Enable GPIOA and GPIOB clock
	RCC->AHB2ENR |= 0x03;
	RCC->APB1ENR1 |= 0x00000001; //set first bit to 1
 
	// Clear bits for PA
	GPIOA->MODER &= 0xFF00F00C; //clear PA0, 2-5, 8-11
	// Set bits for PA: 2-5 = output 8-11 = input
	GPIOA->MODER |= 0x00550002;
	//Set PA0 to AF TIM2
	//GPIOA->MODER |= 0x00000002; //need PA0 to 10 is alternate function for TIM2_CH1
 
	// Clear bits for PB
	GPIOB->MODER &= ~0x3FC3;
	// Set bits for PB: 0 = input 3-6 = output
	GPIOB->MODER |= 0x1540;

	// Clear PUPDR bits in PA
	GPIOA->PUPDR &= ~0x00000FF0;
	// Set PA 2-5 to pull up
	GPIOA->PUPDR |= 0x00000550;
	GPIOA->AFR[0] &= 0xFFFFFFF0; //clear AFR lower half
	GPIOA->AFR[0] |= 0x00000001; //configure TIM2_CH1
	TIM2->CR1 |= 0x0001; //set correct bit
	TIM2->CCMR1 &= 0xFFFFFF8C; //clear bits 0,1, and 4-6 before setting timer as an output (set as 00)
	TIM2->CCMR1 |= 0x00000060;//configure bits 4-6 to 110 for PWM mode 1
	TIM2->CCER &= 0xFFFFFFFC;//clear
	TIM2->CCER |= 0x00000001;//configure pins 1 and 2 for channel 1
	//Period
	TIM2->ARR = 3999; //Period = ARR+1
	//Duty Cycle
	TIM2->CCR1 = 3999;//On-time = CCRy +1. Default full 100%
}
/*---------------------------------------------*/
/*	Function to setup interrupts	*/
/*---------------------------------------------*/
void interruptSetup() {
	// Set SYSCFG Clock
	RCC->APB2ENR |= 0x01;
	// Select PB0 as external interrupt
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB;
	// Set falling edge, clear pending status, unmask
	EXTI->FTSR1 |= EXTI_FTSR1_FT0;
	EXTI->PR1 = EXTI_PR1_PIF0;
	EXTI->IMR1 |= EXTI_IMR1_IM0;
	// Program NVIC
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI0_IRQn);
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
/*------------------------------------------------------------------------------*/
/*	Delay function - create a delay of ~1 sec	*/
/*------------------------------------------------------------------------------*/
void delay() {
	for (int i=0; i < 32; i++) {
		for (int j=0; j < 20000; j++) { //__nop();
		}
	}
}
/*--------------------------------------------------------*/
/*	Count function, 0-9 then roll over	*/
/*--------------------------------------------------------*/
void count() {
	if (cntVal < 9) {
		cntVal++;
	}	
	else { // roll over when cntVal is at 9 
		cntVal = 0;
	}
}
/*--------------------------------------------------------*/
/* 	Display cntVal and buttonVal		*/
/*--------------------------------------------------------*/
void display() {
	if (dispButton == 1) { //Writes button value to output if a button has been pressed
		GPIOB->ODR = (GPIOB->ODR & ~(0x0078)) | (buttonVal << 3);
		dispButton = 0;
		delay();
	}
	else { //Writes count value to output if a button has not been pressed
		count();
		GPIOB->ODR = (GPIOB->ODR & ~(0x0078)) | (cntVal << 3);
		delay();
	}
}
/*---------------------------------------------*/
/*	Find and return the value for which column has been activated  */
/*---------------------------------------------*/
void scan() {
	//Short Delay
	//for (int k = 0; k < 4; k++);
	//Check if read value is low. If it is, assign value according to column 4 and current row then 
	
	writeColumns = 0;
	column = 0;
	GPIOA->ODR &= ~(0x0F00); //clear column pins 8-11
	GPIOA->ODR |= (0x0E00);//write 1110 to pins 8-11
		rowCheck = ((GPIOA->IDR & (0x003C)) >> 2);
	if (rowCheck == rowInput) { // Column | Row 
		column = 4;
	}
	GPIOA->ODR &= ~(0x0F00); //clear column pins 8-11
	GPIOA->ODR |= (0x0D00);//write 1101 to pins 8-11
		rowCheck = (GPIOA->IDR & (0x003C)) >> 2;
	if (rowCheck == rowInput) { // Column | Row
		column = 3;
	}
	GPIOA->ODR &= ~(0x0F00); //clear column pins 8-11
	GPIOA->ODR |= (0x0B00);//write 1011 to pins 8-11
		rowCheck = (GPIOA->IDR & (0x003C)) >> 2;
	if (rowCheck == rowInput) { // Column | Row
		column = 2;
	}
	GPIOA->ODR &= ~(0x0F00); //clear column pins 8-11
	GPIOA->ODR |= (0x0700); //write 0111 to pins 8-11
		rowCheck = (GPIOA->IDR & (0x003C)) >> 2;
	if (rowCheck == rowInput) { // Column | Row
		column = 1;
	}
}
/*-------------------------------------------------------------*/
/*  Handle the interrupt from PB0
	  Finds the current row then calls scan to find the column  */
/*-------------------------------------------------------------*/
void EXTI0_IRQHandler() {
	dispButton = 1;
	row = 0;
	GPIOB->ODR &= ~(0x0078);
	//clears counters
	GPIOA->ODR &= ~(0x0F00);
	//clear column pins 8-11
	for (int k = 0; k < 640; k++); // Short delay for debounce
	rowInput = (GPIOA->IDR & (0x003C)) >> 2;
	if (rowInput == 7) {//0111
		row = 1;
	}
	if (rowInput == 11) {//1011
		row = 2;
	}
	if (rowInput == 13) {//1101
		row = 3;
	}
	if (rowInput == 14) {//1110
		row = 4;
	}
 
// Find the column
	scan();
// Assign the button value according to row and column
	if (row == 1) {		//row == 1
		if (column == 1) {
			buttonVal = 1;
			TIM2->CCR1 = 399; //10% duty cycle
		}
	else if (column == 2) {
		buttonVal = 2;
		TIM2->CCR1 = 799; //20% duty cycle
		}
	else if (column == 3) {
		buttonVal = 3;
		TIM2->CCR1 = 1199; //30% duty cycle
		}
	else {
		buttonVal = 10;
		TIM2->CCR1 = 3999; //100% duty cycle
		}
	}

	if (row == 2) {		//row == 2
		if (column == 1) {
			buttonVal = 4;
			TIM2->CCR1 = 1599; //40% duty cycle
		}
		else if (column == 2) {
			buttonVal = 5;
			TIM2->CCR1 = 1999; //50% duty cycle
		}
		else if (column == 3) {
			buttonVal = 6;
			TIM2->CCR1 = 2399; //60% duty cycle
		}
		else {
			buttonVal = 11;
		}
	}
	
	if (row == 3) {		//row == 3
		if (column == 1) {
			buttonVal = 7;
			TIM2->CCR1 = 2799; //70% duty cycle
		}
		else if (column == 2) {
			buttonVal = 8;
			TIM2->CCR1 = 3199; //80% duty cycle
		}
		else if (column == 3) {
			buttonVal = 9;
			TIM2->CCR1 = 3599; //90% duty cycle
		}
		else {
			buttonVal = 12;
		}
	}
	
	if (row == 4) {		//row == 4
		if (column == 1) {
			buttonVal = 14;
		}
		if (column == 2) {
			buttonVal = 0;
			TIM2->CCR1 = 0; //0% duty cycle
		}
		if (column == 3) {
			buttonVal = 15;
		}
		if (column == 4) {
			buttonVal = 13;
		}
	}
	
		GPIOA->ODR &= ~(0x0F00); //reset all columns back to 0

	// Clear pending bit for interrupt
	EXTI->PR1 |= 0x0001;
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	__enable_irq();
}
/*---------------------------------------------*/
/*  Main Function driving program  */
/*---------------------------------------------*/
int main (void) {
	pinSetup();
	GPIOA->ODR &= ~0x0F00;
	cntVal = 0;
	buttonVal = 0;
	dispButton = 0;
	interruptSetup();
	
	while(1) {
		display();
	}
}