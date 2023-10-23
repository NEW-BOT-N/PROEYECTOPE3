#include<stdint.h>
#include<stdbool.h>
#include"inc/hw_memmap.h"
#include"inc/tm4c123gh6pm.h"
#include"driverlib/sysctl.h"
#include"driverlib/gpio.h"
#include"driverlib/timer.h"
#include"driverlib/interrupt.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/uartstdio.c"
#include "inc/hw_uart.h"
#include "driverlib/pwm.h"
#include "driverlib/rom.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/debug.h"
//PUERTOS DE ENTRADA LDR, SALIDA LEDS Y DISPLAY
#define LEDS GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
#define DISPLAYS GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7
#define LDRS GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
//VARIABLES PARA MOVER EL SERVOMOTOR
#define FRECUENCIA 55
volatile uint32_t CARGA;
volatile uint32_t PWMCLOCK;
volatile uint8_t AJUSTE;
bool SERVO1=false;
bool SERVO2=false;

bool INICIO=false;
bool CONTROL=false;
bool DETENER=false;
bool SECUENCIA=false;
bool NOTDISPLAY = false;
int TIMEINT = 80000;
int P[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
int L[12] = {0};
//VARIABLES PARA TEMPORIZADORES
uint8_t S = 0;
uint8_t C = 2;
uint8_t H = 12;
uint8_t T = 0;
uint8_t t = 100;
uint8_t J = 0;
uint8_t j = 10;
//VARIABLES PARA CONTROLAR LA SECUENCIA Y DECODIFICAR EL PUNTEO
uint8_t LED = 0;
uint8_t UNISCORE = 0;
uint8_t DECSCORE = 0;
uint8_t CODSEGM[10] = {63,6,91,79,102,109,125,7,127,111};
uint32_t ui32Status;
char *CHARS = "0123456789";
char TX[2];
char RX[2];

void TimeOut(void);
void TimeConfig(void);
void UARTConfig(void);
void PWMConfig(void);
void initClock(void);
void initGPIO(void);
void GPIOIntHandler(void);

void UARTIntHandler(void){
    ui32Status = UARTIntStatus(UART3_BASE, true);
    UARTIntClear(UART3_BASE,ui32Status);
    while (UARTCharsAvail(UART3_BASE)){
        RX[0] = UARTCharGet(UART3_BASE);
        RX[1] = UARTCharGetNonBlocking(UART3_BASE);
        if (RX[0]=='I'){
            INICIO=!INICIO;
        }
        if (RX[0]=='D'){
            DETENER=!DETENER;
        }

        if (CONTROL == false){
            int i = 0;
            if (RX[0]=='A'){
                int LTEMP[12] = {0,4,6,2,8,10,3,7,11,1,5,9};
                for (i = 0; i < 12; i++) {
                    P[i] = LTEMP[i]*C;
                }
                SECUENCIA=true;
                INICIO =false;
            }else if (RX[0]=='B'){
                int LTEMP[12] = {5, 7, 11, 4, 6, 9, 1, 3, 10, 0, 2, 8};
                for (i=0; i < 12; i++) {
                    P[i] = LTEMP[i]*C;
                }
                SECUENCIA=true;
                INICIO =false;
            }else if (RX[0]=='C'){
                int LTEMP[12] = {5, 7, 10, 0, 2, 3, 1, 8, 9, 4, 6, 11};
                for (i = 0; i < 12; i++) {
                    P[i] = LTEMP[i]*C;
                }
                SECUENCIA=true;
                INICIO =false;
            }
        }
    }
}

int main(void){
    initClock();
    PWMConfig();
    initGPIO();
    TimeConfig();
    UARTConfig();
    while(1){
        if (S == P[0] | S == P[1] | S == P[2]){
            if(INICIO==true && DETENER==false && CONTROL==true){
                PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, 74 * CARGA / 1000);
            }
            LED = 1;
        }
        else if (S == P[3] | S == P[4] | S == P[5]){
            LED = 2;
        }
        else if (S == P[6] | S == P[7] | S == P[8]){
            if(INICIO==true && DETENER==false && CONTROL==true){
               PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7, 74 * CARGA / 1000);
            }
            LED = 4;
            }
        else if (S == P[9] | S == P[10] | S == P[11]){
            LED = 8;
        }
        else{
            if(INICIO==true && DETENER==false && CONTROL==true){
               PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, 21 * CARGA / 1000);
               PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7, 21 * CARGA / 1000);
            }
            LED = 0;
        }
    }
}
void initClock(void){
    SysCtlClockSet(SYSCTL_OSC_MAIN|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ |SYSCTL_SYSDIV_2_5);
}

void initGPIO(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, DISPLAYS);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, LEDS);
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTF_CR_R = 0x0f;
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, LDRS);
    GPIOPadConfigSet(GPIO_PORTF_BASE, LDRS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_1 | GPIO_INT_PIN_2 | GPIO_INT_PIN_3);
    GPIOIntTypeSet(GPIO_PORTF_BASE, LDRS,GPIO_FALLING_EDGE);
    IntPrioritySet(INT_GPIOF, 0);
    IntRegister(INT_GPIOF, GPIOIntHandler);
    IntMasterEnable();
}
void GPIOIntHandler(void){
    uint32_t pinStatus = GPIOIntStatus(GPIO_PORTF_BASE, true);
    if((UNISCORE + (10*DECSCORE)) < 12){
        if ((pinStatus == 1) && LED == 1) {
            UNISCORE++;
            IntDisable(INT_GPIOF);
        }else if ((pinStatus == 2) && LED == 2) {
            UNISCORE++;
            IntDisable(INT_GPIOF);
        }else if ((pinStatus == 4) && LED == 4) {
            UNISCORE++;
            IntDisable(INT_GPIOF);
        }else if ((pinStatus == 8) && LED == 8) {
            UNISCORE++;
            IntDisable(INT_GPIOF);
        }else{
            GPIOIntClear(GPIO_PORTF_BASE, pinStatus);
            IntEnable(INT_GPIOF);
        }

        if (UNISCORE > 9) {
            UNISCORE = 0;
            DECSCORE++;
        }
        if (DECSCORE > 9) {
            DECSCORE = 0;
        }
        UARTCharPut(UART3_BASE, CHARS[DECSCORE]);
        UARTCharPut(UART3_BASE, CHARS[UNISCORE]);
        UARTCharPut(UART3_BASE, '\r');
        UARTCharPut(UART3_BASE, '\n');
    }

}

void TimeConfig(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A,TIMEINT);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerIntRegister(TIMER0_BASE, TIMER_A, TimeOut);
    //IntPrioritySet(TIMER0_BASE, 1);
    IntEnable(INT_TIMER0A);
    IntMasterEnable();
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void TimeOut(){
    TimerIntClear(TIMER0_BASE,  TIMER_TIMA_TIMEOUT);
    if (NOTDISPLAY==false){
        GPIOPinWrite(GPIO_PORTB_BASE, DISPLAYS,CODSEGM[UNISCORE]);
    }
    else if(NOTDISPLAY==true){
        GPIOPinWrite(GPIO_PORTB_BASE, DISPLAYS,CODSEGM[DECSCORE]+128);
    }
    NOTDISPLAY = !NOTDISPLAY;
    if(INICIO==false && DETENER==false && CONTROL==false && SECUENCIA == true){
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, 21 * CARGA / 1000);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7, 21 * CARGA / 1000);
    }
    if(INICIO==true && DETENER==false && CONTROL==false && SECUENCIA == true){
        GPIOIntClear(GPIO_PORTF_BASE, GPIOIntStatus(GPIO_PORTF_BASE, true));
        IntEnable(INT_GPIOF);
        GPIOPinWrite(GPIO_PORTE_BASE, LEDS, 0);
        UNISCORE = 0;
        DECSCORE = 0;
        S=0;
        T=0;
        J=0;
        LED=0;
        CONTROL = true;
        SECUENCIA = false;
    }
    if(INICIO==true && DETENER==false && CONTROL==true){
        GPIOPinWrite(GPIO_PORTE_BASE, LEDS, LED);
        T = T + 1;
        if (T == t){
            T = 0;
            J = J + 1;
            if (J == j){
                GPIOIntClear(GPIO_PORTF_BASE, GPIOIntStatus(GPIO_PORTF_BASE, true));
                IntEnable(INT_GPIOF);
                J = 0;
                S = S + 1;
                if (S == H*C){
                    INICIO=false;
                    CONTROL=false;
                }
            }
        }
    }

    if(INICIO==false && DETENER==false && CONTROL==true){
       GPIOPinWrite(GPIO_PORTE_BASE, LEDS, LED);
       UNISCORE = UNISCORE;
       DECSCORE = DECSCORE;
       S=S;
       T=T;
       J=J;
       CONTROL = true;
    }if(DETENER == true){
        GPIOIntClear(GPIO_PORTF_BASE, GPIOIntStatus(GPIO_PORTF_BASE, true));
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, 74 * CARGA / 1000);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7, 74 * CARGA / 1000);
        GPIOPinWrite(GPIO_PORTE_BASE, LEDS, 0);
        UNISCORE = 0;
        DECSCORE = 0;
        S=0;
        T=0;
        J=0;
        LED=0;
        DETENER = false;
        CONTROL = false;
        INICIO = false;
        SECUENCIA = false;
    }
}
void UARTConfig(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    GPIOPinConfigure(GPIO_PC6_U3RX);
    GPIOPinConfigure(GPIO_PC7_U3TX);
    UARTConfigSetExpClk(UART3_BASE,SysCtlClockGet(),9600,(UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE));
    IntMasterEnable();
    IntEnable(INT_UART3);
    UARTIntEnable(UART3_BASE, UART_INT_RX | UART_INT_RT);
    UARTEnable(UART3_BASE);
}
void PWMConfig(void){
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
    PWMCLOCK = SysCtlClockGet() / 64;
    CARGA = (PWMCLOCK / FRECUENCIA) - 1;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    GPIOPinTypePWM(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPinConfigure(GPIO_PC4_M0PWM6);
    GPIOPinConfigure(GPIO_PC5_M0PWM7);

    PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, CARGA);
    PWMGenEnable(PWM0_BASE, PWM_GEN_3);
    PWMOutputState(PWM0_BASE, PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, 74 * CARGA / 1000);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7, 74 * CARGA / 1000);
}

