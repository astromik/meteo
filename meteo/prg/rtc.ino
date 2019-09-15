
//-------------------------------------------------------
//  Nastaveni casu v RTC obvodu
//  Po prvnim znaku 'T', ktery zpusobil skok do tohoto podprogramu, ocekava na seriove lince retezec ve tvaru "yyyymmddhhnnssW"
void nastav_RTC(void)
  {
    boolean chyba = false;  // bude provadeno testovani na korektni meze vlozenych cisel. Kdyz dojde k prekroceni mezi, nastavi se znacka 'chyba' na true
  
    pauza100();           // pauza na prijeti vsech znaku ze seriove linky
    if (Serial.available() < 20)           // pro nastaveni casu se ocekava 15 znaku: "YYYYMMDDHHNNSSW"  (W je den v tydnu 1 az 7 pro Po az Ne)
      {
    
        i = 0;
        while (Serial.available() > 0)      // vsechny prijate znaky se ulozi do pole
          {
            prijato[i] = Serial.read();
            i++;
          }

        Serial.print(EEPROM_text_read(285));  // "Prijato:"
        Serial.println(prijato);


      
        // rok
        Serial.print(EEPROM_text_read(22));  // "Rok:"
    
        LOC_rok = prijato_int(0, 4);
        if (LOC_rok > 2099 || LOC_rok < 2016)// test, ze je rok v rozsahu 2016 az 2099
          {
            chyba = true;
          }

    
        Serial.println(LOC_rok);
    
    
        // mesic
        Serial.print(EEPROM_text_read(23));  // "Mesic:"
        LOC_mes = prijato_int(4, 6);
        if (LOC_mes > 12 || LOC_mes < 1)     // test, ze je mesic v rozsahu 1 az 12
          {
            chyba = true;
          }
        Serial.println(LOC_mes);
    
    
        // den
        Serial.print(EEPROM_text_read(24));  // "Den:"
        LOC_den = prijato_int(6, 8);
        if (LOC_den > 31 || LOC_den < 1)     // test, ze je den v rozsahu 1 az 31 (nebere se ohled na to, kolik ma mesic ve skutecnosti dni)
          {
            chyba = true;
          }
        Serial.println(LOC_den);
    
        // hodina
        Serial.print(EEPROM_text_read(25));  // "Hodiny:"
        LOC_hod = prijato_int(8, 10);
        if (LOC_hod > 23 || LOC_hod < 0)     // test, ze jsou hodiny v rozsahu 0 az 23
          {
            chyba = true;
          }
        Serial.println(LOC_hod);
    
        // minuta
        Serial.print(EEPROM_text_read(26));  // "Minuty:"
        LOC_min = prijato_int(10, 12);
        if (LOC_min > 59 || LOC_min < 0)     // test, ze jsou minuty v rozsahu 0 az 60
          {
            chyba = true;
          }
        Serial.println(LOC_min);
    
    
        // sekundy
        Serial.print(EEPROM_text_read(27));  // "Sekundy:"
        LOC_sek = prijato_int(12, 14);
        if (LOC_sek > 59 || LOC_sek < 0)     // test, ze jsou sekundy v rozsahu 0 az 60
          {
            chyba = true;
          }
        Serial.println(LOC_sek);
    
    
        // dny v tydnu
        Serial.print(EEPROM_text_read(17));  // "Den v tydnu:"
        LOC_dvt = prijato[14] - '0';
        if (LOC_dvt > 7 || LOC_dvt < 1)      // test, ze je den v rozsahu 1 az 7 (Po az Ne)
          {
            chyba = true;
          }
        Serial.println(dny[LOC_dvt]);
    
    
      }
    else       // kdyz po seriove lince dorazil jiny pocet znaku, nez 15, tak se vsechno z bufferu 'vysype'
      {
        Serial.flush();
        chyba = true;
      }
  
    if (chyba == true)                              // kdyz je nejaka hodnota mimo povoleny rozsah, nebo neprisel spravny pocet znaku ...
      {
        Serial.println (EEPROM_text_read(18));      // zahlasi se " !! CHYBA !! "
      }
    else                                            // kdyz je vsechno v poradku, zadane promenne se zapisou do RTC
      {
        setDS3231time(LOC_sek, LOC_min, LOC_hod, LOC_den, LOC_mes, LOC_rok, LOC_dvt);  // knihovni funkce pro zapis do RTC obvodu
        priste_prohledavat_od = 0; // po zmene casu v RTC se musi prohledat databaze fazi od zacatku

      }
  
  
  }




//---------------------------------------------------------------------------------------------------
//  podprogramy pro RTC - prevzate z prikladu pro knihovnu DS3231
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
  {
    return ( (val / 10 * 16) + (val % 10) );
  }

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
  {
    return ( (val / 16 * 10) + (val % 16) );
  }


void setDS3231time(byte second, byte minute, byte hour, byte dayOfMonth, byte month, byte year, byte dvt)
  {
    // sets time and date data to DS3231
    Wire.beginTransmission(I2C_ADDR_DS3231);
    Wire.write(0); // set next input to start at the seconds register
    Wire.write(decToBcd(second)); // set seconds
    Wire.write(decToBcd(minute)); // set minutes
    Wire.write(decToBcd(hour)); // set hours
    Wire.write(dvt); // set day of week (1=Pondeli, 7=nedele)
    Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
    Wire.write(decToBcd(month)); // set month
    Wire.write(decToBcd(year - 2000)); // set year (0 to 99)
    Wire.endTransmission();
  }

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
  {
    Wire.beginTransmission(I2C_ADDR_DS3231);
    Wire.write(0); // set DS3231 register pointer to 00h
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR_DS3231, 7);
    // request seven bytes of data from DS3231 starting from register 00h
    *second = bcdToDec(Wire.read() & 0x7f);
    *minute = bcdToDec(Wire.read());
    *hour = bcdToDec(Wire.read() & 0x3f);
    *dayOfWeek = bcdToDec(Wire.read());
    *dayOfMonth = bcdToDec(Wire.read());
    *month = bcdToDec(Wire.read());
    *year = bcdToDec(Wire.read());
  }


// doladeni frekvence krystalu
void setRTC_aging(byte aging)
  {
    // nejdriv se precte puvodni nastaveni Aging registru
    Wire.beginTransmission(I2C_ADDR_DS3231);
    Wire.write(0x10);                        // adresa Aging registru
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR_DS3231, 1);
    Serial.print("Puvodni Aging: ");
    Serial.println(Wire.read());

    // a pak se do registru zapise nova hodnota
    Wire.beginTransmission(I2C_ADDR_DS3231);
    Wire.write(0x10);        // adresa aging registru
    Wire.write(aging);       // zapis hodnoty  do registru
    Wire.endTransmission();
  }



// Protoze se nekdy muze stat, ze se RTC obvod zblazni (napriklad pri prepolovani napajeni) je tu moznost uvest ho do zakladniho nastaveni
//  Pro jistotu se spousti automaticky pred kazdym nastavenim casu
void RTC_default(void)
  {
    Wire.begin();
    Wire.beginTransmission(I2C_ADDR_DS3231);
    Wire.write(0x0E);                            // Control Register
    Wire.write(0b01000000);                      // spustit oscilator a generovat 1Hz signal na SQW pinu
    Wire.endTransmission();
    Wire.beginTransmission(I2C_ADDR_DS3231);
    Wire.write(0x0F);                            // Status Register
    Wire.write(0x00);
    Wire.endTransmission();
  }


//---------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------
// - zjisti hodnoty casu z RTC a ulozi je do promennych "LOC_xxx".
// - prevede hodnoty  lokalniho casu na promenne "UTC_xxx  "
// - pripravi k zobrazeni na displeji citelne datum a cas v promennych "dat_str" a "tim_str"
// - vypocte se rozdil casu v minutach od 1.1.2016 (pro UTC cas) a ulozi se do promenne "minuty2016UTC"

void displayTime(void)
  {
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    // retrieve data from DS3231
    readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  
    LOC_rok = 2000 + year;
    LOC_mes = month;
    LOC_den = dayOfMonth;
    LOC_hod = hour;
    LOC_min = minute;
    LOC_sek = second;
    LOC_dvt = dayOfWeek;
  
    z_LOC_na_UTC(casova_zona);                       // mistni cas, ktery je ulozeny v RTC prevede na aktualni UTC (do promennych "UTC_xxx")
  
  
    dat_str = "";                                   // priprava retezce s citelnym datumem
  
    if (LOC_den < 10 ) dat_str = dat_str + ' ';
    dat_str = dat_str + LOC_den + '.';
    if (LOC_mes < 10 ) dat_str = dat_str + ' ';
    dat_str = dat_str  + LOC_mes + '.';
    DMhn = dat_str + ' ';                           // format datumu a casu, ktery se pouziva pro zaznam minimalni a maximalni teploty ("DD.MM. hh:nn")
    dat_str = dat_str + LOC_rok;
  
    tim_str = "";                                   // priprava retezce s citelnym casem
    if (LOC_hod < 10 ) tim_str = tim_str + ' ';
    tim_str = tim_str + LOC_hod + ':';
    if (LOC_min < 10 ) tim_str = tim_str + '0';
    DMhn = DMhn + tim_str + LOC_min;               // format datumu a casu, ktery se pouziva pro zaznam minimalni a maximalni teploty ("DD.MM. hh:nn")
    tim_str = tim_str + LOC_min + ':';
    if (LOC_sek < 10 ) tim_str = tim_str + '0';
    tim_str = tim_str + LOC_sek ;
  
  
    // pocet dni od zacatku roku pro aktualni mesic
    // napriklad pri datumu 25.4.  se vezme mesic duben (index 3) a k nemu se pripocte pocet CELYCH dni v mesici (25) takze vysledkem je 119+25 = 145 celych (dokoncenych) dni
    // mesic          Led   Uno   Bre   Dub   Kve   Cer   Cec   Srp   Zar   Rij  Lis   Pro
    // index           0     1     2     3     4     5     6     7     8     9    10    11
    int  pomdny[] = { -1,   30,   58,   89,  119,  150,  180,  211,  242,  272,  303,  333 };
  
    byte prestupneroky = ((UTC_rok - 2016)  / 4) + 1;       // pocet celych prestupnych roku od 2016 (muze to byt vcetne aktualniho roku) (2016 je prestupny, tak musi byt +1)
  
    if (UTC_rok % 4 == 0  && UTC_mes < 3)                   // kdyz je aktualni rok prestupny a kdyz je v tomto prestupnem roce aktualni mesic leden, nebo unor
      {
        prestupneroky --;                                   // tak se tento jeden prestupny rok nezapocitava (prestupny den je az na konci unora)
      }
  
    minuty2016UTC = ((UTC_rok - 2016) * 365UL) + prestupneroky;       // pocet dni za uplynule KOMPLETNI CELE roky od 1.1.2016  (pro rok 2016 je vysledek 0, pro rok 2017 je vysledek 366...)
    minuty2016UTC = minuty2016UTC + pomdny[UTC_mes - 1] + UTC_den;    // k tomu se pricte pocet dokoncenych dni od zacatku roku (napr: 10.1.2017 da vysledek 366 + 9)
  
    minuty2016UTC = (minuty2016UTC * 24UL) + UTC_hod;                 // vysledne dny se prepoctou na hodiny a pridaji se "dnesni" hodiny od pulnoci
    minuty2016UTC = (minuty2016UTC * 60UL) + minute;                  // vysledne hodiny se prepoctou na minuty pridaji se minuty z prave probihajici hodiny
  
    if (LOC_rok < 2016)
      {
        tim_str = " 0:00:00";
        dat_str = " 1. 1.0000";
        minuty2016UTC = 0;
      }

    if (zpozdeni_podsvetu > 0)
      {
        T_ser(tim_str);
        D_ser(LOC_dvt,dat_str);    
      }
  }

//---------------------------------------------------------------------------------------------------


// pri prvnim zapnuti RTC, pri vymene baterie, nebo pri nejake chybe, kdyz je v RTC nastaveny rok mimo rozsah 2016 az 2070, tak se nastavi 1.1.2016 00:00:00 Patek
void RTC2016(void)
  {
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
    LOC_rok = 2000 + year;
    if (LOC_rok < 2016 or LOC_rok > 2071)
      {
        setDS3231time(0, 0, 0, 1, 1, 2016, 5);    
      }
  }

