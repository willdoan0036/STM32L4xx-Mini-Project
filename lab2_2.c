	/*====================================================*/
	/* ELEC 3040/3050 - Lab 3, Program 3 */
	/*====================================================*/
	#include "stm32l4xx.h" /* Microcontroller information */
	 
	#include<stdio.h>
	#include<stdlib.h>
	 
	static unsigned int B;
	static unsigned int A;
	static unsigned int direction; 
	unsigned int run;
	unsigned int up;
	 
	/*---------------------------------------------------*/
	/* Initialize GPIO pins used in the program */
	/* PA11 = push button */
	/* PB4 = LDR, PB5 = green LED */
	/*---------------------------------------------------*/
	void PinSetup () {
	   /* Configure PA1 as s1 , PA2(DIO1) as s2 */
	   RCC->AHB2ENR |= 0x01; /* Enable GPIOA clock (bit 0) */
	   GPIOA->MODER &= (0xFFFFFFC3); /* General purpose input mode */
		
		 /* Configure pins to drive LEDs */
		 GPIOA->MODER &= ~(0x003FFFC00); /* Clear PA5-PA12*/
	   GPIOA->MODER |= (0x01555400); /* General Purpose mode */
		
		RCC->AHB2ENR |= 0x02; /* Enable GPIOB clock (bit 1) */
		GPIOB->MODER &= ~(0x000003C0); /* Clear PB4-PB3 mode bits */
		GPIOB->MODER |=  (0x00000140); /* General purpose output mode*/
	}
	 
	 
	/*---------------------------------------------------*/
	/* Configure System Interrupts, used in the program */
	/*---------------------------------------------------*/
	void ConfigureSystemInterupts()  {
	   /* enable SYSCFG clock –only necessary for change of SYSCFG */ 
	   RCC->APB2ENR |= 0x01;  // Set bit 0 of APB2ENR to turn on clock for SYSCFG
	   /* select PB1 as EXTI1 (reset value is PA1) Interrupt when button 1/2 is pressed*/ 
	   SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1;//clear EXTI1 bit in configure
	   SYSCFG->EXTICR[0] |=  SYSCFG_EXTICR1_EXTI1_PA;//select PB1 as interrupt source 
		 SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI2;//clear EXTI2 bit in configure
	   SYSCFG->EXTICR[0] |=  SYSCFG_EXTICR1_EXTI2_PA;//select PB2 as interrupt source 
	   //SYSCFG->EXTICR[0] &= 0xF0FF;   //clear EXTI2 bit field
		 //SYSCFG->EXTICR[0] |=  0x0002;    //clear exit 1 
		
			/* configure and enable EXTI1 as falling-edge triggered */ 
		 EXTI->FTSR1 |= EXTI_FTSR1_FT1;//EXTI1 = falling-edge triggered 
		 EXTI->PR1   |=  EXTI_PR1_PIF1;//Clear EXTI1 pending bit 
		 EXTI->IMR1 |=  EXTI_IMR1_IM1;//Enable EXTI1 
			
		 EXTI->FTSR1 |= EXTI_FTSR1_FT2;//EXTI2 = falling-edge triggered FTSR1 is for top bits 
		 EXTI->PR1   |=  EXTI_PR1_PIF2;//Clear EXTI2 pending bit 
		 EXTI->IMR1 |=  EXTI_IMR1_IM2;//Enable EXTI2 
		
	   /* Program NVIC to clear pending bit and enable EXTI1 --use CMSIS functions*/ 
	   NVIC_ClearPendingIRQ(EXTI1_IRQn); // Clear NVIC pending bit
		 NVIC_ClearPendingIRQ(EXTI2_IRQn); // Clear NVIC pending bit
	   NVIC_EnableIRQ(EXTI1_IRQn); //Enable IRQ
		 NVIC_EnableIRQ(EXTI2_IRQn); //Enable IRQ
	}
	 
	 
	/*----------------------------------------------------------*/
	/* Delay function - do nothing for about .5 second */
	/*----------------------------------------------------------*/
	void delay () {
	   int volatile i,j,n;
	   for (i=0; i<10; i++) { //outer loop
	      for (j=0; j<20000; j++) { //inner loop
	         n = j; //dummy operation for single-step test
	      } //do nothing
	   }
	}
	 
	 void EXTI1_IRQHandler () { 
		 if (run == 0x000)  {
			 GPIOB->BSRR = 0x0008; //Set PB3=1 to turn ON LED
			 run = 0x0002;
		 }
		 else  {
				GPIOB->BSRR = 0x0008 << 16; //Reset PB3=0
				run = 0x0000;
		 }
		 EXTI->PR1   =  EXTI_PR1_PIF1;//Clear EXTI1 pending bit 
		 NVIC_ClearPendingIRQ(EXTI1_IRQn); // Clear NVIC pending bit
	 }
	 
	 
	 void EXTI2_IRQHandler () {
		 if (up == 0x000)  {
			 GPIOB->BSRR = 0x0010; //Set PB4=1 to turn ON LED
			 up = 0x0004;
		 }
		 else  {
				GPIOB->BSRR = 0x0010 << 16; //Reset PB4=0
				up = 0x0000;
		 }
		 EXTI->PR1   =  EXTI_PR1_PIF2;//Clear EXTI2 pending bit 
		 NVIC_ClearPendingIRQ(EXTI2_IRQn); // Clear NVIC pending bit
	 }
	 
	 
	 
	/*----------------------------------------------------------*/
	/* Count function - Counter A:counts up or down from 0 to 9, and rolls over (repeat)*/
	/*----------------------------------------------------------*/
	void count(int direction)  { 
	   if(B < 9)  {
			 B++;
		 }
		 else  {
			 B = 0;
		 }
	   //count down
	   if(direction == 0x0004)  {
	       if (A < 1)  {
	           A = 9;
			 }
	       else  {
	           A--;
			 }
	   }
	   //count up
	   else  {
	      if (A < 9)  {
	         A++;
	      }
	      else  {
	         A = 0;
	      }
	   }
		 GPIOA->ODR &= 0x0000;
		 GPIOA->ODR |= (A << 5) + (B << 9);
	}
	   	 
	/*------------------------------------------------*/
	/* Main program */
	/*------------------------------------------------*/
	int main(void)  {
	   PinSetup(); //Configure GPIO pins
	   ConfigureSystemInterupts(); //Configure System Interrupts
		 __enable_irq();
		 //NVIC_SetPriority(EXTI1_IRQn, 1); 
	   while(1)  {
	      delay();
	     //Read GPIOA and mask all but bit 11
	      //run = GPIOA->IDR & 0x0002;    
	      //up = GPIOA->IDR & 0x0004;
	        
	      //Check if sw1 is in logic one position if not return to top
	      if (run == 0)  {
	         continue;
	      }
	      else  {         
	         count(up);
	      }
	 
	   }
