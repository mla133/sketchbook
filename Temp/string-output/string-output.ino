#include <z80retroshield.h>

// our program (string-output.z80)
// compiled by z80asm and transformed via:
// hd a.bin | cut -d " " -f2-20 | sed 's/\ /, 0x/g' for formatting help below
unsigned char memory[84] =
{
  0xcd, 0x47, 0x00, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x74, 0x65, 0x73,
  0x74, 0x21, 0x0a, 0x24, 0xcd, 0x47, 0x00, 0x41, 0x73, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x63, 0x61,
  0x6e, 0x20, 0x73, 0x65, 0x65, 0x20, 0x77, 0x65, 0x27, 0x72, 0x65, 0x20, 0x70, 0x72, 0x69, 0x6e,
  0x74, 0x69, 0x6e, 0x67, 0x20, 0x49, 0x4e, 0x4c, 0x49, 0x4e, 0x45, 0x20, 0x73, 0x74, 0x72, 0x69,
  0x6e, 0x67, 0x73, 0x21, 0x0a, 0x24, 0x76, 0xe1, 0x7e, 0x23, 0xe5, 0xfe, 0x24, 0xc8, 0xd3, 0x01,
  0xc3, 0x47, 0x00
};

int memory_len = sizeof(memory) / sizeof(memory[0]);

Z80RetroShield cpu;

// RAM I/O function handler
char memory_read(int address)
{
  return (memory[address]);
}

// I/O function handler
char io_read(int address)
{
    if (address == 1)
    {
        while(! Serial.available())
        {
          delay(1);
        }

        char r = Serial.read();
        return (r);        
    }
}

// I/O function handler
char io_write(int address, char byte)
{
    if (address == 1)
     {
        Serial.write(byte);
        return;
     }
     return 0;
}
void setup() {
  Serial.begin(115200);
  
  //Setup a RAM-read callback so program can be
  //fetched from RAM and executed
  cpu.set_memory_read(memory_read);
  //Setup callbacks for every time an IN/OUT instruction
  //is encountered
  cpu.set_io_read(io_read);
  cpu.set_io_write(io_write);

  //We're configured, let's let user know...
  Serial.println("Z80 configured; launching program.");

}

void loop() {
  // Step the CPU
  cpu.Tick();

}
