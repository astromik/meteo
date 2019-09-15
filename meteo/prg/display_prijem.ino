// prijem dat z dotykoveho displeje
void dis_prijem(void)
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
            
            //===================   objekty na hlavni obrazovce   ===================
            if (page == 21)                        // pri kliknuti na cokoliv na hlavni obrazovce se prodluzuje cas podsvetu displeje
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
              }



            if (page == 21  and  objekt == 2)       // kliknuti na tlacitko [nastaveni] v hlavnim okne
              {
                Serial2.print(EEPROM_text_read(243));                     // "page 2"
                SerialFFF();
                if (stav_BT == 1)          Serial2.print(EEPROM_text_read(145));         // "BTC.val=1"
                else                       Serial2.print(EEPROM_text_read(146));         // "BTC.val=0"
                SerialFFF();

                if (SD_aktivni == 1)       Serial2.print(EEPROM_text_read(147));          // "c0.val=1"
                else                       Serial2.print(EEPROM_text_read(148));          // "c0.val=0"
                SerialFFF();

                if (LED_blok == 1)         Serial2.print(EEPROM_text_read(149));          // "c1.val=1"
                else                       Serial2.print(EEPROM_text_read(150));          // "c1.val=0"
                SerialFFF();

                if (zhasinat_podsvet == 1) Serial2.print(EEPROM_text_read(151));          // "c2.val=1"
                else                       Serial2.print(EEPROM_text_read(152));          // "c2.val=0"
                SerialFFF();

                Serial2.print(EEPROM_text_read(153));                                     // "uroven1.txt=\""
                Serial2.print((prepinaci_uroven1 - 500) / 10.0);
                Serial2.print('\"');
                SerialFFF();
                
                Serial2.print(EEPROM_text_read(154));                                     // "uroven2.txt=\""
                Serial2.print((prepinaci_uroven2 - 500) / 10.0);
                Serial2.print('\"');
                SerialFFF();

                aktualizuj_graf_urovni();
                
                screen = 2;
              }

            if (page == 21 and objekt == 153)       // kliknuti na pole rekordu v hlavnim okne
              {

                modra_pamet = 0;
                modra_pamet_pom = 1;
                RAM1_write(EERAM_ADDR_modra_pamet,0);
                obsluha_LED();
                
                zhasni_RG_LED();
                Serial2.print(EEPROM_text_read(245));                     // "page 4"
                SerialFFF();
                screen = 4;
                dis_rekordy();
              }


            if (page == 21  and  objekt == 154)     // kliknuti na hodiny na hlavni obrazovce => zobrazeni kalendare
              {
                mes2016 = ((LOC_rok - 2016) * 12) + LOC_mes - 1 ;        // poradove cislo mesice od roku 2016 (pocitano od 0)
                zobraz_kalendar();
              }

            if (page == 21  and  objekt == 155)     // kliknuti na grafy na hlavni obrazovce
              {
                Serial2.print(EEPROM_text_read(242));                    // "page 1"
                SerialFFF();
                screen = 1;
              }

            if (page == 21  and  objekt == 168)     // kliknuti na Mesic na hlavni obrazovce = detail Mesice
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                Serial2.print(EEPROM_text_read(258));                   // "page 18"
                SerialFFF();
                screen = 18;
                mesdet();
                
              }

            if (page == 21 and objekt == 175)     // kliknuti na sekundy v ukazateli casu zobrazi stranku s detailem pristiho budiku
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                pristi_budik();
              }


            if (page == 21 and objekt == 176)     // kliknuti na oblast zobrazeni teplot a vlhkosti
              {
                zpozdeni_podsvetu = 1;            //  ... zhasina displej
                podsvet(false);
                blokuj_priblizeni = 5;
                detachInterrupt(digitalPinToInterrupt(pin_IR_pohyb));
              }



            //===================   objekty na obrazovce "GRAFY"  ===================
            if (page == 1)                         // pri kliknuti na cokoliv na obrazovce grafu se prodluzuje cas podsvetu displeje
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
              }

            
            if (page == 1  and  objekt == 10)       // kliknuti na [Hlavni obrazovka] pri zobrazeni menu grafy
              {
                displej_page0();
              }



            if (page == 1  and  objekt == 2)       // kliknuti na tlacitko [graf tlaku 12 hodin]
              {
                Serial2.print(EEPROM_text_read(249));       // "page 8"
                SerialFFF();
                screen = 8;
                bargraf(3);
              }

            if (page == 1  and  objekt == 3)       // kliknuti na tlacitko [graf tlaku 6 dni]
              {
                Serial2.print(EEPROM_text_read(250));       // "page 9"
                SerialFFF();
                screen = 9;
                bargraf(4);              
              }

            if (page == 1  and  objekt == 4)       // kliknuti na tlacitko [graf venkovni teploty 12 hodin]
              {
                Serial2.print(EEPROM_text_read(251));       // "page 10"
                SerialFFF();
                screen = 10;
                bargraf(5);
              }

            if (page == 1  and  objekt == 5)       // kliknuti na tlacitko [graf venkovni teploty 6 dni]
              {
                Serial2.print(EEPROM_text_read(252));       // "page 11"
                SerialFFF();
                screen = 11;
                bargraf(6);              
              }


            if (page == 1  and  objekt == 6)       // kliknuti na tlacitko [graf vnitrni teploty 12 hodin]
              {
                Serial2.print(EEPROM_text_read(253));       // "page 12"
                SerialFFF();
                screen = 12;
                bargraf(7);
              }

            if (page == 1  and  objekt == 7)       // kliknuti na tlacitko [graf vnitrni teploty 6 dni]
              {
                Serial2.print(EEPROM_text_read(254));       // "page 13"
                SerialFFF();
                screen = 13;
                bargraf(8);
              
              }

            if (page == 1  and  objekt == 8)       // kliknuti na tlacitko [graf vlhkosti 12 hodin]
              {
                Serial2.print(EEPROM_text_read(255));       // "page 14"
                SerialFFF();
                screen = 14;
                bargraf(9);
              }

            if (page == 1  and  objekt == 9)       // kliknuti na tlacitko [graf vlhkosti 6 dni]
              {
                Serial2.print(EEPROM_text_read(256));       // "page 15"
                SerialFFF();
                screen = 15;
                bargraf(10);
              
              }





            //===================   objekty na obrazovce "NASTAVENI"  ===================

            if (page == 2)                         // pri kliknuti na cokoliv na obrazovce nastaveni se prodluzuje cas podsvetu displeje
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
              }
            
            if (page == 2  and  objekt == 9)       // kliknuti na tlacitko [Hlavni obrazovka] v nastaveni
              {

                Serial2.print(EEPROM_text_read(161));           // "get c1.val"                  // blokovat LED?
                SerialFFF();
                delay(20);
                odpoved_dis = Serial2.read();
                if (odpoved_dis == 0x71)
                  {
                    LED_blok = Serial2.read();
                    RAM1_write(EERAM_ADDR_LEDblok, LED_blok);
                    if (LED_blok == 1)          // pri zablokovani LED se ihned LED zhasinaji (ikony na displeji zustavaji beze zmeny)
                      {
                        pcf.write(0, HIGH);
                        pcf.write(1, HIGH);
                        pcf.write(2, HIGH);
                      }                  
                  }                
                flush_read_buffer2();

                Serial2.print(EEPROM_text_read(162));         // "get c0.val"               // zapnout logovani na SD?
                SerialFFF();
                delay(20);
                odpoved_dis = Serial2.read();
                if (odpoved_dis == 0x71)
                  {
                    SD_aktivni = Serial2.read();
                    RAM1_write(EERAM_ADDR_SDaktiv, SD_aktivni);
                  }                
                flush_read_buffer2();

                Serial2.print(EEPROM_text_read(163));        // "get uroven1.txt"                  // precist nastavenou uroven1 (des.cislo)
                SerialFFF();
                delay(20);
                odpoved_dis = Serial2.read();
                if (odpoved_dis == 0x70)
                  {
                    float f_uroven1 = Serial2.parseFloat();
                    prepinaci_uroven1 =  f_uroven1 * 10;
                    log_pomprom = prepinaci_uroven1;
                    prepinaci_uroven1 = prepinaci_uroven1 + 500;  // prepinaci uroven se okamzite zvysuje o 50'C, tak, aby nikdy nebyla zaporna
                    if (RAM1_read_int(EERAM_ADDR_prepinaci_uroven1) != prepinaci_uroven1)
                      {
                        RAM1_write_int(EERAM_ADDR_prepinaci_uroven1, prepinaci_uroven1);
                        loguj(9);     // logovani zmeny 1. prepinaci urovne  
                      }
                  }                
                flush_read_buffer2();


                Serial2.print(EEPROM_text_read(164));         // "get uroven2.txt"                  // precist nastavenou uroven1 (des.cislo)
                SerialFFF();
                delay(20);
                odpoved_dis = Serial2.read();
                if (odpoved_dis == 0x70)
                  {
                    float f_uroven2 = Serial2.parseFloat();
                    prepinaci_uroven2 =  f_uroven2 * 10;
                    log_pomprom = prepinaci_uroven2;
                    prepinaci_uroven2 = prepinaci_uroven2 + 500;  // prepinaci uroven se okamzite zvysuje o 50'C, tak, aby nikdy nebyla zaporna
                    if (RAM1_read_int(EERAM_ADDR_prepinaci_uroven2) != prepinaci_uroven2)
                      {
                        RAM1_write_int(EERAM_ADDR_prepinaci_uroven2, prepinaci_uroven2);
                        loguj(10);     // logovani zmeny 2. prepinaci urovne  
                      }
                  }                
                flush_read_buffer2();


                Serial2.print(EEPROM_text_read(165));        // "get c2.val"               // zhasinat podsvet?
                SerialFFF();
                delay(20);
                odpoved_dis = Serial2.read();
                if (odpoved_dis == 0x71)
                  {
                    zhasinat_podsvet = Serial2.read();
                    RAM1_write(EERAM_ADDR_zhasinat_podsvet,zhasinat_podsvet);
                  }                
                flush_read_buffer2();

                
                displej_page0();
              }



            if (page == 2  and  objekt == 2)       // kliknuti na bunku [zapnout/vypnout BT]
              {
                Serial2.print(EEPROM_text_read(166));           // "get BTC.val"                  // zapnout BT?
                SerialFFF();
                delay(20);
                odpoved_dis = Serial2.read();
                if (odpoved_dis == 0x71)
                  {
                    odpoved_dis = Serial2.read();
                    if (odpoved_dis == 1)
                      {
                        stav_BT = 1;
                        digitalWrite(pin_BT_start, LOW);
                      }
                    else
                      {
                        stav_BT = 0;                      
                        digitalWrite(pin_BT_start, HIGH);
                      }
                  }                
                flush_read_buffer2();
              }


            if (page == 2  and  objekt == 13)       // kliknuti na tlacitko [Nastaveni casu]
              {
                Serial2.print(EEPROM_text_read(244));                    // "page 3"
                SerialFFF();
                screen = 3;
                edit_hodiny();

              }


            if (page == 2  and  objekt == 14)       // kliknuti na tlacitko [Nastaveni budiku]
              {
                Serial2.print("page 22");                    // "page 22"
                SerialFFF();
                screen = 22;
                edit_budik();

              }






                
            //===================   objekty na obrazovce "CAS"  ===================

            if (page == 3)     // pri jakemkoliv kliknuti na objekt na obrazovce s nastavenim casu se prodluzuje podsviceni displeje
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
              }

            
            if (page == 3  and  objekt == 10)     // kliknuti na [Nastavit] pri editaci datumu a casu
              {
                nastav_dis_hodiny();
                RTC_default();
                displej_page0();
              }

            if (page == 3  and  objekt == 11)     // kliknuti na [STORNO] pri editaci datumu a casu
              {
                displej_page0();
              }




            //===================   objekty na obrazovce "REKORDY"  ===================
            
            if (page == 4  and  objekt == 2)       // kliknuti na [RESET] pri zobrazeni rekordu
              {
                nuluj_rekordy();
                dis_rekordy();                
              }


            if (page == 4  and  objekt == 8)       // kliknuti na [Hlavni obrazovka] pri zobrazeni rekordu
              {
                displej_page0();
              }


            //===================   objekty na obrazovce "ZATMENI"  ===================

            if (page == 5)     // pri jakemkoliv kliknuti na objekt na obrazovce se zatmenimi se prodluzuje podsviceni displeje
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
              }
            
            if (page == 5  and  objekt == 8)     // kliknuti na [Hlavni obrazovka] v zobrazeni zatmeni
              {
                displej_page0();
              }

            if (page == 5  and  (objekt == 9 or objekt == 10))     // posouvani aktualizace zatmeni na zaklade indexu
              {
                // zjistit index nalistovaneho zatmeni z promenne 'inx' v displeji
                Serial2.print(EEPROM_text_read(167));              // "get inx.val"
                SerialFFF();
                delay(20);
                odpoved_dis = Serial2.read();
                if (odpoved_dis == 0x71)
                  {
                    odpoved_dis = Serial2.read();
                    prvni_index_zatmeni = odpoved_dis;
                    zobraz_6_zatmeni(odpoved_dis);
                  }
                iwdg_feed();                           // obcerstveni WD 
              }







            //===================   objekty na obrazovce "BLACK"  ===================
            if (page == 6  and  objekt == 0)     // kliknuti kamkoli na cerny displej
              {
                displej_page0();
              }



            //===================   objekty na obrazovce "svetlo"  ===================
            if (page == 7  and  objekt == 0)     // kliknuti kamkoli na bily displej
              {
                displej_page0();
              }


            //===================   objekty na obrazovce "klavesnice"  ===================
            if (page == 16)     // pri jakemkoliv kliknuti na objekt na obrazovce s klavesnici se prodluzuje podsviceni displeje
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
              }



            //===================   objekty na obrazovce "detail Mesice"  ===================
            if (page == 18  and  objekt == 18)       // kliknuti na libovolne misto na plose pri zobrazeni detailu Mesice
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                displej_page0();
              }


            if (page == 18  and  objekt == 20)       // kliknuti malou ikonu aktualni faze Mesice
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                Serial2.print("page 24");
                SerialFFF();
                screen = 24;

                sluncedet();                         // obrazovka s detailnimi informacemi o Slunci
              }

              

            if (page == 18  and  objekt == 21)     // kliknuti na tlacitko "zatmeni" na strance s detailem Mesice
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                // nalezeni nejblizsiho zatmeni
                for (unsigned int index_databaze = priste_prohledavat_od; index_databaze < pocet_zaznamu ; index_databaze = index_databaze + 2)
                  {
                    // nacte se cely balik casu fazi pro jeden mesicni cyklus
                    T  = EEPROM_read_long(index_databaze); // z externi EEPROM se nacte cas uplnku nebo novu
                    byte zatmeni  = (T  >> 31);  // pokud pri novu dojde k zatmeni Slunce nebo Mesice, zjisti se poradovy index v databazi zatmeni

                    if (zatmeni > 0)
                      {
                        T  = T  & 0b01111111111111111111111111111111;
                        velikost_zatmeni(T);         // uvnitr podprogramu se nastavuje globalni promenna "index_zatmeni", ktera ukazuje na cislo polozky v databazi zatmeni
                        prvni_index_zatmeni = nalezeny_index_zatmeni-1;
                        break;
                      }
                  }
                
                Serial2.print(EEPROM_text_read(246));             // "page 5"           // spusteni obrazovky "ZATMENI"
                SerialFFF();
                screen = 5;

                Serial2.print(EEPROM_text_read(159));             // "inx.val="        // nastaveni parametru stranky tak, aby se jimi dalo listovat
                Serial2.print(nalezeny_index_zatmeni-1);
                SerialFFF();

                Serial2.print(EEPROM_text_read(160));             // "n0.val="
                Serial2.print(nalezeny_index_zatmeni-1);
                SerialFFF();

                zobraz_6_zatmeni(nalezeny_index_zatmeni-1);
              }


            if (page == 18  and  objekt == 19)     // kliknuti na maly kruhovy graf vychodu a zapadu Mesice
              {
                Serial2.print("page 26");
                SerialFFF();

                Serial2.print("azr.txt=\"");
                Serial2.print(mes_vych_azimut);
                Serial2.print('\"');
                SerialFFF();

                Serial2.print("azs.txt=\"");
                Serial2.print(mes_zap_azimut);
                Serial2.print('\"');
                SerialFFF();


                Serial2.print("aza.txt=\"");
                Serial2.print((int)Mes_azimut);
                Serial2.print('\"');
                SerialFFF();
                
                
                velky_graf_VZ(mes_vych_azimut , mes_zap_azimut , kruh_az_a);


                screen = 26;
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;

              }

              
            //===================   objekty na obrazovce "Kalendar"  ===================
            if (page == 19  and  objekt == 43)       // listovaci tlacitko [<-]
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                mes2016 --;
                zobraz_kalendar();
              }

            if (page == 19  and  objekt == 45)       // listovaci tlacitko [->]
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                mes2016 ++;
                zobraz_kalendar();
              }

            if (page == 19  and  objekt < 43)       // kliknutí na libovolný den nebo oznaceni dni v kalendari
              {
                displej_page0();
              }


            //===================   objekty na obrazovce "Nastaveni budiku"  ===================
            if (page == 22)     // pri jakemkoliv kliknuti na objekt na obrazovce s nastavenim budiku se prodluzuje podsviceni displeje
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
              }

            
            if (page == 22  and  objekt == 8)                                  // kliknuti na [Nastavit] pri editaci budiku
              {
                Serial2.print("get bon.val");                                  // "get bon.val"
                SerialFFF();
                delay(20);
                odpoved_dis = Serial2.read();
                if (odpoved_dis == 0x71)
                  {
                    budik_on = Serial2.read() +( Serial2.read()  * 256);          // precte 2 bajty ze seriove linky
                    RAM1_write_int(EERAM_ADDR_dis_on,budik_on);                // cas rozsviceni
                    flush_read_buffer2();
                  }

                Serial2.print("get bof.val");                                  // "get bof.val"
                SerialFFF();
                delay(20);
                odpoved_dis = Serial2.read();
                if (odpoved_dis == 0x71)
                  {
                    budik_off = Serial2.read() +( Serial2.read()  * 256);          // precte 2 bajty ze seriove linky
                    RAM1_write_int(EERAM_ADDR_dis_off,budik_off);               // cas zhasnuti
                    flush_read_buffer2();
                  }

                Serial2.print("get msk.val");                                   // "get bof.val"
                SerialFFF();
                delay(20);
                odpoved_dis = Serial2.read();
                if (odpoved_dis == 0x71)
                  {
                    budik_maska = Serial2.read();                               // precte 1 bajt ze seriove linky
                    RAM1_write(EERAM_ADDR_dis_maska,budik_maska);               // maska Po...Ne
                    flush_read_buffer2();
                  }                

                if (budik_on < budik_off)                                   // spravna funkce je jen v pripade, ze zapnuti displeje je driv, nez vypnuti
                  {                
                    Serial2.print("bh0.bco=65535");
                    SerialFFF();
                    Serial2.print("bm0.bco=65535");
                    SerialFFF();

                    displej_page0();                                        // navrat na hlavni obrazovku
                  }
                else                                                        // kdyz by se jednalo o prechod pres pulnoc, upozorni se na problem pomoci
                  {                                                         //  cervene barvy pozadi u vypinaciho casu (hodnoty na displeji zustavaji)
                    Serial2.print("bh0.bco=63488");
                    SerialFFF();
                    Serial2.print("bm0.bco=63488");
                    SerialFFF();

                    budik_off = budik_on;                                   // skutecny cas vypinani se v pripade spatneho zadani nastavi na stejnou hodnotu, jako zapinani
                    RAM1_write_int(EERAM_ADDR_dis_off,budik_off);           // a prepise se i v EERAM

                                                                            // v pripade chyby se navrat na hlavni obrazovku nekona. Zustava svitit nastaveni budiku se 
                                                                            //   zvyraznenou chybou
                  }
              }

            //===================   objekty na obrazovce "detaily Slunce"  ===================

            if (page == 24 and objekt == 13)     // kliknuti na ikonu Slunce = zobrazeni soumraku
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                soumraky();
              }
            
            if (page == 24 and objekt == 12)     // kliknuti nekam na zbylou plochu na obrazovce detailu Slunce
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                displej_page0();
              }




            //===================   objekty na obrazovce "pristi budik"  ===================
            if (page == 25 and objekt == 2)     // kliknuti nekam na volnou plochu na strance "pristi budik"
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                displej_page0();
              }

            if (page == 25 and objekt == 1)     // kliknuti na tlacitko "Zmenit nastaveni"
              {
                Serial2.print("page 22");                    // "page 22"
                SerialFFF();
                screen = 22;
                edit_budik();
              }

            //===================   objekty na obrazovce azimut vychodu a zapadu Mesice  ===================
            if (page == 26 and objekt == 4)     // kliknuti nekam na volnou plochu
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                displej_page0();
              }



            //===================   objekty na obrazovce soumraku  ===================
            if (page == 28)        // kliknuti na libovolne misto na strance
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
                displej_page0();
              }

            
          }
      }
  }

