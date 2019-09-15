// Podprogramy pro uceni infra kodu

void nastav_infra(void)
  {

    detachInterrupt(digitalPinToInterrupt(pin_infra));       // docasne vypne infra cidlo dalkoveho ovladani
    attachInterrupt(digitalPinToInterrupt(pin_infra)  , infraINT_nastav  , FALLING);      // infracidlo se bude ucit dalkove kody
    
    Serial2.print("page 27");
    index_infra_nastaveni = 0;
    SerialFFF();

    // zobrazeni prvni polozky vcetne puvodniho kodu
    Serial2.print("pop.txt=\"");
    Serial2.print(infra_popisy[0]);
    Serial2.print('\"');
    SerialFFF();

    Serial2.print("hx1.pco=RED");         // cervena barva textu (puvodni kod)
    SerialFFF();

    Serial2.print("hx1.txt=\"0x");
    Serial2.print(EEPROM_read_long(6250),HEX);
    Serial2.print('\"');
    SerialFFF();
    

    uint8_t pocitadlosmycek = 0;
    while (true)        // ze smycky pro skenovani infra kodu se vyskoci stiskem tlacitka "Ukoncit nastaveni", ktere konci resetem kvuli preteceni WD timeru
      {
        infradis();
        if (InfraHexaKod_uceni > 0)          // rezim uceni infracidla
          {

            Serial2.print("hx1.pco=65504");         // zluta barva textu (novy kod)
            SerialFFF();
            
            
            Serial2.print("hx1.txt=\"0x");
            Serial2.print(InfraHexaKod_uceni,HEX);
            Serial2.print('\"');
            SerialFFF();
            posledni_infrakod = InfraHexaKod_uceni;
            InfraHexaKod_uceni = 0;
            delay(500);
            attachInterrupt(digitalPinToInterrupt(pin_infra)  , infraINT_nastav  , FALLING);
          }

        delay(10);                         // kazdych 2,5 sekundy se znovu zkusi attachovat interrupt pro pripad, ze dalkove ovladani vrati InfraHexaKod_uceni = 0
        pocitadlosmycek ++;
        if(pocitadlosmycek == 0)
          {
            iwdg_feed();                   // obcerstveni WD
            attachInterrupt(digitalPinToInterrupt(pin_infra)  , infraINT_nastav  , FALLING);  
          }
      }

        
  }


void infradis(void)
  {
    
    byte page;
    byte objekt;
    byte odpoved_dis;
    while (Serial2.available())
      {
        if (Serial2.read() == 0x65)   // na neco bylo kliknuto
          {
            delay(20);             // chvili pauza na prijem zbytku komunikace (3 bajty s identifikaci stisknuteho objektu + 3x 0xFF)
            page = Serial2.read();
            objekt = Serial2.read();
            flush_read_buffer2();

            
            
            
            //===================   objekty na obrazovce Nastaveni Infra funkci   ===================
            if (page == 27 and objekt == 1)     // listovani nahoru (k nizsim indexum)
              {
                index_infra_nastaveni --;
                if (index_infra_nastaveni < 0) index_infra_nastaveni = 0;

                Serial2.print("in.val=");
                Serial2.print(index_infra_nastaveni);
                SerialFFF();

                
                Serial2.print("pop.txt=\"");
                Serial2.print(infra_popisy[index_infra_nastaveni]);
                Serial2.print('\"');
                SerialFFF();

                Serial2.print("hx1.pco=RED");         // cervena barva textu (puvodni kod)
                SerialFFF();

                Serial2.print("hx1.txt=\"0x");
                Serial2.print(EEPROM_read_long(6250 + index_infra_nastaveni),HEX);
                Serial2.print('\"');
                SerialFFF();
                
              }

            if (page == 27 and objekt == 2)     // listovani dolu (k vyssim indexum)
              {
                index_infra_nastaveni ++;
                if (index_infra_nastaveni > 21) index_infra_nastaveni = 21;
                Serial2.print("in.val=");
                Serial2.print(index_infra_nastaveni);
                SerialFFF();

                Serial2.print("pop.txt=\"");
                Serial2.print(infra_popisy[index_infra_nastaveni]);
                Serial2.print('\"');
                SerialFFF();

                Serial2.print("hx1.pco=RED");         // cervena barva textu (puvodni kod)
                SerialFFF();

                Serial2.print("hx1.txt=\"0x");
                Serial2.print(EEPROM_read_long(6250 + index_infra_nastaveni),HEX);
                Serial2.print('\"');
                SerialFFF();

                
              }

            if (page == 27 and objekt == 6)     // potvrzovaci tlacitko
              {
                // ulozeni naskenovaneho kodu do EEPROM na prislusnou pozici
                EEPROM_write((index_infra_nastaveni*4) + 25000 , (posledni_infrakod >> 24) & 0xFF);
                EEPROM_write((index_infra_nastaveni*4) + 25001 , (posledni_infrakod >> 16) & 0xFF);
                EEPROM_write((index_infra_nastaveni*4) + 25002 , (posledni_infrakod >>  8) & 0xFF);
                EEPROM_write((index_infra_nastaveni*4) + 25003 ,  posledni_infrakod & 0xFF);

                Serial2.print("hx1.pco=RED");         // cervena barva textu (kod byl ulozen)
                SerialFFF();
                
              }

            if (page == 27 and objekt == 7)     // NAVRAT tlacitko
              {

                detachInterrupt(digitalPinToInterrupt(pin_infra));       // docasne vypne infra cidlo dlakoveho ovladani, ktere se az do ted ucilo kody

                Serial2.print("page 0");                         // na displeji zobrazit bootovací obrazovku
                SerialFFF();
                delay(100);
            
                iwdg_init(IWDG_PRE_256, 10);                     // nastaveni WD na 50 milisekund  (10*5)
                iwdg_feed();                                     // a okamzite obcerstveni WD
                delay(1000);                                     // behem pauzy dojde k aktivaci WD a resetu procesoru 

              }

            if (page == 27 and objekt == 9)     // resetovaci tlacitko
              {
                // ulozeni naskenovaneho kodu do EEPROM na prislusnou pozici
                EEPROM_write((index_infra_nastaveni*4) + 25000 , 0xFF);
                EEPROM_write((index_infra_nastaveni*4) + 25001 , 0xFF);
                EEPROM_write((index_infra_nastaveni*4) + 25002 , 0xFF);
                EEPROM_write((index_infra_nastaveni*4) + 25003 , 0xFF);


                Serial2.print("hx1.txt=\"0xFFFFFFFF\"");
                SerialFFF();

                Serial2.print("hx1.pco=RED");         // cervena barva textu (kod byl ulozen)
                SerialFFF();
                
              }


          }
      }
  }

