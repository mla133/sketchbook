#include <z80retroshield.h>

//
// Our program, as hex.
//
// The source code is located in `uc.z80`.
//
unsigned char memory[32] =
{
    0x3e, 0x3e, 0xd3, 0x01, 0xdb, 0x01, 0xfe, 0xff, 0xca, 0x1b, 0x00, 0xfe,
    0x61, 0xda, 0x17, 0x00, 0xfe, 0x7a, 0xd2, 0x17, 0x00, 0xd6, 0x20, 0xd3,
    0x01, 0x18, 0xe9, 0x76
};
int memory_len = sizeof(memory) / sizeof(memory[0]);


//
// Our helper
//
Z80RetroShield cpu;


//
// RAM I/O function handler.
//
char memory_read(int address)
{
    return (memory[address]);
}


//
// I/O function handler.
//
void io_write(int address, char byte)
{
    if (address == 1)
        Serial.write(byte);
}


//
// Setup routine: Called once.
//
void setup()
{
    Serial.begin(115200);


    //
    // Setup callbacks.
    //
    // We must setup a memory-read callback, otherwise the program
    // won't be fetched and executed.
    //
    cpu.set_memory_read(memory_read);

    //
    // Then we setup a callback to be executed every time an "out (x),y"
    // instruction is encountered.
    //
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
    // We stop running after a specific number of cycles
    //
    // Have we reached that point?
    //
    static bool done = false;

    //
    // Count the number of times we've pumped the Z80.
    //
    static int cycles = 0;

    //
    // Are we done?  If so return.
    //
    if (done)
        return;

    //
    // We stop running after running a specific number of cycles
    //
    if (cycles > 4096)
    {
        Serial.print("Z80 processor stopped ");
        Serial.print(cycles);
        Serial.println(" cycles executed.");
        done = true;
        return;
    }

    //
    // Step the CPU.
    //
    cpu.Tick();

    //
    // We've run a tick.
    //
    cycles++;
}
