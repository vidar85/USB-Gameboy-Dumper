

/** INCLUDES *******************************************************/
#include "USB/usb.h"
#include "HardwareProfile.h"
#include "USB/usb_function_generic.h"
#include <delays.h>

/** CONFIGURATION **************************************************/
#define PICDEM_FS_USB
        #pragma config PLLDIV   = 5         // (20 MHz crystal on PICDEM FS USB board)
        #pragma config CPUDIV   = OSC1_PLL2
        #pragma config USBDIV   = 2         // Clock source from 96MHz PLL/2
        #pragma config FOSC     = HSPLL_HS
        #pragma config FCMEN    = OFF
        #pragma config IESO     = OFF
        #pragma config PWRT     = OFF
        #pragma config BOR      = ON
        #pragma config BORV     = 28
        #pragma config VREGEN   = ON      //USB Voltage Regulator
        #pragma config WDT      = OFF
        #pragma config WDTPS    = 32768
        #pragma config MCLRE    = ON
        #pragma config LPT1OSC  = OFF
        #pragma config PBADEN   = OFF
        #pragma config STVREN   = ON
        #pragma config LVP      = OFF
        #pragma config XINST    = OFF       // Extended Instruction Set
        #pragma config CP0      = OFF
        #pragma config CP1      = OFF
        #pragma config CPB      = OFF
        #pragma config WRT0     = OFF
        #pragma config WRT1     = OFF
        #pragma config WRTB     = OFF       // Boot Block Write Protection
        #pragma config WRTC     = OFF
        #pragma config EBTR0    = OFF
        #pragma config EBTR1    = OFF
        #pragma config EBTRB    = OFF


/** VARIABLES ******************************************************/
unsigned char temp;

#pragma udata USB_VARIABLE = 0x47E
unsigned char OUTPacket[64];	//User application buffer for receiving and holding OUT packets sent from the host
unsigned char INPacket[64];		//User application buffer for sending IN packets to the host
unsigned short address;

#if defined(__18CXX)
    #pragma udata
#endif

USB_HANDLE USBGenericOutHandle;
USB_HANDLE USBGenericInHandle;
#if defined(__18CXX)
    #pragma udata
#endif

/** PRIVATE PROTOTYPES *********************************************/
static void InitializeSystem(void);
void USBDeviceTasks(void);
void YourHighPriorityISRCode(void);
void YourLowPriorityISRCode(void);
void USBCBSendResume(void);
void UserInit(void);
void ProcessIO(void);


/** VECTOR REMAPPING ***********************************************/
#if defined(__18CXX)
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018
	#endif

	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
	extern void _startup (void);        // See c018i.c in your C18 compiler dir
	#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
	void _reset (void)
	{
	    _asm goto _startup _endasm
	}
	#endif
	#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
	void Remapped_High_ISR (void)
	{
	     _asm goto YourHighPriorityISRCode _endasm
	}
	#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
	void Remapped_Low_ISR (void)
	{
	     _asm goto YourLowPriorityISRCode _endasm
	}

	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)

	#pragma code HIGH_INTERRUPT_VECTOR = 0x08
	void High_ISR (void)
	{
	     _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#pragma code LOW_INTERRUPT_VECTOR = 0x18
	void Low_ISR (void)
	{
	     _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#endif	//end of "#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER)"

	#pragma code


	//These are your actual interrupt handling routines.
	#pragma interrupt YourHighPriorityISRCode
	void YourHighPriorityISRCode()
	{
        #if defined(USB_INTERRUPT)
	        USBDeviceTasks();
        #endif

	}
	#pragma interruptlow YourLowPriorityISRCode
	void YourLowPriorityISRCode()
	{

	}
#endif

/** DECLARATIONS ***************************************************/
#pragma code
void DumpFull(void);
void DumpLower(void);
void writeCart(unsigned char data, unsigned short addr);
unsigned char readCart(unsigned short addr);
unsigned char readCartFast(void);
unsigned short readFastAddress(void);
void setFastAddress(unsigned short);
void gameboy_init(void);
void DumpRAM(void);
void WriteRAM(void);
void send256Chunk(void);



void main(void)
{
    InitializeSystem();

    #if defined(USB_INTERRUPT)
        USBDeviceAttach();
    #endif

    while(1)
    {
        ProcessIO();
    }
}


static void InitializeSystem(void)
{
    ADCON1 |= 0x0F;                 // Default all pins to digital

    USBGenericOutHandle = 0;
    USBGenericInHandle = 0;

    UserInit();			//Application related initialization.  See user.c

    USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware					//variables to known states.
}



void UserInit(void)
{
    gameboy_init();
    LATC |= (1<<6);
}

void ProcessIO(void)
{
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;

    if(!USBHandleBusy(USBGenericOutHandle))		//Check if the endpoint has received any data from the host.
    {
        switch(OUTPacket[0])					//Data arrived, check what kind of command might be in the packet of data.
        {
            case 0x09: LATC |= (1<<6); setFastAddress(0x0000U); DumpLower(); break;
            case 0x01: LATC |= (1<<6); INPacket[0] = readCart(0x0104); INPacket[1] = readCart(0x0105); USBGenericInHandle = USBGenWrite(1,(BYTE*)&INPacket,USBGEN_EP_SIZE); while(USBHandleBusy(USBGenericInHandle)){} break;
            case 0xDD: LATC &= ~(1<<6); setFastAddress(0x0000U); DumpFull(); LATC |= (1<<6); break;
            case 0xDA: LATC &= ~(1<<6); DumpRAM(); LATC |= (1<<6); break;
            case 0xBB: USBDeviceDetach(); Delay1KTCYx(100); INTCONbits.GIE = 0; _asm goto 0x0F5E _endasm; break;

        }

        USBGenericOutHandle = USBGenRead(USBGEN_EP_NUM,(BYTE*)&OUTPacket,USBGEN_EP_SIZE);
    }
}

void USBCBSuspend(void)
{
}

void USBCBWakeFromSuspend(void)
{
}

void USBCB_SOF_Handler(void)
{
}

void USBCBErrorHandler(void)
{

}

void USBCBCheckOtherReq(void)
{
}

void USBCBStdSetDscHandler(void)
{
    // Must claim session ownership if supporting this request
}

void USBCBInitEP(void)
{
    USBEnableEndpoint(USBGEN_EP_NUM,USB_OUT_ENABLED|USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    USBGenericOutHandle = USBGenRead(USBGEN_EP_NUM,(BYTE*)&OUTPacket,USBGEN_EP_SIZE);
    USBGenericInHandle = USBGenRead(USBGEN_EP_NUM,(BYTE*)&INPacket,USBGEN_EP_SIZE);
}

void USBCBSendResume(void)
{
    static WORD delay_count;

    //First verify that the host has armed us to perform remote wakeup.
    //It does this by sending a SET_FEATURE request to enable remote wakeup,
    //usually just before the host goes to standby mode (note: it will only
    //send this SET_FEATURE request if the configuration descriptor declares
    //the device as remote wakeup capable, AND, if the feature is enabled
    //on the host (ex: on Windows based hosts, in the device manager
    //properties page for the USB device, power management tab, the
    //"Allow this device to bring the computer out of standby." checkbox
    //should be checked).
    if(USBGetRemoteWakeupStatus() == TRUE)
    {
        //Verify that the USB bus is in fact suspended, before we send
        //remote wakeup signalling.
        if(USBIsBusSuspended() == TRUE)
        {
            USBMaskInterrupts();

            //Clock switch to settings consistent with normal USB operation.
            USBCBWakeFromSuspend();
            USBSuspendControl = 0;
            USBBusIsSuspended = FALSE;  //So we don't execute this code again,
                                        //until a new suspend condition is detected.

            //Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
            //device must continuously see 5ms+ of idle on the bus, before it sends
            //remote wakeup signalling.  One way to be certain that this parameter
            //gets met, is to add a 2ms+ blocking delay here (2ms plus at
            //least 3ms from bus idle to USBIsBusSuspended() == TRUE, yeilds
            //5ms+ total delay since start of idle).
            delay_count = 3600U;
            do
            {
                delay_count--;
            }while(delay_count);

            //Now drive the resume K-state signalling onto the USB bus.
            USBResumeControl = 1;       // Start RESUME signaling
            delay_count = 1800U;        // Set RESUME line for 1-13 ms
            do
            {
                delay_count--;
            }while(delay_count);
            USBResumeControl = 0;       //Finished driving resume signalling

            USBUnmaskInterrupts();
        }
    }
}


BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
    switch((INT)event)
    {
        case EVENT_TRANSFER:
            //Add application specific callback task or callback function here if desired.
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_CONFIGURED:
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
            break;
        default:
            break;
    }
    return TRUE;
}

void gameboy_init(void)
{
    TRISB = 0xFF; // Input
    TRISC = 0x00;
    TRISD = 0x00;
    TRISA = 0x00;
    TRISE = 0x00;
    LATC |= (1 << 2); //WR HIGH
    LATC &= ~(1 << 1); //RD LOW

    address = 0x0000U;
    TRISD = 0x00;
}

void WriteRAM(void)
{

    unsigned char MBC = readCart(0x0147);
    unsigned char RAM_Size;
    unsigned char banks = 0;
    unsigned char i, z, k;
    i = readCart(0x149);

    switch(i)
    {
        case 0x01: RAM_Size = 32; banks = 1; break;
        case 0x02: RAM_Size = 128; banks = 1; break;
        case 0x03: RAM_Size = 128; banks = 4; break;
    }

    writeCart(0x0AU, 0x0000);
    for(i=0; i<banks; i++)
    {
        writeCart(i, 0x4000U);
        setFastAddress(0xA000U);
        for(z=0; z<RAM_Size; z++)
        {
            USBGenericOutHandle = USBGenRead(USBGEN_EP_NUM,(BYTE*)&OUTPacket,USBGEN_EP_SIZE);
            while(USBHandleBusy(USBGenericInHandle)){}
            for(k=0; k<64U; k++)
                writeCart(OUTPacket[k], address++);
        }
    }

}

void DumpRAM(void)
{
    unsigned char MBC = readCart(0x0147);
    unsigned char RAM_Size;
    unsigned char banks = 0;
    unsigned char i, z, k;
    i = readCart(0x149);

    switch(i)
    {
        case 0x01: RAM_Size = 32; banks = 1; break;
        case 0x02: RAM_Size = 128; banks = 1; break;
        case 0x03: RAM_Size = 128; banks = 4; break;
    }

    writeCart(0x0AU, 0x0000);
    for(i=0; i<banks; i++)
    {
        writeCart(i, 0x4000U);
        setFastAddress(0xA000U);
        for(z=0; z<RAM_Size; z++)
        {
            for(k=0; k<64U; k++)
                INPacket[k] = readCartFast();
            USBGenericInHandle = USBGenWrite(1,(BYTE*)&INPacket,USBGEN_EP_SIZE);
            while(USBHandleBusy(USBGenericInHandle)){}
        }
    }

}

void DumpLower(void)
{
    unsigned short i, z;
    for(i=0U; i<30U; i++)
    {
        for(z=0U; z<64U; z++)
            INPacket[z] = readCartFast();
        USBGenericInHandle = USBGenWrite(1,(BYTE*)&INPacket,USBGEN_EP_SIZE);
        while(USBHandleBusy(USBGenericInHandle)){}
    }
}

unsigned char readCart(unsigned short addr)
{
    unsigned short add = addr;
    LATD = (unsigned char) add;
    temp = (unsigned char) (add >> 8);
    if ((temp & 0x01) == 0x01U) //A8
        LATC |= 0x01;
    else
        LATC &= ~(0x01);
    if ((temp & 0x02) == 0x02U) //A9
        LATE |= (0x04);
    else
        LATE &= ~(0x04);
    if ((temp & 0x04) == 0x04U) //A10
        LATE |= (0x02);
    else
        LATE &= ~(0x02);
    if ((temp & 0x08) == 0x08U) //A11
        LATE |= (0x01);
    else
        LATE &= ~(0x01);
    if ((temp & 0x10) == 0x10U) //A12
        LATA |= (0x20);
    else
        LATA &= ~(0x20);
    if ((temp & 0x20) == 0x20U) //A13
        LATA |= (0x10);
    else
        LATA &= ~(0x10);
    if ((temp & 0x40) == 0x40U) //A14
        LATA |= (0x08);
    else
        LATA &= ~(0x08);
    if ((temp & 0x80) == 0x80U) //A15
        LATA |= (0x04);
    else
        LATA &= ~(0x04);
    //_delay_us(1);
    return PORTB;
}

void DumpFull(void)
{
    unsigned char banksn = readCart(0x0148);
    unsigned char MBC = readCart(0x0147);
    unsigned char banks = 0;
    unsigned short i, z, addr;
    unsigned char k, j;

    /* set bank size */
    switch(banksn)
    {
	case 0: { banks = 2; break; }
	case 1: { banks = 4; break; }
	case 2: { banks = 8; break; }
	case 3: { banks = 16; break; }
	case 4: { banks = 32; break; }
	case 5: { if(MBC <= 3U) banks = 63U; else banks = 64U; break; }
	case 6: { if(MBC <= 3U) banks = 125U; else banks = 128U; break; }
	case 7: { banks = 255; break; }
	default: break;
    }

    /* Send first bank */
    setFastAddress(0x0000U);
    for(i=0U; i<256U; i++)
    {
	for(z=0; z<64U; z++)
            INPacket[z] = readCartFast();
        USBGenericInHandle = USBGenWrite(1,(BYTE*)&INPacket,USBGEN_EP_SIZE);
        while(USBHandleBusy(USBGenericInHandle)){}
    }

    /* Send remaining banks */
    for(i=0; i<(banks-1); i++)
    {
        addr = 0x2000U;
        writeCart(i+1, addr);
        setFastAddress(0x4000U);
        for(z=0; z<256U; z++)
        {
            for(k=0; k<64U; k++)
                INPacket[k] = readCartFast();
            USBGenericInHandle = USBGenWrite(1,(BYTE*)&INPacket,USBGEN_EP_SIZE);
            while(USBHandleBusy(USBGenericInHandle)){}
        }
    }
}

/* Why does this not work... */
void send256Chunk(void)
{
    INPacket[0] = PORTB;
    INPacket[0] = PORTB;
    INPacket[0] = PORTB; LATD++;
    INPacket[1] = PORTB; LATD++;
    INPacket[2] = PORTB; LATD++;
    INPacket[3] = PORTB; LATD++;
    INPacket[4] = PORTB; LATD++;
    INPacket[5] = PORTB; LATD++;
    INPacket[6] = PORTB; LATD++;
    INPacket[7] = PORTB; LATD++;
    INPacket[8] = PORTB; LATD++;
    INPacket[9] = PORTB; LATD++;
    INPacket[10] = PORTB; LATD++;
    INPacket[11] = PORTB; LATD++;
    INPacket[12] = PORTB; LATD++;
    INPacket[13] = PORTB; LATD++;
    INPacket[14] = PORTB; LATD++;
    INPacket[15] = PORTB; LATD++;
    INPacket[16] = PORTB; LATD++;
    INPacket[17] = PORTB; LATD++;
    INPacket[18] = PORTB; LATD++;
    INPacket[19] = PORTB; LATD++;
    INPacket[20] = PORTB; LATD++;
    INPacket[21] = PORTB; LATD++;
    INPacket[22] = PORTB; LATD++;
    INPacket[23] = PORTB; LATD++;
    INPacket[24] = PORTB; LATD++;
    INPacket[25] = PORTB; LATD++;
    INPacket[26] = PORTB; LATD++;
    INPacket[27] = PORTB; LATD++;
    INPacket[28] = PORTB; LATD++;
    INPacket[29] = PORTB; LATD++;
    INPacket[30] = PORTB; LATD++;
    INPacket[31] = PORTB; LATD++;
    INPacket[32] = PORTB; LATD++;
    INPacket[33] = PORTB; LATD++;
    INPacket[34] = PORTB; LATD++;
    INPacket[35] = PORTB; LATD++;
    INPacket[36] = PORTB; LATD++;
    INPacket[37] = PORTB; LATD++;
    INPacket[38] = PORTB; LATD++;
    INPacket[39] = PORTB; LATD++;
    INPacket[40] = PORTB; LATD++;
    INPacket[41] = PORTB; LATD++;
    INPacket[42] = PORTB; LATD++;
    INPacket[43] = PORTB; LATD++;
    INPacket[44] = PORTB; LATD++;
    INPacket[45] = PORTB; LATD++;
    INPacket[46] = PORTB; LATD++;
    INPacket[47] = PORTB; LATD++;
    INPacket[48] = PORTB; LATD++;
    INPacket[49] = PORTB; LATD++;
    INPacket[50] = PORTB; LATD++;
    INPacket[51] = PORTB; LATD++;
    INPacket[52] = PORTB; LATD++;
    INPacket[53] = PORTB; LATD++;
    INPacket[54] = PORTB; LATD++;
    INPacket[55] = PORTB; LATD++;
    INPacket[56] = PORTB; LATD++;
    INPacket[57] = PORTB; LATD++;
    INPacket[58] = PORTB; LATD++;
    INPacket[59] = PORTB; LATD++;
    INPacket[60] = PORTB; LATD++;
    INPacket[61] = PORTB; LATD++;
    INPacket[62] = PORTB; LATD++;
    INPacket[63] = PORTB; LATD++;

    USBGenericInHandle = USBGenWrite(1,(BYTE*)&INPacket,USBGEN_EP_SIZE);
    while(USBHandleBusy(USBGenericInHandle)){}

    INPacket[0] = PORTB; LATD++;
    INPacket[1] = PORTB; LATD++;
    INPacket[2] = PORTB; LATD++;
    INPacket[3] = PORTB; LATD++;
    INPacket[4] = PORTB; LATD++;
    INPacket[5] = PORTB; LATD++;
    INPacket[6] = PORTB; LATD++;
    INPacket[7] = PORTB; LATD++;
    INPacket[8] = PORTB; LATD++;
    INPacket[9] = PORTB; LATD++;
    INPacket[10] = PORTB; LATD++;
    INPacket[11] = PORTB; LATD++;
    INPacket[12] = PORTB; LATD++;
    INPacket[13] = PORTB; LATD++;
    INPacket[14] = PORTB; LATD++;
    INPacket[15] = PORTB; LATD++;
    INPacket[16] = PORTB; LATD++;
    INPacket[17] = PORTB; LATD++;
    INPacket[18] = PORTB; LATD++;
    INPacket[19] = PORTB; LATD++;
    INPacket[20] = PORTB; LATD++;
    INPacket[21] = PORTB; LATD++;
    INPacket[22] = PORTB; LATD++;
    INPacket[23] = PORTB; LATD++;
    INPacket[24] = PORTB; LATD++;
    INPacket[25] = PORTB; LATD++;
    INPacket[26] = PORTB; LATD++;
    INPacket[27] = PORTB; LATD++;
    INPacket[28] = PORTB; LATD++;
    INPacket[29] = PORTB; LATD++;
    INPacket[30] = PORTB; LATD++;
    INPacket[31] = PORTB; LATD++;
    INPacket[32] = PORTB; LATD++;
    INPacket[33] = PORTB; LATD++;
    INPacket[34] = PORTB; LATD++;
    INPacket[35] = PORTB; LATD++;
    INPacket[36] = PORTB; LATD++;
    INPacket[37] = PORTB; LATD++;
    INPacket[38] = PORTB; LATD++;
    INPacket[39] = PORTB; LATD++;
    INPacket[40] = PORTB; LATD++;
    INPacket[41] = PORTB; LATD++;
    INPacket[42] = PORTB; LATD++;
    INPacket[43] = PORTB; LATD++;
    INPacket[44] = PORTB; LATD++;
    INPacket[45] = PORTB; LATD++;
    INPacket[46] = PORTB; LATD++;
    INPacket[47] = PORTB; LATD++;
    INPacket[48] = PORTB; LATD++;
    INPacket[49] = PORTB; LATD++;
    INPacket[50] = PORTB; LATD++;
    INPacket[51] = PORTB; LATD++;
    INPacket[52] = PORTB; LATD++;
    INPacket[53] = PORTB; LATD++;
    INPacket[54] = PORTB; LATD++;
    INPacket[55] = PORTB; LATD++;
    INPacket[56] = PORTB; LATD++;
    INPacket[57] = PORTB; LATD++;
    INPacket[58] = PORTB; LATD++;
    INPacket[59] = PORTB; LATD++;
    INPacket[60] = PORTB; LATD++;
    INPacket[61] = PORTB; LATD++;
    INPacket[62] = PORTB; LATD++;
    INPacket[63] = PORTB; LATD++;

    USBGenericInHandle = USBGenWrite(1,(BYTE*)&INPacket,USBGEN_EP_SIZE);
    while(USBHandleBusy(USBGenericInHandle)){}

    INPacket[0] = PORTB; LATD++;
    INPacket[1] = PORTB; LATD++;
    INPacket[2] = PORTB; LATD++;
    INPacket[3] = PORTB; LATD++;
    INPacket[4] = PORTB; LATD++;
    INPacket[5] = PORTB; LATD++;
    INPacket[6] = PORTB; LATD++;
    INPacket[7] = PORTB; LATD++;
    INPacket[8] = PORTB; LATD++;
    INPacket[9] = PORTB; LATD++;
    INPacket[10] = PORTB; LATD++;
    INPacket[11] = PORTB; LATD++;
    INPacket[12] = PORTB; LATD++;
    INPacket[13] = PORTB; LATD++;
    INPacket[14] = PORTB; LATD++;
    INPacket[15] = PORTB; LATD++;
    INPacket[16] = PORTB; LATD++;
    INPacket[17] = PORTB; LATD++;
    INPacket[18] = PORTB; LATD++;
    INPacket[19] = PORTB; LATD++;
    INPacket[20] = PORTB; LATD++;
    INPacket[21] = PORTB; LATD++;
    INPacket[22] = PORTB; LATD++;
    INPacket[23] = PORTB; LATD++;
    INPacket[24] = PORTB; LATD++;
    INPacket[25] = PORTB; LATD++;
    INPacket[26] = PORTB; LATD++;
    INPacket[27] = PORTB; LATD++;
    INPacket[28] = PORTB; LATD++;
    INPacket[29] = PORTB; LATD++;
    INPacket[30] = PORTB; LATD++;
    INPacket[31] = PORTB; LATD++;
    INPacket[32] = PORTB; LATD++;
    INPacket[33] = PORTB; LATD++;
    INPacket[34] = PORTB; LATD++;
    INPacket[35] = PORTB; LATD++;
    INPacket[36] = PORTB; LATD++;
    INPacket[37] = PORTB; LATD++;
    INPacket[38] = PORTB; LATD++;
    INPacket[39] = PORTB; LATD++;
    INPacket[40] = PORTB; LATD++;
    INPacket[41] = PORTB; LATD++;
    INPacket[42] = PORTB; LATD++;
    INPacket[43] = PORTB; LATD++;
    INPacket[44] = PORTB; LATD++;
    INPacket[45] = PORTB; LATD++;
    INPacket[46] = PORTB; LATD++;
    INPacket[47] = PORTB; LATD++;
    INPacket[48] = PORTB; LATD++;
    INPacket[49] = PORTB; LATD++;
    INPacket[50] = PORTB; LATD++;
    INPacket[51] = PORTB; LATD++;
    INPacket[52] = PORTB; LATD++;
    INPacket[53] = PORTB; LATD++;
    INPacket[54] = PORTB; LATD++;
    INPacket[55] = PORTB; LATD++;
    INPacket[56] = PORTB; LATD++;
    INPacket[57] = PORTB; LATD++;
    INPacket[58] = PORTB; LATD++;
    INPacket[59] = PORTB; LATD++;
    INPacket[60] = PORTB; LATD++;
    INPacket[61] = PORTB; LATD++;
    INPacket[62] = PORTB; LATD++;
    INPacket[63] = PORTB; LATD++;

    USBGenericInHandle = USBGenWrite(1,(BYTE*)&INPacket,USBGEN_EP_SIZE);
    while(USBHandleBusy(USBGenericInHandle)){}

    INPacket[0] = PORTB; LATD++;
    INPacket[1] = PORTB; LATD++;
    INPacket[2] = PORTB; LATD++;
    INPacket[3] = PORTB; LATD++;
    INPacket[4] = PORTB; LATD++;
    INPacket[5] = PORTB; LATD++;
    INPacket[6] = PORTB; LATD++;
    INPacket[7] = PORTB; LATD++;
    INPacket[8] = PORTB; LATD++;
    INPacket[9] = PORTB; LATD++;
    INPacket[10] = PORTB; LATD++;
    INPacket[11] = PORTB; LATD++;
    INPacket[12] = PORTB; LATD++;
    INPacket[13] = PORTB; LATD++;
    INPacket[14] = PORTB; LATD++;
    INPacket[15] = PORTB; LATD++;
    INPacket[16] = PORTB; LATD++;
    INPacket[17] = PORTB; LATD++;
    INPacket[18] = PORTB; LATD++;
    INPacket[19] = PORTB; LATD++;
    INPacket[20] = PORTB; LATD++;
    INPacket[21] = PORTB; LATD++;
    INPacket[22] = PORTB; LATD++;
    INPacket[23] = PORTB; LATD++;
    INPacket[24] = PORTB; LATD++;
    INPacket[25] = PORTB; LATD++;
    INPacket[26] = PORTB; LATD++;
    INPacket[27] = PORTB; LATD++;
    INPacket[28] = PORTB; LATD++;
    INPacket[29] = PORTB; LATD++;
    INPacket[30] = PORTB; LATD++;
    INPacket[31] = PORTB; LATD++;
    INPacket[32] = PORTB; LATD++;
    INPacket[33] = PORTB; LATD++;
    INPacket[34] = PORTB; LATD++;
    INPacket[35] = PORTB; LATD++;
    INPacket[36] = PORTB; LATD++;
    INPacket[37] = PORTB; LATD++;
    INPacket[38] = PORTB; LATD++;
    INPacket[39] = PORTB; LATD++;
    INPacket[40] = PORTB; LATD++;
    INPacket[41] = PORTB; LATD++;
    INPacket[42] = PORTB; LATD++;
    INPacket[43] = PORTB; LATD++;
    INPacket[44] = PORTB; LATD++;
    INPacket[45] = PORTB; LATD++;
    INPacket[46] = PORTB; LATD++;
    INPacket[47] = PORTB; LATD++;
    INPacket[48] = PORTB; LATD++;
    INPacket[49] = PORTB; LATD++;
    INPacket[50] = PORTB; LATD++;
    INPacket[51] = PORTB; LATD++;
    INPacket[52] = PORTB; LATD++;
    INPacket[53] = PORTB; LATD++;
    INPacket[54] = PORTB; LATD++;
    INPacket[55] = PORTB; LATD++;
    INPacket[56] = PORTB; LATD++;
    INPacket[57] = PORTB; LATD++;
    INPacket[58] = PORTB; LATD++;
    INPacket[59] = PORTB; LATD++;
    INPacket[60] = PORTB; LATD++;
    INPacket[61] = PORTB; LATD++;
    INPacket[62] = PORTB; LATD++;
    INPacket[63] = PORTB; LATD++;

    USBGenericInHandle = USBGenWrite(1,(BYTE*)&INPacket,USBGEN_EP_SIZE);
    while(USBHandleBusy(USBGenericInHandle)){}
}

void setFastAddress(unsigned short addr)
{
    address = addr;
    TRISD = 0x00;
    LATD = (unsigned char) address;
    temp = (unsigned char) (address >> 8);
    if ((temp & 0x01) == 0x01U) //A8
        LATC |= 0x01;
    else
        LATC &= ~(0x01);
    if ((temp & 0x02) == 0x02U) //A9
        LATE |= (0x04);
    else
        LATE &= ~(0x04);
    if ((temp & 0x04) == 0x04U) //A10
        LATE |= (0x02);
    else
        LATE &= ~(0x02);
    if ((temp & 0x08) == 0x08U) //A11
        LATE |= (0x01);
    else
        LATE &= ~(0x01);
    if ((temp & 0x10) == 0x10U) //A12
        LATA |= (0x20);
    else
        LATA &= ~(0x20);
    if ((temp & 0x20) == 0x20U) //A13
        LATA |= (0x10);
    else
        LATA &= ~(0x10);
    if ((temp & 0x40) == 0x40U) //A14
        LATA |= (0x08);
    else
        LATA &= ~(0x08);
    if ((temp & 0x80) == 0x80U) //A15
        LATA |= (0x04);
    else
        LATA &= ~(0x04);
}

unsigned char readCartFast(void)
{
    LATD = (unsigned char) address;
    if((unsigned char) address == 0)
    {
    temp = (unsigned char) (address >> 8);
    if ((temp & 0x01) == 0x01U) //A8
        LATC |= 0x01;
    else
        LATC &= ~(0x01);
    if ((temp & 0x02) == 0x02U) //A9
        LATE |= (0x04);
    else
        LATE &= ~(0x04);
    if ((temp & 0x04) == 0x04U) //A10
        LATE |= (0x02);
    else
        LATE &= ~(0x02);
    if ((temp & 0x08) == 0x08U) //A11
        LATE |= (0x01);
    else
        LATE &= ~(0x01);
    if ((temp & 0x10) == 0x10U) //A12
        LATA |= (0x20);
    else
        LATA &= ~(0x20);
    if ((temp & 0x20) == 0x20U) //A13
        LATA |= (0x10);
    else
        LATA &= ~(0x10);
    if ((temp & 0x40) == 0x40U) //A14
        LATA |= (0x08);
    else
        LATA &= ~(0x08);
    if ((temp & 0x80) == 0x80U) //A15
        LATA |= (0x04);
    else
        LATA &= ~(0x04);
    }
    address++;
    //_delay_us(1);
    return PORTB;
}

void writeCart(unsigned char data, unsigned short addr)
{
    unsigned short addressw = addr;
    TRISB = 0x00;
    LATB = data;
    LATC |= (0x02); //RD High
    LATD = (unsigned char) addressw;
    temp = (unsigned char) (addressw >> 8);
    if ((temp & 0x01) == 0x01U) //A8
        LATC |= 0x01;
    else
        LATC &= ~(0x01);
    if ((temp & 0x02) == 0x02U) //A9
        LATE |= (0x04);
    else
        LATE &= ~(0x04);
    if ((temp & 0x04) == 0x04U) //A10
        LATE |= (0x02);
    else
        LATE &= ~(0x02);
    if ((temp & 0x08) == 0x08U) //A11
        LATE |= (0x01);
    else
        LATE &= ~(0x01);
    if ((temp & 0x10) == 0x10U) //A12
        LATA |= (0x20);
    else
        LATA &= ~(0x20);
    if ((temp & 0x20) == 0x20U) //A13
        LATA |= (0x10);
    else
        LATA &= ~(0x10);
    if ((temp & 0x40) == 0x40U) //A14
        LATA |= (0x08);
    else
        LATA &= ~(0x08);
    if ((temp & 0x80) == 0x80U) //A15
        LATA |= (0x04);
    else
        LATA &= ~(0x04);

    Delay10KTCYx(1);
    LATC &= ~(0x04); //WR LOW
    Delay10KTCYx(1);
    LATC |= (0x04); //WR HIGH
    Delay10KTCYx(1);
    LATC &= ~(1 << 1); //RD LOW
    Delay10KTCYx(1);
    TRISB = 0xFF;
    //_delay_us(1);
}

/** EOF main.c ***************************************************************/