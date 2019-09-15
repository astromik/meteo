
//-----------------------------------------------------
//  loguj(0);     // logovani resetu
//  loguj(1);     // logovani zmeny casu
//  loguj(2);     // logovani zmeny Aging registru
//  loguj(3);     // logovani zmeny korekce vnitrni teploty
//  loguj(4);     // logovani zmeny korekce venkovni teploty
//  loguj(5);     // logovani zmeny korekce tlaku
//  loguj(6);     // logovani zmeny korekce vlhkosti
//  loguj(7);     // logovani mazani rekordu
//  loguj(8);     // logovani mazani pameti
//  loguj(9);     // logovani zmeny 1. prepinaci urovne
// loguj(10);     // logovani zmeny 2. prepinaci urovne
// loguj(11);     // logovani mazani puldennich rekordu
// loguj(12);     // logovani zmeny SEC/SELC
// loguj(13);     // logovani chyby teplomeru
// loguj(14);     // logovani chyby vlhkomeru
// loguj(15);     // logovani zmeny korekce venkovni teploty
// loguj(16);     // logovani zmeny zemepisne sirky, delky a nadmorske vysky

void loguj(byte typ)
  {

    if (SD_aktivni == 1)
      {
        sd.begin(pin_SD_CS,SD_SCK_HZ(F_CPU/4));       //inicializace SD karty pro pripad, ze by byla predtim vytazena
        SdFile::dateTimeCallback(dateTime);
        open_OK = soubor.open("log.txt", O_WRITE | O_APPEND | O_CREAT);
    
        if (open_OK)
          {
            // nejdriv se precte z RTC aktualni datum a cas
            byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
            readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
            unsigned long log_dat = ((2000+year) * 10000) + (month * 100) + dayOfMonth;
            unsigned long log_cas = (hour * 10000) + (minute * 100) + second;
            soubor.print(log_dat);
            soubor.print(' ');
                                                             // dolneni potrebneho poctu nul pred udaj o case
            if (log_cas < 10) soubor.print("00000");              // 00:00:05   --->   000005   
            else if (log_cas < 100) soubor.print("0000");         // 00:00:25   --->   000025   
            else if (log_cas < 1000) soubor.print("000");         // 00:01:05   --->   000105   
            else if (log_cas < 10000) soubor.print("00");         // 00:42:05   --->   004205   
            else if (log_cas < 100000) soubor.print('0');         // 07:30:05   --->   073005
     
            soubor.print(log_cas);
            soubor.print(" -> ");
            
            
            // zapis konkretniho udaje podle typu logu
            switch (typ)
              {
                case 0:
                  soubor.println("RESET");
                  break;
        
                case 1:
                  soubor.println(EEPROM_text_read(62));          //  "Zmena datumu a casu"
                  break;
        
                 case 2:
                   soubor.print(EEPROM_text_read(63));           // "RTC Aging: "
                   soubor.println(log_pomprom);
                   break;
        
                 case 3:
                   soubor.print(EEPROM_text_read(32));                      // "Korekce tepl. OUT:  "
                   soubor.println(log_pomprom);
                   break;
        
        
                 case 4:
                   soubor.print(EEPROM_text_read(29));                    // "Korekce tepl. IN :  "
                   soubor.println(log_pomprom);
                   break;
        
                 case 5:
                   soubor.print(EEPROM_text_read(30));                    // "Korekce tlaku    :  "
                   soubor.println(log_pomprom);
                   break;
        
                 case 6:
                   soubor.print(EEPROM_text_read(31));                   // "Korekce vlhkosti :  "
                   soubor.println(log_pomprom);
                   break;
        
                 case 7:
                   soubor.print(EEPROM_text_read(20));               // "  Minimum a maximum "
                   soubor.println(EEPROM_text_read(21));             // "teplot vynulovano.  "
                   break;
        
                 case 8:
                   soubor.println(EEPROM_text_read(64));             // "Mazani vsech dat z pameti"
                   break;
        
                 case 9:
                   soubor.print(EEPROM_text_read(33));               // "Prepinaci uroven 1: "
                   soubor.println(log_pomprom);
                   break;
        
                 case 10:
                   soubor.print(EEPROM_text_read(34));               // "Prepinaci uroven 2: "
                   soubor.println(log_pomprom);
                   break;
        
                 case 11:
                   soubor.println(EEPROM_text_read(65));             // "Mazani puldennich rekordu:");
                   soubor.print(zobraz_cislo(tep_min1));
                   soubor.print('(');
                   soubor.print(int_to_hodmin(RAM1_read_int(EERAM_ADDR_hhmm_min1)));
                   soubor.println(')');
                   soubor.print(zobraz_cislo(tep_max1));
                   soubor.print('(');
                   soubor.print(int_to_hodmin(RAM1_read_int(EERAM_ADDR_hhmm_max1)));
                   soubor.println(')');
                   break;
        
                 case 12:
                   soubor.print(EEPROM_text_read(66));                //  "Zmena na : "
                   if (log_pomprom == 1) soubor.println("SEC");
                   else                  soubor.println("SELC");
                   break;
    
                 case 13:
                   soubor.print(EEPROM_text_read(277));               // "Teplota ERR : "
                   soubor.println (teplota_ds);
                   break;
    
                 case 14:
                   soubor.println(EEPROM_text_read(294));             // "Chyba vlhkomeru: "
                   soubor.print(EEPROM_text_read(295));               // "minula vlhkost: ");
                   soubor.println (log_pomprom);
                   break;

                 case 15:
                   soubor.print("Korekce tepl. OUT2: ");                    // "Korekce tepl. IN :  "
                   soubor.println(log_pomprom);
                   break;

                 case 16:
                   soubor.println("Zmena GEOsouradnic: ");
                   soubor.print("LAT: ");
                   soubor.println(RAM1_read_int(EERAM_ADDR_GeoLat));
                   soubor.print("LON: ");
                   soubor.println(RAM1_read_int(EERAM_ADDR_GeoLon));
                   soubor.print("ALT: ");
                   soubor.println(RAM1_read_int(EERAM_ADDR_GeoAlt));
                   break;
                   
              }
        
            ikona5(false);                // ikona vykricniku po uspesnem zapisu zmizi
          }
        else
          {
            ikona5(true);                 // pri chybe karty se zobrazi ikona vykricniku    
          }
    
        soubor.close();                   //  zavrit soubor log.txt
      } 

  }

