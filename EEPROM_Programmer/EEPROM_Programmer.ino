#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

const byte numChars = 64;
char mode = 'x';
int size = 0;
int os = 0;
int debug = 0;
char receivedChars[numChars] = {0xaa, 0xaa, 0xaa, 0xaa};  // an array to store the received data


boolean newData = false;


// 4-bit hex decoder for common anode 7-segment display
byte data[] = { 0x81, 0xcf, 0x92, 0x86, 0xcc, 0xa4, 0xa0, 0x8f, 0x80, 0x84, 0x88, 0xe0, 0xb1, 0xc2, 0xb0, 0xb8 };

// 4-bit hex decoder for common cathode 7-segment display
// byte data[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b, 0x77, 0x1f, 0x4e, 0x3d, 0x4f, 0x47 };

void enableWrite() {
  digitalWrite(WRITE_EN, LOW);
}
void disableWrite() {
  digitalWrite(WRITE_EN, HIGH);
}



/*
 * Output the address bits and outputEnable signal using shift registers. Speedup code from TommyPROM https://github.com/TomNisbet/TommyPROM/tree/master/schematics
 */
void setAddress(int addr, bool outputEnable) {
  // Set the highest bit as the output enable bit (active low)
  if (outputEnable) {
    addr &= ~0x8000;
  } else {
    addr |= 0x8000;
  }
  byte dataMask = 0x04;
  byte clkMask = 0x08;
  byte latchMask = 0x10;

  // Make sure the clock is low to start.
  PORTD &= ~clkMask;

  // Shift 16 bits in, starting with the MSB.
  for (uint16_t ix = 0; (ix < 16); ix++) {
    // Set the data bit
    if (addr & 0x8000) {
      PORTD |= dataMask;
    } else {
      PORTD &= ~dataMask;
    }

    // Toggle the clock high then low
    PORTD |= clkMask;
    delayMicroseconds(3);
    PORTD &= ~clkMask;
    addr <<= 1;
  }

  // Latch the shift register contents into the output register.
  PORTD &= ~latchMask;
  delayMicroseconds(1);
  PORTD |= latchMask;
  delayMicroseconds(1);
  PORTD &= ~latchMask;
}


/*
 * Read a byte from the EEPROM at the specified address. Speedup code from TommyPROM https://github.com/TomNisbet/TommyPROM/tree/master/schematics
 */
byte readEEPROM(int address) {
  setDataBusMode(INPUT);
  setAddress(address, /*outputEnable*/ true);
  return readDataBus();
}


/*
 * Write a byte to the EEPROM at the specified address. Speedup code from TommyPROM https://github.com/TomNisbet/TommyPROM/tree/master/schematics
 */
void writeEEPROM(int address, byte data) {
  setAddress(address, /*outputEnable*/ false);
  setDataBusMode(OUTPUT);
  writeDataBus(data);
  delayMicroseconds(1);
  enableWrite();
  delayMicroseconds(1);
  disableWrite();
  delay(10);
}


/*
 * Read the contents of the EEPROM and print them to the serial monitor.
 */
void printContents() {
  for (int base = 0; base <= (size * 128 - 1); base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = readEEPROM(base + offset);
    }

    char buf[80];
    sprintf(buf, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
  Serial.println("DONE!");
}

/*
 * Erase the contents of the EEPROM.
 */
void eraseContents() {
  // Erase entire EEPROM
  for (int address = 0; address <= (size * 128 - 1); address += 1) {
    writeEEPROM(address, 0xff);

    if (address % 64 == 0) {
      Serial.println(address);
    }
  }
  Serial.println("DONE!");
}

/*
 * Write the contents of the EEPROM.
 */
void writeContents() {
  // Program data bytes
  for (int address = 0; address < numChars; address++) {
    writeEEPROM(address + os, receivedChars[address]);
    //Serial.println(address + os);
  }
  newData = false;
}


// new receive numChars bytes per packet
void recvNumChars() {
  static byte ndx = 0;
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();
    if (ndx < numChars) {
      receivedChars[ndx] = rc;
      ndx++;
    } else {
      ndx = 0;
      newData = true;
    }
  }
}

// Original receive until newline
void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0';  // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}


void showNewData() {
  if (newData == true && mode != 'x') {
    Serial.print("Data in: ");
    Serial.println(receivedChars);
    newData = false;
  }
}

// Write the special six-byte code to turn off Software Data Protection.
void disableSoftwareWriteProtect() {
  disableWrite();
  setDataBusMode(OUTPUT);

  setByte(0xaa, 0x5555);
  setByte(0x55, 0x2aaa);
  setByte(0x80, 0x5555);
  setByte(0xaa, 0x5555);
  setByte(0x55, 0x2aaa);
  setByte(0x20, 0x5555);

  setDataBusMode(INPUT);
  delay(10);
}

// Write the special three-byte code to turn on Software Data Protection.
void enableSoftwareWriteProtect() {
  disableWrite();
  setDataBusMode(OUTPUT);

  setByte(0xaa, 0x5555);
  setByte(0x55, 0x2aaa);
  setByte(0xa0, 0x5555);

  setDataBusMode(INPUT);
  delay(10);
}

// Set the I/O state of the data bus.
// The 8 bits data bus are is on pins D5..D12.
void setDataBusMode(uint8_t mode) {
  // On the Uno and Nano, D5..D12 maps to the upper 3 bits of port D and the
  // lower 5 bits of port B.
  if (mode == OUTPUT) {
    DDRB |= 0x1f;
    DDRD |= 0xe0;
  } else {
    DDRB &= 0xe0;
    DDRD &= 0x1f;
  }
}

// Read a byte from the data bus.  The caller must set the bus to input_mode
// before calling this or no useful data will be returned.
byte readDataBus() {
  return (PINB << 3) | (PIND >> 5);
}

// Write a byte to the data bus.  The caller must set the bus to output_mode
// before calling this or no data will be written.
void writeDataBus(byte data) {
  PORTB = (PORTB & 0xe0) | (data >> 3);
  PORTD = (PORTD & 0x1f) | (data << 5);
}

// Set an address and data value and toggle the write control.  This is used
// to write control sequences, like the software write protect.  This is not a
// complete byte write function because it does not set the chip enable or the
// mode of the data bus.
void setByte(byte value, word address) {
  setAddress(address, false);
  writeDataBus(value);

  delayMicroseconds(1);
  enableWrite();
  delayMicroseconds(1);
  disableWrite();
}





void setup() {
  // put your setup code here, to run once:
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  disableWrite();
  pinMode(WRITE_EN, OUTPUT);
  Serial.begin(57600);

  // clear shift registers
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, 0);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, 0);

  while (!Serial)
    ;
  delay(500);
  Serial.println("<TEEPROM flasher ready>");
}



void loop() {
  while (mode == 'x' & newData == false) {
    recvWithEndMarker();
  }
  if (mode == 'x' & newData == true) {
    mode = receivedChars[0];
    memmove(receivedChars, receivedChars + 1, strlen(receivedChars));
    size = atoi(receivedChars);
    Serial.print("Mode: ");
    Serial.print(mode);
    Serial.print("\t\tEEPROM size: ");
    Serial.print(size);
    Serial.println("kb");
    newData = false;
  }
  // read EEPROM
  if (mode == 'r') {
    // Read and print out the contents of the EERPROM
    Serial.println("Reading EEPROM");
    printContents();
    mode = 'x';
  }
  // erase EEPROM
  if (mode == 'e') {
    // Read and print out the contents of the EERPROM
    Serial.println("Erasing EEPROM");
    eraseContents();
    mode = 'x';
  }
  // write EEPROM
  if (mode == 'w') {
    // Read and print out the contents of the EERPROM
    if (newData == true) {
      writeContents();
      os += (numChars);
    }
    if (os >= (size * 128 - 1)) {
      Serial.println("DONE!");
      mode = 'x';
      os = 0;
    }
  }
  recvNumChars();
}
