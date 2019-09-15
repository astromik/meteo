// =======================  obsluha seriove komunikace  (servisni komunikace pres BT, nebo pres USB prevodnik)  ==============================
void komunikace(void)
  {
    char znak = Serial.read();        // zjisti, co je prvni znak v prijate zprave
    if (znak == '#')                  // kdyz je prvni znak '#', bude se neco nastavovat ( '#' jsou bezne dostupne prikazy)
      {
        pauza100();
        if (Serial.available())
          {
            znak = Serial.read();          // znak, ktery nasleduje po '#' urcuje konkretni funkci

            if (znak == '?')               // '#?' vypis dostupnych prikazu
              {
                for (i = 67; i < 114 ; i=i+2)
                  {
                    Serial.print(EEPROM_text_read(i));  
                    Serial.println(EEPROM_text_read(i+1));  
                  }

              }
  
            if (znak == 'T')               // '#T' = bude se nastavovat cas
              {
                RTC_default();
                nastav_RTC();
                najdi_VZM();
                loguj(1);     // logovani zmeny casu
              }
  
  
            if (znak == 'A')               // '#A' = doaldeni frekvence krystalu v RTC obvodu (Aging registr)
              {
                byte korekce =  (Serial.parseInt());      // precteni retezce ze seriove linky a jeho prevod na INT
                setRTC_aging(korekce);
                Serial.print("Novy Aging: ");
                Serial.println(korekce);
                log_pomprom = korekce;
                loguj(2);     // logovani zmeny Aging registru

              }
  
            if (znak == 'I')               // '#I' = korekce teploty (vnitrni z vlhkomeru)    0=-10.0'C;    50=-5.0'C;    100 = 0'C;   200=+10'C;   255 = +15.5'C
              {
                byte korekce =  (Serial.parseInt());      // precteni retezce ze seriove linky a jeho prevod na INT
                RAM1_write(EERAM_ADDR_korekce_IN, korekce);
                znak = 'd';                // (po nastaveni jakekoliv korekce se automaticky provadi vypis informaci do seriove linky - prikaz '#d')
                log_pomprom = korekce;
                loguj(3);     // logovani zmeny korekce vnitrni teploty
              }
  
  
            if (znak == 'O')               // '#O' = korekce teploty pro cidlo DS18B20    0=-10.0'C;    50=-5.0'C;    100 = 0'C;   200=+10'C;   255 = +15.5'C
              {
                byte korekce =  (Serial.parseInt());      // precteni retezce ze seriove linky a jeho prevod na INT
                RAM1_write(EERAM_ADDR_korekce_OUT, korekce);
                znak = 'd';
                log_pomprom = korekce;
                loguj(4);     // logovani zmeny korekce venkovni teploty
              }
  
            if (znak == 'o')               // '#o' = korekce teploty pro druhe cidlo DS18B20    0=-10.0'C;    50=-5.0'C;    100 = 0'C;   200=+10'C;   255 = +15.5'C
              {
                byte korekce =  (Serial.parseInt());      // precteni retezce ze seriove linky a jeho prevod na INT
                RAM1_write(EERAM_ADDR_korekce_OUT2, korekce);
                znak = 'd';
                log_pomprom = korekce;
                loguj(15);     // logovani zmeny korekce druheho venkovniho cidla teploty
              }

  
  
            if (znak == 'P')               // '#P' = korekce tlaku    0=-10hPa;    1=-9,9hPa; 99= -10Pa ;  100 = +0Pa;  101 = +10Pa ;  255 = +15,5hPa
              {
                byte korekce =  (Serial.parseInt());      // precteni retezce ze seriove linky a jeho prevod na INT
                RAM1_write(EERAM_ADDR_korekce_tlak, korekce);
                znak = 'd';
                log_pomprom = korekce;
                loguj(5);     // logovani zmeny korekce tlaku
              }
  
  
            if (znak == 'V')               // '#V' = korekce vlhkosti    0=-100%;    100=0%;    200 = +100%;   255 = +155%
              {
                byte korekce =  (Serial.parseInt());      // precteni retezce ze seriove linky a jeho prevod na INT
                RAM1_write(EERAM_ADDR_korekce_vlhkost, korekce);
                znak = 'd';
                log_pomprom = korekce;
                loguj(6);     // logovani zmeny korekce vlhkosti
              }
  
  
            if (znak == 'M')               // '#M' = dalkove mazani Minima a Maxima
              {
                nuluj_rekordy();
              }
      

            if (znak == 'g')               // '#g' nastaveni zemepisnych souradnic
              {
                float pom_geolat =  (Serial.parseFloat());       // zadavaji se 2 desetinna cisla (oddelovac desetinnych mist je tecka) oddelena carkou
                float pom_geolon = (Serial.parseFloat());

                if (pom_geolat < 47 or pom_geolat > 52 ) pom_geolat = 50;    // omezeni souradnic na obdelnik kolem CR+SR 
                if (pom_geolon < 11 or pom_geolon > 23 ) pom_geolon = 15;    //     (kvuli problemum v sirkach, kde Mesic nezapada a kvuli casovemu pasmu SEC /SELC)

                GeoLat = pom_geolat;
                GeoLon = pom_geolon;

                RAM1_write_int(EERAM_ADDR_GeoLat,(unsigned int)GeoLat * 100);
                RAM1_write_int(EERAM_ADDR_GeoLon,(unsigned int)GeoLon * 100);
                Serial.print(EEPROM_text_read(292));                        // "Geo Latitude = "
                Serial.println(GeoLat);
                Serial.print(EEPROM_text_read(292));                        // "Geo Longitude = "
                Serial.println(GeoLon);
                loguj(16);
                najdi_VZM();
              }

            if (znak == 'G')               // '#G' nastaveni nadmorske vysky
              {
                int pom_geoalt =  (Serial.parseInt()); 

                if (pom_geoalt < 0 or pom_geoalt > 3000 ) pom_geoalt = 0;    // omezeni nadmorske vysky na max 3000 m.n.m.

                GeoAlt = pom_geoalt;

                RAM1_write_int(EERAM_ADDR_GeoAlt,GeoAlt);
                Serial.print("Nadmorska vyska = ");                        // "Nadmorska vyska = "
                Serial.println(GeoAlt);
                loguj(16);
                najdi_VZM();
              }





      
            if (znak == 'm')               // '#m' = dalkove zhasinani LED
              {
                zhasni_vsechny_LED();
              }
  
            if (znak == 'x')               // '#x' = export kratkodobe EERAM
              {
                for (i = 0; i < 150; i++)                  // smycka pro poslednich 150 5-minutovych zaznamu (poslednich 12,5 hodiny)
                  {
                    iwdg_feed();     // obcerstveni WD             
                    Serial.print (i);                      // index
                    Serial.print (';');
                    Serial.print(RAM1_read_int(i + 600));   // casova znacka (minuty od pulnoci aktualniho dne)
                    Serial.print (';');
                    Serial.print(RAM1_read_int(i));         // tlak
                    Serial.print (';');
                    Serial.print(RAM1_read_int(i + 150));    // teplota OUT
                    Serial.print (';');
                    Serial.print(RAM1_read_int(i + 300));   // teplota IN
                    Serial.print (';');
                    Serial.print(RAM1_read_int(i + 450));   // vlhkost
                    Serial.println (';');
                  }
              }
  
            if (znak == 'X')               // '#X' = export dlouhodobe EERAM
              {
                for (i = 0; i < 150; i++)                    // smycka pro 150x hodinove prumery (neco pres 6 dni zpatky)
                  {
                    iwdg_feed();     // obcerstveni WD             
                    Serial.print (i);                        // index zaznamu
                    Serial.print (';');
                    Serial.print(RAM2_read_int(i + 600));     // casova znacka (hodiny od zacatku roku)
                    Serial.print (';');
                    Serial.print(RAM2_read_int(i));     // tlak
                    Serial.print (';');
                    Serial.print(RAM2_read_int(i + 150));     // teplota OUT
                    Serial.print (';');
                    Serial.print(RAM2_read_int(i + 300));     // teplota IN
                    Serial.print (';');
                    Serial.print(RAM2_read_int(i + 450));     // vlhkost
                    Serial.println (';');
                  }
      
              }


            if (znak == 'D')               // '#D' = export citelnych aktualnich udaju z displeje
              {
                Serial.print(EEPROM_text_read(261));                 // "Datum: "
                Serial.print(dat_str);                    // datum  (uvodni nuly nahrazeny mezerami)  priklad: " 6. 3.2018", nebo "13. 8.2020"
                Serial.print(' ');
                Serial.println(dny[LOC_dvt]);
                Serial.print("Cas: ");
                Serial.print(tim_str);                    // cas (hodiny maji uvodni nulu nahrazenou mezerou, minuty a sekundy jsou vcetne uvodnich nul) priklad: " 6:05:08"
                Serial.print(' ');
                if (casova_zona == 1) Serial.println(" SEC");
                else                  Serial.println("SELC");

                
                Serial.print(EEPROM_text_read(51));                    // "Teplota OUT: "
                Serial.print(((suma_OUT - 5000.0) / 100.0));
                Serial.print(" ['C]");
                if (typ_out_teplomeru > 1)
                  {
                    if (akt_cidlo == 1)   Serial.println("  (I.)");
                    if (akt_cidlo == 2)   Serial.println("  (II.)");
                  }
                else
                  {
                    Serial.println(' ');                
                  }
                
                Serial.print(EEPROM_text_read(52));                    // "Teplota IN: "
                Serial.print(((suma_IN - 5000.0) / 100.0));
                Serial.println(" ['C]");


                Serial.print(EEPROM_text_read(262));                 // "Tlak: "
                Serial.print(prumer_tlak);
                Serial.println(" [Pa]");

                Serial.print("   (pro nadm. vysku: ");               // "pro nadmorskou vysku "
                Serial.print(RAM1_read_int(EERAM_ADDR_GeoAlt));
                Serial.println(" m.n.m.)");


                Serial.print(EEPROM_text_read(263));                 // "Vlhkost: "
                Serial.print(vlhkost);
                Serial.println(" [%]");


                Serial.println(EEPROM_text_read(267));                 // "Stavy:"
                Serial.print(EEPROM_text_read(54));                    // "Modra LED: "
                Serial.println(RAM1_read(EERAM_ADDR_modra));
                Serial.print(EEPROM_text_read(55));                    // "Zelena LED: "
                Serial.println(RAM1_read(EERAM_ADDR_rekord_MIN));
                Serial.print(EEPROM_text_read(56));                    // "Cervena LED: "
                Serial.println(RAM1_read(EERAM_ADDR_rekord_MAX));

                Serial.print(EEPROM_text_read(57));                    // "Modra pamet: "
                Serial.println(RAM1_read(EERAM_ADDR_modra_pamet));

                Serial.print(EEPROM_text_read(58));                    // "Bluetooth: "
                Serial.println(stav_BT);


                Serial.print(EEPROM_text_read(59));                    // "SD log: "
                Serial.println(RAM1_read(EERAM_ADDR_SDaktiv));

                Serial.print(EEPROM_text_read(60));                    // "Blokovani LED: "
                Serial.println(RAM1_read(EERAM_ADDR_LEDblok));


                
      
              }
  
            if (znak == 'U')               // '#U' prepinaci uroven pro modrou LED - zadava se v desetinach stupne Celsia).
              {
                byte vyber_urovne;
                float f_uroven;
                if (Serial.available())
                  {
                    vyber_urovne = Serial.read();
                  }
                if (vyber_urovne == '1')
                  {
                    f_uroven = Serial.parseFloat();      // precteni retezce ze seriove linky a jeho prevod na float
                    prepinaci_uroven1 =  f_uroven * 10;
                    log_pomprom = prepinaci_uroven1;
                    prepinaci_uroven1 = prepinaci_uroven1 + 500;  // prepinaci uroven se okamzite zvysuje o 50'C, tak, aby nikdy nebyla zaporna
                    RAM1_write_int(EERAM_ADDR_prepinaci_uroven1, prepinaci_uroven1);
                    loguj(9);     // logovani zmeny 1. prepinaci urovne                    
                  }

                if (vyber_urovne == '2')
                  {
                    f_uroven = Serial.parseFloat();      // precteni retezce ze seriove linky a jeho prevod na float
                    prepinaci_uroven2 =  f_uroven * 10;
                    log_pomprom = prepinaci_uroven2;
                    prepinaci_uroven2 = prepinaci_uroven2 + 500;  // prepinaci uroven se okamzite zvysuje o 50'C, tak, aby nikdy nebyla zaporna
                    RAM1_write_int(EERAM_ADDR_prepinaci_uroven2, prepinaci_uroven2);
                    loguj(10);     // logovani zmeny 2. prepinaci urovne                    
                  }


                znak = 'd';
                  
                


              }

            if (znak == 'L')               // '#L' = Informace o Mesici (a o Slunci)
              {
                Mes_Serial_out();
              }
  
            if (znak == 'd')               // '#d' = zakladni info
              {
                for (i = 0; i < 5; i++)    // cteni 5 bajtu z EEPROM (casova zona, korekce teploty, korekce vlhkosti, korekce tlaku; korekce DS18B20)
                  {
                    Serial.print(EEPROM_text_read(28 + i));                              // textove popisy polozek
                    Serial.println(RAM1_read(i + EERAM_ADDR_timezone)); // korekce se zobrazuji jako obycejne cislo
                  }

                Serial.print("Korekce tepl. OUT2: ");               // pozdeji pridana korekce druheho venkovniho cidla
                Serial.println(RAM1_read(EERAM_ADDR_korekce_OUT2)); // 


                Serial.print(EEPROM_text_read(33));                  // "Prepinaci uroven 1:"
                Serial.print(citelna_teplota(prepinaci_uroven1));    // prepinaci uroven jako citelna teplota ve stupnich Celsia
                Serial.println("'C");
                Serial.print(EEPROM_text_read(34));                  // "Prepinaci uroven 2:"
                Serial.print(citelna_teplota(prepinaci_uroven2));    // prepinaci uroven jako citelna teplota ve stupnich Celsia
                Serial.println("'C");
                
                
                
                Serial.print(EEPROM_text_read(35));        // "Stari Mesice [d] : "
                Serial.println(stari_mesice_D);
      
                Serial.print(EEPROM_text_read(38));        // "Aktualni teplota"
                if (inout_LED == 1)    Serial.print ("OUT: ");
                else                   Serial.print ("IN: ");
      
      
                if (ignoruj_rekordy == 0)
                  {
                    if (inout_LED == 1)     Serial.print(zobraz_out);
                    else                    Serial.print(zobraz_in);
                    Serial.println("'C");
                  }
                else
                  {
                    Serial.println(" ---");
                  }
      
      
                if (STOP_LED_REK > 0)                     // pokud jsou rekordni LED zablokovane, zobrazi se zbyvajici cas v minutach do odblokovani
                  {
                    Serial.print(EEPROM_text_read(40));     // "Blokovani rekordnich"  
                    Serial.print(EEPROM_text_read(270));    // " LED: "   
                    Serial.print(STOP_LED_REK);
                    Serial.println(EEPROM_text_read(271));  // " [min]"            
                  }
      
                Serial.print(EEPROM_text_read(37));         // "Maximalni teplota"
                Serial.print(citelna_teplota2(tep_max));    // tep_min je pro venkovni teplotu v setinach 'C + 5000  (7256 = 22,56'C)
                Serial.print("'C (");
                Serial.print(tep_max_datcas);               // datum a cas maximalniho rekordu
                Serial.println(')');
        
        
                Serial.print(EEPROM_text_read(36));         // "Minimalni teplota"
                Serial.print(citelna_teplota2(tep_min));    // tep_min je v setinach 'C + 5000  (7256 = 22,56'C)
                Serial.print("'C (");
                Serial.print(tep_min_datcas);               // datum a cas minimalniho rekordu
                Serial.println(')');
        
                Serial.print(EEPROM_text_read(19));         // "Reset rekordu : "
                Serial.println(tep_rst_datcas);             // datum a cas posledniho resetu rekordu
        
        
                Serial.println (' ');
                Serial.println(EEPROM_text_read(39));       // "Puldenni rekordy"
                Serial.print(EEPROM_text_read(272));        // "Aktualne od: "
      
      
              
                Serial.print (RAM1_read(EERAM_ADDR_dennoc));
                Serial.println (":00");
                Serial.print(EEPROM_text_read(273));        // "   Minimum:"
                Serial.print(zobraz_cislo(tep_min1));
                Serial.print("'C (");
                Serial.print(int_to_hodmin(RAM1_read_int(EERAM_ADDR_hhmm_min1)));   // hodina a minuta rekordu
                Serial.print(EEPROM_text_read(274));        // ")    Maximum:"
                Serial.print(zobraz_cislo(tep_max1));
                Serial.print("'C (");
                Serial.print(int_to_hodmin(RAM1_read_int(EERAM_ADDR_hhmm_max1)));   // hodina a minuta rekordu
                Serial.println(')');
      
                if (LOC_hod >= 6 and LOC_hod < 18)
                  {
                    Serial.print(EEPROM_text_read(41));          // "Nocni rekordy (18:0"
                    Serial.println(EEPROM_text_read(42));        // "0 az 6:00)"
                  }
                else
                  {
                    Serial.print(EEPROM_text_read(43));          // "Denni rekordy (6:00"
                    Serial.println(EEPROM_text_read(44));        // " az 18:00)"
                  }
      
                Serial.print(EEPROM_text_read(273));        // "   Minimum:"
        
                Serial.print(zobraz_cislo(tep_min2));                              // hodnota rekordu
                Serial.print("'C (");
                Serial.print(int_to_hodmin(RAM1_read_int(EERAM_ADDR_hhmm_min2)));   // hodina a minuta rekordu
        
                Serial.print(EEPROM_text_read(274));        // ")    Maximum:"
                Serial.print(zobraz_cislo(tep_max2));                              // hodnota rekordu
                Serial.print("'C (");
                Serial.print(int_to_hodmin(RAM1_read_int(EERAM_ADDR_hhmm_max2)));   // hodina a minuta rekordu
                Serial.println(')');
      
      
              }
  
  
            if (znak == 'Z')               // '#Z' ukonceni blokovani zelene a cervene LED po nulovani rekordu
              {
                int korekce =  (Serial.parseInt());
                STOP_LED_REK = korekce;
                RAM1_write_int(EERAM_ADDR_stop_RG , korekce);
                Serial.print(EEPROM_text_read(61));         // "Blokovani R/G LED = "
                Serial.print(korekce);
                Serial.print(EEPROM_text_read(271));        // " [min]");
              }
  
  
  
            if (znak == '#')               // '##' zpatky do seriove linky se poslou aktualni hodnoty
              {
                posli_CSV_blok();
              }
      
            if (znak == '-')               // '#-' mazani aktualniho puldenniho rekordu
              {
                loguj(11);     // logovani mazani puldennich rekordu
                tep_min1 = 14994;
                tep_max1 = 0;
                RAM1_write_int(EERAM_ADDR_hhmm_min1 , 0);
                RAM1_write_int(EERAM_ADDR_hhmm_max1 , 0); 
                Serial.print(EEPROM_text_read(286));       // "Aktualni rekordy vyn"
                Serial.println(EEPROM_text_read(287));     // "ulovany"
              }


            if (znak == 'B')               // '#B' zapis budiku do EERAM (je to jen pro servis, takze bez testu korektniho zadani)
              {
                int cislo =  (Serial.parseInt());
                budik_on = cislo;
                RAM1_write_int(EERAM_ADDR_dis_on,cislo);     // cas rozsviceni
                cislo =  (Serial.parseInt());
                budik_off = cislo;
                RAM1_write_int(EERAM_ADDR_dis_off,cislo);    // cas zhasnuti
                cislo = (Serial.parseInt());
                budik_maska = cislo;
                RAM1_write(EERAM_ADDR_dis_maska,cislo);      // denni maska
                znak = 'b';
              }
                
            if (znak == 'b')               // '#b' zobrazeni hodnot nastavenenych na budiku
              {
                Serial.print(EEPROM_text_read(288));       // "Budik ON: "
                Serial.print(budik_on / 60);
                Serial.print(':');
                Serial.println(budik_on % 60);
                
                Serial.print(EEPROM_text_read(289));       // "Budik OFF: "
                Serial.print(budik_off / 60);
                Serial.print(':');
                Serial.println(budik_off % 60);

                Serial.print(EEPROM_text_read(290));       // "Maska Po,..,Ne,X: "
                Serial.print("0b");
                Serial.println(budik_maska , BIN);
              }

          }
      }
    else if (znak == '@')              // specialni servisni prikazy, ktere nejsou bezne dostupne
      {
        pauza100();
        if (Serial.available())
          {
            znak = Serial.read();          // znak, ktery nasleduje po '@' urcuje konkretni funkci
          }
        else
          {
            znak = '?';
          }

        if (znak == '?')               // '@?' vypis dostupnych funkci
          {
            Serial.println("@=[pwd]   tovarni RESET");
            Serial.println("@S        nacteni dat z SD karty do EEPROM");
            Serial.println("@w[adr,byte]     zapis do EERAM1");
            Serial.println("@W[adr,byte]     zapis do EERAM2");
            Serial.println("@r[adr,byte]     cteni z EERAM1");
            Serial.println("@R[adr,byte]     cteni z EERAM2");
            Serial.println("@C               smazani obou EERAM");
            Serial.println("@L               vypis LOG souboru");
            Serial.println("@N               servisni nastaveni");
            Serial.println("@F[graf,pocet]   smazani zacatku grafu");
            Serial.println("@P[graf,polozka] smazani jedne polozky");
            Serial.println("@a[adresa]  ze ktere SLAVE adresy se ma cist teplota");
            Serial.println("@A[adresa]  nastaveni SLAVE adresy na desce teplomeru");
            Serial.println("@i          ulozeni aktualnich infrakodu do souboru");
            Serial.println("@I          nacteni infrakodu ze souboru");
            
          }      

        if (znak == '@')               // '@@' docasna servisni funkce - pak smazat!!!
          {

          }

        if (znak == '=')               // '@=999' obnoveni tovarniho nastaveni z EEPROM do EERAM
          {
            delay(100);
            int cislo =  (Serial.parseInt());
            if(cislo == 999)
              {
                tovarni_reset();

    
                Serial2.print("page 0");  // na displeji zobrazit bootovaci obrazovku
                SerialFFF();
                delay(100);
                                
                iwdg_init(IWDG_PRE_256, 10);          // nastaveni WD na 50 milisekund  (10*5)
                iwdg_feed();                          // a okamzite obcerstveni WD
                delay(1000);                          // behem pauzy dojde k aktivaci WD a resetu procesoru                 
              }

            if(cislo == 9999)           // '@=9999'  total RESET
              {
                Serial.println("total RESET: ");
                Serial.println("EERAM");
                for (i = 0 ; i< 2048 ; i++)
                  {
                    RAM1_write(i, 255);
                    RAM2_write(i, 255);
                    if (i % 100 == 0)
                      {
                        iwdg_feed();     // obcerstveni WD
                        Serial.println(i); 
                      }
                  }
    
                Serial.println("EEPROM");
                for (i = 25000 ; i< 32768 ; i++)
                  {
                    EEPROM_write(i,255);
                    if (i % 100 == 0)
                      {
                        iwdg_feed();     // obcerstveni WD
                        Serial.println(i); 
                       }
                  }
                Serial.println("HOTOVO");
                Serial2.print("page 0");  // na displeji zobrazit bootovací obrazovku
                SerialFFF();
                delay(100);
                                
                iwdg_init(IWDG_PRE_256, 10);          // nastaveni WD na 50 milisekund  (10*5)
                iwdg_feed();                          // a okamzite obcerstveni WD
                delay(1000); 
              }


          }      


        if (znak == 'S')                 // '@S' Kopirovani dat z SDkarty do EEPROM
          {
            SD_EEPROM();
            displej_page0();            // displej se pak rozsviti na hlavni obrazovce
          }

        if (znak == 'N')                 // '@N' Zobrazeni displeje se servisnim nastavenim
          {
            ukaz_servis();               // servis konci zamernym pretecenim WatchDogu a tim padem resetem 
          }



        if (znak == 'R')               // '@R' cteni z EERAM2
          {
            int adresa =  (Serial.parseInt());
            Serial.print("EERAM2_read(");
            Serial.print(adresa);
            Serial.print(") = ");
            Serial.println(RAM2_read(adresa));
          }

        if (znak == 'r')               // '@r' cteni z EERAM1
          {
            int adresa =  (Serial.parseInt());
            Serial.print("EERAM1_read(");
            Serial.print(adresa);
            Serial.print(") = ");
            Serial.println(RAM1_read(adresa));
          }

        if (znak == 'W')               // '@W' servisni primy zapis do EERAM2
          {
            int adresa =  (Serial.parseInt());
            byte hodnota = (Serial.parseInt());
            Serial.print("EERAM2_read(");
            Serial.print(adresa);
            Serial.print(") = ");
            Serial.println(RAM2_read(adresa));
            RAM2_write(adresa,hodnota);
            Serial.print("EERAM2_write(");
            Serial.print(adresa);
            Serial.print(',');
            Serial.print(hodnota);
            Serial.println(')');
          }

        if (znak == 'w')               // '@w' servisni primy zapis do EERAM1
          {
            int adresa =  (Serial.parseInt());
            byte hodnota = (Serial.parseInt());
            Serial.print("EERAM1_read(");
            Serial.print(adresa);
            Serial.print(") = ");
            Serial.println(RAM1_read(adresa));
            RAM1_write(adresa,hodnota);
            Serial.print("EERAM1_write(");
            Serial.print(adresa);
            Serial.print(',');
            Serial.print(hodnota);
            Serial.println(')');
            
          }


        if (znak == 'a')               // '@a' prepnuti, ze ktere SLAVE adresy se maji cist teploty
          {
            byte adresa =  (Serial.parseInt());
            Serial.println("Prepnuti cteni teploty z jine desky");
            Serial.print("Stara SLAVE adresa: ");
            Serial.println(mod_addr);
            if (adresa > 0 and adresa < 255)             // povolene adresy jsou jen v rozsahu 1 az 254
              {
                RAM1_write(EERAM_ADDR_adr_mod,adresa);
                mod_addr = adresa;
                Serial.print("Nova SLAVE adresa: ");
                Serial.println(mod_addr);
              }
            else
              {
                Serial.println("Mimo rozsah (1-254)");
              }


          }


        if (znak == 'A')               // '@A' zmena SLAVE adresy na pripojene desce s teplomery
          {
            byte adresa =  (Serial.parseInt());
            Serial.println("Zmena SLAVE adresy na desce");
            if (adresa > 0 and adresa < 255)             // povolene adresy jsou jen v rozsahu 1 az 254
              {

                if (rs485_set_addr(mod_addr,adresa) == true)
                  {
                    RAM1_write(EERAM_ADDR_adr_mod,adresa);
                    mod_addr = adresa;
                    Serial.print("Prepsano na novou SLAVE adresu: ");
                    Serial.println(mod_addr);
                  }
                else
                  {
                    Serial.println("Chyba komunikace");
                  }
                                
              }
            else
              {
                Serial.println("Mimo rozsah (1-254)");
              }

            
          }



        if (znak == 'C')               // '@C' = reset RAM (zaplneni obou EERAM pameti hodnotou 0xFFFF) = smaze stare grafy
          {                            //   NEMAZE systemove promenne (prepinaci uroven, puldenni rekordy, stav LED, Geo souradnice ...)
            Serial.println(EEPROM_text_read(268));                 // "Mazani obou EERAM"
            for (i = 0; i < 1500; i++)
              {
                iwdg_feed();     // obcerstveni WD             
                RAM1_write_int(i, 65535);    // vsechny namerene hodnoty se prepisou znaky 0xFFFF
                RAM2_write_int(i, 65535);    // vsechny namerene hodnoty se prepisou znaky 0xFFFF
              }

            nuluj_rekordy();           // kvuli aktualizaci minima a maxima teploty se po smazani RAM musi provest nove nastaveni tep_min a tep_max
            loguj(8);     // logovani mazani pameti
            Serial.println(EEPROM_text_read(269));                // ".... Watchdog -> RST"
            for (i = 10; i > 0 ; i--)
              {
                Serial.print(i);
                delay(500);
                Serial.print("....");
                delay(500);
              }
            delay(10000);
          }

        if (znak == 'L')               // '@L' = vypis logovaciho souboru do seriove linky
          {           
            char SDznak;
            byte SDpozice;
            sd.begin(pin_SD_CS,SD_SCK_HZ(F_CPU/4));       //inicializace SD karty pro pripad, ze by byla predtim vytazena
            open_OK = soubor.open("log.txt", O_READ);
            if (open_OK)
              {
                while (soubor.available())
                  {
                    SDpozice ++;
                    if (SDpozice == 255) iwdg_feed();       // kazdych 255 znaku se obcersvi watchdog
                    SDznak = soubor.read();
                    Serial.print(SDznak);
                  }        
                Serial.println("---- Konec souboru ----");  
              }
            else
              {
                Serial.println("Chyba karty");  
              }
            soubor.close();

          }


        if (znak == 'F')               // '@F[graf],[pocet zaznamu]' smazani casti EERAM
          {
            byte graf =  (Serial.parseInt());
            byte poradi = (Serial.parseInt());
            unsigned int clr_adresa;
            if (graf < 9 and graf > 0)
              {
                if (poradi < 150 and poradi > 0)
                  {
                    clr_adresa = ((graf-1) / 2) * 150;

                    for (i = 0; i < poradi; i++)
                      {
                        iwdg_feed();     // obcerstveni WD             

                        if (graf % 2 == 0)           // sude grafy (2, 4, 6, 8) jsou dlouhodobe v EERAM2
                          {
                            RAM2_write_int(i + clr_adresa, 65535);    // vsechny namerene hodnoty se prepisou znaky 0xFFFF
                          }
                        else                         // liche grafy (1, 3, 5, 7) jsou kratkodobe v EERAM1
                          {
                            RAM1_write_int(i + clr_adresa, 65535);    // vsechny namerene hodnoty se prepisou znaky 0xFFFF
                          }
                      
                      }
                    Serial.println("Smazano");

                  }
                else
                  {
                    Serial.println("Pocet mimo rozsah <1...150>");
                  }
              }
            else
              {
                Serial.println("Graf mimo rozsah <1...8>");
              }

            
          }

              
        if (znak == 'P')               // '@P[graf],[polozka]' smazani jedne polozky z EERAM
          {
            byte graf =  (Serial.parseInt());
            byte poradi = (Serial.parseInt());
            unsigned int clr_adresa;
            if (graf < 9 and graf > 0)
              {
                if (poradi < 150)
                  {
                    clr_adresa = poradi + ((graf-1) / 2) * 150;

                    if (graf % 2 == 0)           // sude grafy (2, 4, 6, 8) jsou dlouhodobe v EERAM2
                      {
                        Serial.print("Puvodni hodnota: ");
                        Serial.println(RAM2_read_int(clr_adresa));
                        RAM2_write_int(clr_adresa, 65535);    // 0xFFFF
                      }
                    else                         // liche grafy (1, 3, 5, 7) jsou kratkodobe v EERAM1
                      {
                        Serial.print("Puvodni hodnota: ");
                        Serial.println(RAM1_read_int(clr_adresa));
                        RAM1_write_int(clr_adresa, 65535);    // 0xFFFF
                      }
                  
                    Serial.println("Smazano");

                  }
                else
                  {
                    Serial.println("Polozka mimo rozsah <0...150>");
                  }
              }
            else
              {
                Serial.println("Graf mimo rozsah <1...8>");
              }

          }

        if (znak == 'i')                 // '@i' zapis aktualnich infrakodu do souboru
          {
            sd.begin(pin_SD_CS,SD_SCK_HZ(F_CPU/4));       //inicializace SD karty pro pripad, ze by byla predtim vytazena
            open_OK = soubor.open("infra.txt", O_WRITE | O_CREAT);
            if (open_OK)
              {
                for (byte index_infrakodu = 0; index_infrakodu < 22 ; index_infrakodu++)
                  {
                    iwdg_feed();     // obcerstveni WD 
                    
                    // kvuli cteni musi byt vsechny kody v desitkove soustave, musi byt dlouhe 10 znaku a musi zacinat a koncit uvozovkami
                    //  zbytek radky za druhymi uvozovkami uz je jen nepodstatny komentar
                    //    priklad :
                    //                "1127583855"    //  0x4335906F  [0]  graf tlaku 12 hodin
                    
                    soubor.print('\"');
                    // doplneni nul na zacatek cisla
                    
                    if (infra_btn[index_infrakodu] < 1000000000) soubor.print('0');
                    if (infra_btn[index_infrakodu] < 100000000) soubor.print('0');
                    if (infra_btn[index_infrakodu] < 10000000) soubor.print('0');
                    if (infra_btn[index_infrakodu] < 1000000) soubor.print('0');
                    if (infra_btn[index_infrakodu] < 100000) soubor.print('0');
                    if (infra_btn[index_infrakodu] < 10000) soubor.print('0');
                    if (infra_btn[index_infrakodu] < 1000) soubor.print('0');
                    if (infra_btn[index_infrakodu] < 100) soubor.print('0');
                    if (infra_btn[index_infrakodu] < 10) soubor.print('0');
                    
                    soubor.print(infra_btn[index_infrakodu]);
                    soubor.print("\"    //  0x");
                    soubor.print(infra_btn[index_infrakodu],HEX);
                    soubor.print("  [");
                    soubor.print(index_infrakodu);
                    soubor.print("]  ");
                    soubor.println(infra_popisy[index_infrakodu]);

                    Serial.print(infra_popisy[index_infrakodu]);
                    Serial.print(";0x");
                    Serial.print(infra_btn[index_infrakodu],HEX);
                    Serial.print(';');
                    Serial.println(infra_btn[index_infrakodu]);
                  }
                Serial.println("HOTOVO");
              }
            else
              {
                Serial.println("Chyba karty");
              }
            soubor.close();
          }

        if (znak == 'I')                 // '@I' cteni infrakodu ze souboru
          {
            byte index_infrakodu = 0;
            unsigned long infra_read;
            sd.begin(pin_SD_CS,SD_SCK_HZ(F_CPU/4));       //inicializace SD karty pro pripad, ze by byla predtim vytazena
            open_OK = soubor.open("infra.txt", O_READ);
            if (open_OK)
              {
                while (soubor.available())
                  {
                    iwdg_feed();     // obcerstveni WD   
                    infra_read = SDradka_10znaku();
                    Serial.print(infra_popisy[index_infrakodu]);
                    Serial.print(": ");
                    Serial.println(infra_read,HEX);

                    // zapis precteneho kodu z SD karty do EEPROM
                    // rozlozeni na 4 bajty a zapsani do EEPROM
                    EEPROM_write((index_infrakodu*4) + 25000 , (infra_read >> 24) & 0xFF);
                    EEPROM_write((index_infrakodu*4) + 25001 , (infra_read >> 16) & 0xFF);
                    EEPROM_write((index_infrakodu*4) + 25002 , (infra_read >>  8) & 0xFF);
                    EEPROM_write((index_infrakodu*4) + 25003 ,  infra_read & 0xFF);
                    
                    infra_btn[index_infrakodu] = infra_read;
                    
                    index_infrakodu ++;

                  }        
                Serial.println("-- Konec souboru --");  
              }
            else
              {
                Serial.println("Chyba karty");  
              }
            soubor.close();

          }


      
      }


  
    pauza100();
    while (Serial.available())               // vsechno ostatni z prijimaciho bufferu vymazat
      {
        iwdg_feed();     // obcerstveni WD             
        Serial.read();             
      }
  }



  
