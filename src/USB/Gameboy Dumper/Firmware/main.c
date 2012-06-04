

/** INCLUDES *******************************************************/
#include "USB/usb.h"
#include "HardwareProfile.h"
#include "USB/usb_function_generic.h"
#include <delays.h>

/** CONFIGURATION **************************************************/
#define PICDEM_FS_USB
//#if defined(PICDEM_FS_USB)      // Configuration bits for PICDEM FS USB Demo Board (based on PIC18F4550)
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
//      #pragma config CCP2MX   = ON
        #pragma config STVREN   = ON
        #pragma config LVP      = OFF
//      #pragma config ICPRT    = OFF       // Dedicated In-Circuit Debug/Programming
        #pragma config XINST    = OFF       // Extended Instruction Set
        #pragma config CP0      = OFF
        #pragma config CP1      = OFF
//      #pragma config CP2      = OFF
//      #pragma config CP3      = OFF
        #pragma config CPB      = OFF
//      #pragma config CPD      = OFF
        #pragma config WRT0     = OFF
        #pragma config WRT1     = OFF
//      #pragma config WRT2     = OFF
//      #pragma config WRT3     = OFF
        #pragma config WRTB     = OFF       // Boot Block Write Protection
        #pragma config WRTC     = OFF
//      #pragma config WRTD     = OFF
        #pragma config EBTR0    = OFF
        #pragma config EBTR1    = OFF
//      #pragma config EBTR2    = OFF
//      #pragma config EBTR3    = OFF
        #pragma config EBTRB    = OFF


/** VARIABLES ******************************************************/

unsigned long address, length, tempFast;
unsigned short faddress;
unsigned char temp;

#pragma udata USB_VARIABLE = 0x47E
unsigned char OUTPacket[64];	//User application buffer for receiving and holding OUT packets sent from the host
unsigned char INPacket[64];		//User application buffer for sending IN packets to the host
unsigned short faddr;

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
	//On PIC18 devices, addresses 0x00, 0x08, and 0x18 are used for
	//the reset, high priority interrupt, and low priority interrupt
	//vectors.  However, the current Microchip USB bootloader 
	//examples are intended to occupy addresses 0x00-0x7FF or
	//0x00-0xFFF depending on which bootloader is used.  Therefore,
	//the bootloader code remaps these vectors to new locations
	//as indicated below.  This remapping is only necessary if you
	//wish to program the hex file generated from this project with
	//the USB bootloader.  If no bootloader is used, edit the
	//usb_config.h file and comment out the following defines:
	//#define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER
	//#define PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER
	
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018
	#elif defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)	
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x800
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818
	#else	
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x00
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x08
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x18
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
	//Note: If this project is built while one of the bootloaders has
	//been defined, but then the output hex file is not programmed with
	//the bootloader, addresses 0x08 and 0x18 would end up programmed with 0xFFFF.
	//As a result, if an actual interrupt was enabled and occured, the PC would jump
	//to 0x08 (or 0x18) and would begin executing "0xFFFF" (unprogrammed space).  This
	//executes as nop instructions, but the PC would eventually reach the REMAPPED_RESET_VECTOR_ADDRESS
	//(0x1000 or 0x800, depending upon bootloader), and would execute the "goto _startup".  This
	//would effective reset the application.
	
	//To fix this situation, we should always deliberately place a 
	//"goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS" at address 0x08, and a
	//"goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS" at address 0x18.  When the output
	//hex file of this project is programmed with the bootloader, these sections do not
	//get bootloaded (as they overlap the bootloader space).  If the output hex file is not
	//programmed using the bootloader, then the below goto instructions do get programmed,
	//and the hex file still works like normal.  The below section is only required to fix this
	//scenario.
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
		//Check which interrupt flag caused the interrupt.
		//Service the interrupt
		//Clear the interrupt flag
		//Etc.
        #if defined(USB_INTERRUPT)
	        USBDeviceTasks();
        #endif
	
	}	//This return will be a "retfie fast", since this is in a #pragma interrupt section 
	#pragma interruptlow YourLowPriorityISRCode
	void YourLowPriorityISRCode()
	{
		//Check which interrupt flag caused the interrupt.
		//Service the interrupt
		//Clear the interrupt flag
		//Etc.
	
	}	//This return will be a "retfie", since this is in a #pragma interruptlow section 
#endif




/** DECLARATIONS ***************************************************/
#if defined(__18CXX)
    #pragma code
#endif
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

void WriteRAM(void)
{
    
    unsigned char MBC = readCart(0x0147);
    unsigned char RAM_Size = readCart(0x149);
    unsigned char banks = 0;
    unsigned char i, z, k;
    unsigned short address = 0xA000U;

    switch(RAM_Size)
    {
        case 0x01: RAM_Size = 32; banks = 1; break;
        case 0x02: RAM_Size = 128; banks = 1; break;
        case 0x03: RAM_Size = 128; banks = 4; break;
    }

    writeCart(0x0AU, 0x0000);
    for(i=0; i<banks; i++)
    {
        writeCart(i, 0x4000U);
        address = 0xA000U;
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
    unsigned char RAM_Size = readCart(0x149);
    unsigned char banks = 0;
    unsigned char i, z, k;

    switch(RAM_Size)
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

#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{   
    InitializeSystem();

    #if defined(USB_INTERRUPT)
        USBDeviceAttach();
    #endif

    while(1)
    {
        #if defined(USB_POLLING)
		// Check bus status and service USB interrupts.
        USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
        				  // this function periodically.  This function will take care
        				  // of processing and responding to SETUP transactions 
        				  // (such as during the enumeration process when you first
        				  // plug in).  USB hosts require that USB devices should accept
        				  // and process SETUP packets in a timely fashion.  Therefore,
        				  // when using polling, this function should be called 
        				  // regularly (such as once every 1.8ms or faster** [see 
        				  // inline code comments in usb_device.c for explanation when
        				  // "or faster" applies])  In most cases, the USBDeviceTasks() 
        				  // function does not take very long to execute (ex: <100 
        				  // instruction cycles) before it returns.
        #endif
    				  

		// Application-specific tasks.
		// Application related code may be added here, or in the ProcessIO() function.
        ProcessIO();        
    }//end while
}//end main


static void InitializeSystem(void)
{
 
        ADCON1 |= 0x0F;                 // Default all pins to digital
 

    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;	// See HardwareProfile.h
    #endif
    
	USBGenericOutHandle = 0;	
	USBGenericInHandle = 0;		

    UserInit();			//Application related initialization.  See user.c

    USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
    					//variables to known states.
}//end InitializeSystem



void UserInit(void)
{
    gameboy_init();
    LATC |= (1<<6);
}

void ProcessIO(void)
{
    //Blink the LEDs according to the USB device status, but only do so if the PC application isn't connected and controlling the LEDs.
    //User Application USB tasks below.
    //Note: The user application should not begin attempting to read/write over the USB
    //until after the device has been fully enumerated.  After the device is fully
    //enumerated, the USBDeviceState will be set to "CONFIGURED_STATE".
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;
    
    //As the device completes the enumeration process, the USBCBInitEP() function will
    //get called.  In this function, we initialize the user application endpoints (in this
    //example code, the user application makes use of endpoint 1 IN and endpoint 1 OUT).
    //The USBGenRead() function call in the USBCBInitEP() function initializes endpoint 1 OUT
    //and "arms" it so that it can receive a packet of data from the host.  Once the endpoint
    //has been armed, the host can then send data to it (assuming some kind of application software
    //is running on the host, and the application software tries to send data to the USB device).
    
    //If the host sends a packet of data to the endpoint 1 OUT buffer, the hardware of the SIE will
    //automatically receive it and store the data at the memory location pointed to when we called
    //USBGenRead().  Additionally, the endpoint handle (in this case USBGenericOutHandle) will indicate
    //that the endpoint is no longer busy.  At this point, it is safe for this firmware to begin reading
    //from the endpoint buffer, and processing the data.  In this example, we have implemented a few very
    //simple commands.  For example, if the host sends a packet of data to the endpoint 1 OUT buffer, with the
    //first byte = 0x80, this is being used as a command to indicate that the firmware should "Toggle LED(s)".
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
        
        //Re-arm the OUT endpoint for the next packet:
	    //The USBGenRead() function call "arms" the endpoint (and makes it "busy").  If the endpoint is armed, the SIE will 
	    //automatically accept data from the host, if the host tries to send a packet of data to the endpoint.  Once a data 
	    //packet addressed to this endpoint is received from the host, the endpoint will no longer be busy, and the application
	    //can read the data which will be sitting in the buffer.
        USBGenericOutHandle = USBGenRead(USBGEN_EP_NUM,(BYTE*)&OUTPacket,USBGEN_EP_SIZE);
    }
}//end ProcessIO


void USBCBSuspend(void)
{

}


void USBCBWakeFromSuspend(void)
{

}

void USBCB_SOF_Handler(void)
{
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
}

void USBCBErrorHandler(void)
{

}


void USBCBCheckOtherReq(void)
{
}//end

void USBCBStdSetDscHandler(void)
{
    // Must claim session ownership if supporting this request
}//end


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
            //Add application specific callback task or callback function here if desired.
            //The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
            //FEATURE (endpoint halt) request on an application endpoint which was 
            //previously armed (UOWN was = 1).  Here would be a good place to:
            //1.  Determine which endpoint the transaction that just got terminated was 
            //      on, by checking the handle value in the *pdata.
            //2.  Re-arm the endpoint if desired (typically would be the case for OUT 
            //      endpoints).
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

    faddress = 0x0000U;
    TRISD = 0x00;
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

    /* Send first 16KB */
    
    faddr = 0x0000U;
    setFastAddress(faddr);
    for(k = 0; k<64U; k++)
    {
        send256Chunk();
        faddr += 256U;
        setFastAddress(faddr);
    } 
    
    /*
    setFastAddress(0x0000U);
    for(i=0U; i<256U; i++)
    {
	for(z=0; z<64U; z++)
            INPacket[z] = readCartFast();
        USBGenericInHandle = USBGenWrite(1,(BYTE*)&INPacket,USBGEN_EP_SIZE);
        while(USBHandleBusy(USBGenericInHandle)){}
    }*/

    /*
    for(k=0; k<(banks-1); k++)
    {
        faddr = 0x4000U;
        setFastAddress(faddr);
        writeCart(i+1, 0x2000U);
        for(j=0; j<64U; j++)
        {
            send256Chunk();
            faddr += 256U;
            setFastAddress(faddr);
        }
    }*/
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
    faddr = addr;
    TRISD = 0x00;
    LATD = (unsigned char) faddr;
    temp = (unsigned char) (faddr >> 8);
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
unsigned short readFastAddress(void)
{
    return faddr;
}

unsigned char readCartFast(void)
{
    LATD = (unsigned char) faddr;
    if((unsigned char) faddr == 0)
    {
    temp = (unsigned char) (faddr >> 8);
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
    faddr++;
    //_delay_us(1);
    return PORTB;
}

void writeCart(unsigned char data, unsigned short addr)
{
    unsigned short address = addr;
    TRISB = 0x00;
    LATB = data;
    LATC |= (0x02); //RD High
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
