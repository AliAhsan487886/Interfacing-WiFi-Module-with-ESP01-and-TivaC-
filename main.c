#include <string.h>
#include <stdint.h>

//==============================================================
//                  SYSTEM CONTROL REGISTERS
//==============================================================
#define SYSCTL_RCGCGPIO     (*((volatile unsigned long*)0x400FE608))
#define SYSCTL_RCGCUART     (*((volatile unsigned long*)0x400FE618))

//==============================================================
//                  UART0 REGISTERS  (Laptop USB)
//==============================================================
#define UART0_DR            (*((volatile unsigned long*)0x4000C000))
#define UART0_FR            (*((volatile unsigned long*)0x4000C018))
#define UART0_IBRD          (*((volatile unsigned long*)0x4000C024))
#define UART0_FBRD          (*((volatile unsigned long*)0x4000C028))
#define UART0_LCRH          (*((volatile unsigned long*)0x4000C02C))
#define UART0_CTL           (*((volatile unsigned long*)0x4000C030))

//==============================================================
//                  UART2 REGISTERS  (ESP UART)
//==============================================================
#define UART2_DR            (*((volatile unsigned long*)0x4000E000))
#define UART2_FR            (*((volatile unsigned long*)0x4000E018))
#define UART2_IBRD          (*((volatile unsigned long*)0x4000E024))
#define UART2_FBRD          (*((volatile unsigned long*)0x4000E028))
#define UART2_LCRH          (*((volatile unsigned long*)0x4000E02C))
#define UART2_CTL           (*((volatile unsigned long*)0x4000E030))

//==============================================================
//                  GPIO PORT A (UART0)
//==============================================================
#define GPIO_PORTA_AFSEL    (*((volatile unsigned long*)0x40004420))
#define GPIO_PORTA_PCTL     (*((volatile unsigned long*)0x4000452C))
#define GPIO_PORTA_DEN      (*((volatile unsigned long*)0x4000451C))

//==============================================================
//                  GPIO PORT D (UART2)
//==============================================================
#define GPIO_PORTD_AFSEL    (*((volatile unsigned long*)0x40007420))
#define GPIO_PORTD_PCTL     (*((volatile unsigned long*)0x4000752C))
#define GPIO_PORTD_DEN      (*((volatile unsigned long*)0x4000751C))
#define GPIO_PORTD_LOCK     (*((volatile unsigned long*)0x40007520))
#define GPIO_PORTD_CR       (*((volatile unsigned long*)0x40007524))

//==============================================================
//                       FLAG MASKS
//==============================================================
#define RXFE 0x10        // Receive FIFO Empty
#define TXFF 0x20        // Transmit FIFO Full

//==============================================================
//                       SMALL DELAY
//==============================================================
void Wait(unsigned long d) {
    volatile unsigned long i;
    for(i=0;i<d;i++);
}

//==============================================================
//                      LAPTOP UART0 (USB)
//==============================================================
//System Initialization for Floating Point Unit
void SystemInit (void)
{
	  /* --------------------------FPU settings ----------------------------------*/
	#if (__FPU_USED == 1)
		SCB->CPACR |= ((3UL << 10*2) |                 /* set CP10 Full Access */
                  (3UL << 11*2)  );               /* set CP11 Full Access */
	#endif
}
void LAPTOP_UART0_Init(void) {
    SYSCTL_RCGCUART |= 0x01;   // Enable UART0 clock
    SYSCTL_RCGCGPIO |= 0x01;   // Enable PORT A clock
    Wait(1000);

    GPIO_PORTA_AFSEL |= 0x03;  // PA0, PA1 alternate function
    GPIO_PORTA_PCTL  &= 0xFFFFFF00;
    GPIO_PORTA_PCTL  |= 0x11;
    GPIO_PORTA_DEN   |= 0x03;  // Digital enable

    UART0_CTL = 0;              // Disable UART0 for config
    UART0_IBRD = 8;             // 115200 baud @ 16MHz
    UART0_FBRD = 44;
    UART0_LCRH = 0x70;          // 8-bit, FIFO
    UART0_CTL = 0x301;          // UART enable, TXE, RXE
}

void LAPTOP_PrintChar(char c) {
    while(UART0_FR & TXFF);
    UART0_DR = c;
}

void LAPTOP_Print(char *str) {
    while(*str) LAPTOP_PrintChar(*str++);
}

//==============================================================
//                      ESP UART2
//==============================================================
void ESP_UART2_Init(void) {
    SYSCTL_RCGCUART |= 0x04;  // Enable UART2
    SYSCTL_RCGCGPIO |= 0x08;  // Enable PORT D
    Wait(1000);

    GPIO_PORTD_LOCK = 0x4C4F434B;  // Unlock PD7
    GPIO_PORTD_CR   = 0xC0;        // Commit PD6, PD7

    GPIO_PORTD_AFSEL |= 0xC0;      // PD6, PD7 alternate
    GPIO_PORTD_PCTL  &= 0x00FFFFFF;
    GPIO_PORTD_PCTL  |= 0x11000000;
    GPIO_PORTD_DEN   |= 0xC0;

    UART2_CTL = 0;
    UART2_IBRD = 8;      // 115200 baud
    UART2_FBRD = 44;
    UART2_LCRH = 0x70;
    UART2_CTL = 0x301;   // RXE, TXE, UART ENABLE
}

void ESP_SendChar(char c) {
    while(UART2_FR & TXFF);
    UART2_DR = c;
}

void ESP_SendStr(char *str) {
    while(*str) ESP_SendChar(*str++);
}

char ESP_ReadChar(void) {
    while(UART2_FR & RXFE);
    return (char)(UART2_DR & 0xFF);
}

//==============================================================
//           SEND COMMAND AND READ RESPONSE FROM ESP
//==============================================================
void ESP_CommandWithResponse(char *cmd) {
		unsigned long timeout = 10000000; // adjust if needed
		char c;
    ESP_SendStr(cmd);
//		LAPTOP_Print(cmd);
	

    while(timeout--) {
        if(!(UART2_FR & RXFE)) {
            c = ESP_ReadChar();
						LAPTOP_PrintChar(c);
        }
    }
}

//==============================================================
//             READ ESP IP AUTOMATICALLY
//==============================================================
void ESP_PrintIP(void) {
	  unsigned long timeout = 10000000;
		char c;
    ESP_SendStr("AT+CIFSR\r\n");  // Get IP

    
    while(timeout--) {
        if(!(UART2_FR & RXFE)) {
            c = ESP_ReadChar();
            LAPTOP_PrintChar(c);  // print on Laptop-1
        }
    }
}



//==============================================================
//                      MAIN PROGRAM
//==============================================================
int main(void) {
	char c;
    LAPTOP_UART0_Init();
    ESP_UART2_Init();

    LAPTOP_Print("\r\n=== SYSTEM STARTED ===\r\n");

    // -----------------------------------------------------
    // ESP initialization: AP mode + server
    // -----------------------------------------------------
    ESP_CommandWithResponse("AT\r\n");
    ESP_CommandWithResponse("AT+CWMODE=1\r\n");
		ESP_CommandWithResponse("AT+CWJAP=\"Nabeel\",\"12343211\"\r\n");
    ESP_CommandWithResponse("AT+CIPMUX=1\r\n");  
    ESP_CommandWithResponse("AT+CIPSERVER=1,23\r\n"); 

    // Automatically print ESP IP
    LAPTOP_Print("\r\n=== ESP IP ADDRESS ===\r\n");
    ESP_PrintIP();
    LAPTOP_Print("\r\nConnect Laptop-2 to this IP (RAW/Telnet, port 23)\r\n");
		while(1){
			if(!(UART2_FR & RXFE)) {
            c = ESP_ReadChar();
            LAPTOP_PrintChar(c);  // print on Laptop-1
        }
		}

}
