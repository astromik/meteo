

void Mes_Serial_out(void)
  {

    astro_LOC_rok = LOC_rok;
    astro_LOC_mes = LOC_mes;
    astro_LOC_den = LOC_den;
    astro_LOC_hod = LOC_hod;
    astro_LOC_min = LOC_min;
    z_Astro_LOC_na_Astro_UTC(casova_zona);


    M_souradnice (1 , casova_zona, GeoLon, GeoLat );

    
    Serial.print(EEPROM_text_read(296));    // "Mesic (pro geosourad"
    Serial.print(EEPROM_text_read(297));    // "nice: LAT="
    Serial.print(GeoLat);
    Serial.print(EEPROM_text_read(298));    // " / LON="
    Serial.print(GeoLon);
    Serial.println(')');

    
    Serial.print(EEPROM_text_read(299));    // "Azimut: "       // aktualni azimut Mesice
    Serial.println((int)Mes_azimut);

    Serial.print(EEPROM_text_read(300));    // "Elevace: "      // aktualni elevace Mesice
    Serial.println((int)Mes_elevace);

    Serial.print(EEPROM_text_read(301));    // "Colongitudo: "  // aktualni colongitudo
    Serial.println((int)Mes_colongitudo);

    Serial.print(EEPROM_text_read(302));    //"Osvetleni [%]: " // osvetleni Mesice v %
    Serial.println((int)Mes_osvit);

    Serial.print(EEPROM_text_read(303));    // "Stari [d]: "    // stari Mesice ve dnech
    Serial.println(stari_mesice_D);

    Serial.print(EEPROM_text_read(304));    // "PeriApo [%]: "  // vzdalenost v procentech mezi perigeem a apogeem
    Serial.println(map(Mes_dist,356.41,406.697,100,0));

    Serial.print("pristi faze: ");            // "pristi faze: "
    Serial.print(pristifaze_txt);
    Serial.print("   ");
    Serial.print(faze_LOC_den);
    Serial.print('.');
    Serial.print(faze_LOC_mes);
    Serial.print(".  ");
    Serial.print(faze_LOC_hod);
    Serial.print(':');
    if (faze_LOC_min < 10) Serial.print ('0');
    Serial.println(faze_LOC_min);




    najdi_VZM();

    if (nextRise2016 < nextSet2016)
      {
        Serial.print(EEPROM_text_read(305));       // "Dalsi vychod: "     // nejblizsi dalsi vychod Mesice
        Serial.print(nextRiseD_str);
        Serial.print("  ");
        Serial.println(nextRiseH_str);
        Serial.print("     na azimutu: ");
        Serial.println(mes_vych_azimut);

    
        Serial.print(EEPROM_text_read(306));       // "Dalsi zapad:  "     // nejblizsi dalsi zapad Mesice
        Serial.print(nextSetD_str);
        Serial.print("  ");
        Serial.println(nextSetH_str);
        Serial.print("     na azimutu: ");
        Serial.println(mes_zap_azimut);
      }
    else
      {
        Serial.print(EEPROM_text_read(306));       // "Dalsi zapad:  "     // nejblizsi dalsi zapad Mesice
        Serial.print(nextSetD_str);
        Serial.print("  ");
        Serial.println(nextSetH_str);
        Serial.print("     na azimutu: ");
        Serial.println(mes_zap_azimut);

        Serial.print(EEPROM_text_read(305));       // "Dalsi vychod: "     // nejblizsi dalsi vychod Mesice
        Serial.print(nextRiseD_str);
        Serial.print("  ");
        Serial.println(nextRiseH_str);
        Serial.print("     na azimutu: ");
        Serial.println(mes_vych_azimut);
      } 

    // ------- detaily SLUNCE do seriove linky ---------
        Serial.println(' ');
        Serial.println("Slunce:");

        astro_LOC_rok = LOC_rok;
        astro_LOC_mes = LOC_mes;
        astro_LOC_den = LOC_den;
        astro_LOC_hod = LOC_hod;
        astro_LOC_min = LOC_min;
        z_Astro_LOC_na_Astro_UTC(casova_zona);
    
        M_souradnice (2 , casova_zona, GeoLon, GeoLat );
          
        Serial.print(" Aktualni azimut:  ");
        Serial.println((int)(Slu_azimut + 0.5));
    
        Serial.print(" Aktualni elevace: ");
        Serial.println((int)(Slu_elevace+0.5));

        Pozice_Slunce();

        Serial.print(" Aktualni uhel Mesic - Slunce:");
        Serial.println(MS_angle);
        Serial.println(' ');
    
    
        sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat , 1);
        Serial.print(" Dnes vychazi: ");
        Serial.println(hhh_to_hmin(sunrisex));


        astro_LOC_hod = (int)sunrisex;
        astro_LOC_min = (sunrisex - astro_LOC_hod) * 60;
        z_Astro_LOC_na_Astro_UTC(casova_zona);
        M_souradnice (2 , casova_zona, GeoLon, GeoLat );
        Serial.print("     na azimutu: ");
        Serial.println((int)(Slu_azimut + 0.5));
    


        Serial.print(" Dnes zapada:  ");
        Serial.println(hhh_to_hmin(sunsetx));

        astro_LOC_hod = (int)sunsetx;
        astro_LOC_min = (sunsetx - astro_LOC_hod) * 60;  
        z_Astro_LOC_na_Astro_UTC(casova_zona);
        M_souradnice (2 , casova_zona, GeoLon, GeoLat );
        Serial.print("     na azimutu: ");
        Serial.println((int)(Slu_azimut + 0.5));
  }



void mesdet(void)
  {

    astro_LOC_rok = LOC_rok;
    astro_LOC_mes = LOC_mes;
    astro_LOC_den = LOC_den;
    astro_LOC_hod = LOC_hod;
    astro_LOC_min = LOC_min;
    z_Astro_LOC_na_Astro_UTC(casova_zona);


    M_souradnice (1, casova_zona, GeoLon, GeoLat );

    Serial2.print(EEPROM_text_read(235));          // "maz.txt=\""           // aktualni azimut Mesice
    Serial2.print((int)Mes_azimut);
    Serial2.print('\"');
    SerialFFF();

    Serial2.print(EEPROM_text_read(236));          // "mel.txt=\""           // aktualni elevace Mesice
    Serial2.print((int)Mes_elevace);
    Serial2.print('\"');
    SerialFFF();

    Serial2.print(EEPROM_text_read(237));          // "col.txt=\"");         // aktualni colongitudo
    Serial2.print((int)Mes_colongitudo);
    Serial2.print('\"');
    SerialFFF();

    Serial2.print(EEPROM_text_read(238));          // "im.pic="              // ikona Mesice podle Colongituda
    Serial2.print((int)(Mes_colongitudo/3) + 111);                // V resourcich jsou ikony od indexu 111 a zobrazuji Mesic po 3 stupmich
    SerialFFF();


    Serial2.print(EEPROM_text_read(239));          // "mos.txt=\""         // osvetleni Mesice v %
    Serial2.print((int)Mes_osvit);
    Serial2.print(" %\"");
    SerialFFF();

    Serial2.print(EEPROM_text_read(14));          // "msta.txt=\""        // stari Mesice ve dnech
    Serial2.print(stari_mesice_D);
    Serial2.print(" d\"");
    SerialFFF();


    Pozice_Slunce();
    Serial2.print("msad.txt=\"");          // "msad.txt=\" "        // uhlova vzdalenost Slunce-Mesic
    Serial2.print((int)(MS_angle+0.5));
    Serial2.print("\"");
    SerialFFF();


    Serial2.print(EEPROM_text_read(15));          // "mdis.txt=\""        // vzdalenost v procentech mezi perigeem a apogeem
    Serial2.print(map(Mes_dist,356.41,406.697,100,0));
    Serial2.print(" %\"");
    SerialFFF();




    Serial2.print(EEPROM_text_read(16));          // "ntp.pic="           // ikona pristi faze Mesice
    Serial2.print (pamet_ikofaze);
    SerialFFF();

    
    Serial2.print(EEPROM_text_read(307));         // "pfa.txt=\""         // pristi cela faze Mesice (1. ctvrt / uplnek / posledni ctvrt / nov)
    Serial2.print (faze_LOC_den);
    Serial2.print ('.');
    Serial2.print (faze_LOC_mes);
    Serial2.print (". ");

    Serial2.print (faze_LOC_hod);
    Serial2.print (':');
    if (faze_LOC_min < 10) Serial2.print ('0');
    Serial2.print (faze_LOC_min);
    Serial2.print("\"");          
    SerialFFF();

    


    najdi_VZM();

    if (nextRise2016 < nextSet2016)
      {
        Serial2.print(EEPROM_text_read(308));         // "prvz1.pic=233"        // nejblizsi dalsi vychod Mesice
        SerialFFF();

        Serial2.print(EEPROM_text_read(309));         // "prvz2.pic=234"        // Zapad Mesice je az druha udalost
        SerialFFF();
        
        Serial2.print(EEPROM_text_read(310));         // "vz1d.txt=\""          // nejblizsi dalsi vychod Mesice
        Serial2.print(nextRiseD_str);
        Serial2.print('\"');
        SerialFFF();
        Serial2.print(EEPROM_text_read(311));         // "vz1h.txt=\""
        Serial2.print(nextRiseH_str);
        Serial2.print('\"');
        SerialFFF();
    
        Serial2.print(EEPROM_text_read(312));         // "vz2d.txt=\""          // nejblizsi dalsi zapad Mesice
        Serial2.print(nextSetD_str);
        Serial2.print('\"');
        SerialFFF();
        Serial2.print(EEPROM_text_read(313));         // "vz2h.txt=\""
        Serial2.print(nextSetH_str);
        Serial2.print('\"');
        SerialFFF();
      }
    else
      {
        Serial2.print(EEPROM_text_read(314));         // "prvz1.pic=234"        // nejblizsi dalsi zapad Mesice
        SerialFFF();

        Serial2.print(EEPROM_text_read(315));         // "prvz2.pic=233"        // vychod Mesice je az druha udalost
        SerialFFF();

        Serial2.print(EEPROM_text_read(310));         // "vz1d.txt=\""   // nejblizsi dalsi zapad Mesice
        Serial2.print(nextSetD_str);
        Serial2.print('\"');
        SerialFFF();
        Serial2.print(EEPROM_text_read(311));         // "vz1h.txt=\""
        Serial2.print(nextSetH_str);
        Serial2.print('\"');
        SerialFFF();

        Serial2.print(EEPROM_text_read(312));         // "vz2d.txt=\""         // nejblizsi dalsi vychod Mesice
        Serial2.print(nextRiseD_str);
        Serial2.print('\"');
        SerialFFF();
        Serial2.print(EEPROM_text_read(313));         // "vz2h.txt=\""
        Serial2.print(nextRiseH_str);
        Serial2.print('\"');
        SerialFFF();
    
      }


      

// --------------  dvoudenni graf elevace Mesice -----------------
    astro_LOC_rok = LOC_rok;
    astro_LOC_mes = LOC_mes;
    astro_LOC_den = LOC_den;
    astro_LOC_hod = 0;
    astro_LOC_min = 0;


    
    z_Astro_LOC_na_Astro_UTC(casova_zona);

    Serial2.print(EEPROM_text_read(316));         // "addt 4,0,480"
    SerialFFF();
    delay(50);

    unsigned int vyska;
    for (int graf2d = 0 ; graf2d < 480 ; graf2d ++)
      {

        if (graf2d % 100 == 0)     iwdg_feed();         // obcerstveni WD kazdych 100 bodu
        
        astro_UTCtoLOC();
        M_souradnice ( 1 , casova_zona, GeoLon, GeoLat );

        
        if (Mes_elevace > 0)
          {
            vyska = Mes_elevace;
          }
        else
          {
            vyska = 0;
          }
        Serial2.write(vyska + 28);
        for(int zrychli = 0 ; zrychli < 6 ; zrychli++) // pri kresleni grafu se zobrazuje kazda 6.minuta. Ostatni se preskakuji
          {
            plusminuta_UTC();
          }  
        
      }

    // zelena dvojcara na aktualnim case
    vyska = ((LOC_hod * 60) + LOC_min) / 6;
    Serial2.print(EEPROM_text_read(144));         // "line "
    Serial2.print(vyska);
    Serial2.print(",148,");          // ",450,"
    Serial2.print(vyska);
    Serial2.print(EEPROM_text_read(291));         // ",237,GREEN"
    SerialFFF();   
    vyska = ((LOC_hod * 60) + LOC_min) / 6;
    Serial2.print(EEPROM_text_read(144));         // "line "
    Serial2.print(vyska+1);
    Serial2.print(",148,");          // ",450,"
    Serial2.print(vyska+1);
    Serial2.print(EEPROM_text_read(291));         // ",237,GREEN"
    SerialFFF();   


      
  }


// rychle nalezeni dvou nasledujicich udalosti (prvniho zapadu a prvniho vychodu Mesice)
void najdi_VZM(void)
  {
    unsigned int krok_priblizeni = 1;

    iwdg_feed();                                     // obcerstveni WD    

    kruh_az_v = 999;
    kruh_az_z = 999;
    mes_vych_azimut = 999;
    
    astro_LOC_rok = LOC_rok;
    astro_LOC_mes = LOC_mes;
    astro_LOC_den = LOC_den;
    astro_LOC_hod = LOC_hod;
    astro_LOC_min = LOC_min;
    z_Astro_LOC_na_Astro_UTC(casova_zona);
    
    M_souradnice ( 1 , casova_zona, GeoLon, GeoLat );       //  nejdriv se zjisti, jestli je Mesic aktualne pod, nebo nad horizontem

    if (Mes_elevace > 0)
      {
        nadhorizontem = true;
        kruh_az_v = Mes_azimut;     // kdyz je Mesic nad horizontem, bude kruhovy graf zacinat aktualnim azimutem
      }
    else
      {
        nadhorizontem = false;
      }
    kruh_az_a = Mes_azimut;        // aktualni azimut na kruhovem grafu bude zvyrazneny

    unsigned int M_event = 0;
    while(M_event < 2)      // hledaji se jen 2 nasledujici udalosti (prvni zapad a vychod nebo prvni vychod a zapad) od aktualniho casu
      {
        astro_UTCtoLOC();
        M_souradnice ( 1 , casova_zona, GeoLon, GeoLat );

        if (nadhorizontem == true and Mes_elevace < 0)
          {
            if (kruh_az_z == 999)            // byl nalezen prvni zapad od aktualniho casu
              {
                kruh_az_z = Mes_azimut;           // azimut pristiho zapadu 
                mes_zap_azimut = kruh_az_z;       // azimut pristiho zapadu se pro dalsi zpracovani uklada jeste do globalni promenne
              }
            nextSetD_str = String(pred_den) + '.' + String(pred_mes) +  ".";
            if (pred_hod < 10)
              {
                nextSetH_str = ' ' + String(pred_hod) + ':';
              }
            else
              {
                nextSetH_str = String(pred_hod) + ':';
              }
              
            if (pred_min < 10) nextSetH_str = nextSetH_str + '0';
            nextSetH_str = nextSetH_str + String(pred_min);
            
            nextSet2016 = loc_min_2016(pred_rok, pred_mes, pred_den, pred_hod, pred_min);
            M_event ++;
            krok_priblizeni = 200;
            iwdg_feed();                                     // obcerstveni WD    

            nadhorizontem = false;
            Mes_elevace = -99;
          }

        if (nadhorizontem == false and Mes_elevace > 0)
          {
            if (kruh_az_v == 999)            // byl nalezen prvni vychod od aktualniho casu (kdyz je Mesic aktualne nad horizontem, zacina kruh na aktualnim azimutu - nastavuje se na zacatku podprogramu)
              {
                kruh_az_v = Mes_azimut;

              }            
            if (mes_vych_azimut == 999)            // byl nalezen prvni vychod od aktualniho casu
              {
                mes_vych_azimut = Mes_azimut;      // azimut pristiho vychodu se pro dalsi zpracovani uklada jeste do globalni promenne
              }
            
            nextRiseD_str = String(astro_LOC_den) + '.' + String(astro_LOC_mes) + ".";

            if (astro_LOC_hod < 10)
              {
                nextRiseH_str = ' ' + String(astro_LOC_hod) + ':';
              }
            else
              {
                nextRiseH_str = String(astro_LOC_hod) + ':';
              }

            if (astro_LOC_min < 10) nextRiseH_str = nextRiseH_str + '0';
            nextRiseH_str = nextRiseH_str + String(astro_LOC_min);
            nextRise2016 = loc_min_2016(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, astro_LOC_hod, astro_LOC_min);
            

            M_event ++;

            krok_priblizeni = 200;
            iwdg_feed();                                     // obcerstveni WD    
            nadhorizontem = true;
          }


        while (Mes_elevace > 10 or Mes_elevace < -10)
          {
            for(int zrychli = 0 ; zrychli < krok_priblizeni ; zrychli++)  // dalsi vychod nenastane driv, nez za 200 minut po zapadu
              {
                plusminuta_UTC();
              }        
            astro_UTCtoLOC();
            M_souradnice ( 1 , casova_zona, GeoLon, GeoLat );
            krok_priblizeni = priblizeni_casu();
          }
          
        plusminuta_UTC();
        
        pred_rok = astro_LOC_rok;
        pred_mes = astro_LOC_mes;
        pred_den = astro_LOC_den;
        pred_hod = astro_LOC_hod;
        pred_min = astro_LOC_min;

        
      }

    if (nextRise2016 <= nextSet2016)                         // nejdriv vychod a pak zapad
      {
        Serial2.print(EEPROM_text_read(319));           // "hlv.zah.txt=\""            // horni radka = zluty vychod
        Serial2.print(nextRiseH_str);
        Serial2.print('\"');  
        SerialFFF();          
    
        Serial2.print(EEPROM_text_read(320));           // "hlv.zahd.txt=\""
        Serial2.print(nextRiseD_str);
        Serial2.print('\"');  
        SerialFFF();          

        Serial2.print(EEPROM_text_read(321));           // "hlv.zahiko.pic=235"
        SerialFFF();          

        Serial2.print(EEPROM_text_read(322));           // "hlv.zah.pco=65504"
        SerialFFF();          
        Serial2.print(EEPROM_text_read(323));           // "hlv.zahd.pco=65504"
        SerialFFF();          
        
        Serial2.print(EEPROM_text_read(324));           // "hlv.zat.txt=\""                 // spodni radka = sedy zapad
        Serial2.print(nextSetH_str);
        Serial2.print('\"');  
        SerialFFF();          
    
        Serial2.print(EEPROM_text_read(325));           // "hlv.zatd.txt=\""
        Serial2.print(nextSetD_str);
        Serial2.print('\"');  
        SerialFFF();          

        Serial2.print(EEPROM_text_read(326));           // "hlv.zatiko.pic=236"
        SerialFFF();          

        Serial2.print(EEPROM_text_read(327));           // "hlv.zat.pco=50712"
        SerialFFF();          
        Serial2.print(EEPROM_text_read(328));           // "hlv.zatd.pco=50712"
        SerialFFF();          


      }
    else                         // nejdriv zapad a pak vychod
      {
        Serial2.print(EEPROM_text_read(329));           // "hlv.zah.txt=\""           // horni radka = sedy zapad
        Serial2.print(nextSetH_str);
        Serial2.print('\"');  
        SerialFFF();          
    
        Serial2.print(EEPROM_text_read(330));           // "hlv.zahd.txt=\""
        Serial2.print(nextSetD_str);
        Serial2.print('\"');  
        SerialFFF();          

        Serial2.print(EEPROM_text_read(331));           // "hlv.zahiko.pic=236"
        SerialFFF();          

        Serial2.print(EEPROM_text_read(332));           // "hlv.zah.pco=50712"
        SerialFFF();          
        Serial2.print(EEPROM_text_read(333));           // "hlv.zahd.pco=50712"
        SerialFFF();          
        
        Serial2.print(EEPROM_text_read(334));           // "hlv.zat.txt=\""            // spodni radka = zluty vychod
        Serial2.print(nextRiseH_str);
        Serial2.print('\"');  
        SerialFFF();          
    
        Serial2.print(EEPROM_text_read(335));           // "hlv.zatd.txt=\""
        Serial2.print(nextRiseD_str);
        Serial2.print('\"');  
        SerialFFF();          

        Serial2.print(EEPROM_text_read(336));           // "hlv.zatiko.pic=235"
        SerialFFF();          

        Serial2.print(EEPROM_text_read(337));           // "hlv.zat.pco=65504"
        SerialFFF();          
        Serial2.print(EEPROM_text_read(338));           // "hlv.zatd.pco=65504"
        SerialFFF();          
        
      }
    if (screen == 18)   // pokud je na displeji zobrazena stranka s detailem Mesice, vykresli se kruhovy azimutarni graf
      {
        kruhovy_graf(kruh_az_v , kruh_az_z , kruh_az_a);     // nakresli kruhovy graf azimutu se zvyraznenym aktualnim azimutem      
      }


    
  }

// zrychene priblizeni k okamziku dalsiho vychodu, nebo zapadu Mesice v zavislosti na aktualni elevaci
unsigned int priblizeni_casu(void)
  {
    if (Mes_elevace >= 3 or Mes_elevace <= -3)
      {
        return (abs(Mes_elevace) * 2);
      }
    else
      {
        return Mes_elevace + 1;
      }
  }


//---------- prepocet lokalniho casu na UTC minuty od roku 2016 -------------
unsigned long loc_min_2016(unsigned int r2016, byte m2016, byte d2016, byte h2016, byte i2016)
  {

    // pocet dni od zacatku roku pro aktualni mesic
    // napriklad pri datumu 25.4.  se vezme mesic duben (index 3) a k nemu se pripocte pocet CELYCH dni v mesici (25) takze vysledkem je 119+25 = 145 celych (dokoncenych) dni
    // mesic          Led   Uno   Bre   Dub   Kve   Cer   Cec   Srp   Zar   Rij  Lis   Pro
    // index           0     1     2     3     4     5     6     7     8     9    10    11
    int  pomdny[] = { -1,   30,   58,   89,  119,  150,  180,  211,  242,  272,  303,  333 };
  
    byte prestupneroky = ((r2016 - 2016)  / 4) + 1;       // pocet celych prestupnych roku od 2016 (muze to byt vcetne aktualniho roku) (2016 je prestupny, tak musi byt +1)
  
    if (r2016 % 4 == 0  && m2016 < 3)                   // kdyz je aktualni rok prestupny a kdyz je v tomto prestupnem roce aktualni mesic leden, nebo unor
      {
        prestupneroky --;                                   // tak se tento jeden prestupny rok nezapocitava (prestupny den je az na konci unora)
      }
  
    unsigned long m2016UTC = ((r2016 - 2016) * 365UL) + prestupneroky;       // pocet dni za uplynule KOMPLETNI CELE roky od 1.1.2016  (pro rok 2016 je vysledek 0, pro rok 2017 je vysledek 366...)
    m2016UTC = m2016UTC + pomdny[m2016 - 1] + d2016;    // k tomu se pricte pocet dokoncenych dni od zacatku roku (napr: 10.1.2017 da vysledek 366 + 9)
  
    m2016UTC = (m2016UTC * 24UL) + h2016;                 // vysledne dny se prepoctou na hodiny a pridaji se "dnesni" hodiny od pulnoci
    m2016UTC = (m2016UTC * 60UL) + i2016;                  // vysledne hodiny se prepoctou na minuty pridaji se minuty z prave probihajici hodiny

    m2016UTC - (60 * casova_zona);

    return m2016UTC;
  }








//---------------------------------------------------
// Podle zadaneho UTC, LOC a lokalnich souradnic a posunu casu vypocte azimut a elevaci Mesice
void M_souradnice(byte teleso, byte utcdif, double mlong, double mlat  )
  {
    //               d   ,           m   ,           y   ,           h   ,           n 
    mesic (astro_UTC_den , astro_UTC_mes , astro_UTC_rok , astro_UTC_hod , astro_UTC_min);       // UTC

    if (teleso == 1)
      {
    //                  ra   ,    dec  ,           y   ,           m   ,           d   ,           h   ,           n   ,   TZ   ,  long ,  lat
        RaToAzim ( 1 , Mes_RA,  Mes_DE , astro_LOC_rok , astro_LOC_mes , astro_LOC_den , astro_LOC_hod , astro_LOC_min , utcdif , mlong , mlat  );  
      }

    if (teleso == 2)
      {
    //                  ra   ,    dec  ,           y   ,           m   ,           d   ,           h   ,           n   ,   TZ   ,  long ,  lat
        RaToAzim ( 2 , Slu_RA,  Slu_DE , astro_LOC_rok , astro_LOC_mes , astro_LOC_den , astro_LOC_hod , astro_LOC_min , utcdif , mlong , mlat  );  
      }
  
  
  }


//---------------------------------------------------------
// k astronomickym casovym promennym v UTC prida 1 minutu
void plusminuta_UTC(void)
  {

    byte maxden[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (astro_UTC_rok % 4 == 0)
      {
        maxden[2]=29;
      }
    else
      {
        maxden[2]=28;      
      }
    
    astro_UTC_min ++;
    if(astro_UTC_min == 60)
      {
        astro_UTC_min = 0;
        astro_UTC_hod++;
        if (astro_UTC_hod==24)
          {
            astro_UTC_hod=0;
            astro_UTC_den++;
            if (astro_UTC_den > maxden[astro_UTC_mes])
              {
                astro_UTC_den = 1;
                astro_UTC_mes++;
                if (astro_UTC_mes==13)
                  {
                    astro_UTC_mes=1;
                    astro_UTC_rok++;
                  }
              }
          }  
      }
  }


//---------------------------------------------------------
// k astronomickym datovym promennym v LOC prida 1 den
void plusden_LOC(void)
  {

    byte maxden[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (astro_LOC_rok % 4 == 0)
      {
        maxden[2]=29;
      }
    else
      {
        maxden[2]=28;      
      }
    
    astro_LOC_den++;
    if (astro_LOC_den > maxden[astro_LOC_mes])
      {
        astro_LOC_den = 1;
        astro_LOC_mes++;
        if (astro_LOC_mes==13)
          {
            astro_LOC_mes=1;
            astro_LOC_rok++;
          }
      }
  }



//-------------------------------------------------------------
// vstupem jsou globalni promenne "astro_LOC_xxx"
// vysledky jsou v promennych "astro_UTC_xxx"
void z_Astro_LOC_na_Astro_UTC(int offset)    // posun z lokalniho casu do UTC (pro SEC je to 1, pro SELC je to 2)
  {
  
    if ((astro_LOC_rok % 4) == 0)     // pri prestupnem roku se nastavuje unor na 29 dni
      {
        pomdny_d[2] = 29;
      }
    else
      {
        pomdny_d[2] = 28;
      }
    
    astro_UTC_min = astro_LOC_min;                       // na zacatku se UTC promenne nastavi na stejnou hodnotu jako mistni cas
    astro_UTC_rok = astro_LOC_rok;
    astro_UTC_mes = astro_LOC_mes;
    astro_UTC_den = astro_LOC_den;
    astro_UTC_hod = astro_LOC_hod - offset;              // ... akorat od hodin se odecte casovy posun (1 nebo 2 hodiny)
  
    if (astro_UTC_hod < 0)                         // kontrola, jestli po odecteni jedne, nebo dvou hodin nedoslo k podteceni casu do predchozicho dne
      {
        astro_UTC_hod = astro_UTC_hod + 24;
        astro_UTC_den = astro_UTC_den - 1;
        if (astro_UTC_den < 1)                     // kontrola podteceni do predchoziho mesice
          {
            astro_UTC_mes = astro_UTC_mes - 1;
            if (astro_UTC_mes < 1)                 // kontrola podteceni do predchoziho roku
              {
                astro_UTC_rok = astro_UTC_rok - 1;
                astro_UTC_mes = 12;
              }
            astro_UTC_den = pomdny_d[astro_UTC_mes];       // cislo dne se nastavi na nejvyssi cislo dne v predchozim mesici
          }
      
      }
    
  }
  


//---------------------------------------------------------
//   prevod promennych z 'astro_UTC_xxx' na 'astro_LOC_xxx' podle globalni promenne 'casova_zona'
void astro_UTCtoLOC()
  {

    byte maxden[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (astro_UTC_rok % 4 == 0)      // prestupny rok (pro rozsah let 2016 az 2070 neni treba resit speciality Gregorianskeho kalendare - napr. rok 2100)
      {
        maxden[2]=29;
      }

    astro_LOC_rok = astro_UTC_rok;
    astro_LOC_mes = astro_UTC_mes;
    astro_LOC_den = astro_UTC_den;
    astro_LOC_min = astro_UTC_min;
    
    astro_LOC_hod = astro_UTC_hod + casova_zona;
    if (astro_LOC_hod > 23)
      {
        astro_LOC_hod = astro_LOC_hod-24;
        astro_LOC_den++;
        if (astro_LOC_den > maxden[astro_UTC_mes])
          {
            astro_LOC_den = 1;
            astro_LOC_mes++;
            if (astro_LOC_mes == 13)
              {
                astro_LOC_mes = 1;
                astro_LOC_rok++;
              }
          }
      }  
  }
//---------------------------------------------------------



//---------------------------------------------------------
//  vypocty pro polohu Mesice v rovnikovych souradnicich
// vystupy: globalni promenne 
//       Mes_RA          ... Rektascenze Mesice
//       Mes_DE          ... Deklinace Mesice
//       Mes_dist        ... vzdalenost Mesice od Zeme
//       Mes_colongitudo ... uhel (selenograficka delka) ranniho terminatoru
//       Mes_osvit       ... osvetlena cast v %
//       Slu_RA          ... Rektascenze Slunce
//       Slu_DE          ... Deklinace Slunce

//
//  vstupy UTC:
//           den  ,   mes  ,   rok,     hod  ,  min
void mesic(int xd , int xm , int xy , int xh ,int xmin)
  {

    double days = day2000(xy, xm, xd, xh + xmin/60.0);  
    double t = days / 36525.0;
    
    //  Data Slunce
    double L1 = rozsah(280.466 + 36000.8 * t);
    double M1 = rozsah(357.529+35999*t - 0.0001536* t*t + t*t*t/24490000.0);
    double C1 = (1.915 - 0.004817* t - 0.000014* t * t)* dsin(M1);   
           C1 = C1 + (0.01999 - 0.000101 * t)* dsin(2*M1);
           C1 = C1 + 0.00029 * dsin(3*M1);
    double V1 = M1 + C1;
    double EC1 = 0.01671 - 0.00004204 * t - 0.0000001236 * t*t;
    double R1 = 0.99972 / (1 + EC1 * dcos(V1));
    double Th1 = L1 + C1;
    double OM1 = rozsah(125.04 - 1934.1 * t);
    double LaM1 = Th1 - 0.00569 - 0.00478 * dsin(OM1);
    double Obl = (84381.448 - 46.815 * t)/3600;
    double Ra1 = datan2(dsin(Th1) * dcos(Obl) - dtan(0)* dsin(Obl), dcos(Th1));
    double DEC1 = dasin(dsin(0)* dcos(Obl) + dcos(0)*dsin(Obl)*dsin(Th1));
    
    //  Data Mesic
    double F = rozsah(93.2721 + 483202 * t - 0.003403 * t* t - t * t * t/3526000.0);
    double L2 = rozsah(218.316 + 481268 * t);
    double Om2 = rozsah(125.045 - 1934.14 * t + 0.002071 * t * t + t * t * t/450000.0);
    double M2 = rozsah(134.963 + 477199 * t + 0.008997 * t * t + t * t * t/69700.0);
    double D = rozsah(297.85 + 445267 * t - 0.00163 * t * t + t * t * t/545900.0);
    double D2 = 2*D;
    double R2 = 1 + (-20954 * dcos(M2) - 3699 * dcos(D2 - M2) - 2956 * dcos(D2)) / 385000.0;
    double R3 = (R2 / R1) / 379.168831168831;

    
    double Bm = 5.128 * dsin(F) + 0.2806 * dsin(M2 + F);
           Bm = Bm + 0.2777 * dsin(M2 - F) + 0.1732 * dsin(D2 - F);
    double Lm = 6.289 * dsin(M2) + 1.274 * dsin(D2 -M2) + 0.6583 * dsin(D2); 
           Lm = Lm + 0.2136 * dsin(2*M2) - 0.1851 * dsin(M1) - 0.1143 * dsin(2 * F); 
           Lm = Lm +0.0588 * dsin(D2 - 2*M2) ;
           Lm = Lm + 0.0572* dsin(D2 - M1 - M2) + 0.0533* dsin(D2 + M2);
           Lm = Lm + L2;
    double Ra2 = (datan2(dsin(Lm) * dcos(Obl) - dtan(Bm)* dsin(Obl), dcos(Lm))) / 15.0;
    double Dec2 = dasin(dsin(Bm)* dcos(Obl) + dcos(Bm)*dsin(Obl)*dsin(Lm));



    double HLm = rozsah(LaM1 + 180 + (180/PI) * R3 * dcos(Bm) * dsin(LaM1 - Lm));
    double HBm = R3 * Bm;
          
          //  Selenografické koordinaty vzhledem k Zemi
          //  dava geocentrickou libraci
          //  librace ignoruji nutaci 
    double I = 1.54242;
    double W = Lm - Om2;
    double qY = dcos(W) * dcos(Bm);
    double qX = dsin(W) * dcos(Bm) * dcos(I) - dsin(Bm) * dsin(I);
    double A = datan2(qX, qY);
    double EL = A - F;
    double EB = dasin(-dsin(W) * dcos(Bm) * dsin(I) - dsin(Bm) * dcos(I));
          
          //  Selenograficke koordinaty vzhledem k Slunci. 
          //  Popisuje prubeh terminatoru
           W = rozsah(HLm - Om2);
           qY = dcos(W) * dcos(HBm);
           qX = dsin(W) * dcos(HBm) * dcos(I) - dsin(HBm) * dsin(I);
           A = datan2(qX, qY);
    double SL = rozsah(A - F);
    double SB = dasin(-dsin(W) * dcos(HBm) * dsin(I) - dsin(HBm) * dcos(I));
    double Co = 0;
           if (SL < 90)  Co = 90 - SL;
           else          Co = 450 - SL;


           //  Vypocita osvetlenou cast
           A = dcos(Bm) * dcos(Lm - LaM1);
    double Psi = 90 - datan(A / sqrt(1- A*A));
           qX = R1 * dsin(Psi);
           qY = R3 - R1* A;
    double Il = datan2(qX, qY);
    double K = (1 + dcos(Il))/2;

    
    Mes_dist = R2 * 60.268511 * 6.378;
    Mes_RA = Ra2;
    Mes_DE = Dec2;
    Mes_colongitudo = Co;
    Mes_osvit = K*100;

    Slu_RA = Ra1 / 15.0;
    Slu_DE = DEC1;

  }
//---------------------------------------------------------


// VSTUP: globalni promenne Geo souradnice + lokalni cas
// VYSTUP: globalni promenne Slu_azimut, Slu_elelevace, Mes_azimut, Mes_elevace, MS_angle
void Pozice_Slunce(void)
  {

    astro_LOC_rok = LOC_rok;
    astro_LOC_mes = LOC_mes;
    astro_LOC_den = LOC_den;
    astro_LOC_hod = LOC_hod;
    astro_LOC_min = LOC_min;
    z_Astro_LOC_na_Astro_UTC(casova_zona);

    M_souradnice (1 , casova_zona, GeoLon, GeoLat );
    M_souradnice (2 , casova_zona, GeoLon, GeoLat );
      
      
    MS_angle = dacos((dcos(Mes_elevace) * dcos(Slu_elevace) * dcos(Mes_azimut - Slu_azimut)) + (dsin(Mes_elevace) * dsin(Slu_elevace)));
    
  }



//---------------------------------------------------------
// ruzna aritmetika - opsano nekde z internetu (prevedeno z BASICu do PHP a pak sem do Wiringu)
//   asi by to slo zjednodusit 
double rozsah(double x)
  {
    double b = x / 360.0;
    double a = 360 * (b - ipart(b));
    if (a  < 0 )
      {
        a = a + 360;
      }
  return a;
  }


double ipart(double x)
  {
    double a;
    if (x > 0)
      {
        a = floor(x);
      }
    else
      {
        a = ceil(x);
      } 
  return a;
  }



double dsin(double x)
  {
    return sin(PI / 180.0 * x);
  }
  
double dcos( double x)
  {
    return cos(PI / 180.0 * x);
  }

double dtan( double x)
  {
    return tan(PI / 180.0 * x);
  }


//   počet dnů od J2000
double day2000(int y, int m, int d, double h)
  {
    unsigned long greg = y*10000 + m*100 + d;
    if (m == 1 || m == 2)
      {
        y = y - 1;
        m = m + 12;
      }

    double a = floor(y/100);
    double b = 2 - a  + floor(a/4);
      
    int c = floor(365.25 * y);
    int d1 = floor(30.6001 * (m + 1));
    return ( b + c + d1 -730550.5 + d + h/24.0);
  } 



double datan2(double y, double x)
  {
    double a;
    if ((x == 0) && (y == 0))
      {
        return 0;
      }
    else
      {
        a = datan(y / x);
        if (x < 0)
          {
            a = a + 180; 
          }
        if (y < 0 && x > 0)
          {
            a = a + 360;
          }
        return a;
      }
  }



double dasin( double x)
  {
    return 180/ PI * asin(x);
  }

double dacos( double x)
  {
    return 180/ PI * acos(x);
  }
  
double datan(double x)
  {
    return 180/ PI * atan(x);
  }



//---------------------------------------------------------
//  prevede Rektascenzi a Deklinaci na Azimut a Elevaci (podle lokalniho casu, mistni casove zony a zemepisnych souradnic)
void RaToAzim( byte teleso , double qRA, double qDECL, int qrok, byte qmes, byte qden, byte qhod, byte qmin , byte qutcdif, double qlong, double qlat)
  {
    double qcas = qhod + (qmin / 60.0);
    double dj2000 = floor(day2000(qrok, qmes, qden, qhod +12)) + ((qcas - qutcdif) / 24.0) - 0.5;
    double lst = 100.46 + 0.985647 * dj2000 + qlong + 15.0 * (qcas - qutcdif);
  
  
    while (lst <= 0)
      {
        lst = lst + 360;
      }
    
    while (lst >= 360)
      {
        lst = lst - 360;
      }
  
  
  
    double ha = lst - qRA * 15.0;
  
    
    while (ha <= 0)
      {
        ha = ha + 360;
      }
    
    while (ha >= 360)
      {
        ha = ha - 360;
      }
  
    double alt = dasin(dsin(qDECL) * dsin(qlat) + dcos(qDECL) * dcos(qlat) * dcos(ha));
    double azm = datan2(dsin(ha), dcos(ha) * dsin(qlat) - dtan(qDECL) * dcos(qlat)) + 180;
    if (azm >= 360) 
     {
      azm = azm - 360;
     }

     if (teleso == 1)      // teleso je Mesic
       {
         Mes_azimut = azm;
         Mes_elevace = alt;
       }

     if (teleso == 2)      // teleso je Slunce
       {
         Slu_azimut = azm;
         Slu_elevace = alt;
       }


          
  }



void velky_graf_VZ(unsigned int az_vy , unsigned int az_za, unsigned int az_ak)
  {
    unsigned int x_bod1;
    unsigned int y_bod1;
    
    zpozdeni_podsvetu = KON_ZPOZD_PODSV;
    iwdg_feed();                                     // obcerstveni WD    

    // cara pro azimut vychodu 
    for (float stupne = 0; stupne < 3 ; stupne = stupne + 0.1)  // tloustka cary bude 3 stupne po desetine stupne
      {
        x_bod1 = 120 - (dsin(az_vy + stupne - 1)  * 107);
        y_bod1 = 206 + (dcos(az_vy + stupne - 1)  * 107);
        Serial2.print(EEPROM_text_read(144));         // "line "
        Serial2.print("120,206,");
        Serial2.print(x_bod1);
        Serial2.print(',');
        Serial2.print(y_bod1);
        Serial2.print(',');
        Serial2.print("2016");                        // zelena     
        SerialFFF();
      }
    
    // cara pro azimut zapadu 
    for (float stupne = 0; stupne < 3 ; stupne = stupne + 0.1)  // tloustka cary bude 3 stupne po desetine stupne
      {
        x_bod1 = 120 - (dsin(az_za + stupne - 1)  * 107);
        y_bod1 = 206 + (dcos(az_za + stupne - 1)  * 107);
        Serial2.print(EEPROM_text_read(144));         // "line "
        Serial2.print("120,206,");
        Serial2.print(x_bod1);
        Serial2.print(',');
        Serial2.print(y_bod1);
        Serial2.print(',');
        Serial2.print("63519");                       // fialova    
        SerialFFF();
      }


    // cara pro aktualni azimut Mesice
    for (float stupne = 0; stupne < 3 ; stupne = stupne + 0.1)  // tloustka cary bude 3 stupne po desetine stupne
      {
        x_bod1 = 120 - (dsin(az_ak + stupne - 1)  * 70);        // cara aktualniho azimutu je kratsi
        y_bod1 = 206 + (dcos(az_ak + stupne - 1)  * 70);
        Serial2.print(EEPROM_text_read(144));         // "line "
        Serial2.print("120,206,");
        Serial2.print(x_bod1);
        Serial2.print(',');
        Serial2.print(y_bod1);
        Serial2.print(',');
        Serial2.print("2047");                        // svetle modra     
        SerialFFF();
      }
    
    
    zpozdeni_podsvetu = KON_ZPOZD_PODSV;    
  }




void kruhovy_graf(unsigned int az_vy , unsigned int az_za , unsigned int az_akt)
  {
    
    zpozdeni_podsvetu = KON_ZPOZD_PODSV;
    iwdg_feed();                                     // obcerstveni WD    
        
    for (unsigned int xy_azimut = az_vy; xy_azimut < az_za+1 ; xy_azimut = xy_azimut + 2)
      {
        unsigned int x_bod1 = 337 - (dsin(xy_azimut)  * 28);
        unsigned int y_bod1 = 298 + (dcos(xy_azimut)  * 28);
        unsigned int x_bod2 = 337 - (dsin(xy_azimut)  * 22);
        unsigned int y_bod2 = 298 + (dcos(xy_azimut)  * 22);
        Serial2.print(EEPROM_text_read(144));         // "line "
        Serial2.print(x_bod1);
        Serial2.print(',');
        Serial2.print(y_bod1);
        Serial2.print(',');
        Serial2.print(x_bod2);
        Serial2.print(',');
        Serial2.print(y_bod2);
        Serial2.print(',');
        Serial2.print("RED");                   
        SerialFFF();
      }

    iwdg_feed();                                     // obcerstveni WD    
    
    for (unsigned int xy_azimut = az_akt-2; xy_azimut < az_akt+3 ; xy_azimut ++)
      {
        if (xy_azimut >= 360) xy_azimut = xy_azimut - 360;

        unsigned int x_bod1 = 337 - (dsin(xy_azimut)  * 28);
        unsigned int y_bod1 = 298 + (dcos(xy_azimut)  * 28);
        unsigned int x_bod2 = 337 - (dsin(xy_azimut)  * 22);
        unsigned int y_bod2 = 298 + (dcos(xy_azimut)  * 22);

        if (y_bod1 > 319)
          {
            x_bod2 = 337 - (dsin(xy_azimut)  * 17);
            y_bod2 = 298 + (dcos(xy_azimut)  * 17);
            y_bod1 = 298 + (dcos(xy_azimut)  * 21);
            x_bod1 = 337 - (dsin(xy_azimut)  * 21);
          }
        
        
        Serial2.print(EEPROM_text_read(144));         // "line "
        Serial2.print(x_bod1);
        Serial2.print(',');
        Serial2.print(y_bod1);
        Serial2.print(',');
        Serial2.print(x_bod2);
        Serial2.print(',');
        Serial2.print(y_bod2);
        Serial2.print(',');
        Serial2.print("YELLOW");                   
        SerialFFF();
    
      }    
    iwdg_feed();                                     // obcerstveni WD    

    
    zpozdeni_podsvetu = KON_ZPOZD_PODSV;
  }
















void sluncedet(void)
  {
    astro_LOC_rok = LOC_rok;
    astro_LOC_mes = LOC_mes;
    astro_LOC_den = LOC_den;
    astro_LOC_hod = LOC_hod;
    astro_LOC_min = LOC_min;
    z_Astro_LOC_na_Astro_UTC(casova_zona);

    M_souradnice (2 , casova_zona, GeoLon, GeoLat );
      
    Serial2.print("az.txt=\"");
    Serial2.print((int)(Slu_azimut + 0.5));
    Serial2.print('\"');
    SerialFFF();

    Serial2.print("el.txt=\"");
    Serial2.print((int)(Slu_elevace+0.5));
    Serial2.print('\"');
    SerialFFF();


    sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 1);
    int s_r_d_homi = (int)((sunrisex * 60) + 0.5);        // (sun_rise_dnes)
    int s_s_d_homi = (int)((sunsetx * 60) + 0.5);         // (sun_set_dnes)

    int s_delka_dne = s_s_d_homi - s_r_d_homi;


    Serial2.print("ded.txt=\"");
    Serial2.print(s_delka_dne / 60);
    Serial2.print(" hod. ");
    Serial2.print(s_delka_dne % 60);
    Serial2.print(" min. ");
    Serial2.print('\"');
    SerialFFF();    


    astro_LOC_hod = (int)sunrisex;
    astro_LOC_min = (sunrisex - astro_LOC_hod) * 60;
    z_Astro_LOC_na_Astro_UTC(casova_zona);
    M_souradnice (2 , casova_zona, GeoLon, GeoLat );

    Serial2.print("vaz.txt=\"");
    Serial2.print((int)(Slu_azimut + 0.5));
    Serial2.print('\"');
    SerialFFF();    

    astro_LOC_hod = (int)sunsetx;
    astro_LOC_min = (sunsetx - astro_LOC_hod) * 60;  
    z_Astro_LOC_na_Astro_UTC(casova_zona);
    M_souradnice (2 , casova_zona, GeoLon, GeoLat );


    Serial2.print("zaz.txt=\"");
    Serial2.print((int)(Slu_azimut + 0.5));
    Serial2.print('\"');
    SerialFFF();    

    Serial2.print("v1.txt=\"");
    Serial2.print(hhh_to_hmin(sunrisex));
    Serial2.print('\"');
    SerialFFF();

    Serial2.print("z1.txt=\"");
    Serial2.print(hhh_to_hmin(sunsetx));
    Serial2.print('\"');
    SerialFFF();

    
    plusden_LOC();

    sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat , 1);
    int s_r_z_homi = (int)((sunrisex * 60) + 0.5);        // (sun_rise_zitra)
    int s_s_z_homi = (int)((sunsetx * 60) + 0.5);         // (sun_set_zitra)

    Serial2.print("v2.txt=\"");
    Serial2.print(hhh_to_hmin(sunrisex));
    Serial2.print('\"');
    SerialFFF();

    Serial2.print("z2.txt=\"");
    Serial2.print(hhh_to_hmin(sunsetx));
    Serial2.print('\"');
    SerialFFF();


    Serial2.print("rv.txt=\"");
    Serial2.print(s_r_z_homi - s_r_d_homi);
    Serial2.print(" min\"");
    SerialFFF();

    Serial2.print("rz.txt=\"");
    Serial2.print(s_s_z_homi - s_s_d_homi);
    Serial2.print(" min\"");
    SerialFFF();
    
  }





void sunevent(int fxYear, int fxMonth, int fxDay, int tz, double glong, double glat , byte typ_soumraku)
  {
    double yz, yp, utrise, utset;
    double rads  = 1.74532925199433E-02;
    
    double sinho1;
    
    switch (typ_soumraku) 
      {
        case 1:
          sinho1 = sin(rads * -0.833);   // vychod / zapad
          break;
        case 2:
          sinho1 = sin(rads * -6);      // obcansky soumrak
          break;
        case 3:
          sinho1 = sin(rads * -12);     // namorni soumrak
          break;
        case 4:
          sinho1 = sin(rads * -18);     // astronomicky soumrak
          break;
      }  




  
  
    double sglat = sin(rads * glat);
    double cglat = cos(rads * glat);
    double ddate = mjd(fxYear, fxMonth, fxDay) - tz / 24.0;    
    double nz = 0;
    double z1 = 0;
    double z2 = 0;
    double xe = 0;
    double ye = 0;
    byte Rise = 0;
    byte sett = 0;
    byte above = 0;
    byte houro = 1;
    double ym = SinAltSun(ddate, (houro - 1), glong, cglat, sglat) - sinho1;
    
    if (ym > 0)    above = 1;
  
    while (houro < 25 and (sett == 0 or Rise == 0))
     {
      yz = SinAltSun(ddate,   houro   , glong, cglat, sglat) - sinho1;
      yp = SinAltSun(ddate,(houro + 1), glong, cglat, sglat) - sinho1;
      quad (ym, yz, yp, nz, z1, z2, xe, ye);

      if (s_nz == 1)
        {
          if (ym < 0) 
            {
              utrise = houro + s_z1;
              Rise = 1;
            }
          else
            {
              utset = houro + s_z1;
              sett = 1;
            }
        }
      if (s_nz == 2) 
        {
          if (s_ye < 0) 
            {
              utrise = houro + s_z2;
              utset  = houro + s_z1;
            }
          else
            {
              utrise = houro + s_z1;
              utset  = houro + s_z2;
            }
          
          Rise = 1;
          sett = 1;
        }
      ym = yp;
      houro = houro + 2;
    }
  
    if (Rise == 1)
      {
        sunrisex = utrise;          
      }
    else                      // Slunce nevychazi, nebo soumrak nenastava
      {
        sunrisex = 9999;
      }
      
    if (sett == 1)
      {
        sunsetx = utset;        
      }
    else                      // Slunce nezapada, nebo soumrak nenastava
      {    
        sunsetx  = 9999;
      }
    
    
  }


//-------------------------------------
//  prevede cas z formatu hodin v desetinnem cisle na retezec "hh:mm" vcetne nul pred minutami, nebo mezer pred hodinami
//   priklady hhh = 15.05         vystup = "15:03"
//            hhh = 6.8           vystup = " 6:48"
String hhh_to_hmin(double hhh)
  {
    String hmin = "00:00";
    if (hhh == 9999)
      {
        hmin = "--:--";
      }
    else
      {
        byte hhh_hod = (int)hhh;
        byte hhh_min = (int)(((hhh - hhh_hod ) * 60));
        if (hhh_hod < 10 )
          {
            hmin = ' ' + String(hhh_hod) + ':';
          }
        else
          {
            hmin = String(hhh_hod) + ':';
          }
    
        if (hhh_min < 10 )
          {
            hmin = hmin + '0' + String(hhh_min);
          }
        else
          {
            hmin = hmin + String(hhh_min);
          }
        return hmin;
        
      }
    
    
  }



double SinAltSun( double mjd0, double houro, double glong, double cglat, double sglat)
{
  double rads  = 1.74532925199433E-02;
  double mjdxx = mjd0 + houro / 24;
  double t = (mjdxx - 51544.5) / 36525.0;
  minisun (t);
  
  double tau = 15.0 * (lmst(mjdxx, glong) - sunra);
  double salt = sglat * sin(rads * sundec) + cglat * cos(rads * sundec) * cos(rads * tau);
  return salt;
}



void minisun(double t)
  {
   
    double p2 = 6.283185307;
    double coseps = 0.91748;
    double sineps = 0.39778;
    double m = p2 * frac(0.993133 + 99.997361 * t);
    double DL = 6893.0 * sin(m) + 72 * sin(2 * m);
    double l = p2 * frac(0.7859453 + m / p2 + (6191.2 * t + DL) / 1296000.0);
    double SL = sin(l);
    double sunx = cos(l);
    double suny = coseps * SL;
    double z = sineps * SL;
    double RHO = sqrt(1 - z * z);
    sundec = (360 / p2) * atan(z / RHO);
    sunra = (48 / p2) * atan(suny / (sunx + RHO));
    if (sunra < 0)
    {
      sunra = sunra + 24;
    }


  
  }

double frac(double promX)
  {
    return promX - ipart(promX);
  }



double lmst(double mjd, double glong)
  {
    double d = mjd - 51544.5;
    double t = d / 36525.0;
    double lst = rozsah(280.46061837 + 360.98564736629 * d + 0.000387933 * t * t - t * t * t / 38710000.0);
    return lst / 15.0 + glong / 15.0;
  }

double mjd(int fxYear, int fxMonth, int fxDay)
  {  
    if (fxMonth <= 2) 
    {
      fxMonth = fxMonth + 12;
      fxYear = fxYear - 1;
    }
    double b = ipart(fxYear / 400) - ipart(fxYear / 100) + ipart(fxYear / 4);
    double a = 365 * fxYear - 679004;
    return a + b + ipart(30.6001 * (fxMonth + 1)) + fxDay;
  }



void quad(double ym, double yz, double yp, double nz, double z1, double z2, double xe, double ye)
  {
    double dx;
    nz = 0;
    double a = 0.5 * (ym + yp) - yz;
    double b = 0.5 * (yp - ym);
    double c = yz;
    xe = -b / (2 * a);
    ye = (a * xe + b) * xe + c;
    double dis = b * b - 4 * a * c;
    if (dis > 0) 
      {
        dx = 0.5 * sqrt(dis) / abs(a);
        z1 = xe - dx;
        z2 = xe + dx;
        if (abs(z1) <= 1)
         {
           nz = nz + 1;
         }
       if (abs(z2) <= 1) 
         {
           nz = nz + 1;
         }
       if (z1 < -1) 
         {
           z1 = z2;
         }
      }

    s_z1 = z1;
    s_z2 = z2;
    s_nz = nz;
    s_xe = xe;
    s_ye = ye;


  }


void soumraky(void)
  {
      Serial2.print("page 28");
      SerialFFF();

      // je po 9:00, zacne se pocitat dnesni zapad a zitrejsi vychod
      astro_LOC_rok = LOC_rok;
      astro_LOC_mes = LOC_mes;
      astro_LOC_den = LOC_den;
      astro_LOC_hod = LOC_hod;
      astro_LOC_min = LOC_min;
      z_Astro_LOC_na_Astro_UTC(casova_zona);

      if (astro_LOC_hod >= 9)        // pro 9:00 az pulnoc se zobrazuji dnesni soumraky a zitrejsi rozbresky
        {
          Serial2.print("dd.txt=\"");
          Serial2.print(astro_LOC_den);
          Serial2.print('.');
          Serial2.print(astro_LOC_mes);
          Serial2.print(".\"");
          SerialFFF();
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 1);
          Serial2.print("za.txt=\"");
          Serial2.print(hhh_to_hmin(sunsetx));         // dnesni zapad Slunce
          Serial2.print('\"');
          SerialFFF();      
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 2);
          Serial2.print("so.txt=\"");
          Serial2.print(hhh_to_hmin(sunsetx));         // dnesni obcansky soumrak
          Serial2.print('\"');
          SerialFFF();
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 3);
          Serial2.print("sn.txt=\"");
          Serial2.print(hhh_to_hmin(sunsetx));         // dnesni namorni soumrak
          Serial2.print('\"');
          SerialFFF();
          
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 4);
          Serial2.print("sa.txt=\"");
          Serial2.print(hhh_to_hmin(sunsetx));         // dnesni astronomicky soumrak
          Serial2.print('\"');
          SerialFFF();
          
    
          // zitrejsi vychody a usvity
          
          plusden_LOC();
    
          Serial2.print("dz.txt=\"");
          Serial2.print(astro_LOC_den);
          Serial2.print('.');
          Serial2.print(astro_LOC_mes);
          Serial2.print(".\"");
          SerialFFF();
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 1);
          Serial2.print("vy.txt=\"");
          Serial2.print(hhh_to_hmin(sunrisex));         // zitrejsi vychod Slunce
          Serial2.print('\"');
          SerialFFF();      
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 2);
          Serial2.print("uo.txt=\"");
          Serial2.print(hhh_to_hmin(sunrisex));         // zitrejsi obcansky usvit
          Serial2.print('\"');
          SerialFFF();      
          
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 3);
          Serial2.print("un.txt=\"");
          Serial2.print(hhh_to_hmin(sunrisex));         // zitrejsi namorni usvit
          Serial2.print('\"');
          SerialFFF();      
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 4);
          Serial2.print("ua.txt=\"");
          Serial2.print(hhh_to_hmin(sunrisex));         // zitrejsi astronomicky usvit
          Serial2.print('\"');
          SerialFFF();      
          
        }
      else     // pro pulnoc az 9:00 se zobrazuji dnesni rozbresky a nasledne dnesni soumraky
        {      //              (aby odpovidaly popisy ve sloupcich, jsou vlevo zobrazeny vecerni soumraky a az pak ranni usvity toho sameho dne)
          Serial2.print("dz.txt=\"");
          Serial2.print(astro_LOC_den);
          Serial2.print('.');
          Serial2.print(astro_LOC_mes);
          Serial2.print(".\"");
          SerialFFF();
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 1);
          Serial2.print("vy.txt=\"");
          Serial2.print(hhh_to_hmin(sunrisex));         // dnesni vychod Slunce
          Serial2.print('\"');
          SerialFFF();      
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 2);
          Serial2.print("uo.txt=\"");
          Serial2.print(hhh_to_hmin(sunrisex));         // dnesni obcansky usvit
          Serial2.print('\"');
          SerialFFF();      
          
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 3);
          Serial2.print("un.txt=\"");
          Serial2.print(hhh_to_hmin(sunrisex));         // dnesni namorni usvit
          Serial2.print('\"');
          SerialFFF();      
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 4);
          Serial2.print("ua.txt=\"");
          Serial2.print(hhh_to_hmin(sunrisex));         // dnesni astronomicky usvit
          Serial2.print('\"');
          SerialFFF();  

          Serial2.print("dd.txt=\"");
          Serial2.print(astro_LOC_den);
          Serial2.print('.');
          Serial2.print(astro_LOC_mes);
          Serial2.print(".\"");
          SerialFFF();
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 1);
          Serial2.print("za.txt=\"");
          Serial2.print(hhh_to_hmin(sunsetx));         // dnesni zapad Slunce
          Serial2.print('\"');
          SerialFFF();      
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 2);
          Serial2.print("so.txt=\"");
          Serial2.print(hhh_to_hmin(sunsetx));         // dnesni obcansky soumrak
          Serial2.print('\"');
          SerialFFF();
    
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 3);
          Serial2.print("sn.txt=\"");
          Serial2.print(hhh_to_hmin(sunsetx));         // dnesni namorni soumrak
          Serial2.print('\"');
          SerialFFF();
          
          sunevent(astro_LOC_rok, astro_LOC_mes, astro_LOC_den, casova_zona, GeoLon, GeoLat, 4);
          Serial2.print("sa.txt=\"");
          Serial2.print(hhh_to_hmin(sunsetx));         // dnesni astronomicky soumrak
          Serial2.print('\"');
          SerialFFF();

                               
        }
   
  }








