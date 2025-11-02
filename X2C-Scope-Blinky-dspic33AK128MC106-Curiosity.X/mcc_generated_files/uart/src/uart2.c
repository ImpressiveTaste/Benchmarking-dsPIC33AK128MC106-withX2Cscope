/**
 * UART2 Generated Driver Source File
 * 
 * @file      uart2.c
 *            
 * @ingroup   uartdriver
 *            
 * @brief     This is the generated driver source file for the UART2 driver.
 *
 * @skipline @version   PLIB Version 1.0.2
 *            
 * @skipline  Device : dsPIC33AK128MC106
*/

/*
© [2025] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

// Section: Included Files
#include <stdint.h>
#include <stddef.h>
#include <xc.h>
#include <stddef.h>
#include "../uart2.h"

// Section: Macro Definitions
#define UART2_CLOCK 100000000U
#define UART2_BAUD_TO_BRG_WITH_FRACTIONAL(x) (UART2_CLOCK/(x))
#define UART2_BAUD_TO_BRG_WITH_BRGS_1(x) (UART2_CLOCK/(4U*(x))-1U)
#define UART2_BAUD_TO_BRG_WITH_BRGS_0(x) (UART2_CLOCK/(16U*(x))-1U)
#define UART2_BRG_TO_BAUD_WITH_FRACTIONAL(x) (UART2_CLOCK/(x))
#define UART2_BRG_TO_BAUD_WITH_BRGS_1(x) (UART2_CLOCK/(4U*((x)+1U)))
#define UART2_BRG_TO_BAUD_WITH_BRGS_0(x) (UART2_CLOCK/(16U*((x)+1U)))

#define UART2_MIN_ACHIEVABLE_BAUD_WITH_FRACTIONAL 95U
#define UART2_MIN_ACHIEVABLE_BAUD_WITH_BRGS_1 24U

// Section: Driver Interface

const struct UART_INTERFACE X2CScopeUart = {
    .Initialize = &UART2_Initialize,
    .Deinitialize = &UART2_Deinitialize,
    .Read = &UART2_Read,
    .Write = &UART2_Write,
    .IsRxReady = &UART2_IsRxReady,
    .IsTxReady = &UART2_IsTxReady,
    .IsTxDone = &UART2_IsTxDone,
    .TransmitEnable = &UART2_TransmitEnable,
    .TransmitDisable = &UART2_TransmitDisable,
    .TransmitInterruptEnable = NULL,
    .TransmitInterruptDisable = NULL,
    .AutoBaudSet = &UART2_AutoBaudSet,
    .AutoBaudQuery = &UART2_AutoBaudQuery,
    .AutoBaudEventEnableGet = &UART2_AutoBaudEventEnableGet,
    .BRGCountSet = &UART2_BRGCountSet,
    .BRGCountGet = &UART2_BRGCountGet,
    .BaudRateSet = &UART2_BaudRateSet,
    .BaudRateGet = &UART2_BaudRateGet,
    .ErrorGet = &UART2_ErrorGet,
    .RxCompleteCallbackRegister = NULL,
    .TxCompleteCallbackRegister = NULL,
    .TxCollisionCallbackRegister = NULL,
    .FramingErrorCallbackRegister = NULL,
    .OverrunErrorCallbackRegister = NULL,
    .ParityErrorCallbackRegister = NULL,
};

// Section: Private Variable Definitions
static union
{
    struct
    {
        uint16_t frammingError :1;
        uint16_t parityError :1;
        uint16_t overrunError :1;
        uint16_t txCollisionError :1;
        uint16_t autoBaudOverflow :1;
        uint16_t reserved :11;
    };
    size_t status;
} uartError;

// Section: UART2 APIs

void UART2_Initialize(void)
{
/*    
     Set the UART2 module to the options selected in the user interface.
     Make sure to set LAT bit corresponding to TxPin as high before UART initialization
*/
    // MODE Asynchronous 8-bit UART; RXEN ; TXEN ; ABDEN ; BRGS ; SENDB ; BRKOVR ; RXBIMD ; WUE ; SIDL ; ON disabled; FLO ; TXPOL ; C0EN ; STP 1 Stop bit sent, 1 checked at RX; RXPOL ; RUNOVF ; HALFDPLX ; CLKSEL Standard Speed Peripheral Clock; CLKMOD enabled; ACTIVE ; SLPEN ; 
    U2CON = 0x8000000UL;
    // TXCIF ; RXFOIF ; RXBKIF ; CERIF ; ABDOVIF ; TXCIE ; RXFOIE ; RXBKIE ; FERIE ; CERIE ; ABDOVIE ; PERIE ; TXMTIE ; STPMD ; TXWRE ; RXWM ; TXWM ; 
    U2STAT = 0x2E0080UL;
    // BaudRate 115207.37; Frequency 100000000 Hz; BRG 868; 
    U2BRG = 0x364UL;
    
    U2CONbits.ON = 1;   // enabling UART ON bit
    U2CONbits.TXEN = 1;
    U2CONbits.RXEN = 1;
}

void UART2_Deinitialize(void)
{
    U2CON = 0x0UL;
    U2STAT = 0x2E0080UL;
    U2BRG = 0x0UL;
}

uint8_t UART2_Read(void)
{
    while((U2STATbits.RXBE == 1))
    {
        
    }

    if ((U2STATbits.RXFOIF == 1))
    {
        U2STATbits.RXFOIF = 0;
    }
    
    return U2RXB;
}

void UART2_Write(uint8_t txData)
{
    while(U2STATbits.TXBF == 1)
    {
        
    }

    U2TXB = txData;    // Write the data byte to the USART.
}

bool UART2_IsRxReady(void)
{
    return (U2STATbits.RXBE == 0);
}

bool UART2_IsTxReady(void)
{
    return ((!U2STATbits.TXBF) && U2CONbits.TXEN);
}

bool UART2_IsTxDone(void)
{
    return (bool)(U2STATbits.TXMTIF && U2STATbits.TXBE);
}

void UART2_TransmitEnable(void)
{
    U2CONbits.TXEN = 1;
}

void UART2_TransmitDisable(void)
{
    U2CONbits.TXEN = 0;
}

void UART2_AutoBaudSet(bool enable)
{
    U2UIRbits.ABDIF = 0U;
    U2UIRbits.ABDIE = enable;
    U2CONbits.ABDEN = enable;
}

bool UART2_AutoBaudQuery(void)
{
    return U2CONbits.ABDEN;
}

bool UART2_AutoBaudEventEnableGet(void)
{ 
    return U2UIRbits.ABDIE; 
}

size_t UART2_ErrorGet(void)
{
    uartError.status = 0;
    if(U2STATbits.FERIF == 1U)
    {
        uartError.status = uartError.status|(uint16_t)UART_ERROR_FRAMING_MASK;
    }
    if(U2STATbits.PERIF== 1U)
    {
        uartError.status = uartError.status|(uint16_t)UART_ERROR_PARITY_MASK;
    }
    if(U2STATbits.RXFOIF== 1U)
    {
        uartError.status = uartError.status|(uint16_t)UART_ERROR_RX_OVERRUN_MASK;
        U2STATbits.RXFOIF = 0;
    }
    if(U2STATbits.TXCIF== 1U)
    {
        uartError.status = uartError.status|(uint16_t)UART_ERROR_TX_COLLISION_MASK;
        U2STATbits.TXCIF = 0;
    }
    if(U2STATbits.ABDOVIF== 1U)
    {
        uartError.status = uartError.status|(uint16_t)UART_ERROR_AUTOBAUD_OVERFLOW_MASK;
        U2STATbits.ABDOVIF = 0;
    }
    
    return uartError.status;
}

void UART2_BRGCountSet(uint32_t brgValue)
{
    U2BRG = brgValue;
}

uint32_t UART2_BRGCountGet(void)
{
    return U2BRG;
}

void UART2_BaudRateSet(uint32_t baudRate)
{
    uint32_t brgValue;
    
    if((baudRate >= UART2_MIN_ACHIEVABLE_BAUD_WITH_FRACTIONAL) && (baudRate != 0U))
    {
        U2CONbits.CLKMOD = 1;
        U2CONbits.BRGS = 0;
        brgValue = UART2_BAUD_TO_BRG_WITH_FRACTIONAL(baudRate);
    }
    else if(baudRate >= UART2_MIN_ACHIEVABLE_BAUD_WITH_BRGS_1)
    {
        U2CONbits.CLKMOD = 0;
        U2CONbits.BRGS = 1;
        brgValue = UART2_BAUD_TO_BRG_WITH_BRGS_1(baudRate);
    }
    else
    {
        U2CONbits.CLKMOD = 0;
        U2CONbits.BRGS = 0;
        brgValue = UART2_BAUD_TO_BRG_WITH_BRGS_0(baudRate);
    }
    U2BRG = brgValue;
}

uint32_t UART2_BaudRateGet(void)
{
    uint32_t brgValue;
    uint32_t baudRate;
    
    brgValue = UART2_BRGCountGet();
    if((U2CONbits.CLKMOD == 1U) && (brgValue != 0U))
    {
        baudRate = UART2_BRG_TO_BAUD_WITH_FRACTIONAL(brgValue);
    }
    else if(U2CONbits.BRGS == 1)
    {
        baudRate = UART2_BRG_TO_BAUD_WITH_BRGS_1(brgValue);
    }
    else
    {
        baudRate = UART2_BRG_TO_BAUD_WITH_BRGS_0(brgValue);
    }
    return baudRate;
}
