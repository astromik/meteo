
//-----------------------------------------------------------------------------
// zapis a cteni dvojbajtoveho cisla 'do' a 'z' externi RAM
// prvni bajt je MSB, druhy bajt je LSB
void RAM1_write_int(unsigned int polozka, unsigned int RAM_data)
  {
    RAM1_write(polozka * 2   , RAM_data >> 8);
    RAM1_write(polozka * 2 + 1, RAM_data % 256);
  }

unsigned int RAM1_read_int(unsigned int polozka)
  {
    return (256 * RAM1_read(polozka * 2))  +   RAM1_read(polozka * 2 + 1);
  }


//-----------------------------------------------------------------------------
// zapis a cteni dvojbajtoveho cisla 'do' a 'z' externi RAM
// prvni bajt je MSB, druhy bajt je LSB
void RAM2_write_int(unsigned int polozka, unsigned int RAM_data)
  {
    RAM2_write(polozka * 2   , RAM_data >> 8);
    RAM2_write(polozka * 2 + 1, RAM_data % 256);
  }

unsigned int RAM2_read_int(unsigned int polozka)
  {
    return (256 * RAM2_read(polozka * 2))  +   RAM2_read(polozka * 2 + 1);
  }





//----------------------------------------------------------------------------
// nastaveni obou RAM pro automaticke ukladani celeho obsahu do EEPROM pri vypadku napajeni
byte RAM_setup()
  {
    delay(10);

    delayMicroseconds(5);
    Wire.beginTransmission(I2C_ADDR_RAM1_CTRL);    // zacatek komunikace s RAM
    Wire.write(0);                                // adresa status registru
    delayMicroseconds(3);
    Wire.write(2);                                // hodnota k zapsani do status registru
    Wire.endTransmission();                       // konec komunikace
    delayMicroseconds(5);
    Wire.requestFrom(I2C_ADDR_RAM1_CTRL, 1);       // precist zpatky status registr
    Wire.read();

    delayMicroseconds(5);
    Wire.beginTransmission(I2C_ADDR_RAM2_CTRL);    // zacatek komunikace s RAM
    Wire.write(0);                                 // adresa status registru
    delayMicroseconds(3);
    Wire.write(2);                                 // hodnota k zapsani do status registru
    Wire.endTransmission();                        // konec komunikace
    Wire.requestFrom(I2C_ADDR_RAM2_CTRL, 1);       // precist zpatky status registr
    return Wire.read();

  }
  


//--------------------------------------------------------------------------------
// zapis a cteni jednobajtovych hodnot 'do' a 'z' externi RAM
void RAM1_write(unsigned int addr, byte data)
  {
    delayMicroseconds(8);
    Wire.beginTransmission(I2C_ADDR_RAM1_RW);    // zacatek komunikace s RAM
    Wire.write(byte(addr >> 8));                      // MSB z adresy
    Wire.write(byte(addr % 256));                     // LSB z adresy
    delayMicroseconds(3);
    Wire.write(data);                           // ulozeni hodnoty
    Wire.endTransmission();                     // konec komunikace
  }
  

byte RAM1_read(unsigned int addr)
  {
    delayMicroseconds(8);
    Wire.beginTransmission(I2C_ADDR_RAM1_RW);    // zacatek komunikace s RAM
    Wire.write(byte(addr >> 8));                      // MSB z adresy
    Wire.write(byte(addr % 256));                     // LSB z adresy
    Wire.endTransmission();
    delayMicroseconds(8);
    Wire.requestFrom(I2C_ADDR_RAM1_RW, 1);
    return Wire.read();
  }


//--------------------------------------------------------------------------------
// zapis a cteni jednobajtovych hodnot 'do' a 'z' externi RAM
void RAM2_write(unsigned int addr, byte data)
  {
    delayMicroseconds(8);
    Wire.beginTransmission(I2C_ADDR_RAM2_RW);    // zacatek komunikace s RAM
    Wire.write(byte(addr >> 8));                      // MSB z adresy
    Wire.write(byte(addr % 256));                     // LSB z adresy
    delayMicroseconds(3);
    Wire.write(data);                           // ulozeni hodnoty
    Wire.endTransmission();                     // konec komunikace
  }
  

byte RAM2_read(unsigned int addr)
  {
    delayMicroseconds(8);
    Wire.beginTransmission(I2C_ADDR_RAM2_RW);    // zacatek komunikace s RAM
    Wire.write(byte(addr >> 8));                      // MSB z adresy
    Wire.write(byte(addr % 256));                     // LSB z adresy
    Wire.endTransmission();
    delayMicroseconds(8);
    Wire.requestFrom(I2C_ADDR_RAM2_RW, 1);
    return Wire.read();
  }




// podprogramy pro praci s externi EEPROM, ve ktere je ulozena databaze fazi Mesice
uint32_t EEPROM_read_long(unsigned int polozka)
  {
    byte bajt3 = EEPROM_read( polozka * 4);         // MSB
    byte bajt2 = EEPROM_read((polozka * 4) + 1);
    byte bajt1 = EEPROM_read((polozka * 4) + 2);
    byte bajt0 = EEPROM_read((polozka * 4) + 3);    // LSB
  
    return   (uint32_t)bajt3 << 24  | (uint32_t)bajt2 << 16 | (uint32_t)bajt1 << 8 | (uint32_t)bajt0;
  }


// podprogramy pro praci s externi EEPROM, ve ktere je ulozena databaze zatmeni (od adresy 20000)
uint32_t EEPROM_read_longZ(unsigned int adresa)
  {
    byte bajt3 = EEPROM_read(adresa);               // MSB
    byte bajt2 = EEPROM_read(adresa + 1);
    byte bajt1 = EEPROM_read(adresa + 2);
    byte bajt0 = EEPROM_read(adresa + 3);           // LSB
  
    return   (uint32_t)bajt3 << 24  | (uint32_t)bajt2 << 16 | (uint32_t)bajt1 << 8 | (uint32_t)bajt0;
  }


byte EEPROM_read(unsigned int addr)
  {
    delayMicroseconds(7);
    Wire.beginTransmission(I2C_ADDR_EEPROM_RW);           // zacatek komunikace s EEPROM
    Wire.write(byte(addr >> 8));                          // MSB z adresy
    Wire.write(byte(addr % 256));                         // LSB z adresy
    Wire.endTransmission();
    delayMicroseconds(2);
    Wire.requestFrom(I2C_ADDR_EEPROM_RW, 1);
    return Wire.read();
  }




//----------------------------------------------------------
//  zapis hodnot do externi EEPROM
void EEPROM_write(unsigned int adresa, byte data)
  {
    delay(5);
    Wire.beginTransmission(I2C_ADDR_EEPROM_RW); // zacatek komunikace s EEPROM
    Wire.write(byte(adresa >> 8));              // MSB z adresy
    Wire.write(byte(adresa % 256));             // LSB z adresy
    delay(1);
    Wire.write(data);                           // ulozeni hodnoty
    Wire.endTransmission();                     // konec komunikace
    delay(5);
  }




void EEPROM_write_int(unsigned int adresa, unsigned int data)
  {
    EEPROM_write(adresa   , data >> 8);
    EEPROM_write(adresa + 1, data % 256);
  }

unsigned int EEPROM_read_int(unsigned int adresa)
  {
    return (256 * EEPROM_read(adresa))  +   EEPROM_read(adresa + 1);
  }
  





//-----------------------------------------------------
// cteni textu z externi EEPROM
// vysledkem je retezec dlouhy 20 znaku, nebo v pripade, ze retezec obsahuje kod '\0', tak muze byt i kratsi
String EEPROM_text_read(unsigned int index)
  {
    String navrat = "                    ";              // delka je 20 mezer a koncovy znak \0
    byte delka=0;
    for (unsigned int EEPROM_ADDR = (index * 20) + EEPROM_ADDR_texty ; EEPROM_ADDR < (index * 20) + EEPROM_ADDR_texty + 20 ; EEPROM_ADDR ++) // kazdy text v EEPROM je dlouhy 20 znaku
      {
        byte znakEEPROM = EEPROM_read(EEPROM_ADDR);
        navrat[EEPROM_ADDR - ((index * 20) + EEPROM_ADDR_texty)] = znakEEPROM;
        if (znakEEPROM == '\0')    break;
        delka++;
      }
    navrat.remove(delka);
    return navrat;
  }



