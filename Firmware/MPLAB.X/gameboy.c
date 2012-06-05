void WriteRAM(void)
{

    unsigned char MBC = readCart(0x0147);
    unsigned char RAM_Size = readCart(0x149);
    unsigned char banks = 0;
    unsigned char i, z, k;
    address = 0xA000U;

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
