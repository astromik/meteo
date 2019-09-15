//----------------------------------------------------------------------------------------
// polozka  3 ... 5 min. tlak
// polozka  4 ... 1 hod. tlak
// polozka  5 ... 5 min. teplota OUT
// polozka  6 ... 1 hod. teplota OUT
// polozka  7 ... 5 min. teplota IN
// polozka  8 ... 1 hod. teplota IN
// polozka  9 ....5 min. vlhkost
// polozka 10 ....1 hod. vlhkost
void bargraf(byte polozka)
  {
    unsigned int pocatek;
    unsigned int konec;
    unsigned int vyska;
    unsigned int min_hodnota = 65535;
    unsigned int max_hodnota = 0;
    unsigned int k_mrizka;
    byte velikost;
  
    switch (polozka)
      {
        case 3:                      // (screen 8) tlak - 5-minutove vzorky
          pocatek = 0;
          konec = 150;
          break;
    
        case 4:                      // (screen 9) tlak - hodinove vzorky
          pocatek = 0;
          konec = 150;
          break;
    
        case 5:                      // (screen 10) venkovni teploty - 5-minutove vzorky
          pocatek = 150;
          konec = 300;
          break;
    
        case 6:                      // (screen 11) venkovni teploty - hodinove vzorky
          pocatek = 150;
          konec = 300;
          break;
    
        case 7:                      //(screen 12) vnitrni teploty - 5-minutove vzorky 
          pocatek = 300;
          konec = 450;
          break;
    
        case 8:                      //(screen 13) vnitrni teploty - hodinove vzorky
          pocatek = 300;
          konec = 450;
          break;
    
        case 9:                      //(screen 14) vlhkost - 5-minutove vzorky
          pocatek = 450;
          konec = 600;
          break;
    
        default:                     // (screen 15) vlhkost - hodinove vzorky
          pocatek = 450;
          konec = 600;
          break;
    
      }
  
  

    if (polozka % 2 == 0)          // dlouhodobe grafy (6 dni) jsou sude polozky a nachazi se v EERAM2
      {
        // nalezeni minima a maxima z poslednich 150 vzorku
        velikost = 0;
        for (i = pocatek ; i < konec; i++)
          {
            if (RAM2_read_int(i) < 65535)      // pokud je pametova bunka jeste nezaplnena (hodnota 0xFFFF), nepravadi se test na minimum a maximum
              {
                velikost ++;
                if (RAM2_read_int(i) > max_hodnota)  max_hodnota = RAM2_read_int(i);
                if (RAM2_read_int(i) < min_hodnota)  min_hodnota = RAM2_read_int(i);
              } 
          }
      
    
        if (velikost > 0)
          {
            Serial2.print(EEPROM_text_read(222));            // "addt 1,0,"
            Serial2.print(3*(velikost));
            SerialFFF();
            delay(50);
            for (i = pocatek; i < konec ; i++)
              {
                if (RAM2_read_int(i) < 65535)      // pokud je pametova bunka jeste nezaplnena (hodnota 0xFFFF), neprovadi se test na minimum a maximum
                  {
                    vyska = map(RAM2_read_int(i) , min_hodnota , max_hodnota , 1 , 200);     
                    for (byte sg=0; sg<3; sg++)         // jedna hodnota se naopiruje 3xvedle sebe
                      {
                        Serial2.write(vyska);
                      }
                  }
              }
            
          }
      }
    else                          // kratkodobe grafy (12 hodin) jsou liche polozky a nachazi se v EERAM1
      {
        // nalezeni minima a maxima z poslednich 150 vzorku
        velikost = 0;
        for (i = pocatek ; i < konec; i++)
          {
            if (RAM1_read_int(i) < 65535)      // pokud je pametova bunka jeste nezaplnena (hodnota 0xFFFF), nepravadi se test na minimum a maximum
              {
                velikost ++;
                if (RAM1_read_int(i) > max_hodnota)  max_hodnota = RAM1_read_int(i);
                if (RAM1_read_int(i) < min_hodnota)  min_hodnota = RAM1_read_int(i);
              } 
          }


      
        if (velikost > 0)
          {
    
    
            Serial2.print(EEPROM_text_read(223));        // "addt 1,0,"
            Serial2.print(3*(velikost));
            SerialFFF();
            delay(50);
            for (i = pocatek; i < konec ; i++)
              {
                if (RAM1_read_int(i) < 65535)      // pokud je pametova bunka jeste nezaplnena (hodnota 0xFFFF), neprovadi se test na minimum a maximum
                  {
                    vyska = map(RAM1_read_int(i) , min_hodnota , max_hodnota , 1 , 200);
                    for (byte sg=0; sg<3; sg++)         // jedna hodnota se naopiruje 3xvedle sebe
                      {
                        Serial2.write(vyska);
                      }
                  }
              }
          }
      }
  


    SerialFFF();

    if (velikost > 0)
      {

        if (polozka >=5 and polozka <= 8)                        // pro teploty se zobrazuji na okraji grafu krajni hodnoty
          {
            Serial2.print(EEPROM_text_read(125));               // "gmax.txt=\""
            Serial2.print(citelna_teplota(max_hodnota),1);
            Serial2.print("\"");
            SerialFFF();            
        
            Serial2.print(EEPROM_text_read(126));                // "gmin.txt=\""
            Serial2.print(citelna_teplota(min_hodnota),1);
            Serial2.print("\"");
            SerialFFF();

            if (polozka == 5 or polozka == 6)
              {
                for (int mrizka = 0; mrizka < 1500 ; mrizka = mrizka + 50)
                  {
                    if (max_hodnota >= mrizka and min_hodnota <= mrizka)
                      {
                        int barva=65535;                           // bila cara kazdych 5'C
                        if (mrizka == 500) barva = 2047;           // svetlemodra cara pro 0'C
                        if (mrizka == 700) barva = 65504;          // zluta cara pro 20'C
            
                        vyska = map(mrizka , min_hodnota , max_hodnota , 237 , 38);  
                        Serial2.print(EEPROM_text_read(143));          // "line 0,"
                        Serial2.print(vyska);
                        Serial2.print(EEPROM_text_read(218));          // ",450,"
                        Serial2.print(vyska);
                        Serial2.print(',');
                        Serial2.print(barva);
                        SerialFFF();             
                      }
                    
                  }
                
              }
            else                 // vnitrni teplota ma jemnejsi horizontalni deleni
              {
                for (int mrizka = 0; mrizka < 1500 ; mrizka = mrizka + 20)
                  {
                    if (max_hodnota >= mrizka and min_hodnota <= mrizka)
                      {
                        int barva=65535;                           // bila cara kazde 2°C
                        if (mrizka == 760) barva = 63504;          // ruzova cara pro 26°C
                        if (mrizka == 700) barva = 65504;          // zluta cara pro 20°C
            
                        vyska = map(mrizka , min_hodnota , max_hodnota , 237 , 38);  
                        Serial2.print(EEPROM_text_read(143));          // "line 0,"
                        Serial2.print(vyska);
                        Serial2.print(EEPROM_text_read(218));          // ",450,"
                        Serial2.print(vyska);
                        Serial2.print(',');
                        Serial2.print(barva);
                        SerialFFF();             
                      }
                    
                  }
                
              }
    
        
          }
    
        if (polozka == 9 or polozka == 10)                        // pro vlhkost se zobrazuji na okraji grafu krajni hodnoty v %
          {
            Serial2.print(EEPROM_text_read(125));               // "gmax.txt=\""
            Serial2.print(max_hodnota);
            Serial2.print("%\"");
            SerialFFF();            
        
            Serial2.print(EEPROM_text_read(126));                // "gmin.txt=\""
            Serial2.print(min_hodnota);
            Serial2.print("%\"");
            SerialFFF();            
    
            for (int mrizka = 0; mrizka <= 100 ; mrizka = mrizka + 5)
              {
                if (max_hodnota >= mrizka and min_hodnota <= mrizka)
                  {
                    int barva=65535;                          // bila cara kazdych 10%
                    if (mrizka == 50) barva = 2047;           // svetlemodra cara pro 50%
        
                    vyska = map(mrizka , min_hodnota , max_hodnota , 237 , 38);  
                    Serial2.print(EEPROM_text_read(143));          // "line 0,"
                    Serial2.print(vyska);
                    Serial2.print(EEPROM_text_read(218));          // ",450,"
                    Serial2.print(vyska);
                    Serial2.print(',');
                    Serial2.print(barva);
                    SerialFFF();             
                  }
                
              }        
          }

        if (polozka == 3 or polozka == 4)                        // pro tlak se zobrazuji na okraji grafu krajni hodnoty absolutniho tlaku v Pa
          {
            Serial2.print(EEPROM_text_read(125));               // "gmax.txt=\""
            Serial2.print(max_hodnota+60000);
            Serial2.print("\"");
            SerialFFF();            
        
            Serial2.print(EEPROM_text_read(126));                // "gmin.txt=\""
            Serial2.print(min_hodnota+60000);
            Serial2.print("\"");
            SerialFFF();            

            if (polozka == 3)      // pro 12h graf je jemnejsi horizontalni deleni, protoze za 12 se tlak nemuze zmenit o moc velkou hodnotu
              {
                for (int mrizka = 0; mrizka < 60000 ; mrizka = mrizka + 200)
                  {
                    if (max_hodnota >= mrizka and min_hodnota <= mrizka)
                      {
                        int barva=65535;                               // bila cara kazdych 2kPa
                        if (mrizka == 40000) barva = 65504;            // zluta cara pro 100 kPa
            
                        vyska = map(mrizka , min_hodnota , max_hodnota , 237 , 38);  
                        Serial2.print(EEPROM_text_read(143));          // "line 0,"
                        Serial2.print(vyska);
                        Serial2.print(EEPROM_text_read(218));          // ",450,"
                        Serial2.print(vyska);
                        Serial2.print(',');
                        Serial2.print(barva);
                        SerialFFF();             
                      }
                    
                  }
                
              }
            else
              {
                for (int mrizka = 0; mrizka < 60000 ; mrizka = mrizka + 1000)
                  {
                    if (max_hodnota >= mrizka and min_hodnota <= mrizka)
                      {
                        int barva=65535;                               // bila cara kazdych 10kPa
                        if (mrizka == 40000) barva = 65504;            // zluta cara pro 100 kPa
            
                        vyska = map(mrizka , min_hodnota , max_hodnota , 237 , 38);  
                        Serial2.print(EEPROM_text_read(143));          // "line 0,"
                        Serial2.print(vyska);
                        Serial2.print(EEPROM_text_read(218));          // ",450,"
                        Serial2.print(vyska);
                        Serial2.print(',');
                        Serial2.print(barva);
                        SerialFFF();             
                      }
                    
                  }
                
              }
          
          }




        if (polozka % 2 == 0)          // dlouhodobe grafy (6 dni) jsou sude polozky (2., 4., 6., 8. graf)
          {
            for (i = pocatek ; i < konec; i++)
              {
                if (RAM2_read_int(i + 600 - pocatek) % 24 == 0)      // znacka kazdou pulnoc
                  {
                    Serial2.print(EEPROM_text_read(144));           // "line "
                    Serial2.print(3*(i-pocatek) + 1);
                    Serial2.print(EEPROM_text_read(224));           // ",237,"
                    Serial2.print(3*(i-pocatek) + 1);
                    Serial2.print(EEPROM_text_read(132));           // ",38,GREEN"
                    SerialFFF();


                    // Zobrazeni znacek "Po" - "Ne" do dlouhodobych grafu
                    for (byte grafden = 0; grafden < 7 ; grafden ++)       // projde se 7 polozek v EERAM2
                      {
                        if (RAM2_read_int(i + 600 - pocatek) == RAM2_read_int(grafden + 750))  // a kdyz souhlasi prave nastavena pulnoc s nejakou z tech 7 polozek
                          {  
                            if (i-pocatek < 136)  // omezeni pro pripad, ze je pulnoc moc vpravo a obrazek dne v tydnu by mohl prepisovat MIN hodnotu
                              {
                                Serial2.print("pic ");
                                Serial2.print(((i-pocatek)*3)+ 3);
                                Serial2.print(",238,");
                                Serial2.print(57 + grafden);
                                SerialFFF();
                              }


                            
                          }
                      }
                  }
              }
            
          }
        else                       // kratkodobe grafy jsou liche polozky (1., 3., 5., 7. graf)
          {
            for (i = pocatek ; i < konec; i++)
              {
                if (RAM1_read_int(i + 600 - pocatek) % 120 == 0)  // znacka kazdou sudou celou hodinu
                  {
                    Serial2.print(EEPROM_text_read(144));           // "line "
                    Serial2.print(3*(i-pocatek) + 1);
                    Serial2.print(EEPROM_text_read(224));           // ",237,"
                    Serial2.print(3*(i-pocatek) + 1);
                    if (RAM1_read_int(i + 600 - pocatek) == 0)              // modra svisla cara pro pulnoc
                      {
                        Serial2.print(EEPROM_text_read(135));              // ",38,BLUE"
                      }
                    else if (RAM1_read_int(i + 600 - pocatek) == 720)       // cervena svisla cara  pro poledne
                      {
                        Serial2.print(EEPROM_text_read(133));              // ",38,RED"
                      }
                    else                                                     // vsechny ostatni svisle cary jsou zelene
                      {
                        Serial2.print(EEPROM_text_read(132));            // ",38,GREEN"
                      }
                    SerialFFF();

                    //   zobrazeni cisla hodiny dole u svisle cary
                    if (i-pocatek < 136)  // omezeni pro pripad, ze je svisla cara moc vpravo a cislo hodiny by mohlo zakryt MIN hodnotu
                      {

                        // "xstr  x, y , w, h, fontid, fontcolor, backcolor, xcenter, ycenter, sta, string"
                        // "xstr 60,238,16,16,2,YELLOW,16,0,0,1,"20"
                        
                        Serial2.print("xstr ");
                        Serial2.print(((i-pocatek)*3));
                        Serial2.print(",238,16,16,2,YELLOW,16,0,0,1,\"");
                        Serial2.print(RAM1_read_int(i + 600 - pocatek) / 60);
                        Serial2.print('\"');
                        SerialFFF();
                      }



                    
                  }
              }
            
          }
      }
    else            // pro grafy jeste nejsou zaznamenane zadne hodnoty
      {
        Serial2.print(EEPROM_text_read(127));                            // "gmax.txt=\"--\""
        SerialFFF();            
    
        Serial2.print(EEPROM_text_read(128));                            // "gmin.txt=\"--\""
        SerialFFF(); 
      }
  
  }


