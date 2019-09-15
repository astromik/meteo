

void SD_EEPROM(void)
  {
    Serial2.print("page 20");   // "page 20"   (tady se nesmi pouzivat EEPROM_text_read, protoze EEPROM je zatim prazdna)
    SerialFFF();
    Serial.println("COPY SD --->> EEPROM");       // "COPY SD --->> EEPROM"
    sd.begin(pin_SD_CS,SD_SCK_HZ(F_CPU/4));       //inicializace SD karty pro pripad, ze by byla predtim vytazena
    
    SD_EEPROM_faze();
    SD_EEPROM_zatmeni();
    SD_EEPROM_texty();
  }




// --------------- mesicni faze (jedna faze na 1 radku v soubou) ------------
               //"0000363589"       //   2016 Sep 09 11:49 (UTC)  - First Quarter
               //"2147857753"       //  Zatmeni Mesice 2016 Sep 16 19:05 (UTC)  - Full Moon
void SD_EEPROM_faze(void)
  {
    unsigned long SDCAS2016;
    unsigned int progress_max;
    byte progress_val;
    unsigned int pocitadlo_zaznamu;
    unsigned int SDadresa=0;
    
    open_OK = soubor.open("faze.txt", O_READ);
    if (open_OK)
      {
        progress_max = SDradka_10znaku();  // precte pocet fazi v souboru
        
        pocitadlo_zaznamu = 0;
        while (soubor.available())
          {
            pocitadlo_zaznamu ++;
            
            SDCAS2016 = SDradka_10znaku();
            Serial.println(SDCAS2016);

            //------------------------------------------------
            // rozlozeni na 4 bajty a zapsani do EEPROM
            EEPROM_write(SDadresa  , (SDCAS2016 >> 24) & 0xFF);
            EEPROM_write(SDadresa+1, (SDCAS2016 >> 16) & 0xFF);
            EEPROM_write(SDadresa+2, (SDCAS2016 >>  8) & 0xFF);
            EEPROM_write(SDadresa+3, SDCAS2016 & 0xFF);
  
            SDadresa = SDadresa + 4;
            //------------------------------------------------

            progress_val=map(pocitadlo_zaznamu,0,progress_max,0,100);
            Serial2.print("j1.val=");
            Serial2.print(progress_val);
            SerialFFF();
            delay(10);
            Serial2.print("ref j1");
            SerialFFF();

            
            iwdg_feed();

          }

        Serial.println("FAZE - HOTOVO");
        
      }
    else
      {
        Serial.println("FAZE - Chyba karty");  
      }
    soubor.close();

  }




// --------------- mesicni zatmeni (jedno zatmeni na 2 radky v soubou) ------------
            //"0000586113"     // 11.2.2017
            //"0000000201"     //  polostinove zatmeni Mesice (11.2.2017)

void SD_EEPROM_zatmeni(void)
  {
    unsigned long SDCAS2016;
    unsigned long SDdata;
    unsigned int progress_max;
    byte progress_val;
    unsigned int pocitadlo_zaznamu;
    unsigned int SDadresa=0;


    open_OK = soubor.open("zatmeni.txt", O_READ);
    if (open_OK)
      {
        progress_max = SDradka_10znaku();  // precte pocet zatmeni v souboru
        
        pocitadlo_zaznamu = 0;
        SDadresa = 20000;
        while (soubor.available())
          {
            pocitadlo_zaznamu ++;
            
            SDCAS2016 = SDradka_10znaku();
            Serial.print(SDCAS2016);
            Serial.print (" .... ");
            
            SDdata = SDradka_10znaku();
            Serial.println(SDdata);


            //------------------------------------------------
            // rozlozeni na 4 + 1 bajt a zapsani do EEPROM
            EEPROM_write(SDadresa  , (SDCAS2016 >> 24) & 0xFF);
            EEPROM_write(SDadresa+1, (SDCAS2016 >> 16) & 0xFF);
            EEPROM_write(SDadresa+2, (SDCAS2016 >>  8) & 0xFF);
            EEPROM_write(SDadresa+3, SDCAS2016 & 0xFF);
            EEPROM_write(SDadresa+4, SDdata & 0xFF);
  
            SDadresa = SDadresa + 5;
            //------------------------------------------------



            progress_val=map(pocitadlo_zaznamu,0,progress_max,0,100);
            Serial2.print("j2.val=");
            Serial2.print(progress_val);
            SerialFFF();
            delay(20);
            Serial2.print("ref j2");
            SerialFFF();

            
            iwdg_feed();

          }

        Serial.println("ZATMENI - HOTOVO");
        
      }
    else
      {
        Serial.println("ZATMENI - Chyba karty");  
      }
    soubor.close();
    
  }



// --------------- texty do programu  ------------
            //"Stari [d]: \0        "     //  napis[303]
            //"PeriApo [%]: \0      "     //  napis[304]
            
void SD_EEPROM_texty(void)
  {
    unsigned int progress_max;
    byte progress_val;
    unsigned int pocitadlo_zaznamu;
    String SDnapis;
    
    open_OK = soubor.open("texty.txt", O_READ);
    if (open_OK)
      {
        progress_max = SDradka_10znaku();  // precte pocet textovych radek v souboru
        
        pocitadlo_zaznamu = 0;
        while (soubor.available())
          {

            SDnapis = "                    ";
            SDnapis = SDradka_text();
            Serial.println(SDnapis);

           for (i = 0; i < 20 ; i++)
             {
               EEPROM_write(EEPROM_ADDR_texty + i + (20*pocitadlo_zaznamu), SDnapis[i]) ;
             }

            pocitadlo_zaznamu ++;
            
            progress_val=map(pocitadlo_zaznamu,0,progress_max,0,100);
            Serial2.print("j3.val=");
            Serial2.print(progress_val);
            SerialFFF();
            Serial2.print("ref j3");
            SerialFFF();

            
            iwdg_feed();

          }

        Serial.println("TEXTY - HOTOVO");
        
      }
    else
      {
        Serial.println("TEXTY - Chyba karty");  
      }
    soubor.close();
    
  }







// ----------------------------------------------------
//   cteni radky z prave otevreneho souboru a prevod prvnich 12 znaku na cislo.
//     Cislo samotne ma 10 znaku vcetne uvodnich nul a predpoklada se, ze je uzavrene v uvozovkach.
// priklad:   "0000000201"     //  polostinove zatmeni Mesice (20.12.2048)
//    vrati cislo 201


unsigned long SDradka_10znaku(void)
  {
    unsigned long SDcislo;
    char SDznak;
    
    SDznak = soubor.read();
    if (SDznak == '\"')      // prvni znak na radce musi byt uvozovky
      {
        SDznak  = soubor.read();
        SDcislo =           (SDznak - 48) * 1000000000UL;
        SDznak  = soubor.read();
        SDcislo = SDcislo + (SDznak - 48) * 100000000UL;
        SDznak  = soubor.read();
        SDcislo = SDcislo + (SDznak - 48) * 10000000UL;
        SDznak  = soubor.read();
        SDcislo = SDcislo + (SDznak - 48) * 1000000UL;
        SDznak  = soubor.read();
        SDcislo = SDcislo + (SDznak - 48) * 100000UL;
        SDznak  = soubor.read();
        SDcislo = SDcislo + (SDznak - 48) * 10000UL;
        SDznak  = soubor.read();
        SDcislo = SDcislo + (SDznak - 48) * 1000UL;
        SDznak  = soubor.read();
        SDcislo = SDcislo + (SDznak - 48) * 100UL;
        SDznak  = soubor.read();
        SDcislo = SDcislo + (SDznak - 48) * 10UL;
        SDznak  = soubor.read();
        SDcislo = SDcislo + (SDznak - 48);
      }
    else
      {
        Serial.println("ERR prvni uvozovky");
        return 0;
      }

    SDznak = soubor.read();         // tohle by mely byt ukoncovaci uvozovky
    if (SDznak != '\"')             // posledni znak na radce musi byt uvozovky
      {
        Serial.print("chyba: ");
        Serial.println(SDcislo);
        return 0;
      }

    while (SDznak != 10 and soubor.available())      // prolistovani az na konec radky (dokud nenalezne znak <LF>)
      {
        SDznak  = soubor.read();
      }

    return SDcislo;

  }







// ----------------------------------------------------
//   cteni radky z prave otevreneho souboru a prevod prvnich 12 znaku na cislo.
//     Cislo samotne ma 10 znaku vcetne uvodnich nul a predpoklada se, za je uzavrene v uvozovkach.
// priklad:   "0000000201"     //  polostinove zatmeni Mesice (20.12.2048)
//    vrati cislo 201


String SDradka_text(void)
  {
    String SDretezec = "                     ";
    char SDznak;
    byte SDpozice;
    
    SDznak = soubor.read();
    if (SDznak == '\"')      // prvni znak na radce musi byt uvozovky
      {
        for (SDpozice = 0; SDpozice < 20 ; SDpozice ++)
          {
            SDznak  = soubor.read();
            if (SDznak == '\\')
              {
                 SDznak  = soubor.read();
                 if (SDznak == '0')   SDznak = 0;
                 if (SDznak == '\"')  SDznak = '\"';
              }
            SDretezec[SDpozice] = SDznak;
          }
      }
    else
      {
        Serial.println("ERR prvni uvozovky");
        return  "ERR";
      }

    SDznak = soubor.read();         // tohle by mely byt ukoncovaci uvozovky
    if (SDznak != '\"')             // prvni znak na radce musi byt uvozovky
      {
        Serial.print("chyba: ");
        Serial.println(SDretezec);
        return "ERR";
      }

    while (SDznak != 10 and soubor.available())      // prolistovani az na konec radky (dokud nenalezne znak <LF>)
      {
        SDznak  = soubor.read();
      }

    return SDretezec;

  }




