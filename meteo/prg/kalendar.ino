               
                
// zobrazeni kalendare pro poradove cislo mesice (od ledna 2016)
// jako globalni promenna vsupuje do podprogramu 'mes2016'
void zobraz_kalendar()                
  {
    byte KAL_mes;
    unsigned int KAL_rok;
    uint32_t  KAL_min2016;
    byte dvt_konst[] = {0,7,3,3,6,1,4,6,2,5,7,3,5};                 // konstanty pro vypocet 1. dne v mesici
    byte pocetdni[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};      // celkovy pocet dni v mesici
    unsigned int index_databaze;                                    // zaznam v EEPROM, na kterem se nachazi informace o nasledujici fazi Mesice od 1. dne v mesice
    byte typ_faze;
    byte den_s_fazi;
    boolean zatmeni;
    byte prestupny_rok = 0;
    unsigned int index_dne;

    unsigned int svatky[]= { 
                               0,  // {1,1},     // novy rok
                               0,  // {99,99},   // velky patek           (pocitano a dosazeno nize)
                               0,  // {99,99},   // velikonocni pondeli   (pocitano a dosazeno nize)
                             120,  // {1,5},     // svatek prace
                             127,  // {8,5},     // den vitezstvi
                             185,  // {5,7},     // Cyril a Metodej
                             186,  // {6,7},     // Hus
                             270,  // {28,9},    // den statnosti
                             300,  // {28,10},   // vznik Ceskoslovenska
                             320,  // {17,11},   // den boje
                             357,  // {24,12},   // stedry den
                             358,  // {25,12},   // vanoce 1
                             359   // {26,12}    // vanoce 2
                           };


    
    
    
    // pocet dni od zacatku roku pro aktualni mesic
    // napriklad pri datumu 25.4.  se vezme mesic duben (index 3) a k nemu se pripocte pocet CELYCH dni v mesici (25) takze vysledkem je 119+25 = 145 celych (dokoncenych) dni
    // mesic          Led   Uno   Bre   Dub   Kve   Cer   Cec   Srp   Zar   Rij  Lis   Pro
    // index           0     1     2     3     4     5     6     7     8     9    10    11
    int  pomdny[] = { -1,   30,   58,   89,  119,  150,  180,  211,  242,  272,  303,  333 };
  

  



    zpozdeni_podsvetu = KON_ZPOZD_PODSV;

    KAL_rok = (mes2016 / 12) + 2016;
    KAL_mes = (mes2016 % 12) + 1;


    if (KAL_rok % 4 == 0)
      {
        pocetdni[2] = 29;    // pri prestupnem roce se upravuje pocet dni pro unor
        prestupny_rok = 1;    
      }
    
    //----------------------------------------
    // vypocet velikonocni nedele (plati pro roky 1900 az 2099 - s vyjimkou roku 1954 a 1981) 
    // vysledkem je pocet dni od 1.1. prislusneho roku (vcetne pripadneho prestupneho dne)
    // zdroj: http://www.ian.cz/detart_fr.php?id=1739
    unsigned int nedele_od_1_1;
    byte veliko_m = 24;
    byte veliko_n = 5;
    byte veliko_a = KAL_rok % 19;
    byte veliko_b = KAL_rok % 4;
    byte veliko_c = KAL_rok % 7;

    unsigned int veliko_d = ((19 * veliko_a) + veliko_m) % 30;
    unsigned int veliko_e = (veliko_n + (2 * veliko_b) + (4 * veliko_c) + (6 * veliko_d)) % 7;

    if (22 + veliko_d + veliko_e <= 31)    // velikonoce jsou v breznu
      {
        nedele_od_1_1 = 31 + 28 + (22 + veliko_d + veliko_e) ;     // pocet dni v lednu + pocet dni v beznem unoru + pocet dni do breznove nedele
      }
    else                                  // velikonoce jsou v dubnu
      {
        nedele_od_1_1 = 31 + 28 + 31 + (veliko_d + veliko_e - 9) ; // pocet dni v lednu + pocet dni v beznem unoru + cely brezen + pocet dni do dubnove nedele    
      }


    svatky[1] = nedele_od_1_1 - 2 - 1;    // Velikonocni patek    (posledni jednicka se odecita kvuli pocitani dni od nuly (1.1. = 0))
    svatky[2] = nedele_od_1_1 + 1 - 1;    // Velikonocni pondeli  (posledni jednicka se odecita kvuli pocitani dni od nuly (1.1. = 0))
    //----------------------------------------







    // vypocet den v tydnu pro 1. den v mesici
    // https://kalendar.beda.cz/jak-jednoduse-zjistit-den-v-tydnu-pro-dane-datum
    int KAL_pomprom = dvt_konst[KAL_mes] + (KAL_rok - 2000 ) + (KAL_rok - 2000) / 4;
    if ((KAL_rok % 4 == 0) and KAL_mes < 3)  KAL_pomprom --;
    KAL_pomprom = KAL_pomprom % 7;

    // prevod na pondeli = 0 ; utery = 1 .... nedele = 6
    if (KAL_pomprom == 0) KAL_pomprom = 7;
    KAL_pomprom --;

    
    Serial2.print(EEPROM_text_read(259));      // "page 19"       // zobrazeni strany 19 na displeji
    SerialFFF();
    screen = 19;

    Serial2.print(EEPROM_text_read(229));      //"p43.pic="       // obrazek se jmenem mesice (Leden, Unor ...)
    Serial2.print(KAL_mes + 247);
    SerialFFF();

    Serial2.print(EEPROM_text_read(230));      // "t49.txt=\""    // cislo roku 
    Serial2.print(KAL_rok);
    Serial2.print('\"');
    SerialFFF();




    for (i = KAL_pomprom ; i < pocetdni[KAL_mes] + KAL_pomprom ; i++)
      {
        Serial2.print(EEPROM_text_read(231));  // "vis p"        // skryti ikon fazi Mesice
        Serial2.print(i);
        Serial2.print(",0");
        SerialFFF();
        
        Serial2.print(EEPROM_text_read(232));  // "vis t"        // zobrazeni cisel dni
        Serial2.print(i);
        Serial2.print(",1");
        SerialFFF();

    
        Serial2.print('t');
        Serial2.print(i);
        Serial2.print(EEPROM_text_read(227));  // ".txt=\""
        Serial2.print((i-KAL_pomprom)+1);
        Serial2.print('\"');
        SerialFFF();

        Serial2.print('t');      // nastavit barvu fontu pro dny
        Serial2.print(i);

        unsigned int barvadne = 50712;              // zakladem je seda barva

        if ((i % 7) == 5)                           // soboty se nejdriv nastavi na zeleno
          {
            barvadne = 2016;
          }

        if ((i % 7) == 6)                           // nedele se nejdriv nastavi na cerveno (jako svatky)
          {
            barvadne = 63488;
          }

        
        if (LOC_den == ((i-KAL_pomprom)+1) and LOC_rok == KAL_rok and LOC_mes == KAL_mes )   // dnesni den zvyraznit
          {
            barvadne = 2047;                      // dnesni den zvyraznit svetle modrym pozadim (nezavisle na tom, jestli je dneska svatek, sobota, nebo nedele)
          }
        else
          {
            index_dne = (i - KAL_pomprom) + pomdny[KAL_mes-1] + 1 + prestupny_rok;   // index dne od zacatku roku

            for (byte sv = 0; sv < 13 ; sv ++)         // pro kazdy den se projde databaze 13 svatku
              {
                if (index_dne == svatky[sv] + prestupny_rok)
                  {
                    barvadne = 64512;            // svatkum nastavit pozadi na oranzovou (prepisuje i soboty a nedele)
                  }
              }
          }

        Serial2.print(EEPROM_text_read(234));      // ".bco=" 
        Serial2.print(barvadne);
        SerialFFF();
        
      }

    
    if (KAL_pomprom > 0)
      {
        for (i = 0 ; i < KAL_pomprom ; i++)   // zneviditelneni dni z predchoziho mesice
          {
            Serial2.print(EEPROM_text_read(232));  // "vis t"        // skryti policek s cisly dni
            Serial2.print(i);
            Serial2.print(",0");
            SerialFFF();

            Serial2.print(EEPROM_text_read(231));  // "vis p"        // skryti ikon fazi Mesice
            Serial2.print(i);
            Serial2.print(",0");
            SerialFFF();
          }
      }
   
    for (i = pocetdni[KAL_mes] + KAL_pomprom ; i < 42 ; i++)   // zneviditelneni dni z nasledujiciho mesice
      {
        Serial2.print(EEPROM_text_read(231));      // "vis p"        // skryti ikon fazi Mesice
        Serial2.print(i);
        Serial2.print(",0");
        SerialFFF();        
        
        Serial2.print(EEPROM_text_read(232));      // "vis t"     // skryti policek s cisly dni
        Serial2.print(i);
        Serial2.print(",0");
        SerialFFF();
      }


    // --------- nalezeni hlavnich fazi Mesice a prirazeni k zobrazovanym dnum ---------
    byte prestupneroky = ((KAL_rok - 2016)  / 4) + 1;       // pocet celych prestupnych roku od 2016 (muze to byt vcetne aktualniho roku) (2016 je prestupny, tak musi byt +1)
  
    if (KAL_rok % 4 == 0  && KAL_mes < 3)                   // kdyz je aktualni rok prestupny a kdyz je v tomto prestupnem roce aktualni mesic leden, nebo unor
      {
        prestupneroky --;                                   // tak se tento jeden prestupny rok nezapocitava (prestupny den je az na konci unora)
      }
  
    KAL_min2016 = ((KAL_rok - 2016) * 365UL) + prestupneroky;       // pocet dni za uplynule KOMPLETNI CELE roky od 1.1.2016  (pro rok 2016 je vysledek 0, pro rok 2017 je vysledek 366...)
    KAL_min2016 = KAL_min2016 + pomdny[KAL_mes - 1] + 1;            // k tomu se pricte pocet dokoncenych dni od zacatku roku (napr: 10.1.2017 da vysledek 366 + 9)
    KAL_min2016 = KAL_min2016 * 1440;                               // vysledek se prevede minuty
    //  v promenne 'KAL_min2016' by mel byt pocet minut od 1.1.2016 do 1. dne zvoleneho mesice a roku


    index_databaze = (49 * (KAL_rok - 2016)) + (5 * (KAL_mes - 1)) - 15;   // prvni rychly nastrel umisteni zaznamu v EEPROM
                                                                          // (vysledek by mel byt v kazdem pripade nizsi index zaznamu, nez je skutecny index pozadovaneho zaznamu)

    T0 = 0;
    while (KAL_min2016 > T0 and index_databaze < pocet_zaznamu)     
      {
        index_databaze ++;
        T0  = EEPROM_read_long(index_databaze)  & 0b01111111111111111111111111111111 ;     // z externi EEPROM se nacte cas
      }

    // tady uz bude 'index_databaze' odpovidat skutecne prvni rozpoznane fazi Mesice od zacatku mesice
      
     den_s_fazi = 0;
     while (den_s_fazi <= pocetdni[KAL_mes])
       {
        
        T0  = EEPROM_read_long(index_databaze)  & 0b01111111111111111111111111111111 ;     // z externi EEPROM se nacte cas     
        typ_faze = index_databaze % 4;
    
        if (T0 != EEPROM_read_long(index_databaze))  zatmeni = true;
        else                                         zatmeni = false;
        
        den_s_fazi = ((T0 - KAL_min2016) / 1440) + 1;

        if (den_s_fazi <= pocetdni[KAL_mes])
          {
            // nastaveni viditelnosti ikony faze nebo zatmeni v kalendari
            Serial2.print(EEPROM_text_read(231));   // "vis p"        // zobrazeni ikony faze Mesice
            Serial2.print(KAL_pomprom + den_s_fazi-1);
            Serial2.print(",1");
            SerialFFF();
    
            Serial2.print('p');        // nastaveni typu ikony
            Serial2.print(KAL_pomprom + den_s_fazi-1);
            Serial2.print(EEPROM_text_read(233));   // ".pic="
            
            if (zatmeni == 0)
              {
                Serial2.print(238+typ_faze);            
              }
            else
              {
                if (typ_faze == 0)  Serial2.print(263);          // zatmeni Slunce 
                else                Serial2.print(262);          // zatmeni Mesice 
              }
       
            SerialFFF();
            
          }

        index_databaze ++;
        
       }
 
  }




