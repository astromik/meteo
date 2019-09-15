// vyhodnoceni tlacitek na IR dalkovem ovladaci
void infraSwitch(void)
  {

    
    if (InfraIndex < 99)
      {

        if (budik_bezi == true and piskani == false)   // pokud aktualne bezi budik, ale uz nepiska, tak se jakymkoliv infrakodem blokuje (zhasina displej)
          {
            budik_blok = true;
            budik_bezi = false;
            zpozdeni_podsvetu = 1;                      // a displej se pohybem zhasne (v pristi smycce)
            podsvet(false);
            InfraIndex = 99;                           // po tomto stisku DO se vsechno ostatni ignoruje
          }

        
        if (budik_bezi == true and piskani == true)   // pokud aktualne bezi budik a zaroven probiha piskani ...
          {
            piskani = false;                // ... tak se jakymkoliv infra tlacitkem se piskani vypne, ale displej sviti dal
            noTone(pin_bzuk);               // skutecne vypnuti zvuku
            InfraIndex = 99;                // po tomto stisku DO se vsechno ostatni ignoruje (zustava zobrazena hlavni obrazovka)
          }

        if (InfraIndex < 8)                // grafy
          {
            zpozdeni_podsvetu = KON_ZPOZD_PODSV;

            aktualni_graf = InfraIndex + 1;
            infragraf(InfraIndex + 1);
          }


        if (InfraIndex == 20)         // Informace o pristim budiku
          {
            zpozdeni_podsvetu = KON_ZPOZD_PODSV;
            pristi_budik();
          }


        if (InfraIndex == 15)         // Detailni info o Mesici
          {
            zpozdeni_podsvetu = KON_ZPOZD_PODSV;
            Serial2.print("page 18");          // "page 18"
            SerialFFF();

            podsvet(true);
            
            screen = 18;
            mesdet();
          }


        if (InfraIndex == 16)         // Detailni info o Slunci
          {
            zpozdeni_podsvetu = KON_ZPOZD_PODSV;
            Serial2.print("page 24");          // "page 24"
            SerialFFF();

            podsvet(true);
            
            screen = 24;
            sluncedet();
          }





        if (InfraIndex == 18)         // Rekordy
          {
            zpozdeni_podsvetu = KON_ZPOZD_PODSV;
            modra_pamet = 0;
            RAM1_write(EERAM_ADDR_modra_pamet,0);
            zhasni_RG_LED();
            obsluha_LED();
            
            Serial2.print(EEPROM_text_read(245));          // "page 4"
            SerialFFF();
            screen = 4;
            dis_rekordy();
          }
           
        if (InfraIndex == 8)          // Zhasinani rekordovych LED
          {
            zhasni_vsechny_LED();
          }
      

        if (InfraIndex == 9)          // displej ON/OFF
          {
            if (zpozdeni_podsvetu >= 1)             // kdyz podsvet sviti, tak se tlacitkem zhasne)
              {
                zpozdeni_podsvetu = 1;
                podsvet(false);
              }
            else                                    // kdyz podsvet nesviti, tak se zelenym tlacitkem rozsviti
              {
                displej_page0();
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                podsvet(true);
              }
          }


        if (InfraIndex == 10)          // funkce "Lampicka"
          {
            if (screen != 7)
              {
                Serial2.print(EEPROM_text_read(248));     // "page 7"
                SerialFFF();
                screen = 7;
              }
            if (zpozdeni_podsvetu >= 1)             // kdyz podsvet sviti, tak se modrym zhasne)
              {
                zpozdeni_podsvetu = 1;
                podsvet(false);
              }
            else                                    // kdyz podsvet nesviti, tak se modrym tlacitkem rozsviti
              {
                Serial2.print(EEPROM_text_read(248));     // "page 7"
                SerialFFF();
                zpozdeni_podsvetu = 254;
                screen = 7;
                podsvet(true);
                Serial2.print(EEPROM_text_read(115));      // "dim=100"
                SerialFFF();
              }
          }


 
        if (InfraIndex == 11)            // Bluetooth
          {
            if (stav_BT == 0)                    // kdyz je BT vypnute...
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                displej_page0();
                digitalWrite(pin_BT_start, LOW);     // zlutym tlacitkem se zapne
                stav_BT = 1;
                Serial2.print(EEPROM_text_read(174));      // "ik4.pic=77"
                SerialFFF();                  
              }
            else                                     // Kdyz je BT zapnute
              {
                digitalWrite(pin_BT_start, HIGH);    //  zlutym tlacitkem se vypne
                stav_BT = 0;
                Serial2.print(EEPROM_text_read(175));      // "ik4.pic=80"       // aktualizace ikony BT
                SerialFFF();                  
                zpozdeni_podsvetu = 1;
                podsvet(false);
              }
      
          }
        
     
        if (InfraIndex == 12)            // Navrat odkudkoliv na hlavni obrazovku (nebo rozsviceni displeje na hlavni obrazovce)
          {
            displej_page0();
            zpozdeni_podsvetu = KON_ZPOZD_PODSV;
          }


        if (InfraIndex == 17)            // Kalendar
          {
            mes2016 = ((LOC_rok - 2016) * 12) + LOC_mes - 1 ;        // poradove cislo mesice od roku 2016 (pocitano od 0)
            zobraz_kalendar();
          }





        if (InfraIndex == 19)            // Zobrazeni velkych hodin a aktualni venkovni teploty
          {
            
            Serial2.print(EEPROM_text_read(257));              // "page 17"
            SerialFFF();
            screen = 17;

            Serial2.print(EEPROM_text_read(260));              // "t0.pco=65504"   zluty font pro cas
            SerialFFF();
            
            Serial2.print(EEPROM_text_read(116));              // "t0.txt=\""
            Serial2.print(tim_str.substring(0,5));
            Serial2.print('\"');
            SerialFFF();

            podsvet(true);
         
            for (i=0; i<3 ; i++)                               // 3 sekundy pauza
              {
                delay(1000);  
                iwdg_feed();     // obcerstveni WD
              }            

            Serial2.print("t0.pco=34815");                     // "t0.pco=34815"   svetle modry font pro teplotu
            SerialFFF();
           
            Serial2.print(EEPROM_text_read(116));              // "t0.txt=\""
            if (chyba_cidla1 == 0)
              {
                Serial2.print(citelna_teplota2(suma_OUT),1);            
              }
            else
              {
                Serial2.print("---.-");
              }
            Serial2.print('\"');
            SerialFFF();
            
            Serial2.print("p0.pic=54");                     // znacka pro velky "'C" na spodni strane displeje
            SerialFFF();


            for (i=0; i<3 ; i++)                              // 3 sekundy pauza
              {
                delay(1000);  
                iwdg_feed();     // obcerstveni WD
              }            


            Serial2.print(EEPROM_text_read(117));             // "t0.txt=\" \""
            SerialFFF();
            
            Serial2.print("p0.pic=55");                     // smazani znacky "'C" na spodni strane displeje
            SerialFFF();
            
            zpozdeni_podsvetu = 1;
            podsvet(false);
            displej_page0();
            
          }

        if (InfraIndex == 14)            // Listovani [-] 
          {
            zpozdeni_podsvetu = KON_ZPOZD_PODSV;
            if (screen == 19)
              {
                mes2016 --;
                zobraz_kalendar();
              }
            
            if (screen == 5)
              {
                prvni_index_zatmeni ++;              // (v zatmenich se listuje opacnym smerem)
                if (prvni_index_zatmeni == 91)   prvni_index_zatmeni=90;
                Serial2.print(EEPROM_text_read(160));               // "n0.val="
                Serial2.print(prvni_index_zatmeni);
                SerialFFF();
                Serial2.print(EEPROM_text_read(159));               // "inx.val="
                Serial2.print(prvni_index_zatmeni);
                SerialFFF();

                zobraz_6_zatmeni(prvni_index_zatmeni);
                
              }

            if (screen != 19 and screen != 5)                     // na vsech ostatnich obrazovkach zpusobi tlacitko [-] zobrazeni grafu a jejich listovani
              {
                aktualni_graf ++;              // (v grafech se listuje opacnym smerem)
                if (aktualni_graf == 9) aktualni_graf = 1;
                infragraf(aktualni_graf);                      
              }
          }


        if (InfraIndex == 13)            // Listovani [+]
          {
            zpozdeni_podsvetu = KON_ZPOZD_PODSV;
            if (screen == 19)
              {
                mes2016 ++;
                zobraz_kalendar();
              }

            
            if (screen == 5)
              {
                if (prvni_index_zatmeni == 0)   prvni_index_zatmeni=1;
                prvni_index_zatmeni --;              // (v zatmenich se listuje opacnym smerem)

                Serial2.print(EEPROM_text_read(160));             // "n0.val="
                Serial2.print(prvni_index_zatmeni);
                SerialFFF();

                Serial2.print(EEPROM_text_read(159));             // "inx.val="
                Serial2.print(prvni_index_zatmeni);
                SerialFFF();
                
                zobraz_6_zatmeni(prvni_index_zatmeni);
              }

            if (screen != 19 and screen !=5)                   // na vsech ostatnich obrazovkach zpusobi tlacitko [+] zobrazeni grafu a jejich listovani
              {
                aktualni_graf --;              // (v grafech se listuje opacnym smerem)
                if (aktualni_graf == 0) aktualni_graf = 8;
                infragraf(aktualni_graf);
              }
          }

        if (InfraIndex == 21)            // Soumraky
          {
            zpozdeni_podsvetu = KON_ZPOZD_PODSV;
            soumraky();
          }



        
        

      }

    InfraIndex = 99;  
    attachInterrupt(digitalPinToInterrupt(pin_infra)  , infraINT  , FALLING);   // obnoveni interruptu
    
  }

