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

#pragma udata USB_VARIABLE = 0x4FE
unsigned short address;
