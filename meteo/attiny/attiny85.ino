//
//
//
//======================================================================================
//     Program pro zjistovani teploty ze dvou cidel DS18B20 pres komunikacni linku RS485
//
//                                     Kod pro SLAVE  (ATtiny85)
// verze 6 (30.8.2019)
//======================================================================================
//  Zapojeni:
//=============
//                                              ATtiny85
//                                             +--\__/--+
//                          (RESET)    - PB5  1|        |8  Vcc  - napajeni
//       Prevodnik RS485 (pin RO / Rx) - PB3  2|        |7  PB2  - Ovladani smeru komunikace RS485 (piny DE+RE)  
//       Prevodnik RS485 (pin DI / Tx) - PB4  3|        |6  PB1  - teplomery DS18B20
//                                       GND  4|        |5  PB0  - signalizacni LED
//                                             +--------+
//
//
//    POPIS KOMUNIKACE
//   ----------------------------
//  Zadost od MASTERa o cteni teplot:
//
//                                     0x02 , 0xFF , 0xFE
//               adresa ________________|      |      |
//               (65536 - adresa) _____________|______|
//
//
//  SLAVE odpovi takto:
//                                           0x02 , BAJT1 , BAJT2 , BAJT3 , BAJT4 ,   0   ,   0   , CRC_H , CRC_L , 0xXX
//               adresa ______________________|       |       |       |       |       |       |       |       |      |
//               zakodovane teploty __________________|_______|_______|_______|       |       |       |       |      |
//               rezerva _____________________________________________________________|_______|       |       |      |
//               (65536 - vsechny cisla) _____________________________________________________________|_______|      |
//               jeden bajt navic kvuli nejake chybe, ktera poskozuje posledni bajt pri prepinani sbernice __________|               
//
//
//    Zpusob kodovani teploty ve 4 bajtech (2 teploty):
//    Kazda teplota se prevadi na cislo v rozsahu 0 az 15000 pro -50'C az +100'C
//        Prevodni funkce vcetne zaokrouhleni posledniho mista:
//              (unsigned int)cislo = ((float)zmerena_teplota + 50.005 ) * 100
//
//    BAJT1:         MSB z prvni zakodovene teploty
//    BAJT2:         LSB z prvni zakodovene teploty
//    BAJT3:         MSB z druhe zakodovene teploty
//    BAJT4:         LSB z druhe zakodovene teploty
//
// V pripade chyby cidla se vraci teplota 201'C (pro chybu cidla 1 = cislo 25100), nebo 202'C (pro chybu cidla 2 = cislo 25200)
//
//
//
//
// Specialni funkce pro zmenu adresy:
//---------------------------------------
//  Zadost:
//                                                           0x02 , 0x05 , 0xFF , 0xF9
//                aktualni adresa ____________________________|      |      |      |
//                nova adresa _______________________________________|      |      |
//                (65536 - puvodni adresa - nova adresa) ___________________|______|
//
//  Odpoved:
//                                                           0x05 , 0x02 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xFF , 0xF9 , 0xXX
//                nova adresa ________________________________|      |      |      |      |      |      |      |      |      |
//                puvodni adresa ____________________________________|      |      |      |      |      |      |      |      |
//                5 x 0 ____________________________________________________|______|______|______|______|      |      |      |
//                (65536 - puvodni adresa - nova adresa) ______________________________________________________|______|      |
//               jeden bajt navic kvuli nejake chybe, ktera poskozuje posledni bajt pri prepinani sbernice __________________|               
//
//
//
//   Adresu je mozne zmenit i hardwarove:
//     Pri zapnuti napajeni musi byt pin DE_RE pritazeny na +5V. Tim se nastavi adresa desky na 1. Po uspesnem nastaveni adresy se rozblika infoLED.
//     Pak je nutne provest normalni reset (tentokrat uz bez propojeneho pinu DE_RE na +5V)


//============================================================================================================================================


#define F_CPU 8000000                      // Nastaveni vnitrni frekvence procesoru na 8 MHz


#define pin_TX         4                   // RS485 (DI)
#define pin_RX         3                   // RS485 (RO)
#define pin_DE_RE      2                   // RS485 - (DE,RE) - prepinani smeru komunikace
#define ONE_WIRE_BUS   1                   // Pin pro teplomery DS18B20
#define pin_LED        0                   // Pin pro informacni LED


#include "SoftwareSerial.h"                // Knihovna pro seriovou komunikaci (prevzato z https://thewanderingengineer.com/2013/05/05/software-serial-on-the-attiny85/)
#include <OneWire.h>                       // Knihovny pro DS18B20 (prevzato z https://www.arduinoslovakia.eu/blog/2019/1/attiny85---teplomer-s-ds18b20)
#include <DallasTemperature.h>
#include <EEPROM.h>                        // Knihovna pro praci s EEPROM (v EEPROM na sdrese 0x000) je ulozena unikatni adresa desky


// =============================== Globalni promenne  ==============================================

int CRC16;                                 // 16-bitovy kontrolni soucet
boolean patri_mi;                          // Globalni znacka, jestli je komunikace na sbernici zadost od MASTERa tomuto zarizeni

byte ModADDR;                              // SLAVE adresa desky pro komunikaci

double teplota_A;                          // Teplota primo z 1. cidla
double teplota_B;                          // Teplota primo z 2. cidla

unsigned int conv_tep_A;                   // Prevedena teplota z prvniho cidla 
unsigned int conv_tep_B;                   // Prevedena teplota z druheho cidla 

byte akt_teplota;                          // V kazde smycce se aktualizuje jen 1 teplota 

unsigned int pocitadlo_smycek;             // Smycka beha hodne rychle (100x za sekundu), ale teplota se meri jen kazdou sekundu. Tato promenna odpocitava smycky.

byte zadost[15];                           // Do pole zadost[] se ukladaji prijata data od MASTERa
byte odpoved[15];                          // Do pole odpoved[] se pripravuji hodnoty k odeslani do MASTERa

SoftwareSerial mySerial(pin_RX, pin_TX);   // Spusteni knihovny pro seriovou komunikaci
OneWire oneWire(ONE_WIRE_BUS);             // Spusteni knihovny pro OneWire komunikaci s cidly
DallasTemperature sensors(&oneWire);       // Spusteni knihovny pro DS18B20




//==========================================================================================================================
void setup(void)
  {

   //----------------------------------------------
   // Servisní HW reset adresy na 1
   //  Vyuziva se toho, ze LED na DE/RE pinech na nich udrzuje stav "LOW".
   //  Kdyz je potreba zmenit adresu, musi se na ty piny piny privest VCC = "HIGH".  
    pinMode(pin_DE_RE,INPUT);              // Prepinaci pin prijem / vysilani pro RS485 se hned na zacatku prepne na vstup (bez Pull-Up)
    delay(200);
    if (digitalRead(pin_DE_RE) == HIGH)    // Kdyz je pin pevne pritazeny na HIGH ...
      {
        EEPROM.write(0,1);                 // ... nastavi se adresa modulu v EEPROM na 1
        while (true)                       // a infoLED se rozblika v nekonecne smycce
          {                                // Vystup ze smycky je mozny jen normalnim resetem bez pinu DE/RE pripojenych na HIGH
            digitalWrite(pin_LED,HIGH);
            delay(200);
            digitalWrite(pin_LED,LOW);
            delay(400);
          }
      }
   //----------------------------------------------
   

    
    pinMode(pin_DE_RE,OUTPUT);               // Prepinaci pin prijem / vysilani pro RS485
    pinMode(pin_LED,OUTPUT);                 // Informacni LED

    mySerial.begin(9600);
     
    delay(20);
    digitalWrite(pin_DE_RE,LOW);             // Prepnuti RS485 na prijem na strane SLAVE (defaultni stav sbernice)
    digitalWrite(pin_LED,LOW);               // Zhasnuti informacni LED

    ModADDR = EEPROM.read(0);
    if (ModADDR == 255 or ModADDR == 0)      // Povolene adresy jsou od 1 do 254
      {
        EEPROM.write(0,1);                   // Pri prvnim zapnuti je v EEPROM "nepovolena" hodnota 0xFF, tak se adresa nastavi na 1           
        ModADDR = 1;
      }

    sensors.begin();                         // zakladni nastaveni DS18B20
    delay(1000);
    sensors.setResolution(12);
    sensors.requestTemperatures();  



 // priprava odpovedi
    
    odpoved[0] = ModADDR;   // adresa modulu SLAVE
 // odpoved[1] = 0;         // MSB 1. teplota
 // odpoved[2] = 0;         // LSB 1. teplota
 // odpoved[3] = 0;         // MSB 2. teplota
 // odpoved[4] = 0;         // LSB 2. teplota
 // odpoved[5] = 0;         // rezerva
 // odpoved[6] = 0;         // rezerva
 // odpoved[7] = 0;         // MSB z kontrolniho souctu.
 // odpoved[8] = 0;         // LSB z kontrolniho souctu.



  }
  
  
// ===========================================  Hlavni smycka ===============================================================
void loop(void)
  {

    digitalWrite(pin_LED,LOW);                    // Zhasnuti informacni LED na zacatku kazde smycky
    if (mySerial.available())                     // Kdyz se na sbernici objevi nejaka komunikace, je nutne ji zacit sledovat.
      {
        digitalWrite(pin_LED,HIGH);               // Rozsviceni informacni LED (na sbernici je nejaka komunikace) - zhasne se az v pristi smycce
        prijem_dat();                             // Podprogram pro cteni dat na komunikacni lince a vyhodnoceni, jestli to je zadost urcena pro tento SLAVE2.
        
        if (patri_mi == true)                     // Kdyz se ukaze, ze je tato komunikace urcena pro toto zarizeni (adresa a CRC souhlasi), ...
          {
            CRC16 = CRC_pole(odpoved,7);          // ... spocitej CRC z prvnich 7 bajtu v poli odpoved[] (indexy 0 az 6).
            odpoved[7] = (CRC16 >> 8) & 255;      // Doplneni pole odpoved[] o kontrolni soucet (MSB)
            odpoved[8] = CRC16 & 255;             // Doplneni pole odpoved[] o kontrolni soucet (LSB)



            delay(40);   
                                                 // Odesli pripravenou odpoved:
            digitalWrite(pin_DE_RE,HIGH);        // Prepnuti pinu pro rizeni smeru RS485 na vysilani.
            delay(10);                           // Kratka pauza pred zacatkem vysilani.
            mySerial.write(odpoved,10);          // Cela pripravena sekvence v poli odpoved[] (9 bajtu) + 1 bajt se odesle do seriove linky.
                                                 //  (protoze se z neznameho duvodu posledni bajt obcas pri komunikaci poskodi, odesila se na zaver jeste jeden 'zbytecny' bajt)
            delay(20);                           // Pauza pro prijimaci buffer u MASTERa.
            digitalWrite(pin_DE_RE,LOW);         // Prepnuti pinu pro rizeni smeru RS485 zpatky do defaultniho stavu - na prijem.
          }
        
      }
      
    pocitadlo_smycek ++;
    if (pocitadlo_smycek > 100)                  // Kazdou sekundu se zmeri jedna teplota
      {
        pocitadlo_smycek = 0;
        zmer_teplotu();
      }

    delay(10);                                   // 100 smycek za sekundu
  } 
//        ==============  konec hlavni smycky =================



//------------------------------------------------------------
void zmer_teplotu(void)
  {
    if (akt_teplota == 1)                                         // Pri kazdem mereni (kazdou sekundu) se cte na stridacku jen 1 teplomer
      {
        teplota_A = sensors.getTempCByIndex(0);
        akt_teplota = 0;    
        if (teplota_A > 100 or teplota_A < -50) teplota_A = 201;  // Kdyz je teplomer A v poruse, nastavi se mu nesmyslne vysoka hodnota 201'C
        conv_tep_A = (teplota_A + 50.005) * 100;                  // Prevedeni teploty z desetinneho cisla na unsigned int a zaokrouhleni posledniho mista
        odpoved[1] = conv_tep_A / 256;                            // MSB z prvniho teplomeru
        odpoved[2] = conv_tep_A % 256;                            // LSB z prvniho teplomeru
      }
    else
      {
        teplota_B = sensors.getTempCByIndex(1);    
        akt_teplota = 1;    
        if (teplota_B > 100 or teplota_B < -50) teplota_B = 202;  // Kdyz je teplomer B v poruse, nastavi se mu nesmyslne vysoka hodnota 202'C
        conv_tep_B = (teplota_B + 50.005) * 100;                  // Prevedeni teploty z desetinneho cisla na unsigned int a zaokrouhleni posledniho mista
        odpoved[3] = conv_tep_B / 256;                            // MSB z druheho teplomeru
        odpoved[4] = conv_tep_B % 256;                            // LSB z druheho teplomeru
      }
    sensors.requestTemperatures();                                // zadost o mereni teploty v cidle. Hodnota se precte az za dalsi sekundu

  }


//------------------------------------------------------------
// Funkce, ktera sleduje komunikaci na sbernici a vyhodnocuje, jestli je to pozadavek od MASTERA, nebo nejaka odpoved od ciziho SLAVE.
void prijem_dat()
  {
    byte i = 0;                                           // pomocna promenna
    patri_mi = true;                                      // Docasna znacka ze je komunikace urcena pro mne.
    delay(10);                                            // Prichozi zadost obsahuje 3 bajty, tak se pocka, az se vsechno nacte do prijimaciho bufferu.

    while (mySerial.available())                          // Vsechny prijate bajty z prijimaciho bufferu se prepisou do pole 'zadost[]'.
      {
        zadost[i] = mySerial.read();
        i++;
      }


    // Od MASTERAa ocekavam sekvenci bajtu {ModADDR, CRC(MSB), CRC(LSB)}                 pro beznou zadost o teploty
    //                  pripadne           {ModADDR, nova adresa, CRC(MSB), CRC(LSB)}    pro zadost o zmenu adresy
    // Pokud prijde neco jineho, nebo kdyz nesouhlasi CRC, tak komunikaci ignoruji (patri_mi = false). 



    if (zadost[0] != ModADDR)  patri_mi = false;          // Prvni bajt obsahuje adresu. Kdyz neni roven moji adrese, komunikace mi nepatri.


    //----------------------------------------------------------
    if (i == 3 and patri_mi == true)                      // Bezna zadost o teplotu
      {
        CRC16 = CRC_pole(zadost,1);                       // Vypocet "kontrolniho souctu" z prvniho bajtu (z adresy) v poli zadost[]
                                                          //             Priklad:  Pro adresu 0x02 by mel byt CRC: 0xFF, 0xFE
        if (CRC16 != ((zadost[1] *256) + zadost[2]))      // Porovnani vypocteneho a prijateho CRC
          {
            patri_mi = false;                             // CRC error - Kontrolni soucet nesouhlasi, nastala nejaka chyba pri komunikaci.
          }           
      }
      //----- konec bezne zadosti o teplotu -----      



    //----------------------------------------------------------
    if (i == 4 and patri_mi == true)                      // Specialni funkce pro zmenu adresy
      {
        CRC16 = CRC_pole(zadost,2);                       // Vypocet "kontrolniho souctu" z prvnich 2 bajtu (z puvodni a nove adresy) v poli zadost[]
                                                          //   Pro puvodni adresu 0x02 a novou adresu 0x05 by mel byt CRC: 0xFF, 0xF9
        if (CRC16 == ((zadost[2] *256) + zadost[3]))      // Porovnani vypocteneho a prijateho CRC
          {
            ModADDR = zadost[1];
            EEPROM.write(0,ModADDR);                      // Nova adresa se ulozi do EEPROM
            
            odpoved[0] = ModADDR;                         // Nova adresa
            odpoved[1] = zadost[0];                       // Puvodni adresa
            odpoved[2] = 0x00;                            //  0
            odpoved[3] = 0x00;                            //  0
            odpoved[4] = 0x00;                            //  0
            odpoved[5] = 0x00;                            //  0
            odpoved[6] = 0x00;                            //  0
            
            CRC16 = CRC_pole(odpoved,7);                  // Spocitej CRC z prvnich 7 bajtu v poli odpoved[] (indexy 0 az 6).
            odpoved[7] = (CRC16 >> 8) & 255;              // Doplneni pole odpoved[] o kontrolni soucet (MSB)
            odpoved[8] = CRC16 & 255;                     // Doplneni pole odpoved[] o kontrolni soucet (LSB)


            delay(40);   
                                                          // Odesli pripravenou odpoved:
            digitalWrite(pin_DE_RE,HIGH);                 // Prepnuti pinu pro rizeni smeru RS485 na vysilani.
            delay(10);                                    // Kratka pauza pred zacatkem vysilani.
            mySerial.write(odpoved,10);                   // Cela pripravena sekvence v poli odpoved[] (9 bajtu) + 1 bajt se odesle do seriove linky.
                                                          //  (protoze se z neznameho duvodu posledni bajt obcas pri komunikaci poskodi, odesila se na zaver jeste jeden 'zbytecny' bajt)

            delay(20);                                    // Pauza pro buffer u MASTERa.
            digitalWrite(pin_DE_RE,LOW);                  // Prepnuti pinu pro rizeni smeru RS485 zpatky do defaultniho stavu - na prijem.
          }           
        patri_mi = false;                                 // At uz predchozi testy CRC dopadnou jakkoliv, dalsi data s teplotami se po teto zadosti neodeslou
      }
      //----- konec specialni funkce pro zmenu adresy -----      

  
  }

  
//========================================================================================================
// Jen jednoduchy pokus o kontrolni soucet
//  Odecteni hodnot zadaneho poctu bajtu v poli od cisla 65536
unsigned int CRC_pole(byte pole[], byte pocet)
  {
    unsigned int crc = 0;
    for (byte pos = 0; pos < pocet; pos++)
      {
        crc = crc - pole[pos];
      }
    return crc;  
  }

//========================================================================================================
  

