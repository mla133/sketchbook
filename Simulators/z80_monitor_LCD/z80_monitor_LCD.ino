
#include <z80retroshield.h>

//
// Our program.
//
unsigned char memory[1024] =
{
    0x31, 0xfb, 0x00, 0x3e, 0x3e, 0xd3, 0x01, 0x21, 0xfb, 0x00, 0x36, 0x0a, 0xdb, 0x01, 0x77, 0xfe, 
    0x0a, 0x28, 0x03, 0x23, 0x18, 0xf4, 0xcd, 0xb3, 0x00, 0x21, 0xfb, 0x00, 0x7e, 0x23, 0xfe, 0x43, 
    0x28, 0x18, 0xfe, 0x44, 0x28, 0x1d, 0xfe, 0x49, 0x28, 0x41, 0x3e, 0x45, 0xd3, 0x01, 0x3e, 0x52,
    0xd3, 0x01, 0xd3, 0x01, 0x3e, 0x0a, 0xd3, 0x01, 0x18, 0xc9, 0xcd, 0x91, 0x00, 0x21, 0x03, 0x00, 
    0xe5, 0xc5, 0xc9, 0x7e, 0xfe, 0x0a, 0x28, 0x07, 0xcd, 0x91, 0x00, 0xed, 0x43, 0xe5, 0x00, 0x2a, 
    0xe5, 0x00, 0xcd, 0xc8, 0x00, 0x06, 0x10, 0x3e, 0x20, 0xd3, 0x01, 0x4e, 0xcd, 0xd1, 0x00, 0x23,
    0x10, 0xf5, 0x3e, 0x0a, 0xd3, 0x01, 0x22, 0xe5, 0x00, 0x18, 0x98, 0x7e, 0xfe, 0x20, 0x28, 0x07, 
    0xcd, 0x91, 0x00, 0xed, 0x43, 0xe7, 0x00, 0x7e, 0x23, 0xfe, 0x20, 0x28, 0xfa, 0xfe, 0x0a, 0x28, 
    0xe8, 0x2b, 0xcd, 0x9a, 0x00, 0xed, 0x4b, 0xe7, 0x00, 0x02, 0x03, 0xed, 0x43, 0xe7, 0x00, 0x18,
    0xe6, 0xcd, 0x9a, 0x00, 0x47, 0xcd, 0x9a, 0x00, 0x4f, 0xc9, 0x7e, 0xcd, 0xab, 0x00, 0x87, 0x87, 
    0x87, 0x87, 0x57, 0x23, 0x7e, 0xcd, 0xab, 0x00, 0xb2, 0x23, 0xc9, 0xd6, 0x30, 0xfe, 0x0a, 0xd8, 
    0xd6, 0x07, 0xc9, 0x21, 0xfa, 0x00, 0x23, 0x7e, 0xfe, 0x0a, 0xc8, 0xfe, 0x61, 0x38, 0xf7, 0xfe,  
    0x7a, 0x30, 0xf3, 0xd6, 0x20, 0x77, 0x18, 0xee, 0x4c, 0xcd, 0xd1, 0x00, 0x4d, 0xcd, 0xd1, 0x00, 
    0xc9, 0x79, 0x1f, 0x1f, 0x1f, 0x1f, 0xcd, 0xda, 0x00, 0x79, 0xe6, 0x0f, 0xc6, 0x90, 0x27, 0xce, 
    0x40, 0x27, 0xd3, 0x01, 0xc9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3e, 0x48, 0xd3, 0x01, 0x3e, 0x45, 0xd3, 0x01, 0x3e, 0x4c, 0xd3, 0x01, 0xd3, 0x01, 0x3e, 0x4f,
    0xd3, 0x01, 0xc3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    
};
int memory_len = sizeof(memory) / sizeof(memory[0]);

//
// Scratch area for format-strings.
//
char tmp[64] = { '\0' };

//
// Our helper
//
Z80RetroShield cpu;


//
// RAM I/O function handler.
//
char memory_read(int address)
{
    if (address >= 0 && address < memory_len)
        return (memory[address]) ;
    else
    {
        sprintf(tmp, "Attempted read out of bounds memory %04x", address);
        Serial.println(tmp);
        return 0;
    }
}

//
// RAM I/O function handler.
//
void memory_write(int address, byte value)
{
    if (address >= 0 && address < memory_len)
        memory[address] = value;
    else
    {
        sprintf(tmp, "Attempted write to %04x - %02d", address, value);
        Serial.println(tmp);
    }
}


//
// I/O function handler.
//
char io_read(int address)
{
    if (address == 1)
    {
        char r;

        //
        // Ensure there is at least one character available.
        //
        while (! Serial.available())
        {
            delay(1);
        }

        //
        // Read and return the character.
        //
        r = Serial.read();
        return (r);
    }
    else
    {
        sprintf(tmp, "Attempted to read I/O port %d", address);
        Serial.println(tmp);
    }

    return 0;
}

//
// I/O function handler.
//
void io_write(int address, char byte)
{
    if (address == 1)
        Serial.write(byte);
    else
    {
        sprintf(tmp, "Attempted write to I/O port %02X with value %02X",
                address, byte);
        Serial.println(tmp);

    }
}


//
// Setup routine: Called once.
//
void setup()
{
    Serial.begin(115200);


    //
    // Setup callbacks for memory read/writes.
    //
    cpu.set_ram_read(memory_read);
    cpu.set_ram_write(memory_write);

    //
    // Now setup a callback to be executed every time an IN/OUT
    // instruction is encountered.
    //
    cpu.set_io_read(io_read);
    cpu.set_io_write(io_write);

    //
    // Configured.
    //
    Serial.println("Z80 configured; launching program.");
}


//
// Loop function: Called forever.
//
void loop()
{
    //
    // Step the CPU.
    //
    cpu.Tick();
}