//      Meteostanice s hodinami, ukazatelem fazi Mesice, kalendarem,
//        grafy s historii hodnot a zaznamem rekordu
//
//                         Hlavni program
//==============================================================================
//
//
// Detaily: http://www.astromik.org/raspi/meteo2/
// verze STM + displej TJC4832 - (15.9.2019)
//
//
// (Pri prvnim spusteni programu musi byt zasunuta karta s datovymi soubory pro EEPROM)
//====================================================================================================================

#include "PCF8574.h"                       // knihovna pro praci s expanderem
#define EXPANDER_I2C_ADDRESS  0x20         // I2C adresa expanderu pri vsech adresovacich pinech v LOW (pouzity expander PCA8574T)


#include <Wire.h>                                  // knihovna pro I2C komunikaci
#include <OneWire.h>                               // knihovna pro OneWire komunikaci (pouziva ji teplomer DS18B20)
#include <DallasTemperature.h>                     // knihovna pro teplomer DS18B20
#include <Adafruit_BMP085.h>                       // knihovna pro cidlo tlaku BMP085
#include <DHT.h>                                   // knihovna pro cidlo vlhkosti a teploty DHT11 / DHT22

#include "SdFat.h"                                 // nova knihovna pro praci s SD kartou
#include "sdios.h"

#include <libmaple/iwdg.h>                         // podpora pro Watchdog


// nastaveni I2C adres jednotlivych periferii
#define I2C_ADDR_RAM1_RW       0b1010110           // I2C adresa EERAM1 pro cteni a zapis dat
#define I2C_ADDR_RAM2_RW       0b1010010           // I2C adresa EERAM2 pro cteni a zapis dat

#define I2C_ADDR_RAM1_CTRL     0b0011110           // I2C adresa EERAM1 (ridici registr)
#define I2C_ADDR_RAM2_CTRL     0b0011010           // I2C adresa EERAM2 (ridici registr)

#define I2C_ADDR_EEPROM_RW     0b1010111           // I2C adresa EEPROM pro cteni a zapis dat (vsechny 3 sdresovaci piny v HIGH)
#define I2C_ADDR_DS3231        0b1101000           // I2C adresa RTC obvodu  (0x68)
#define I2C_ADDR_opto          0x39                // I2C adresa cidla priblizeni / jasu (APDS9960) 


// textove definice adres v zalohovane externi EERAM
#define EERAM_ADDR_rekord_MIN          1800          // znacka pro zelenou LED, ze doslo k poklesu pod minimalni hodnotu   (hodnota 0 nebo 1)
#define EERAM_ADDR_rekord_MAX          1801          // znacka pro cervenou LED, ze doslo k prekroceni maximalni hodnoty   (hodnota 0 nebo 1)
#define EERAM_ADDR_modra               1802          // znacka pro modrou LED, ze se ma rozsvitit (doslo  prekroceni, nebo poklesu nastavene urovne)   (hodnota 0 nebo 1)
#define EERAM_ADDR_modra_pamet         1804          // pamet, ze alespon na chvili doslo k rozsviceni modre LED

#define EERAM_ADDR_prepinaci_uroven1    925          // polozka 925 = adresy 1850 a 1851: nastavena 1. uroven pro ovladani modre LED
#define EERAM_ADDR_prepinaci_uroven2    926          // polozka 926 = adresy 1852 a 1853: nastavena 2. uroven pro ovladani modre LED

#define EERAM_ADDR_min_teplota          910          // polozka 910 = adresy 1820 a 1821: minimalni namerena hodnota venkovni teploty
#define EERAM_ADDR_max_teplota          911          // polozka 911 = adresy 1822 a 1823: maximalni namerena hodnota venkovni teploty
#define EERAM_ADDR_min_datum           1824          // tady se uklada citelny datum a cas pri dosazeni rekordni zimy
#define EERAM_ADDR_max_datum           1836          // tady se uklada citelny datum a cas pri dosazeni rekordniho tepla
#define EERAM_ADDR_rst_rekordy         1805          // tady se uklada citelny datum a cas pri nulovani rekordu
#define EERAM_ADDR_poslednich_5min     1854          // pocet minut aktualni hodiny pri poslednim ukladani 5-minutovych zaznamu (ochrana pred vice zapisy behem jedne minuty)
#define EERAM_ADDR_posledni_1hod       1855          // aktualni hodina pri poslednim ukladani hodinovych zaznamu (ochrana pred vice zapisy behem jedne hodiny)
#define EERAM_ADDR_rec_file            1803          // cislo souboru, ktere se zvetsuje pri kazdem resetu celkovych rekordu

#define EERAM_ADDR_tep_min1             800          // minimalni teplota od denniho nulovani
#define EERAM_ADDR_hhmm_min1            801          // minuty od pulnoci, kdy doslo k minimalnimu rekordu od denniho nulovani
#define EERAM_ADDR_tep_max1             802          // maximalni teplota od denniho nulovani
#define EERAM_ADDR_hhmm_max1            803          // minuty od pulnoci, kdy doslo k maximalnimu rekordu od denniho nulovani

#define EERAM_ADDR_tep_min2             804          // minimalni hodnota za minuly ukonceny pulden
#define EERAM_ADDR_hhmm_min2            805          // minuty od pulnoci, kdy doslo k minimalnimu rekordu pro ukonceny pulden
#define EERAM_ADDR_tep_max2             806          // maximalni hodnota za minuly ukonceny pulden
#define EERAM_ADDR_hhmm_max2            807          // minuty od pulnoci, kdy doslo k minimalnimu rekordu pro ukonceny pulden

#define EERAM_ADDR_dennoc              1817          // znacka, ze doslo k ukonceni zaznamu jednoho bloku rekordu a preplo se na druhy (DEN/NOC)

#define EERAM_ADDR_testper1            1818          // pro test funkcnosti EERAM1
#define EERAM_ADDR_testper2            2047          // pro test funkcnosti EERAM2
#define EEPROM_ADDR_testper3          26000          // pro test funkcnosti EEPROM

#define EERAM_ADDR_stop_RG              924          // 2 bajty v EERAM, kam se uklada cas nulovani teplotnich rekordu. Po dobu jednoho dne od tohoto casu se blokuje  cervena a zalena LED

#define EERAM_ADDR_dis_on               928          // pro funkci automatickeho rozsvecovani displeje kazde pravovni rano
#define EERAM_ADDR_dis_off              929          // pro funkci automatickeho zhasinani displeje kazde pravovni rano
#define EERAM_ADDR_dis_maska           1860          // bitova pro maska pro urceni pracovnich dni 0b[Po][Ut][St][Ct][Pa][So][Ne][X]


#define EERAM_ADDR_timezone            1500          // adresa v pracovní EERAM s aktualni hodnotou casove zony (1=SEC, 2=SELC)
#define EERAM_ADDR_korekce_IN          1501          // adresa v pracovní EERAM s hodnotou korekce vnitrni teploty
#define EERAM_ADDR_korekce_tlak        1502          // adresa v pracovní EERAM s hodnotou korekce tlaku
#define EERAM_ADDR_korekce_vlhkost     1503          // adresa v pracovní EERAM s hodnotou korekce vlhkosti
#define EERAM_ADDR_korekce_OUT         1504          // adresa v pracovní EERAM s hodnotou korekce venkovni teploty
#define EERAM_ADDR_korekce_OUT2        1505          // adresa v pracovní EERAM s hodnotou korekce druheho čidla venkovni teploty
#define EERAM_ADDR_typ_teplomeru       1513          // typ venkovniho teplomeru (1=1x blizky ; 2=2x blizky ; 3=RS485 vzdaleny))

#define EERAM_ADDR_zhasinat_podsvet    1512          // podsvet bude svitit trvale bez ohledu na odpocet casu, nebo se bude po minute zhasinat
#define EERAM_ADDR_SDaktiv             1506          // adresa v pracovní EERAM s informaci, jestli je aktivni SD karta
#define EERAM_ADDR_LEDblok             1507          // adresa v pracovní EERAM s informaci, jestli se maji blokovat LED
#define EERAM_ADDR_GeoLat               754          // adresa v pracovní EERAM (2 bajty)se zemepisnou sirkou (50)
#define EERAM_ADDR_GeoLon               755          // adresa v pracovní EERAM (2 bajty) se zemepisnou delkou (15)
#define EERAM_ADDR_GeoAlt               757          // adresa v EERAM s nadmorskou vyskou (kvuli prepoctu tlaku na hladinu more)


#define EERAM_ADDR_adr_mod             1819          // adresa SLAVE desky s teplomery, se ktrou ma meteostanice komunikovat pres RS485


#define EEPROM_ADDR_texty             26020          // pocatecni adresa externi EEPROM s textovymi popisy

#define EEPROM_ADDR_infrakody         25000          // pocatecni adresa externi EEPROM, kde je ulozeno 21 nactenych kodu pro tlacitka infra DO


#define KON_ZPOZD_PODSV    60                      // doba prodlouzeneho sviceni podsvetu na displeji 


#define pocet_zaznamu                2737          // pocet casu mesicnich fazi v databazi (cislo je ulozeno v EEPROM na adresach 25000 a 25001)
//     (v prvni verzi databaze je tam cislo 2737 ... faze az do 30.4.2071)
#define pocet_zatmeni                  97          // pocet zatmeni v databazi (cislo je ulozeno v EEPROM na adresach 25002 a 25003)
//     (v prvni verzi databaze je tam cislo 97 ... zatmeni az do roku 16.3.2071 )


#define pin_vlhkomer        PB3                    // pin pro cteni vlhkomeru DHT
#define pin_DS18B20         PA8                    // pin pro pripojeni cidla DS18B20
#define pin_DS18B20PWR      PB8                    // pin pro napajeni cidla DS18B20 (kvuli resetu cidla pri poruse)

#define pin_LED_info        PB14                   // pin pro informacni LED (prijem infra signalu, chyba karty, chyba teplomeru)

#define pin_bzuk             PB5                   // pin pro piskak (pasivni - bez prerusovace)

#define pin_BT_start         PB1                   // pin pro spinani napajeni do BT modulu
#define pin_infra            PB9                   // pin, na kterem je pripojene infracidlo
#define pin_SD_CS            PA4                   // CS na SD karte

#define pin_RTC_INT         PB13                   // sekundovy inerrupt od RTC obvodu
#define pin_IR_pohyb        PB12                   // pin pro detekci pohybu, ktery bude probouzet zhasnuty displej

#define pin_DERE             PA1                   // pin pro prepinani smeru komunikace RS485 se vzdalenym cidlem  //!!RS485


boolean uceni;                                     // infracidlo pracuje ve dvou rezimech (bezny rezim a ucici-se rezim)
int index_infra_nastaveni;                         // pri rezimu uceni infra ovladace se v teto promenne uchovava aktualne vybrana funkce ze seznamu nize

String infra_popisy[] = {                                       // popisy pro servisni prirazeni tlacitek dalkoveho infra ovladace k prislusnym funkcim
                            "graf tlaku 12 hodin",              // [0]
                            "graf tlaku 6 dni",                 // [1]
                            "graf venkovni teploty 12 hodin",   // [2]
                            "graf venkovni teploty 6 dni",      // [3]
                            "graf vnitrni teploty 12 hodin",    // [4]
                            "graf vnitrni teploty 6 dni",       // [5]
                            "graf vlhkosti 12 hodin",           // [6]
                            "graf vlhkosti 6 dni",              // [7]
                            "zhasinani rekordovych LED",        // [8]
                            "displej ON / OFF",                 // [9]
                            "funkce Lampicka",                  // [10]
                            "Bluetooth",                        // [11]
                            "Hlavni obrazovka",                 // [12]
                            "listovani v polozkach [+]",        // [13]
                            "listovani v polozkach [-]",        // [14]
                            "Informace o Mesici",               // [15]
                            "Informace o Slunci",               // [16]
                            "Kalendar",                         // [17]
                            "Rekordy",                          // [18]
                            "Velke hodiny",                     // [19]
                            "Pristi budik",                     // [20]
                            "Soumraky"                          // [21]
                        }; 

unsigned long default_infra_btn[] =                  // definice infra kodu dalkoveho ovladani (pri prvnim zapnuti se prekopiruji do EEPROM a pak do "pracovnich" promennych)
  {
    0x4335906F,                   //  [0]  graf tlaku 12 hodin
    0x43358877,                   //  [1]  graf tlaku 6 dni
    0x4335A05F,                   //  [2]  graf venkovni teploty 12 hodin
    0x4335F807,                   //  [3]  graf venkovni teploty 6 dni
    0x4335D02F,                   //  [4]  graf vnitrni teploty 12 hodin
    0x4335B04F,                   //  [5]  graf vnitrni teploty 6 dni
    0x43359867,                   //  [6]  graf vlhkosti 12 hodin
    0x4335D827,                   //  [7]  graf vlhkosti 6 dni
    0x43359C63,                   //  [8]  zhasinani rekordovych LED
    0x4335DC23,                   //  [9]  zhasinani displeje
    0x4335FC03,                   // [10]  funkce Lampicka
    0x4335BC43,                   // [11]  Bluetooth
    0x4335E11E,                   // [12]  Hlavni obrazovka
    0x4335C837,                   // [13]  listovani v polozkach [+]
    0x4335F00F,                   // [14]  listovani v polozkach [-]
    0x4335B847,                   // [15]  Informace o Mesici
    0x4335E41B,                   // [16]  Informace o Slunci
    0x4335847B,                   // [17]  Kalendar
    0x4335C43B,                   // [18]  Rekordy
    0x4335E01F,                   // [19]  Velke hodiny
    0x4335E817,                   // [20]  Pristi budik
    0x4335C03F                    // [21]  Soumraky
  };

unsigned long infra_btn[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };     // tohle budou "pracovni" infrakody



volatile unsigned long InfraHexaKod_uceni;         // kdyz je infracidlo v rezimu uceni, vraci sem kod dalkoveho ovladani
unsigned long posledni_infrakod;                   // posledni stisknute tlacitko pri uceni infracidla



const unsigned int melodie[] =    // definice frekvenci 30 tonu pro budik (jeden ton pipne kazde 2 sekundy)
  {
     440, 1047,  494, 1175,  523, 1319,  587, 1397,  659, 1568,  698, 1760,  784, 1976,  880,
    2093,  988, 2349, 1047, 2637, 1175, 2794, 1319, 3136, 1397, 3520, 1568, 3951, 1760, 4186
  };
byte ukazatel_melodie = 0;        // ukazatel do pole pole melodie (kazde 2 sekundy se zvysuje o +1) 



String dat_str = "";                               // retezec pro konstrukci citelneho datumu
String tim_str = "";                               // retezec pro konstrukci citelneho casu
String DMhn = "";                                  // retezec pro konstrukci datumu a casu pouzivaneho pro zaznam minimalnich a maximalnich teplot ("DD.MM. hh:nn")

String tep_min_datcas = "DD.MM. hh:nn";            // pro zaznam datumu casu minimalni a maximalni teploty a pro ukladani casu nulovani rekordu
String tep_max_datcas = tep_min_datcas;
String tep_rst_datcas = tep_min_datcas;

String tep_max_datcas_S = tep_min_datcas;          // pomocne promenne pro ukladani rekordu do souboru na SD karte
String tep_min_datcas_S = tep_min_datcas;


String tep_min_den1 = "hh:nn";                      // pro zaznam casu minimalni teploty od denniho nulovani
String tep_max_den1 = "hh:nn";                      // pro zaznam casu maximalni teploty od denniho nulovani
String tep_min_den2 = "hh:nn";                      // pro zaznam casu minimalni teploty za predchozi den (noc)
String tep_max_den2 = "hh:nn";                      // pro zaznam casu maximalni teploty za predchozi den (noc)

unsigned int tep_min1;                              // pro zaznam hodnoty minimalni a maximalni teploty od denniho vynulovani
unsigned int tep_max1;

unsigned int tep_min2;                              // pro zaznam hodnoty minimalni a maximalni teploty za predchozi den (noc)
unsigned int tep_max2;


unsigned int tep_min;                              // pro zaznam hodnoty minimalni a maximalni teploty od vynulovani
unsigned int tep_max;

char prijato[16];                                  // pole, do ktreho se ukladaji znaky, prijate pres seriovou linku (nastaveni casu, zadost o informace, korekce ....)
byte zadost[15];                                   // do pole zadost[] se pripravuje MODBUS zadost o zjisteni teploty ze vzdaleneho cidla  //!!RS485
byte odpoved[15];                                  // do pole odpoved se pijimaji MODBUS data od SLAVE zarizeni  //!!RS485
byte mod_addr;                                     // ze ktereho modulu se maji stahovat teploty


unsigned long STOP_LED_REK;                        // po resetu rekordnich teplot se nejakou dobu (24 hodin) nebudou rozsvecovat cervena nebo zelena LED
                                                   // do teto promenne se uklada odpocet minut po nulovani rekordu.
                                                   //  Tuto promennou je mozne upravovat seriovym prikazem "#Z".

volatile byte InfraIndex = 99;                     // index prijateho tlacitka z dalkoveho ovladace
volatile boolean aktualizuj_cas;                   // pri preruseni od RTC obvodu 
volatile boolean byl_pohyb;                        // pri preruseni od infra cidla pohybu

byte LED_blok;                                     // prepinac v nastaveni jestli se maji blokovat LED

boolean open_OK;                                   // pomocna promenna, jestli funguje zapis na kartu (jestli je zasunuta karta)

char jmeno_souboru[] = "20190125.csv";             // retezec pro konstrukci jmena souboru
char jmeno_slozky[] = "2019";                      // retezec pro konstrukci nazvu slozky
char cesta[] = "2019/20190125.csv";                // retezec pro konstrukci cele cesty k souboru

unsigned long rokmesden;                           // rok mesic a den ve formatu yyyymmdd
unsigned long predchozi_obdobi;                    // pri hodinovem ukladani na kartu se ke kazdemu zaznamu pridava 'rokmesden' + 'LOC_hod'  (format je tedy yyyymmddhh)
                                                   // na kazdou radku v CSV souboru se pak pridava jeste 'predchozi_obdobi' + [0] + 'LOC_min'



// pocet dni v mesici pro ruzne pocitani dni od zacatku roku (unor se pak pri prestupnem roce jeste upravuje)
//   x , Led, Uno, Bre, Dub, Kve, Cer, Cec, Srp, Zar, Rij, Lis, Pro
int  pomdny_d[] = { 0,   31,  28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31 };



String dny[] = {"xx" , "Po" , "Ut" , "St" , "Ct" , "Pa" , "So" , "Ne"}; // pro vypis nazvu dni




byte index_iko;                                    // index ikony 1 az 16
byte minuly_index_iko = 200;                       // kvuli tomu, aby se inona Mesice nemusela prekreslovat kazde 2 sekundy, tak se zapamatuje posledni hodnota
byte index_mini;                                   // index mini ikony 0 az 5

unsigned int stari_mesice;                         // stari Mesice v minutach od posledniho novu
float stari_mesice_D;                              // stari Mesice ve dnech a desetinach dne od posledniho novu

int LOC_hod, LOC_den, LOC_mes, LOC_min, LOC_sek;   // promenne, ktere obsahuji aktualni cas v RTC (mistni cas, ktery se zobrazuje na displeji)
unsigned int LOC_rok ;
byte LOC_dvt;                                      // den v tydnu (po ... ne)

int UTC_hod, UTC_den, UTC_mes, UTC_min;            // pomocne promenne, ktere obsahuji cas v UTC zone (pouzivaji se k prepoctu pristich fazi)
unsigned int UTC_rok ;

//int faze_UTC_hod, faze_UTC_den, faze_UTC_mes, faze_UTC_min;  // promenne, ktere obsahuji cas pristi faze v UTC (nacitaji se z databaze fazi)
//unsigned int faze_UTC_rok ;

int faze_LOC_hod, faze_LOC_den, faze_LOC_mes, faze_LOC_min;  // promenne, ktere obsahuji cas pristi faze v mistnim case (prepocitavaji se z casu v databazi fazi)
unsigned int faze_LOC_rok ;

String pristifaze_txt;           // promenna, do ktere se bude ukladat typ pristi faze v textove podobe

byte pamet_ikofaze;                                // aby se po prepnuti na detail Mesice nemuselo znova prepocitavat, jaka bude dalsi faze
byte blikani;                                      // v pripade bliziciho-se zatmeni zajistuje prepinani informaci o zatmeni a o vychodech a zapadech Mesice 

byte casova_zona;                                  // posun casu proti UTC (SEC = 1; SELC = 2)
byte modra_pamet;                                  // znacka jestli doslo k rozsviceni modre LED
byte modra_pamet_pom;                              // pouzito pro zjistovani prechodu mezi aktivni a neaktivni pameti (aby se nemusela kazdou sekundu obcerstvovat pametova ikona)

int prepinaci_uroven1;                             // 1. prepinaci uroven pro modrou LED
int prepinaci_uroven2;                             // 2. prepinaci uroven pro modrou LED

byte zmena_minut;                                  // pro ruzne odpocty se pomoci teto docasne znacky zjistuje, ze doslo ke zmene minut (napriklad pro blokovani R/G LED)                          

int log_pomprom;                                   // pomocna promenna urcena pro logovani zmen korekci

byte stav_BT;                                      // Aktualni stav Bluetooth modulu (vypnuto / zapnuto)
byte zhasinat_podsvet;                             // kdyz je nastavena promenna na 0, podsvet bude svitit trvale bez ohledu na odpocet casu

byte typ_out_teplomeru;                            // typ venkovniho teplomeru (1x / 2x / RS485) 
byte akt_cidlo;                                    // v pripade zvoleneho dvojiteho teplomeru udava index cidla s nizsi teplotou, ktera se povazuje za spravnejsi


byte pocitadlo = 0;                                // pomocna promenna pro pocitani smycek
unsigned long tlak[10];                            // pole, ze ktereho se vypocitava klouzavy prumer pro tlak
unsigned long teplota_IN[10];                      // pole, ze ktereho se vypocitava klouzavy prumer teploty z vlhkomeru
unsigned long teplota_OUT[10];                     // pole, ze ktereho se vypocitava klouzavy prumer pro teplotu z cidla DS18B20

float prvni_klouzak_OK = false;                    // znacka, ze pri zapnuti napajeni funguje venkovni teplomer a teplota se zkopiruje na vsech 10 pozic do pole klouzaku

byte screen=0;                                     // promenna, ve ktere je informace o aktualne prepnute obrazovce na displeji

unsigned long suma;                                // pomocna promena pro soucet vsech hodnot v poli klouzavych prumeru vlhkosti a tlaku
unsigned long suma_IN;                             // pomocna promena pro soucet vsech hodnot v poli klouzavych prumeru vnitrnich teplot
unsigned long suma_OUT;                            // pomocna promena pro soucet vsech hodnot v poli klouzavych prumeru venkovnich teplot
unsigned long prumer_tlak;                         // klouzavy prumer tlaku z poslednich 10 mereni
unsigned long prumer_teplota_IN;                   // klouzavy prumer teploty z poslednich 10 mereni (cidlo vlhkosti)
unsigned long prumer_teplota_OUT;                  // klouzavy prumer teploty z poslednich 10 mereni (cidlo DS18B20)
unsigned long prumer;                              // pomocna promenna pro pocitani prumeru ruznych velicin

float teplota_ds;                                  // posledni zjistena hodnota teploty primo z cidla
unsigned int teplota_1;                            // globalni promenna pro uchovani posledni zmerene tepoty z prvniho cidla DS18B20 v unsigned int formatu po zapocteni korekce
unsigned int teplota_2;                            // globalni promenna pro uchovani posledni zmerene tepoty z druheho cidla DS18B20 v unsigned int formatu po zapocteni korekce

float teplota_dht;                                 // posledni zjistena hodnota primo z cidla

unsigned long minut_tep_IN[60];                    // promenne pro zapis hodnot na SD kartu s minutovymi hodnotami
unsigned long minut_tep_OUT[60];
unsigned long minut_tlak[60];
unsigned long minut_vlhkost[60];
byte minut_casovazona[60];
byte minut_indexcidla[60];                         // v pripade dvou cidel se zaznamenava index prave aktivniho cidla (cidla, ktere ukazuje nizsi teplotu)

float zobraz_out;
float zobraz_in;


unsigned int budik_on;                             // cas v minutach od pulnoci, pri kterem se ma rozsvitit displej
unsigned int budik_off;                            // cas v minutach od pulnoci, pri kterem se ma zhasnout displej
byte budik_maska;                                  // bitova definice pracovnich dni
unsigned int akt_LOC_hodmin;                       // aktualni pocet minut od pulnoci
boolean budik_blok = false;                        // kvuli moznosti zhasnout rucne displej v pripade, ze je automaticky rozsviceny          
boolean budik_bezi = false;                        // aby i ostatni casti programu vedely, ze je podsvet automaticky rozsviceny.
boolean piskani = false;                           // v prvni minute po automatickem rozsviceni displeje se jeste piska (pokud neni piskak odpojeny vypinacem)
unsigned int zbyva_do_budiku;                      // za jak dlouho nastane dalsi budik (v minutach)
byte dvt_pro_budik;                                // pro ktery den je nastaveny nastedujici budik (pouziva se v ikone na strance detailu pristiho budiku) - 43=Po, 44=Ut, ...49=Ne

volatile byte zpozdeni_podsvetu = KON_ZPOZD_PODSV; // odpocet doby, nez podsvet zhasne (nastavuje se pri preruseni vyvolanem infracidlem pohybu)

byte logovani = 0;                                 // pri resetu, pri nejake zmene konrekci nebo zmene parametru se loguje aktualni stav do specialniho souboru "log.txt"

int vlhkost;
int minula_vlhkost;                                // v pripade chyby cidla vlhkosti se do zaznamu uklada minula hodnota vlhkosti
float cidlo_vlhkosti;                              // hodnota primo z cidla 


byte inout_LED = 1;                                // znacka, jestli se pro LED a rekordy bude pouzivat venkovni, nebo vnitrni LED (na zacatku vnejsi teplota)


byte klouzak;                                      // ukazatel do pole namerenych hodnot pro klouzave prumery
unsigned int i;                                    // pomocna promenna pro ruzne smycky

byte posledni_zaznam_A;                            // znacka, ze uz byl proveden zaznam do historie prubehu tlaku
byte posledni_zaznam_B;                            // znacka, ze uz bylo provedeno hodinove prumerovani tlaku

unsigned int priste_prohledavat_od = 0;            // kvuli rychlejsimu prohledavani databaze casu fazi se uklada posledni nalezeny blok fazi do promenne a priste zacina hledani od tohoto bloku

int polozka_menu = 1;                              // rotatorem se vybra polozka v menu

byte ignoruj_rekordy = 90;                         // cislo 90 odpovida asi 180s (3minuty), po ktere je ignorovana teplota k zapisu do rekordu

int blokuj_priblizeni;                             // v nekterych pripadech se cidlo priblizeni docasne vypina (kdyz je rozsviceny displej, nebo kdyz se displej rucne zhasne)

byte chyba_cidla1 = 0;                             // utrzene cidlo DS18B20 blokuje zaznamy do pameti, testovani rekordu a ovladani LED
byte stav_cidla1  = 0;                             // stav cidla (pri zmene stavu se rozsveci, nebo zhasina ikona vykricniku)  0=OK; 1=ERR

byte chyba_cidla2 = 0;                             // utrzene cidlo DS18B20 blokuje zaznamy do pameti, testovani rekordu a ovladani LED
byte stav_cidla2  = 0;                             // stav cidla (pri zmene stavu se rozsveci, nebo zhasina ikona vykricniku)  0=OK; 1=ERR

byte nalezeny_index_zatmeni;                       // kolikata polozka v databazi zatmeni se nasla jako nasledujici
byte index_zatmeni;                                // pouzito pro listovani v zatmenich
byte prvni_index_zatmeni;                          // pouzito pro listovani v zatmenich pomoci infra DO

uint32_t T0, T1, T2, T3, T0n, T;                   // casy jednotlivych fazi od novu do pristiho novu v jednom mesicnim cyklu. Casy jsou v minutach od 1.1.2016 0:00 UTC
uint32_t cas_pristi_faze;                          // cas pristi faze (vybrany cas T1, T2, T3, nebo T0n )
byte typ_pristi_faze;                              // pristi faze bude prvni ctvrt, uplnek, posledni ctvrt, nebo nov

byte info_zatmeni;                                 // hodnota, ktera udava velikost a typ zatmeni

uint32_t minuty2016UTC = 0;                        // pocet minut UTC casu od 1.1.2016 0:00
unsigned int mes2016;                              // pocet mesicu od ledna 2016 (pouziva se pri listovani v kalendari)

byte SD_aktivni;                                   // v nastaveni paramateru se da zvolit, jestli je aktivni ukladani na SD kartu

byte priblizeni;                                   // promenne pro cidlo priblizeni
unsigned int svetlo;
byte svetlo_L;
byte svetlo_H;

byte status_byte;                                  // bajt, ktery se v pripade pozadavku odesila do seriove linky (obsahuje aktualni stav vsech LED a nekterych ikon)

byte aktualni_graf = 1;                            // pri prepinani grafu pomoci tlacitek [CH+] a [CH-] ukazuje na aktualni graf



//------------------  Astronomicke vypocty Mesice -------------------------------
unsigned int astro_UTC_rok;                                              // promenne pro vypocty souradnic Mesice (pracuje se nimi v ruznych smyckach, tak to nemohou byt bezne 'UTC_xxx' promenne)
int astro_UTC_mes, astro_UTC_den, astro_UTC_hod, astro_UTC_min;

unsigned int astro_LOC_rok;
int astro_LOC_mes, astro_LOC_den, astro_LOC_hod, astro_LOC_min;


unsigned int pred_rok;                                                   // Pri zapadu Mesice se musi zaznamenat cas, ktery byl tesne pred tim, nez Mesic zapadnul 
byte pred_mes, pred_den, pred_hod, pred_min;

boolean nadhorizontem;
double Mes_RA, Mes_DE, Mes_elevace, Mes_azimut, Mes_colongitudo, Mes_osvit, Mes_dist;
double Slu_RA, Slu_DE, Slu_elevace, Slu_azimut;

int mes_vych_azimut, mes_zap_azimut;                                     // Azimut pristiho vychodu a zapadu Mesice
int slu_vych_azimut, slu_zap_azimut;                                     // Azimut dnesniho vychodu a zapadu Slunce

double MS_angle;

//------------------  Astronomicke vypocty Slunce -------------------------------

double z1, z2, nz, xe, ye;
double  sunrisex , sunsetx;
double sunra, sundec;

double s_z1 , s_z2 , s_nz , s_xe , s_ye;


//-------------------------------------------------------------------------------


String nextRiseD_str = "25.12.";
String nextRiseH_str = "15:58";
String nextSetD_str = "25.12";
String nextSetH_str = "15:58";

double GeoLat, GeoLon;
int    GeoAlt;

unsigned long nextRise2016 = 0;
unsigned long nextSet2016 = 0;


unsigned int kruh_az_v;       // pro vypocty s kruhovym grafem azimutu
unsigned int kruh_az_z;
unsigned int kruh_az_a;


//-----------

SdFat sd;                                          // pro SD kartu
SdFile soubor;

PCF8574 pcf(EXPANDER_I2C_ADDRESS);         // spusteni expanderu



Adafruit_BMP085 bmp;                               // inicializace tlakoveho senzoru

DHT vlhkomerDHT(pin_vlhkomer, DHT22);              // inicializace DHT senzoru s nastavenym pinem a typem senzoru

OneWire oneWire_teplomer(pin_DS18B20);             // inicializace teplomeru DS18B20
DallasTemperature DS18B20(&oneWire_teplomer);



//=============================================================================================================
void setup(void)
  {
    afio_cfg_debug_ports(AFIO_DEBUG_NONE);         // zprovozneni JTAG pinu jako obycejnych GPIO (tyka se hlavne PB3, na ktere je cervena LED)
    
    Serial.begin(38400);                           // start komunikace na pinech PA9, PA10 (servisni komunikace / Bluetooth)
    Serial2.begin(38400);                          // start komunikace na pinech PB10,PB11  (displej TJC4832 / Nextion)
    Serial1.begin(9600);                           // start komunikace na pinech PA2, PA3 (RS485 komunikace se vzdalenym cidlem) //!!RS485

    Wire.begin();                                  // start I2C komunikace mezi EERAM, EEPROM, cidlem tlaku a cidlem pohybu 

    screen = 99;                                   // bootovaci obrazovka ma sice index 0, ale protoze je z historickych duvodu pouzit index 0 pro hlavni obrazovku, musi se tady nastavit neco jineho
  
    pcf.begin();                                   // inicializace expanderu


    pinMode(pin_SD_CS, OUTPUT);

    delay(500);

    pinMode(pin_DS18B20PWR,OUTPUT);                 
    digitalWrite(pin_DS18B20PWR,LOW);              // kvuli problemum se startem cidla DS18B20 se na chvili jeho napajeni hodi do LOW
    delay(500);
    digitalWrite(pin_DS18B20PWR,HIGH);             // pak se trvale zapne



    pinMode(pin_LED_info,OUTPUT);
    pinMode(pin_IR_pohyb,INPUT_PULLUP);
    pinMode(pin_DERE,OUTPUT);                      //!!RS485


    pinMode(pin_bzuk,OUTPUT);                     // bzuk pri startu vypnout
    digitalWrite(pin_bzuk, LOW);


    zhasni_R();                       // LEDky maji spolecnou anodu, tak se pomoci HIGH zhasinaji
    zhasni_G();
    zhasni_B();
    zhasni_info();
  
    inout_LED = 1;        // pro LED a rekordy se bude pouzivat venkovni teplota  
  
  
    pinMode(pin_infra, INPUT_PULLUP);
    pinMode(pin_BT_start, OUTPUT_OPEN_DRAIN);
    digitalWrite(pin_BT_start, HIGH);
    stav_BT = 0;
    
    digitalWrite(pin_DERE, LOW);     // smer komunikace RS485 se nastavi na defaultni stav: prijem (SLAVE -->>> MASTER)

    zpozdeni_podsvetu = KON_ZPOZD_PODSV;
    RAM_setup();                                   // nastaveni EERAM na automaticke ukladani do vlastni EEPROM pri vypnuti napajeni
    delay(10);

    Serial2.print("setent.pic=274");
    SerialFFF();



    // prvni balik testu periferii (EEPROM, EERAM, RTC, DISPLEJ)
    // tohle jsou tak zasadni chyby, ze kdyz nastanou, tak se dal nepokracuje
    if (test_periferii_1() == false)             
      {
        // velky napis na bootovaci obrazovku: "Chyba periferii - zastavuji cinnost"                              
        Serial2.print("vis t0,1");
        SerialFFF();
        Serial2.print("t0.txt=\"CHYBA - STOP\"");
        SerialFFF();

        while (true)                 // pri chybe se zastavi program zastavi a ceka na RESET
          {}  
      }


    test_periferii_2();     // otestuje cidla teploty, tlaku a vlhkosti
                            // barvy u polozek se prepinaji bud na zeleno, nebo na zluto i kdyz dojde k nejake chybe, neznamena to konec


    displayTime();                   // prvotni nastaveni casovych promennych



    // posledni test se tyka SD karty
    //  pri prvnim spusteni programu je SD karta s datovymi soubory nutnost
    //     v pripade, ze soubory na karte nejsou, nebo nejsou citelne ... KONEC 
    if (EEPROM_text_read(34) != "Prepinaci uroven 2: ")             
      {
        if (test_periferii_3() == false)             
          {
            // velky napis na bootovaci obrazovku: "Chyba SD karty - zastavuji cinnost"                              
            Serial2.print("vis t0,1");
            SerialFFF();
            Serial2.print("t0.txt=\"CHYBA - STOP\"");
            SerialFFF();
    
            while (true)                 // pri chybe se zastavi program zastavi a ceka na RESET
              {}  
    
          }
        else                             // kdyz SD karta obsahuje potrebne soubory
          {
            SD_EEPROM();                 // prenesou se do EEPROM (zobrazi se obrazovka s progress bary)

            for (i = 0 ; i < 22 ; i++)   // zapis defaultnich infrakodu do EEPROM
              {
                // rozlozeni na 4 bajty a zapsani do EEPROM
                EEPROM_write((i*4) + 25000 , (default_infra_btn[i] >> 24) & 0xFF);
                EEPROM_write((i*4) + 25001 , (default_infra_btn[i] >> 16) & 0xFF);
                EEPROM_write((i*4) + 25002 , (default_infra_btn[i] >>  8) & 0xFF);
                EEPROM_write((i*4) + 25003 ,  default_infra_btn[i] & 0xFF);
              }
            
            tovarni_reset();                             
    
            Serial2.print("page 0");  // na displeji zobrazit bootovací obrazovku
            SerialFFF();
            delay(100);
                                                  
                                                  //  po skonceni prenosu okamzity reset:
            iwdg_init(IWDG_PRE_256, 10);          // nastaveni WD na 50 milisekund  (10*5)
            iwdg_feed();                          // a okamzite obcerstveni WD
            delay(1000);                          // behem pauzy dojde k aktivaci WD a resetu procesoru 
          }
       
        
      }
    
    mod_addr = RAM1_read(EERAM_ADDR_adr_mod);     // adresa SLAVE desky s teplomery




    // pri prvnim spusteni se nastavi tovarni hodnoty do EERAM
    casova_zona = RAM1_read(EERAM_ADDR_timezone);         // zjisteni casove zony z EERAM
    if (casova_zona < 1 or casova_zona > 2)               // kdyz neni casova zona zona v EERAM spravne nastavena
      {
        casova_zona =  2 - pcf.read(4);                   // a casova zona se nastavi podle prepnuteho prepinace v pozici 'A'  (ON="0"=SELC=UTC+2 ; OFF="1"=SEC=UTC+1)
        tovarni_reset();                                  // provede se tovarni reset
      }


    //------------------
    // pokud je pri bootovani stisknut odkaz na web, zmeni se obrazek v objektu 'setent' na displeji na modre kolecko (pic=76)
    // v tom pripade se vleze na servisni displej
    Serial2.print("get setent.pic");
    SerialFFF();
    delay(20);
    Serial2.read();                          // prvni prijaty kod z displeje by mel byt 0x71
    if (Serial2.read() == 76)
      {
        ukaz_servis();                       // servis konci celkovym resetem a pokracuje nactenim ulozenych arametru
      }
    flush_read_buffer2();    
    //--------------------------------------


    GeoLat = RAM1_read_int(EERAM_ADDR_GeoLat) / 100.0;    // zjisteni zemepisne sirky z EERAM a prevedeni na desetinne cislo
    GeoLon = RAM1_read_int(EERAM_ADDR_GeoLon) / 100.0;    // zjisteni zemepisne delky z EERAM a prevedeni na desetinne cislo
    GeoAlt = RAM1_read_int(EERAM_ADDR_GeoAlt);            // zjisteni nadmorske vysky z EERAM


    screen = 0;

    displayTime();                   // prvotni nastaveni casovych promennych a jejich zobrazeni na displeji
    najdi_VZM();

    pauza100();                                    // musi byt nejaka pauza pro obnoveni RAM z EEPROM po zapnuti napajeni (podle kat listu 5ms + nejaka rezerva)




    iwdg_init(IWDG_PRE_256, 1250);                   // nastaveni WD na 8 sekund
    iwdg_feed();                                     // a okamzite obcerstveni WD


    //----------------------------------------------

    SD_aktivni = RAM1_read(EERAM_ADDR_SDaktiv); 

    loguj(0);                                      // loguj reset  

    // nacteni infrakodu z EEPROM do pracovnich promennych
    for (i = 0 ; i < 22 ; i++)
      {
        infra_btn[i] = EEPROM_read_long(6250 + i);       // v EEPROM jsou kody ulozeny od adresy 25000   (25000 / 4 bajty na kod = 6250)
      }



    LED_blok = RAM1_read(EERAM_ADDR_LEDblok); 


    typ_out_teplomeru = RAM1_read(EERAM_ADDR_typ_teplomeru);

  
    // po resetu se nactou casy poslednich zaznamu ze zalohovane EERAM
    posledni_zaznam_A = RAM1_read(EERAM_ADDR_poslednich_5min);
    posledni_zaznam_B = RAM1_read(EERAM_ADDR_posledni_1hod);
  
  
    // zjisteni rekordnich teplot z EERAM
    tep_min = RAM1_read_int(EERAM_ADDR_min_teplota);
    tep_max = RAM1_read_int(EERAM_ADDR_max_teplota);

  
    // zjisteni datumu a casu, kdy doslo k teplotnim rekordum
    for (i = EERAM_ADDR_min_datum; i < EERAM_ADDR_min_datum + 12; i++)
      {
        tep_min_datcas[i - EERAM_ADDR_min_datum] = RAM1_read(i);
      }
  
    for (i = EERAM_ADDR_max_datum; i < EERAM_ADDR_max_datum + 12; i++)
      {
        tep_max_datcas[i - EERAM_ADDR_max_datum] = RAM1_read(i);
      }

    tep_min_datcas_S = tep_min_datcas;
    tep_max_datcas_S = tep_max_datcas;
                        
    // zjisteni datumu a casu, kdy doslo k nulovani rekordu
    for (i = EERAM_ADDR_rst_rekordy; i < EERAM_ADDR_rst_rekordy + 12; i++)
      {
        tep_rst_datcas[i - EERAM_ADDR_rst_rekordy] = RAM1_read(i);
      }
  
  
    budik_on  = RAM1_read_int(EERAM_ADDR_dis_on);
    budik_off = RAM1_read_int(EERAM_ADDR_dis_off);
    budik_maska = RAM1_read(EERAM_ADDR_dis_maska);

  
    prepinaci_uroven1 = RAM1_read_int(EERAM_ADDR_prepinaci_uroven1);    // prepinaci uroven 1 pro modrou LED
    prepinaci_uroven2 = RAM1_read_int(EERAM_ADDR_prepinaci_uroven2);    // prepinaci uroven 2 pro modrou LED

    tep_min1 = RAM1_read_int(EERAM_ADDR_tep_min1);
    tep_max1 = RAM1_read_int(EERAM_ADDR_tep_max1);


    if (tep_min1 >= tep_max1)
      {
        tep_min1 = 14994;
        tep_max1 = 0;
        RAM1_write_int(EERAM_ADDR_hhmm_min1 , 0);
        RAM1_write_int(EERAM_ADDR_hhmm_max1 , 0);
      }
  
    tep_min2 = RAM1_read_int(EERAM_ADDR_tep_min2);
    tep_max2 = RAM1_read_int(EERAM_ADDR_tep_max2);
  
    if (tep_min2 >= tep_max2)
      {
        tep_min2 = 14994;
        tep_max2 = 0;
        RAM1_write_int(EERAM_ADDR_hhmm_min2 , 0);
        RAM1_write_int(EERAM_ADDR_hhmm_max2 , 0);
      }
  

    modra_pamet = RAM1_read(EERAM_ADDR_modra_pamet);
    if (modra_pamet == 0)  modra_pamet_pom = 1;
    else                   modra_pamet_pom = 0;
    
    zhasinat_podsvet = RAM1_read(EERAM_ADDR_zhasinat_podsvet);

    opto_setup();

    zmena_minut = 99;




    if (typ_out_teplomeru < 3)             // pokud je jako venkovni cidlo nastaveno DS18B20 (1x nebo 2x), provede se inicializace sbernice
      {
        oneWire_teplomer.reset();
        delay(300);
    
     
        DS18B20.setResolution(11);                     // nastaveni prumerovani hodnot teplomeru DS18B20 na 11-bitove rozliseni
        iwdg_feed();     // obcerstveni WD
        delay(1000);
    
        DS18B20.requestTemperatures();
        iwdg_feed();     // obcerstveni WD
        delay(1000);                                   // tady musi byt nejaka pauza na stabilizaci prvniho mereni
        
      }
  
  
    // prvni balik dat pro klouzave prumerovani tlaku a teplot se naplni najednou aktualni hodnotou
    teplota_IN[0]  = 500 + (10 * vlhkomerDHT.readTemperature()) - 100 + RAM1_read(EERAM_ADDR_korekce_IN);  // korekce zobrazovane teploty o -10'C  az +15.5'C  pro cisla 0 az 255


    if (typ_out_teplomeru == 3)             // pokud je jako venkovni cidlo nastaveno zarizeni s komunikaci RS485
      {
        // !!RS485
        rs485_get_temp(mod_addr);
        byte pokusy = 0;
        while (chyba_cidla1 > 0 and pokusy < 5)
          { 
            iwdg_feed();     // obcerstveni WD
            pokusy ++;
            delay(1000);
            rs485_get_temp(mod_addr);
          }
        
        if (pokusy >= 4)
          {
            loguj(13);                                     // pri chybe teplomeru se po 5 pokusech loguje 
            teplota_1=1500;                                // falesna teplota se nastavi na 100'C
          }
        
        
      }


    if (typ_out_teplomeru < 3)                     // pokud je jako venkovni cidlo nastaveno DS18B20 (1x nebo 2x)
      {
        teplota_2 = 1500;         // teplota 2 se docasne nastavi na 100'C aby mohla byt ignorovana v pripade, ze bude pouzito jen 1 cidlo
        teplota_1 = ds18b20_1();  // zjisteni teploty z prvniho venkovniho cidla teploty

        byte pokusy = 0;
        while (chyba_cidla1 > 0 and pokusy < 5)
          { 
            digitalWrite(pin_DS18B20PWR,LOW);              // pokus o reset napajeni 
            delay(500);
            digitalWrite(pin_DS18B20PWR,HIGH);             // pak se trvale napajeni zapne
            delay(500);
    
            DS18B20.setResolution(11);                     // nastaveni prumerovani hodnot teplomeru DS18B20 na 11-bitove rozliseni
            iwdg_feed();     // obcerstveni WD
            delay(1000);
            DS18B20.requestTemperatures();
            iwdg_feed();     // obcerstveni WD
            pokusy ++;
            delay(1000);
            teplota_1 = ds18b20_1();  // zjisteni teploty z prvniho venkovniho cidla teploty
          }
        
        if (pokusy >= 4)
          {
            loguj(13);                                     // pri chybe teplomeru se po 5 pokusech loguje 
            teplota_1=1500;                                // a falesna teplota se nastavi na 100'C
          }
      }

    if (typ_out_teplomeru == 2)                 // kdyz jsou nastavena 2 venkovni cidla, zjisti se stejnym zpuseobem jeste hodnota druheho cidla
      {
        teplota_2 = ds18b20_2();                     // zjisteni teploty z druheho venkovniho cidla teploty

        byte pokusy = 0;
        while (chyba_cidla2 > 0 and pokusy < 5)
          { 
            digitalWrite(pin_DS18B20PWR,LOW);              // pokus o reset napajeni 
            delay(500);
            digitalWrite(pin_DS18B20PWR,HIGH);             // pak se trvale napajeni zapne
            delay(500);
    
            DS18B20.setResolution(11);                     // nastaveni prumerovani hodnot teplomeru DS18B20 na 11-bitove rozliseni
            iwdg_feed();     // obcerstveni WD
            delay(1000);
            DS18B20.requestTemperatures();
            iwdg_feed();     // obcerstveni WD
            pokusy ++;
            delay(1000);
            teplota_2 = ds18b20_2();                     // zjisteni teploty z druheho venkovniho cidla teploty
          }
        
        if (pokusy >= 4)
          {
            loguj(13);                                     // pri chybe teplomeru se po 5 pokusech loguje 
            teplota_2=1500;                                // a falesna teplota se nastavi na 100'C
          }

      }

    if (teplota_2 > teplota_1)
      {
        teplota_OUT[0] = teplota_1;
        akt_cidlo = 1;
      }
    else
      {
        teplota_OUT[0] = teplota_2;
        akt_cidlo = 2;
      }

    if (typ_out_teplomeru == 1)
      {
        akt_cidlo = 0;
      }
    
    zobraz_index_cidla();

    chyba_cidla1 = 0;      // pocitadla chyb se vynuluji, aby se v hlavni smycce mohly spravne zobrazit ikony vykricniku
    chyba_cidla2 = 0;


    if  (teplota_OUT[0] > 10 and teplota_OUT[0] < 1400)  prvni_klouzak_OK = true; // kdyz je prvni zmerena teplota v poradku, nastavi se znacka

  
  
    prumer_teplota_OUT = teplota_OUT[0];           // zaroven nastaveni prvnich prumeru na tuto prvni namerenou hodnotu
    prumer_teplota_IN  = teplota_IN[0];

    suma_OUT = 10* teplota_OUT[0];                 // kvuli pripadnemu prepoctu tlaku na hladinu more musi byt pripravena promenna suma_OUT
    tlak[0]        = prepocet_tlaku();             // zjisteni tlaku a jeho pripadny prepocet z absolutni hodnoty na hladinu more
    prumer_tlak = tlak[0];

    for (i = 1; i < 10 ; i++)                      // zaplneni celych poli klouzaku prvni namerenou hodnotou
      {
        tlak[i] = tlak[0];
        teplota_OUT[i] = teplota_OUT[0];
        teplota_IN[i] = teplota_IN[0];
      }
  

    byte pokusy = 0;                               // 5 pokusu o zjisteni vlhkosti pri zapnuti napajeni
    vlhkost = 0;
    while (vlhkost < 1 and pokusy < 5)
      { 
        vlhkost = (int)vlhkomerDHT.readHumidity() - 100 + RAM1_read(EERAM_ADDR_korekce_vlhkost);           // vlhkost se neprumeruje
        pokusy ++;
        iwdg_feed();     // obcerstveni WD
        delay(1500);
      }
    if (pokusy == 4)                              // pri chybe vlhkomeru se po 5 pokusech loguje 
      {
        vlhkost = 50;                             // nastavi se nejaka falesna vlhkost a bude se doufat, ze dalsi mereni uz bude OK
        log_pomprom = 999;
        loguj(14);
      }


    if (LOC_rok < 2016 or LOC_rok > 2070)
      {
        Serial2.print(EEPROM_text_read(244));           // "page 3"
        SerialFFF();
      }
    else
      {
        Serial2.print("page 21");           // "page 21"
        SerialFFF();       
      }



    zobrazeni_minigrafu();  // zobrazeni grafu po zapnuti napajeni
    dis_pulrekordy();    



    
    attachInterrupt(digitalPinToInterrupt(pin_infra)  , infraINT  , FALLING);      // pri prijmu nejake infrakomunikace se spusti preruseni
    attachInterrupt(digitalPinToInterrupt(pin_RTC_INT)  , rtcINT  , FALLING);      // pravidelne sekundove obcerstvovani casu
    attachInterrupt(digitalPinToInterrupt(pin_IR_pohyb)  , pohybINT  , FALLING);   // testovani infracidla pohybu


  
  }
  


//===========================================================================
//                     H L A V N I    S M Y C K A
//===========================================================================
void loop(void)
  {
    pocitadlo ++;                     // pocitadlo smycek
  
    //--------------
    // zobrazeni aktualniho datumu a casu na displeji (aktualizace probiha jen tehdy, kdyz prijde sekundovy impulz od RTC)
    if (aktualizuj_cas == true)
      {

        displayTime();
        odpocet_podsvet();
        aktualizuj_cas = false;

      }
    //--------------
  
  
    switch (pocitadlo)                // pak se v kazde smycce provadi jen jedna operace:
      {
        case 1:                       // v kazde prvni smycce meri tlak a uklada se do pole hodnot, ze kterych se pak vypocitava klouzavy prumer
          mereni_tlaku();
          break;
    
        case 2:                       // v kazde 2. smycce se meri teplota z DS18B20 a uklada do pole hodnot, ze kterych se pak vypocitava klouzavy prumer
          mereni_teploty();
          break;
    
        case 3:                       // v kazde 3. smycce se zmeri a zobrazi vlhkost
          zobrazeni_vlhkosti();
          break;
    
        case 4:                       // v kazde 4. smycce se pocita prumerny tlak pro 5-minutove ukladani do databaze
          vypocti_prumer_tlaku();
          test_pohybu();              // tady se jeste navic otestuje, jestli nedoslo k priblizeni ruky
          break;

        case 5:                       // v kazde 5. smycce se testuje funkce teplotniho cidla
          if (stav_cidla1 == 0 and chyba_cidla1 > 2)      // kdyz je cidlo v poradku a objevi se treti chyba v rade ...
            {
              Serial2.print(EEPROM_text_read(220));      // "ik6.pic=79"              // rozsviceni ikony vykricniku
              SerialFFF();
              rozsvit_info();                            // info LED prejde do inverzniho rezimu (bude trvale svitit a pri priblizeni k cidlu kratce pohasne)
              stav_cidla1 = 1;
              
            }
          if (stav_cidla1 == 1 and chyba_cidla1 == 0)     // kdyz bylo cidlo nefunkcni, ale ted se vzpamatovalo
            {
              Serial2.print(EEPROM_text_read(221));      // "ik6.pic=80"              // zhasnuti ikony vykricniku
              SerialFFF();
              stav_cidla1 = 0;
              zhasni_info();                             // info LED prejde do normalniho rezimu (bude trvale zhasnuta a pri priblizeni k cidlu kratce blikne)
              
            }      

          if (stav_cidla2 == 0 and chyba_cidla2 > 2)      // kdyz je cidlo v poradku a objevi se treti chyba v rade ...
            {
              Serial2.print(EEPROM_text_read(220));      // "ik6.pic=79"              // rozsviceni ikony vykricniku
              SerialFFF();
              rozsvit_info();                            // info LED prejde do inverzniho rezimu (bude trvale svitit a pri priblizeni k cidlu kratce pohasne)
              stav_cidla2 = 1;
              
            }
          if (stav_cidla2 == 1 and chyba_cidla2 == 0)     // kdyz bylo cidlo nefunkcni, ale ted se vzpamatovalo
            {
              Serial2.print(EEPROM_text_read(221));      // "ik6.pic=80"              // zhasnuti ikony vykricniku
              SerialFFF();
              stav_cidla2 = 0;
              zhasni_info();                             // info LED prejde do normalniho rezimu (bude trvale zhasnuta a pri priblizeni k cidlu kratce blikne)
              
            }      
          
          
          break;

        case 6:
         if (opto_Read(0x9C) < 50)            // pri oddaleni ruky smazat priblizovaci interrupt 
           {
              opto_Write(0xE5,0b00000000);
           }
         break;

    

        case 7:                       // v kazde 7. smycce se zmeri vnitrni teplota z vlhkomeru  a zobrazi se prumer na displeji
          zobraz_vnitrni_teplotu();
          break;
    
        case 8:                       // v kazde 8. smycce se vyhodnoti prumerna teplota DS18B20 a urci se stav LED podle nastavene funkce
          vyhodnot_vnejsi_teplotu();
          break;


        case 9:                      // v kazde 9. smycce se otestuje pohyb
          test_pohybu();
          break;


        case 10:                      // v kazde 10. smycce se testuje infra
          infraSwitch();
          break;

    
    
        case 11:                      // v kazde 11. smycce se obslouzi LEDky (pokud maji zapnutou prislusnou funkci)
          obsluha_LED();
          break;

    
        case 12:                      // v kazde 12. smycce se budou provadet vypocty ohledne fazi Mesice
          vypocty_mesice();          
          break;

        case 14:                      // v kazde 14. smycce se otestuje pohyb
          test_pohybu();
          break;
    
        case 15:                      // v kazde 15. smycce se testuje, jestli se nemají prepnout denni a nocni rekordy
          prepinac_rekordu();
          break;


        case 16:                      // v kazde 16. smycce se otestuje stav prepinace SEC/SELC a zobrazi se prislusny napis (SEC/SELC)
          vyhodnot_SEC_SELC();
          break;

   
        case 17:                      // v kazde 17. smycce se testuje jestli se zmenila hodnota minut (kvuli ruznym casovym blokovacim pauzam)
          if (LOC_min != zmena_minut)
            {
              zmena_minut = LOC_min;
              STOP_LED_REK = RAM1_read_int(EERAM_ADDR_stop_RG);
              if (STOP_LED_REK > 0)
                {
                  STOP_LED_REK --;
                  RAM1_write_int(EERAM_ADDR_stop_RG,STOP_LED_REK);
                }
    
              priprav_minutove_zaznamy();          // priprava dat pro SD kartu se provadi kazdou minutu

              aktualizuj_soubor_s_rekordy();

              if ((nextRise2016 < minuty2016UTC + (casova_zona * 60))  or (nextSet2016 < minuty2016UTC + (casova_zona * 60)))  // kazdou minutu se zkontroluje, jestli by se nemely prepocitat vychody a zapady Mesice
                {
                  najdi_VZM();
                }


              // zaznam do "rychle" oblasti RAM - posledni 3 hodiny stare prubehy tlaku, teploty a vlhkosti
              if (LOC_min % 5 == 0 and LOC_min != posledni_zaznam_A)  // kazdych 5 minut se ukladaji zmerene hodnoty do pole (ale jen v pripade, ze v aktualni minute jeste nebylo nic uozeno)
                {
                  zaznam_5min();
                }
            
            
            
              // zaznam do oblasti RAM s hodinovymi prumery 
              if (LOC_min == 0 and LOC_hod != posledni_zaznam_B)  // kazdou celou hodinu se uklada prumer zmerenych hodnot do pole (ale jen v pripade, ze v tuto hodinu jeste nebylo nic ulozeno)
                {
                  zaznam_1hod();
                  zobrazeni_minigrafu();         // kazdou hodinu se aktualizuji grafy
                }
            }
          break;


        case 18:                                            // automaticke rozsviceni displeje v nastaveny cas
          akt_LOC_hodmin = (LOC_hod * 60) + LOC_min;

          budik_iko();                                      // zobrazeni, nebo nezobrazeni ikony budiku - podle toho, jestli dalsi zvoneni nastane za mene nez 24 hodin
          
          if (akt_LOC_hodmin > budik_off or akt_LOC_hodmin < budik_on)  // kdyz je cas mimo nastavenou oblast pro sviceni displeje ...
            {
              budik_blok = false;                                       // ... budik se 'natahne'
              budik_bezi = false;
            }

          if (akt_LOC_hodmin != budik_on)             // budik piska jen v pripade, ze se shoduje aktualni cas a cas automatickeho rozsviceni podsvetu
            {
              piskani = false;                        // po minute piskani se piskani vypina (pokud bylo vubec aktivovane maskou)
            }

          if (budik_blok == false)
            {
              if (akt_LOC_hodmin >= budik_on and akt_LOC_hodmin < budik_off)
                {
                  if (bitRead(budik_maska, 8-LOC_dvt) == true)
                    {
                      budik_bezi = true;
                      if (zpozdeni_podsvetu < 5)
                        {
                          ukazatel_melodie = 0;
                          zpozdeni_podsvetu = 15;
                          podsvet(true);
                          displej_page0();         // displej se musi rozsvitit na hlavni obrazovce
                          piskani = true;          // znacka, ze se ma zacit piskat (piska se jen prvni minutu z nastaveneho intervalu)          

                        }
                      zpozdeni_podsvetu = 15;          // kdyz je "budik" aktivni, rozsviti se na 12 sekund podsvet. Pri dalsi smycce se cas prodlouzi na dalsich 12 sekund
                      
                    }
                }
            }
          break;
    

        case 19:                                            // ovladani piskaku (prvni minuta pri automatickem rozsvecovani displeje)
          if (piskani == true)
            {
              if (pcf.read(5) == HIGH)
                {
                  tone(pin_bzuk,melodie[ukazatel_melodie]);
                }
              ukazatel_melodie ++;
              if (ukazatel_melodie > 29) ukazatel_melodie = 0;
            }
          break;
    


        case 20:                      // v kazde 20. smycce se znova otestuje pohyb
          test_pohybu();
          break;


        case 21:                      // v kazde 21. smycce se znova testuje infra
          infraSwitch();
          break;



        case 24:                                            // at se deje cokoliv, tak ve 24. smycce se piskak vypina (kdyz je budik zapnuty, tak docasne, jinak trvale)
          noTone(pin_bzuk);                                 // pri aktivnim budiku bude dalsi zapiskani az v 19. smycce (po 1,6 sekunde)
          break;


    
        case 25:                      // v posledni smycce se pocitadlo smycek vynulje.
          pocitadlo = 0;
          if (ignoruj_rekordy > 0)  ignoruj_rekordy --;        // odpocet casu po restartu, po ktery se nezaznamenavaji rekordy
          test_pohybu();              // zaroven se jeste otestuje, jestli nedoslo k priblizeni ruky k cidlu
          break; 
      }
  
  
    
    //--------------
    if (Serial.available())  // neco prislo ze serioveho portu
      {
        komunikace();
      }
    //--------------
  

    //--------------
    if (Serial2.available())  // neco prislo z dotykoveho panelu
      {
        dis_prijem();
      }
    //--------------
  
  
    iwdg_feed();     // obcerstveni WD v hlavni smycce
    delay(80);       // hlavni casovani smycky (25 smycek * 0.08s = 2 sekundy na cely cyklus. K tomu je treba jeste pripocitat dalsi zpozdeni spojene s vypocty a displejem )
  
  }

// =======================   KONEC HLAVNI SMYCKY ====================================









// ================  PODPROGRAMY ===================================


// opakujici-se sekvence prikazu pri zobrazeni grafu pres infra ovladac
void infragraf(byte cislo_grafu)
  {
    if (zpozdeni_podsvetu > 0)
      {
        zpozdeni_podsvetu = KON_ZPOZD_PODSV;        // ochrana proti nahodnemu rozsveceni displeje
      }
     
    Serial2.print(EEPROM_text_read(240));     // "page "
    Serial2.print(cislo_grafu + 7);
    SerialFFF();
    screen = cislo_grafu + 7;
    
    bargraf(cislo_grafu + 2);   
  }



void podsvet(boolean zapni)
  {
    if (zapni == true)
      {
        // zjisteni okolniho jasu cidlem a podle zjistene hodnoty se rozsviti podsvet displeje
        //  !!! tohle jeste  doladil podle skutecne zmerenych hodnot
        
        byte okolni_jas = map(opto_svetlo(),5,1025,30,90);

        Serial2.print("dim=");
        Serial2.print(okolni_jas);
      }
    else
      {
        if (zhasinat_podsvet == 1)       // podsvet se zhasne, jen kdyz je to povolene
          {
            Serial2.print("dim=0");      // pri zhasnuti podsvetu se nastavi prazdna obrazovka ("BLACK"), na ktere se da kamkoliv kliknout
            SerialFFF();
            Serial2.print(EEPROM_text_read(247));        // "page 6"
            screen = 6;
          }
      }
    SerialFFF();
  }


void priprav_minutove_zaznamy(void)
  {
    if (LOC_min != 0)                       // kdykoliv krome nulte minuty ...
      {
        rokmesden = LOC_rok * 10000 + LOC_mes*100 + LOC_den;  // ... se zaznamena zacatek obdobi, pro ktere plati zaznamenane hodnoty (napriklad pri aktualnim case "23.5.2019 16:48" se zaznamena 2019052316.)
        predchozi_obdobi = (rokmesden * 100) + LOC_hod;

      }
    else                                    // na zacatku kazde hodiny se zaznamy z predchoziho obdobi nejdrive zapisi na SD kartu vcetne informace, o ktere obdobi se jedna
      {

        save_file();                        // na zacatku kazde hodiny ulozi celou predchozi hodinu na kartu
        
      }

    // v kazdem pripade se pak aktualni hodnoty ulozi do prislusnych minutovych bunek v polich
    minut_tep_IN[LOC_min]     = prumer_teplota_IN;
    minut_tep_OUT[LOC_min]    = prumer_teplota_OUT;
    minut_tlak[LOC_min]       = prumer_tlak - 60000; 
    minut_vlhkost[LOC_min]    = vlhkost;
    minut_casovazona[LOC_min] = casova_zona;
    minut_indexcidla[LOC_min] = akt_cidlo;
  }





//---------------------------------------
// zobrazi na displeji velkou ikonu Mesice
//  ikony jsou po 3 stupnich colongituda a v resourcich zacinaji od indexu 275
void zobraziko(byte index_iko)
  {
    Serial2.print("mesiko.pic=");
    Serial2.print(index_iko + 275);     // 275 je prvni obrazek z baliku malych ikon Mesice v resourcich
    SerialFFF();
  }








void save_file()
  {
    if (SD_aktivni == 1)
      {
        jmeno_slozky[0]  = 48 + ((LOC_rok % 10000) / 1000 );   // tisice let
        jmeno_slozky[1]  = 48 + ((LOC_rok % 1000)  /  100 );   // stovky let
        jmeno_slozky[2]  = 48 + ((LOC_rok % 100)   /   10 );   // desitky let
        jmeno_slozky[3]  = 48 + ((LOC_rok % 10))           ;   // jednotky roku
        
        jmeno_souboru[0]  = 48 + ((rokmesden % 100000000)   /  10000000 );   // rad tisicu roku
        jmeno_souboru[1]  = 48 + ((rokmesden % 10000000)    /   1000000 );   // rad stovek roku
        jmeno_souboru[2]  = 48 + ((rokmesden % 1000000)     /    100000 );   // rad desitek roku
        jmeno_souboru[3]  = 48 + ((rokmesden % 100000)      /     10000 );   // rad jednotek roku
        jmeno_souboru[4]  = 48 + ((rokmesden % 10000)       /      1000 );   // rad desitek mesicu
        jmeno_souboru[5]  = 48 + ((rokmesden % 1000)        /       100 );   // rad jednotek mesicu
        jmeno_souboru[6]  = 48 + ((rokmesden % 100)         /        10 );   // rad desitek dni
        jmeno_souboru[7]  = 48 + ((rokmesden % 10)                      );   // rad jednotek dni
    
    //  jmeno_souboru[8]  = '.';
    //  jmeno_souboru[9]  = 'c';
    //  jmeno_souboru[10] = 's';
    //  jmeno_souboru[11] = 'v';
    //  jmeno_souboru[12] = '\0';
    
        for (byte i = 0; i< 4 ; i++)
          {
            cesta[i] = jmeno_slozky[i];
          }
    
        cesta[4] = '/';
        for (byte i = 5; i< 18 ; i++)
          {
            cesta[i] = jmeno_souboru[i-5];
          }
    
    
    
       
        
        sd.begin(pin_SD_CS,SD_SCK_HZ(F_CPU/4));       //inicializace SD karty pro pripad, ze by byla predtim vytazena
    
        if (!sd.exists(jmeno_slozky)) 
          {
            sd.mkdir(jmeno_slozky);            
          }
    
        boolean bude_hlavicka = false;
        if (!sd.exists(cesta))
          {
            bude_hlavicka = true;
          } 
    
        SdFile::dateTimeCallback(dateTime);
        open_OK = soubor.open(cesta, O_WRITE | O_APPEND | O_CREAT);
        
        if (open_OK)
          {
            if (bude_hlavicka == true)
              {
                soubor.println("Datum a cas;casova zona;teplota_IN [dig];teplota_OUT [dig];index OUT cidla;tlak [Pa]-60000;vlhkost [%];teplota_IN ['C];teplota_OUT ['C];tlak [Pa]");
              }
         
        
            for (byte zaznamy = 0; zaznamy < 60 ; zaznamy ++)       // odeslani hodnot z posledni hodiny na SD kartu
              {
                 soubor.print(predchozi_obdobi/100.0);          // poradove cislo na zacatku radky odpovida datumu a casu (yyyymmddhh)- napriklad "2 019 052 316" = 23.5.2019 16:00 az 16:59
                 soubor.print(':');
                 if (zaznamy < 10) soubor.print ('0');    // pak nasleduje cislo minut (cislo zaznamu) vcetne uvodni nuly (00 az 59)
                 soubor.print (zaznamy);
                 soubor.print(';');                       // a pak uz jen zaznamenana data oddelena strednikem
                 if (minut_tep_IN[zaznamy] == 0 and minut_tep_OUT[zaznamy] == 0) // kdyz ale zadna data nejsou, ulozi se misto nul jen prazdne hodnoty
                   {
                    soubor.println(";;;;;;;;");
                   }
                 else
                   {
                     soubor.print(minut_casovazona[zaznamy]);
                     soubor.print(';');
                     soubor.print(minut_tep_IN[zaznamy]);
                     soubor.print(';');
                     soubor.print(minut_tep_OUT[zaznamy]);
                     soubor.print(';');
                     soubor.print(minut_indexcidla[zaznamy]);
                     soubor.print(';');
                     soubor.print(minut_tlak[zaznamy]);
                     soubor.print(';');
                     soubor.print(minut_vlhkost[zaznamy]);
                     soubor.print(';');
                     soubor.print(zobraz_cislo(minut_tep_IN[zaznamy]*10 , ','));   // jako oddelovac desetinne casti se v teplotach pouzije carka (jednodussi import do CZ-Excelu)
                     soubor.print(';');
                     soubor.print(zobraz_cislo(minut_tep_OUT[zaznamy]*10, ','));
                     soubor.print(';');
                     soubor.println(minut_tlak[zaznamy]+60000);  // posledni hodnota na radce je ukoncena CRLF
                   }      
              }    
            ikona5(false);                // ikona vykricniku po uspesnem zapisu zmizi
          }
        else
          {
            ikona5(true);                 // pri chybe karty se zobrazi ikona vykricniku
          }
       soubor.close();
      }   
  }



// zobrazeni vykricniku nebo prazdneho policka na pozici ik6 podle toho, jestli je nejaky problem s kartou
void ikona5(boolean vykricnik)
  {
    if (SD_aktivni == 1)
      {
        if (vykricnik == true)
          {
            Serial2.print(EEPROM_text_read(317));      // "hlv.ik5.pic=265"              // vykricnik na ikone SD karty
          }
        else
          {
            Serial2.print(EEPROM_text_read(318));      // "hlv.ik5.pic=78"               // normalni ikona SD karty
          }
        SerialFFF();
      }
    else
      {
        Serial2.print(EEPROM_text_read(186));           // "ik5.pic=80"            // zadna ikona SD karty
        SerialFFF();
      }
  
  }



// zobrazi ikonku pristi faze a z databazoveho udaje o poctu minut vypocte datum ("D.M.") nebo cas ("hh:nn")
void pristifaze (byte ikofaze, uint32_t minuty_faze)
  {

    int  pomdny[] = { 31,  28,   31,   30,  31,  30,  31,  31,  30,  31,  30,  31 };
  
    int dny2016 = minuty_faze / 1440;           // v databazi jsou casy fazi v UTC
  
    UTC_min = minuty_faze % 1440;
    UTC_hod = UTC_min / 60;
    UTC_min = UTC_min - (UTC_hod * 60);

    unsigned int rok = 2016;
    while (dny2016 > 0)
      {
        if ((rok % 4) == 0)     // pri prestupnem roku se nastavuje unor na 29 dni
          {
            pomdny[1] = 29;
          }
        else
          {
            pomdny[1] = 28;
          }
    
    
        for (byte mes = 0; mes < 12 ; mes++)
          {
            dny2016 = dny2016 - pomdny[mes];
            if (dny2016 < 0)
              {
                UTC_den = dny2016 + pomdny[mes] + 1;
                UTC_mes = mes + 1;
                UTC_rok = rok;
                z_UTC_na_LOC(casova_zona);          // vsechny vypocty casu pristi faze se delaji v UTC, ale na zaver pred zobrazenim se UTC datumy a casy prevedou na mistni datumy a casy pristi faze
                //                  (nastavi se promenne "faze_LOC_xxx")

                pamet_ikofaze = ikofaze + 36;

                break;
              }
          }
    
        rok ++;
      }
  


  
  
  }


//-------------------------------------------------------------
//    vsechny casy v RTC, na displeji a pri zadavani jsou lokalni. Vypocty fazi Mesice jsou ale v UTC proto se obcas musi prevadet mistni datum a cas na UTC
// vysledky jsou v promennych "UTC_xxx"
void z_LOC_na_UTC(int offset)    // posun z lokalniho casu do UTC (pro SEC je to 1, pro SELC je to 2)
  {
  
    if ((LOC_rok % 4) == 0)     // pri prestupnem roku se nastavuje unor na 29 dni
      {
        pomdny_d[2] = 29;
      }
    else
      {
        pomdny_d[2] = 28;
      }
    
    UTC_min = LOC_min;                       // na zacatku se UTC promenne nastavi na stejnou hodnotu jako mistni cas
    UTC_rok = LOC_rok;
    UTC_mes = LOC_mes;
    UTC_den = LOC_den;
    UTC_hod = LOC_hod - offset;              // ... akorat od hodin se odecte casovy posun (1 nebo 2 hodiny)
  
    if (UTC_hod < 0)                         // kontrola, jestli po odecteni jedne, nebo dvou hodin nedoslo k podteceni casu do predchozicho dne
      {
        UTC_hod = UTC_hod + 24;
        UTC_den = UTC_den - 1;
        if (UTC_den < 1)                     // kontrola podteceni do predchoziho mesice
          {
            UTC_mes = UTC_mes - 1;
            if (UTC_mes < 1)                 // kontrola podteceni do predchoziho roku
              {
                UTC_rok = UTC_rok - 1;
                UTC_mes = 12;
              }
            UTC_den = pomdny_d[UTC_mes];       // cislo dne se nastavi na nejvyssi cislo dne v predchozim mesici
          }
      
      }
    
  }
  

//-------------------------------------------------------------
// prevod z UTC casu faze na mistni cas faze
// podobny podprogram jako v predchozim pripade, akorat se misto odecitani casove zony pricita
// vysledky jsou v promennych "faze_LOC_xxx"
void z_UTC_na_LOC(int offset)
  {
  
    if ((UTC_rok % 4) == 0)                              // pri prestupnem roku se nastavuje unor na 29 dni
      {
        pomdny_d[2] = 29;
      }
    else
      {
        pomdny_d[2] = 28;
      }
  
    faze_LOC_min = UTC_min;                              // na zacatku se mistni datumy a casy nastavi na stejnou hodnotu, jako UTC
    faze_LOC_rok = UTC_rok;
    faze_LOC_mes = UTC_mes;
    faze_LOC_den = UTC_den;
    faze_LOC_hod = UTC_hod + offset;                     //  ... akorat k hodinam se pripocte posun casove zony
  
  
    if (faze_LOC_hod > 23)                               // kontrola preteceni hodin do nasledujiciho dne
      {
        faze_LOC_hod = faze_LOC_hod - 24;
        faze_LOC_den = faze_LOC_den + 1;
        if (faze_LOC_den > pomdny_d[faze_LOC_mes])         // kontrola preteceni dnu do nasledujiciho mesice
          {
            faze_LOC_mes = faze_LOC_mes + 1;
            if (faze_LOC_mes > 12)                       // kontrola preteceni mesicu do nasledujiciho roku
              {
                faze_LOC_rok = faze_LOC_rok + 1;
                faze_LOC_mes = 1;
              }
            faze_LOC_den = 1;
          }
    
      }
  
  }



// vypocte z globalnich LOC_xxx promennych (z udaju v RTC) pocet hodin od zacatku roku
unsigned int LOC_hod_v_roce(void)
  {
    unsigned int vysledek = 0;
    if ((LOC_rok % 4) == 0)     // pri prestupnem roku se nastavuje unor na 29 dni
      {
        pomdny_d[2] = 29;
      }
    else
      {
        pomdny_d[2] = 28;
      }
  
    for (i = 1; i < LOC_mes ; i++)       // nejdriv se poscitaji vsechny DOKONCENE mesice od zacatku roku
      {
        vysledek = vysledek + pomdny_d[i];
      }
  
    vysledek = vysledek + LOC_den - 1;   // pricte se k tomu pocet dni v aktualnim mesici - BEZ DNESKA
    vysledek = vysledek * 24;            // dny se prevedou na hodiny
    vysledek = vysledek + LOC_hod;       // a prictou se k tomu hodiny od dnesni pulnoci
    return vysledek;
  }



// ------------------------------------------------------------
// zpozdene zhasnuti podsvetu po 60s
void odpocet_podsvet(void)
  {
    // obsluha podsvetu
    if (zpozdeni_podsvetu > 0)
      {
        
        if (zpozdeni_podsvetu == KON_ZPOZD_PODSV)
          {
            podsvet(true);   // rozsveceni podsvetu
          }
        if (zpozdeni_podsvetu == 1)
          {
            if (zhasinat_podsvet == 1)
              {
                podsvet(false);  // zhasinani podsvetu po nastavenem poctu prubehu hlavni smyckou                  
              }
            else                                           // kdyz je nastavene trvale sviceni, tak se po ukonceni odpoctu aktivuje dalsi odpocet automaticky
              {
                zpozdeni_podsvetu = KON_ZPOZD_PODSV + 1;
              }
          }

        zpozdeni_podsvetu --;        
      }

  }


//-------------------------------------------------------------------------------------------
// z databaze zatmeni zjisti velikost zatmeni Slunce nebo typ zatmeni Mesice
String velikost_zatmeni (uint32_t cas)
  {
    String navrat;
    byte hodnota;
    for (i = 20000; i < 20000 + (pocet_zatmeni * 5) ; i = i + 5 )
      {
        unsigned long cas_zatmeni = EEPROM_read_longZ(i);
        if (cas_zatmeni == cas)
          {
            hodnota = EEPROM_read(i + 4);
            if (hodnota <= 100)                               // Pri zatmeni Slunce se vraci velikost zatmeni v %
              {
                navrat = String(hodnota) + String("% Za.S.");
              }
            else                                              // Pri zatmeni Mesice se vraci zkratka typu zatmeni
              {
                if (hodnota == 201) navrat = "Polostin";               // polostinove zatmeni
                if (hodnota == 202) navrat = "Ca. Za.M.";              // castecne zatmeni (bez udanych procent)
                if (hodnota == 203) navrat = "100%  M.";         // uplne zatmeni
                if (hodnota > 100 and hodnota < 200) navrat = String(hodnota - 100)  + "% Za.M." ; // castecne zatmeni s udanymi procenty
              }
            nalezeny_index_zatmeni = (i - 20000) / 5;
            break;
          }
      }
   
    return navrat;
  }




//----------------------------------------------------------------------------------
// zaznam vsech 4 kanalu (tlak, teplota OUT, teplota IN, vlhkost) do RAM - kazdych 5 minut
void zaznam_5min(void)
  {
    if (chyba_cidla1 > 0 or chyba_cidla2 > 0) return;        // pri utrzenem cidle nic neukladej
  
    for (i = 0; i < 149 ; i++)
      {
        RAM1_write_int(i      , RAM1_read_int(i +   1));  // cela pamet historie tlaku se posune o jednu pozici vlevo (nejstarsi prvek na pozici 0 se smaze)
        RAM1_write_int(i + 150, RAM1_read_int(i + 151));  // cela pamet historie venkovni teploty se posune o jednu pozici
        RAM1_write_int(i + 300, RAM1_read_int(i + 301));  // cela pamet historie vnitrni teploty se posune o jednu pozici vlevo
        RAM1_write_int(i + 450, RAM1_read_int(i + 451));  // cela pamet historie vlhkosti se posune o jednu pozici vlevo
        RAM1_write_int(i + 600, RAM1_read_int(i + 601));  // cela pamet zaznamu casovych znacek se posune o jednu pozici vlevo
      }
  
    RAM1_write_int(149, prumer_tlak - 60000);         // na konec zaznamu se ulozi aktualni hodnota tlaku (do RAM se uklada hodnota tlaku snizena o 60000, takze se do dvou bajtu vejde hodnota od 60000Pa do 125535Pa)
    RAM1_write_int(299, prumer_teplota_OUT);          // na konec zaznamu se ulozi aktualni hodnota teploty
  
    RAM1_write_int(449, prumer_teplota_IN);          // na konec zaznamu se ulozi aktualni hodnota teploty
  
    RAM1_write_int(599, vlhkost);                    // na konec zaznamu se ulozi aktualni hodnota vlhkosti
    RAM1_write_int(749, (LOC_hod * 60) + LOC_min);   // na konec zaznamu se ulozi znacka aktualniho casu v minutach od pulnoci
  
    posledni_zaznam_A = LOC_min;                    // aby po kratkem resetu nedochazelo k opakovanemu zapisu, uklada se do EERAM i aktualni hodnota minut
    RAM1_write(EERAM_ADDR_poslednich_5min, posledni_zaznam_A);
  }





//------------------------------
// prumerovani poslednich 12 zaznamu z kazdeho kanalu a ulozeni do RAM - kazdou hodinu
void zaznam_1hod()
  {

    if (chyba_cidla1 > 0 or chyba_cidla2 > 0) return;        // pri utrzenem cidle nic neukladej  
    
    for (i = 0; i < 149 ; i++)
      {
        RAM2_write_int(i      , RAM2_read_int(i +   1));  // cela pamet historie tlaku se posune o jednu pozici vlevo (nejstarsi prvek na pozici 250 se smaze)
        RAM2_write_int(i + 150, RAM2_read_int(i + 151));  // cela pamet historie venkovni teploty se posune o jednu pozici vlevo (nejstarsi prvek na pozici 400 se smaze)
        RAM2_write_int(i + 300, RAM2_read_int(i + 301));  // cela pamet historie vnitrni teploty se posune o jednu pozici vlevo (nejstarsi prvek na pozici 550 se smaze)
        RAM2_write_int(i + 450, RAM2_read_int(i + 451));  // cela pamet historie vlhkosti se posune o jednu pozici vlevo (nejstarsi prvek na pozici 700 se smaze)
        RAM2_write_int(i + 600, RAM2_read_int(i + 601));  // cela pamet casovych znacek se posune o jednu pozici vlevo (nejstarsi znacka na pozici 850 se smaze)
      }
  
    // vypocet prumerneho tlaku z poslednich 12 zaznamu (to je z posledni hodiny)
    suma = 0;
    byte pocet = 0;
    for (i = 138; i < 150; i++)
      {
        if (RAM1_read_int(i) < 65535)
          {
            suma = suma + RAM1_read_int(i);
            pocet ++;
          }
      }
    prumer = (suma + (pocet / 2)) / pocet;      // vypocet prumeru se zaokrouhlenim
    RAM2_write_int(149, prumer);  // na konec zaznamu se ulozi aktualni hodnota tlaku
  
  
    // vypocet prumerne venkovni teploty z poslednich 12 zaznamu (to je z posledni hodiny)
    suma = 0;
    pocet = 0;
    for (i = 288; i < 300; i++)
      {
        if (RAM1_read_int(i) < 65535)
          {
            suma = suma + RAM1_read_int(i);
            pocet ++;
          }
      }
    prumer = (suma + (pocet / 2)) / pocet;      // vypocet prumeru se zaokrouhlenim
    RAM2_write_int(299, prumer);  // na konec zaznamu se ulozi aktualni hodnota venkovni teploty
  
  
  
  
  
    // vypocet prumerne vnitrni teploty z poslednich 12 zaznamu (to je z posledni hodiny)
    suma = 0;
    pocet = 0;
    for (i = 438; i < 450; i++)
      {
        if (RAM1_read_int(i) < 65535)
          {
            suma = suma + RAM1_read_int(i);
            pocet ++;
          }
      }
    prumer = (suma + (pocet / 2)) / pocet;      // vypocet prumeru se zaokrouhlenim
    RAM2_write_int(449, prumer);  // na konec zaznamu se ulozi aktualni hodnota vnitrni teploty
  
  
  
  
    // vypocet prumerne vlhkosti z poslednich 12 zaznamu (to je z posledni hodiny)
    suma = 0;
    pocet = 0;
    for (i = 588; i < 600; i++)
      {
        if (RAM1_read_int(i) < 65535)
          {
            suma = suma + RAM1_read_int(i);
            pocet ++;
          }
      }
    prumer = (suma + (pocet / 2)) / pocet;      // vypocet prumeru se zaokrouhlenim
    RAM2_write_int(599, prumer);                 // na konec zaznamu se ulozi aktualni hodnota vlhkosti
  
    unsigned int hodiny_od_1_1 = LOC_hod_v_roce();       // hodiny od zacatku roku
  
    RAM2_write_int(749, hodiny_od_1_1);         // na konec zaznamu se ulozi aktualni casova znacka (pocet hodin od zacatku roku)
  
    posledni_zaznam_B = LOC_hod;                // aby po kratkem resetu nedochazelo k opakovanemu zapisu, uklada se do EERAM i aktualni hodnota hodin
    RAM1_write(EERAM_ADDR_posledni_1hod, posledni_zaznam_B);

    if ((hodiny_od_1_1 % 24) == 0)   // pokud je prave pulnoc zaznamena se jeste znacka do pameti dnu v tydnu (kvuli dlouhodobym grafum)
      {
        RAM2_write_int(749 + LOC_dvt , hodiny_od_1_1);       // uklada se na pozice 750 (Po) az 756 (Ne) 
      }
    
  }







//----------------------------------------------------------
// podprogramy pro praci s cidlem priblizeni

// zakladni nastaveni pro praci v rezimu mereni vzdalenosti a jasu (barev). Pin INT se aktivuje automaticky pri priblizeni.
void opto_setup(void)
  {
    opto_Write(0x80,0b00100111);        // zapnout rezim priblizeni a rezim zjistovani jasu (barev)

    opto_Write(0x89,0);                 // nastaveni okna, pri kterem NEDOCHAZI k priblizovacimu interruptu v rozsahu (0 az 50)
    opto_Write(0x8B,50);                //               (interrupt nastane kdyz priblizeni prekroci cislo 50)
    
    opto_Write(0x8C,0b00100000);        // prvni 2 vzorky priblizeni v rade, ktere budou mimo vymezene okno, nastavi INT pin do LOW  (bity <7> az <4>)
    opto_Write(0x8E,0b01011111);        // delka pulzu (bity <7> a <6>) a pocet pulzu (bity <5> az <0>) pro rezim priblizeni (nastavuje se tim citlivost)

    opto_Write(0x8F,0b11001111);        // proud IR diodou a nastaveni zesileni pro cidlo priblizeni a cidla barev

    opto_Write(0x90,0b00000000);        // saturacni interrupty zrusit

    opto_Write(0xE5,0b00000000);        // smazani interruptu (pokud nejaky nastal) - INT vystup nastavi do HIGH
  }


// zjisti aktualni stav osvetleni
unsigned int opto_svetlo(void)
  {
    svetlo_L = opto_Read(0x94);
    svetlo_H = opto_Read(0x95);
    svetlo = (svetlo_H << 8) | svetlo_L ;
    return svetlo;
  }



void opto_Write(uint8_t registr, uint8_t val)
  {
    
   
    Wire.beginTransmission(I2C_ADDR_opto);
    Wire.write(registr);
    Wire.write(val);
    Wire.endTransmission();
  }


uint8_t opto_Read(uint8_t registr)
  {
    byte val;

    Wire.beginTransmission(I2C_ADDR_opto);
    Wire.write(registr);
    Wire.endTransmission();
    
    Wire.requestFrom(I2C_ADDR_opto, 1);
    while (Wire.available())
      {
        val = Wire.read();
      }
    return val;
  }






//---------------------------------------------------------------------------------------------------
// prevede cast pole prijato[] na integer   (reseni s podprogramem je hlavne kvuli uspore mista v PROGMEM)
// priklad:                            prijato = { 50,  48,  49,  56}
// v ASCII znacich to je               prijato = {'2', '0', '1', '8'}
//   tak zavolani funkce  "prijato_int(0,4)" vrati cislo 2018
unsigned int prijato_int( byte znak_start, byte znak_stop)
  {
    unsigned int soucet = 0;
    unsigned int mocnina;
    for (i = znak_start; i < znak_stop; i++)
      {
    
        switch (znak_stop - i)
          {
            case 1:
              mocnina = 1;
              break;
            case 2:
              mocnina = 10;
              break;
            case 3:
              mocnina = 100;
              break;
            case 4:
              mocnina = 1000;
              break;
          }
      
        soucet = soucet + (prijato[i] - '0') * mocnina ;
      }
    return soucet;
  }


//---------------------------------------------------------
// zhasina LED pro signalizaci rekordu pri nulovani rekordu, nebo pri prohlizeni rekordu
void zhasni_RG_LED(void)
  {
    RAM1_write(EERAM_ADDR_rekord_MIN, 0);     // Znacky pro ovladani LED s funkci pro upozorneni na rekord se nastavi tak, aby v pristim cyklu zhasly
    RAM1_write(EERAM_ADDR_rekord_MAX, 0);
    zhasni_R();                              // LEDky maji spolecnou anodu, tak se pomoci HIGH zhasinaji
    zhasni_G();
  }









//--------------------------------- podprogramy, ktere se vykonavaji v hlavni smycce ---------------------------

// v kazde prvni smycce meri tlak a uklada se do pole hodnot, ze kterych se pak vypocitava klouzavy prumer
void mereni_tlaku(void)
  {
    tlak[klouzak] = prepocet_tlaku();             // zjisteni tlaku a jeho pripadny prepocet z absolutni hodnoty na hladinu more
  }


// v kazde 2. smycce se meri teplota z DS18B20 a uklada do pole hodnot, ze kterych se pak vypocitava klouzavy prumer a zobrazuje se na displeji
void mereni_teploty(void)
  {

    detachInterrupt(digitalPinToInterrupt(pin_RTC_INT));     // docasne zruseni interruptu, aby se mohla v klidu precist hodnota z cidla
    detachInterrupt(digitalPinToInterrupt(pin_infra));       // docasne vypne infra cidlo dlakoveho ovladani
    detachInterrupt(digitalPinToInterrupt(pin_IR_pohyb));    // docasne vypne testovani infracidla pohybu

    
    if (typ_out_teplomeru == 3)             //!!RS485
      {
        rs485_get_temp(mod_addr);   // zjisteni teploty ze vzdaleneho cidla teploty
      }

    if (typ_out_teplomeru < 3)                // jako venkovni cidlo je nastavene DS18B20 (1x nebo 2x)
      {
        teplota_1 = ds18b20_1();
        teplota_2 = 1500;                     // druha teplota se docasne nastavi na 100'C, aby se v pripade, ze je pouzito jen 1 cidlo, ignorovala
      }
   
    if (typ_out_teplomeru == 2)               // jako venkovni cidlo je nastavene 2x DS18B20
      {
        teplota_2 = ds18b20_2();
      }
  

    if (chyba_cidla1  == 5 or chyba_cidla2  == 5)        // POUZE pri 5. chybe nektereho cidla v rade se loguje problem do souboru
      {
        loguj(13);
      }

    if (chyba_cidla1  > 4 or chyba_cidla2  > 4 )      // po 4. chybe v rade na displeji nastavi "---,-" 
      {
        if (screen == 0)
          {
            Serial2.print(EEPROM_text_read(121));         // "to0.pic=31"
            SerialFFF();
            Serial2.print(EEPROM_text_read(122));         // "to1.pic=31"
            SerialFFF();
            Serial2.print(EEPROM_text_read(123));         // "to2.pic=31"
            SerialFFF();
            Serial2.print(EEPROM_text_read(124));         // "to3.pic=31"
            SerialFFF();
          }

        akt_cidlo = 0;
        zobraz_index_cidla();
      }

    if (typ_out_teplomeru < 3)                // jako venkovni cidlo je nastavene DS18B20 (1x nebo 2x)
      {
        if (chyba_cidla1  == 10 or chyba_cidla2  == 10)    // pri 10. chybe v rade se zkusi resetovat napajeni cidla
          {
            digitalWrite(pin_DS18B20PWR,LOW);              // kvuli problemum se startem cidla DS18B20 se na chvili jeho napajeni hodi do LOW
            delay(300);
            digitalWrite(pin_DS18B20PWR,HIGH);             // pak se znova zapne
            delay(200);
            DS18B20.begin();                               // start venkovniho teplomeru
            oneWire_teplomer.reset();
            delay(300);
            DS18B20.setResolution(11);                     // nastaveni prumerovani hodnot teplomeru DS18B20 na 11-bitove rozliseni
          }
      }


    if (chyba_cidla1 == 0 and chyba_cidla2 == 0)      // kdyz se zda byt zjistena hodnota v poradku, ulozi se do klouzaveho prumeru a prumer se pak zobrazi na displeji
      {
        if (prvni_klouzak_OK == false)                // pokud bylo pri zapnuti vadne cidlo, tak ted, kdyz uz je cidlo v poradku, se zaplni cele pole klouzaku zmerenou hodnotou
          {
            prvni_klouzak_OK = true;
            for (i = 0; i<10 ; i++)
              {
                teplota_OUT[i] = teplota_1;           // plni se teplotou_1, ale predpokladam, že je hodne podobna druhe teplote. Po 10 merenich hodnota srovna.
              }
          }
        
        if (teplota_1 <= teplota_2)                // nizsi teplota ze dvou cidel se uklada do klouzaku
          {
            teplota_OUT[klouzak] = teplota_1;
            akt_cidlo = 1;

          }
        else
          {
            teplota_OUT[klouzak] = teplota_2;
            akt_cidlo = 2;
          }        

        if (typ_out_teplomeru == 1)
          {
            akt_cidlo = 0;
          }
      
        klouzak ++;
        if (klouzak == 10) klouzak = 0;        // pole je velke 10 prvku, pak se zacnou prvky prepisovat
    
        suma_OUT = 0;
        for (i = 0 ; i < 10 ; i++)
          {
            suma_OUT = suma_OUT + teplota_OUT[i];
          }

        prumer_teplota_OUT = (suma_OUT + 5) / 10;   // orezava posledni misto        
        zobraz_out = citelna_teplota2(suma_OUT);
        if (zobraz_out < 100 and zobraz_out > -50)
          {
            dis_N (zobraz_cislo(suma_OUT), 'o');
            zobraz_index_cidla();      
          }
      
      }
    DS18B20.requestTemperatures();
    
    attachInterrupt(digitalPinToInterrupt(pin_infra)  , infraINT  , FALLING);      // obnoveni infracidla DO
    attachInterrupt(digitalPinToInterrupt(pin_RTC_INT)  , rtcINT  , FALLING);      // obnoveni sekundoveho interruptu
    attachInterrupt(digitalPinToInterrupt(pin_IR_pohyb)  , pohybINT  , FALLING);   // obnoveni testovani infracidla pohybu

    delay(50);

  
  }


void test_pohybu(void)
  {

    if (blokuj_priblizeni > 0) blokuj_priblizeni --;         // odpocet doby, kdy je blokovane cidlo priblizeni
    
    if (byl_pohyb == true)
      {
        
        if(zpozdeni_podsvetu == 0)                        // kdyz je displej zhasnuty, tak se rozsviti
          {
            if (blokuj_priblizeni == 0)                   // ale jen v pripade, ze neni zablokovane cidlo
              {
                displej_page0();        
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;
              }
        
          }
        
        byl_pohyb = false;


        if (budik_bezi == true and piskani == false)   // pokud aktualne bezi budik, ale uz nepiska, tak se priblizenim blokuje (zhasina displej)
          {
            budik_blok = true;
            budik_bezi = false;


            if (akt_LOC_hodmin == budik_off)           // drive se pri pokusu o rozsviceni displeje pohybem v case "budik_off" displej nerozsvecoval
              {
                displej_page0();                       // touto podminkou by se to melo opravit
                podsvet(true);
                zpozdeni_podsvetu = KON_ZPOZD_PODSV;                
              }
            else                                       // kdyz je budik aktivni (ale uz nepiska),
              {
                zpozdeni_podsvetu = 1;                 // displej se pohybem zhasne (v pristi smycce)
                podsvet(false);
              }

          }

        
        if (budik_bezi == true and piskani == true)   // pokud aktualne bezi budik a zaroven probiha piskani ...
          {
            piskani = false;                // ... tak se pri priblizeni piskani vypne, ale displej sviti dal
            noTone(pin_bzuk);               // skutecne vypnuti zvuku
          }
      
        attachInterrupt(digitalPinToInterrupt(pin_infra)  , infraINT  , FALLING);   // obnoveni interruptu pro daklove ovladani, ktere se zablokovalo pri priblizeni

      }    
  }

    

// v kazde 3. smycce se zobrazi vlhkost
void zobrazeni_vlhkosti(void)
  {
    
    if (vlhkost > 0)
      {
        minula_vlhkost = vlhkost;                  // obcas cidlo posila nesmyslnou nulovou hodnotu, proto se zapamatuje predchozi hodnota    
      }


    
    cidlo_vlhkosti = vlhkomerDHT.readHumidity();

    if (isnan(cidlo_vlhkosti))                                // kdyz je namerena hodnota nesmysl (NEcislo), je to chyba cidla a pouzije se posledni spravne namerena hodnota
      {
        vlhkost = minula_vlhkost;
      }
    else                                                    // kdyz je hodnota z cidla spravna, prevede se pomoci korekce na INT
      {
        vlhkost = (int)cidlo_vlhkosti  + (RAM1_read(EERAM_ADDR_korekce_vlhkost) - 100);    
      }

      
    if (vlhkost == 0)
      {
        log_pomprom = minula_vlhkost;  
        loguj(14);
        vlhkost = minula_vlhkost;
      }

    dis_V(vlhkost);    
  }


// v kazde 4. smycce se pocita prumerny tlak pro 5-minutove ukladani do databaze
void vypocti_prumer_tlaku(void)
  {
    suma = 0;                     // vypocet prumerneho tlaku
    for (i = 0 ; i < 10 ; i++)
      {
        suma = suma + tlak[i];
      }
    prumer_tlak = (suma + 5) / 10; // +5 je kvuli spravnemu zaokrouhlovani, protoze deleni jen osekne desetinnou cast (priklad 88 / 10 = 8, ale (88+5) / 10 = 9)
  }


// Zobrazeni dvou malych grafu na hlavni obrazovce na displeji 
void zobrazeni_minigrafu(void)
  {
    if (screen == 0)
      {
        // nalezeni minima a maxima z poslednich 2,5 dni
        unsigned int max_hodnota = 0;
        unsigned int min_hodnota = 65535;
        unsigned int G_60hod;  
        unsigned int vyska_sloupce;
        byte velikost = 0;

        // smazani predchozich zelenych rysek
        Serial2.print(EEPROM_text_read(156));           // "fill 18,212,120,14,BLACK"
        Serial2.print(EEPROM_text_read(134));
        SerialFFF();
        
        Serial2.print(EEPROM_text_read(155));           // "fill 18,302,120,14,BLACK"
        Serial2.print(EEPROM_text_read(134));
        SerialFFF();
  
        // graf venkovni teploty
        for (i = 240; i < 300 ; i++)
          {
            G_60hod = RAM2_read_int(i);
            if ( G_60hod < 65535)         // budou se ignorovat zaznamy v RAM, ktere jeste neobsahuji zadne hodnoty
              {
                velikost ++;
                if (G_60hod > max_hodnota)  max_hodnota = G_60hod;
                if (G_60hod < min_hodnota)  min_hodnota = G_60hod;
    
                // rozdelovaci znacky pro dny v grafu se nastavi na x-pozici, ktera signalizuje pulnoc
                if ( (RAM2_read_int(i+450) % 24) == 0)
                  {    
                                      
                    // zelena pulnocni ryska v grafu teploty
                    // line 80,212,80,225,GREEN
                    Serial2.print(EEPROM_text_read(144));           // "line "
                    Serial2.print(((i-240)*2)+18);
                    Serial2.print(",212,");
                    Serial2.print(((i-240)*2)+18);
                    Serial2.print(EEPROM_text_read(136));               // ",225,GREEN"
                    SerialFFF();
    
                    // zelena pulnocni ryska v grafu tlaku
                    // line 80,302,80,315,GREEN
                    Serial2.print(EEPROM_text_read(144));           // "line "
                    Serial2.print(((i-240)*2)+18);
                    Serial2.print(",302,");
                    Serial2.print(((i-240)*2)+18);
                    Serial2.print(EEPROM_text_read(137));              // ",315,GREEN"
                    SerialFFF();

                  
                  
                  }
    
                
              }
          }
    
    
    
    
    
        for (i = 240; i < 300 ; i++)
          {
            // vyska sloupce se nastavuje na 0 az 100%. Kdyz neni hodnota jeste zmerena, sloupec se nezobrazi
            if (RAM2_read_int(i) < 65535) vyska_sloupce = map(RAM2_read_int(i), min_hodnota,max_hodnota,3,100);
            else                         vyska_sloupce = 0;
          
            // odeslani 60 hodnot do horniho grafu na displeji na hlavni obrazovce       
            Serial2.print('j');
            Serial2.print(i-240);
            Serial2.print(".val=");
            Serial2.print(vyska_sloupce);
            SerialFFF();
    
          } 
        if (velikost > 0)        // kdyz nejsou jeste zadne namerene hodnoty, tak se vedle grafu zobrazovaly nesmyslne udaje, proto se v tom pripade min a max hodnota nezobrazuje
          {
            // na bok teplotniho grafu zapis max a min hodnoty
            Serial2.print(EEPROM_text_read(138));                    // "g1max.txt=\""
            Serial2.print((max_hodnota - 500.0) / 10.0,1);
            Serial2.print("\"");
            SerialFFF();    
              
            Serial2.print(EEPROM_text_read(139));                    // "g1min.txt=\""
            Serial2.print((min_hodnota - 500.0) / 10.0,1);
            Serial2.print("\"");
            SerialFFF();    
          }
        else
          {
            // na bok teplotniho grafu zapis jen pomlcky
            Serial2.print(EEPROM_text_read(140));                     // "g1max.txt=\"--\""
            SerialFFF();    
              
            Serial2.print(EEPROM_text_read(141));                     // "g1min.txt=\"--\""
            SerialFFF(); 
          }
              
        delay(100);
        // vodorovna modra cara pro 0'C
        if (min_hodnota < 500 and max_hodnota > 500)
          {
            vyska_sloupce = map(500,min_hodnota,max_hodnota,212,148);  // 500=0'C; spodni okraj grafu je 212, horni okraj grafu je 148
            // line 18,160,137,160,2047
            Serial2.print(EEPROM_text_read(142));                   // "line 18,"
            Serial2.print(vyska_sloupce);
            Serial2.print(EEPROM_text_read(225));                   // ",137,"
            Serial2.print(vyska_sloupce);
            Serial2.print(",2047");
            SerialFFF();
          }
    
        // vodorovna zluta cara pro 20'C
        if (min_hodnota < 700 and max_hodnota > 700)
          {
            vyska_sloupce = map(700,min_hodnota,max_hodnota,212,148);  // 700=20'C; spodni okraj grafu je 212, horni okraj grafu je 148
            // line 18,160,137,160,YELLOW
            Serial2.print(EEPROM_text_read(142));                   // "line 18,"
            Serial2.print(vyska_sloupce);
            Serial2.print(EEPROM_text_read(225));                   // ",137,"
            Serial2.print(vyska_sloupce);
            Serial2.print(EEPROM_text_read(131));                   // ",YELLOW"
            SerialFFF();
          }
    
    
    
    
        // graf tlaku ... to same jako vyse, ale pro jinou oblast objektu a jine adresy EERAM
        max_hodnota = 0;
        min_hodnota = 65535;
        
        for (i = 90; i < 150 ; i++)
          {
            G_60hod = RAM2_read_int(i);
            if ( G_60hod < 65535)         // budou se ignorovat zaznamy v RAM, ktere jeste neobsahuji zadne hodnoty
              {
                if (G_60hod > max_hodnota)  max_hodnota = G_60hod;
                if (G_60hod < min_hodnota)  min_hodnota = G_60hod;
              }
          }
    
        for (i = 90; i<150 ; i++)
          {
            // vyska sloupce se nastavuje na 0 az 100%. Kdyz neni hodnota jeste zmerena, sloupec se nezobrazi
            if (RAM2_read_int(i) < 65535) vyska_sloupce = map(RAM2_read_int(i), min_hodnota,max_hodnota,3,100);
            else                         vyska_sloupce = 0;
          
            // odeslani 60 hodnot do dolnihoo grafu na displeji na hlavni obrazovce
    
            Serial2.print('j');
            Serial2.print(60 + i-90);
            Serial2.print(".val=");
            Serial2.print(vyska_sloupce);
            SerialFFF();
    
          }

        if (velikost > 0)        // kdyz nejsou jeste zadne namerene hodnoty, tak se vedle grafu zobrazovaly nesmyslne udaje, proto se v tom pripade min a max hodnota nezobrazuje
          {
            // na bok tlakoveho grafu zapis max a min hodnoty v hPa (vcetne zaokrouhleno na cela cisla)
            Serial2.print("g2max.txt=\"");                    // "g2max.txt=\""
            Serial2.print((max_hodnota + 60050) / 100);
            Serial2.print("\"");
            SerialFFF();    
              
            Serial2.print("g2min.txt=\"");                    // "g2min.txt=\""
            Serial2.print((min_hodnota + 60050) / 100);
            Serial2.print("\"");
            SerialFFF();    
          }
        else
          {
            // na bok teplotniho grafu zapis jen pomlcky
            Serial2.print("g2max.txt=\"--\"");                     // "g2max.txt=\"--\""
            SerialFFF();    
              
            Serial2.print("g2min.txt=\"--\"");                     // "g2min.txt=\"--\""
            SerialFFF(); 
          }
        delay(100);

        // vodorovna zluta cara pro 1020 hPa
        if (min_hodnota < 42000 and max_hodnota > 42000)
          {
            vyska_sloupce = map(42000,min_hodnota,max_hodnota,302,238);  // 42000=1020hPa; spodni okraj grafu je 302, horni okraj grafu je 238
            // line 18,260,137,260,YELLOW
            Serial2.print(EEPROM_text_read(142));                   // "line 18,"
            Serial2.print(vyska_sloupce);
            Serial2.print(EEPROM_text_read(225));                   // ",137,"
            Serial2.print(vyska_sloupce);
            Serial2.print(EEPROM_text_read(131));                   // ",YELLOW"
            SerialFFF();
          }



        // vodorovna bila cara pro 1000 hPa
        if (min_hodnota < 40000 and max_hodnota > 40000)
          {
            vyska_sloupce = map(40000,min_hodnota,max_hodnota,302,238);  // 40000=1000hPa; spodni okraj grafu je 302, horni okraj grafu je 238
            // line 18,260,137,260,YELLOW
            Serial2.print(EEPROM_text_read(142));                   // "line 18,"
            Serial2.print(vyska_sloupce);
            Serial2.print(EEPROM_text_read(225));                   // ",137,"
            Serial2.print(vyska_sloupce);
            Serial2.print(",WHITE");                                // ",WHITE"
            SerialFFF();
          }

        // vodorovna modra cara pro 980 hPa
        if (min_hodnota < 38000 and max_hodnota > 38000)
          {
            vyska_sloupce = map(38000,min_hodnota,max_hodnota,302,238);  // 38000=980hPa; spodni okraj grafu je 302, horni okraj grafu je 238
            // line 18,260,137,260,BLUE
            Serial2.print(EEPROM_text_read(142));                   // "line 18,"
            Serial2.print(vyska_sloupce);
            Serial2.print(EEPROM_text_read(225));                   // ",137,"
            Serial2.print(vyska_sloupce);
            Serial2.print(",BLUE");                                 // ",BLUE"
            SerialFFF();
          }
          
        
      } 
  
  }




// v kazde 7. smycce z 20 se zjisti hodnota teploty z vlhkomeru, vypocte se klouzavy prumer z poslednich 10 mereni a zobrazi se na displeji
void zobraz_vnitrni_teplotu(void)
  {

    teplota_dht = vlhkomerDHT.readTemperature();
        
    if(teplota_dht > 0)     // namerena teplota se zpracovava jen v pripade, ze je vetsi nez 0
      {
        teplota_IN[klouzak] = 500 + (10 * teplota_dht) - 100 + RAM1_read(EERAM_ADDR_korekce_IN);         // korekce zobrazovane teploty
    
    
        suma_IN = 0;
        for (i = 0 ; i < 10 ; i++)
          {
            suma_IN = suma_IN + teplota_IN[i];
          }
      
        zobraz_in = citelna_teplota2(suma_IN);
      
        prumer_teplota_IN = (suma_IN + 5) / 10;       // orezava posledni misto
          
    
        dis_N (zobraz_cislo(suma_IN), 'i');
      }
    else       // chyba cidla byla pravdepodobne zpusobena rychlymi dotazy
      {
        delay(500);     // proto se pred pristim ctenim 0,5 sekundy pocka
      }
  }



// v kazde 8. smycce se vyhodnocuje stav LED podle nastavene funkce
void vyhodnot_vnejsi_teplotu(void)
  {
    if (chyba_cidla1 > 0 or chyba_cidla2 > 0 )   // pri utrzenem cidle nic nevyhodnocuj
      {
        RAM1_write(EERAM_ADDR_modra,0);          // a modrou LED zhasni
        return;
      }
  
    unsigned int porovnavaci_teplota;
    unsigned int suma_porovnani;
  
  
    porovnavaci_teplota =  prumer_teplota_OUT;       // vsechny operace se vztahuji k venkovni teplote
    suma_porovnani = suma_OUT;

   
    //   vyhodnoceni modre LED (sviti, kdyz je v teplota v intervalu urcenem dvema mezemi)
    if (prepinaci_uroven1 < prepinaci_uroven2)     // kdyz je prvni uroven mensi, nez druha, bude LED svitit v intervalu "uroven 1" az "uroven2" 
      {
        if (porovnavaci_teplota >= prepinaci_uroven1  and  porovnavaci_teplota <= prepinaci_uroven2)
          {
            // v pristich testech rozsviti modrou LED;
            RAM1_write(EERAM_ADDR_modra,1);

            modra_pamet = 1;
            modra_pamet_pom = 0;
            RAM1_write(EERAM_ADDR_modra_pamet,1);

          }
        else
          {
            // v pristich testech zhasne modrou LED;
            RAM1_write(EERAM_ADDR_modra,0);
          }
      }

  
    if (prepinaci_uroven1 > prepinaci_uroven2)     // kdyz je prvni uroven vetsi, nez druha, bude LED v intervalu "uroven 1" az "uroven2" zhasnuta
      {
        if (porovnavaci_teplota <= prepinaci_uroven1  and  porovnavaci_teplota >= prepinaci_uroven2)
          {
            // v pristich testech zhasne modrou LED;
            RAM1_write(EERAM_ADDR_modra,0);
          }
        else
          {
            // v pristich testech rozsviti modrou LED;
            RAM1_write(EERAM_ADDR_modra,1);

            modra_pamet = 1;
            modra_pamet_pom = 0;
            RAM1_write(EERAM_ADDR_modra_pamet,1);

          }
      }
      
    if (prepinaci_uroven1 == prepinaci_uroven2)     // kdyz jsou obe urovne stejne
      {
        // v pristich testech zhasne modrou LED;
        RAM1_write(EERAM_ADDR_modra,0);        
      }
      
  
  
  
    if (ignoruj_rekordy == 0)               // prvni 3 minuty po restartu se minimalni a maximalni rekordy neprepisuji (kvuli stabilizaci cidel)
      {
    
        if (suma_porovnani > tep_max1)       // zaznamenej prekroceni maxima od puldenniho vynulovani
          {
            tep_max1 = suma_porovnani;
            RAM1_write_int(EERAM_ADDR_tep_max1  , tep_max1);
            RAM1_write_int(EERAM_ADDR_hhmm_max1 , hodmin_to_int(LOC_hod, LOC_min));
          }
    
        if (suma_porovnani < tep_min1)       // zaznamenej prekroceni minima od denniho vynulovani
          {
            tep_min1 = suma_porovnani;
            RAM1_write_int(EERAM_ADDR_tep_min1  , tep_min1);
            RAM1_write_int(EERAM_ADDR_hhmm_min1 , hodmin_to_int(LOC_hod, LOC_min));
          }
    
    
        if (suma_porovnani  < tep_min)   // zapis minimalni teploty
          {
            RAM1_write(EERAM_ADDR_rekord_MIN, 1);               // uloz do zalohovane pameti, ze minimalni rekord byl prekonan
            tep_min = suma_porovnani;
            RAM1_write_int(EERAM_ADDR_min_teplota, tep_min);
            tep_min_datcas = DMhn;
            for (i = EERAM_ADDR_min_datum; i < EERAM_ADDR_min_datum + 12; i++)           // zapis datumu a casu pri minimalni teplote
              {
                char znak = DMhn.charAt(i - EERAM_ADDR_min_datum);
                byte bznak = znak;
                RAM1_write(i, bznak);
              }
          }
    
        if (suma_porovnani > tep_max)   // zapis maximalni teploty
          {
      
            RAM1_write(EERAM_ADDR_rekord_MAX, 1);               // uloz do zalohovane pameti, ze maximalni rekord byl prekonan
            tep_max = suma_porovnani;
            RAM1_write_int(EERAM_ADDR_max_teplota, tep_max);
            tep_max_datcas = DMhn;
            for (i = EERAM_ADDR_max_datum; i < EERAM_ADDR_max_datum + 12; i++)
              {
                char znak = DMhn.charAt(i - EERAM_ADDR_max_datum);         // zapis datumu a casu pri maximalni teplote
                byte bznak = znak;
                RAM1_write(i, bznak);
              }
          }
      
      }
  
  }
  





// v kazde 10. smycce se otestuje stav prepinace SEC/SELC a zobrazi se prislusny napis (SEC/SELC)
void vyhodnot_SEC_SELC(void)
  {

    if (pcf.read(4) == HIGH and casova_zona == 2)   // prave doslo k prepnuti prepinace ze SELC na SEC
      {
        log_pomprom = 1;
        loguj(12);     // logovani zmeny SEC/SELC
        casova_zona = 1;
        RAM1_write(EERAM_ADDR_timezone, 1);
        // na podzim se 1 hodina odecte (ze 3:00 na 2:00 ... je delsi noc)
        if (LOC_hod > 0)       // kdyz je mozne od poctu hodin jednu hodinu odecist, tak se hned odecte
          { //    pokud by odecteni znamenalo prechod do predchoziho dne, tak se nic neodecte
            LOC_hod --;
          }
    
        setDS3231time(LOC_sek, LOC_min, LOC_hod, LOC_den, LOC_mes, LOC_rok, LOC_dvt);  // knihovni funkce pro zapis do RTC obvodu
        pauza100();
      }
  
    if (pcf.read(4) == LOW and casova_zona == 1)   // prave doslo k prepnuti prepinace ze SEC na SELC
      {
        log_pomprom = 2;
        loguj(12);     // logovani zmeny SEC/SELC
        casova_zona = 2;
        RAM1_write(EERAM_ADDR_timezone, 2);
        // na jare se 1 hodina pricte (ze 2:00 na 3:00 ... je kratsi noc)
        if (LOC_hod < 23)       // kdyz je mozne k poctu hodin jednu hodinu pricist, tak se hned pricte
          { //    pokud by pricteni znamenalo prechod do dalsiho dne, tak se nic nepricita
            LOC_hod ++;
          }
      
        setDS3231time(LOC_sek, LOC_min, LOC_hod, LOC_den, LOC_mes, LOC_rok, LOC_dvt);  // knihovni funkce pro zapis do RTC obvodu
        pauza100();
      }



    //zobrazeni SEC / SELC na displeji
    if (screen == 0)
      {
        Serial2.print(EEPROM_text_read(226));                   // "selc.pic="
        Serial2.print(casova_zona + 50);
        SerialFFF();
      }  
  }


// v kazde 11 smycce se obslouzi LEDky (pokud maji zapnutou prislusnou funkci)
void obsluha_LED(void)
  {
    //----------------  obsluha cervene a zelene LED ----------
      
    if (tep_min == tep_max)  // po vynulovani maxima a minima se sice zapisuji aktualni hodnoty jako rekordy ...
      {
        zhasni_RG_LED();     // ... ale v tomto pripade se obe LED (R a G) zhasnou
      }
    else
      {
        if (STOP_LED_REK  == 0)       // LED se ovladaji jen v pripade, ze je vynulovane pocitadlo blokovani LED (funkce #Z)
          {
            if (RAM1_read(EERAM_ADDR_rekord_MIN) ==  1)  // kdyz byl prekrocen zaporny rekord ...
              {
                rozsvit_G();            //   ... rozsvit zelenou LED
              }
            else
              {
                zhasni_G();           //   kdyz rekord prekroceny nebyl, tak ji zhasni
              }
        
            if (RAM1_read(EERAM_ADDR_rekord_MAX) ==  1) // kdyz byl prekrocen kladny rekord ...
              {
                rozsvit_R();            //   ... rozsvit cervenou LED
              }
            else
              {
                zhasni_R();           //   kdyz rekord prekroceny nebyl, tak ji zhasni
              }
          }
      }  
  
    //----------------  obsluha modre LED ----------
    if (RAM1_read(EERAM_ADDR_modra) ==  0)                              // kdyz ma byt modra LED zhasnuta...
      {
        zhasni_B();                                 //   ... tak se zhasne
      }
    else
      {
        rozsvit_B();                                 //  jinak se rozsviti
      }

    if (screen == 0)
      {
        if (modra_pamet == 1 and modra_pamet_pom == 0)
          {
            Serial2.print(EEPROM_text_read(157));         // "ik8.pic=83"       // aktualizace ikony pro pamet modre LED
            SerialFFF();            
            modra_pamet_pom = 1;      
          }
        if (modra_pamet == 0 and modra_pamet_pom == 1)
          {
            Serial2.print(EEPROM_text_read(158));         // "ik8.pic=80"       // aktualizace ikony pro pamet modre LED
            SerialFFF();            
            modra_pamet_pom = 0;      
          }
      }  
  }



// v kazde 12. smycce se budou provadet vypocty ohledne fazi Mesice
void vypocty_mesice(void)
  {
    // nalezeni nejblizsich casu v databazi
    for (unsigned int index_databaze = priste_prohledavat_od; index_databaze < pocet_zaznamu ; index_databaze = index_databaze + 4)
      {
        // nacte se cely balik casu fazi pro jeden mesicni cyklus
    
        T0  = EEPROM_read_long(index_databaze);     // z externi EEPROM se nacte cas novu
        T1  = EEPROM_read_long(index_databaze + 1); // z externi EEPROM se nacte cas prvni ctvrti
        T2  = EEPROM_read_long(index_databaze + 2); // z externi EEPROM se nacte cas uplnku
        T3  = EEPROM_read_long(index_databaze + 3); // z externi EEPROM se nacte cas posledni ctvrti
        T0n = EEPROM_read_long(index_databaze + 4); // z externi EEPROM se nacte cas novu v nasledujicim cyklu
        if (minuty2016UTC < (T0n & 0b01111111111111111111111111111111) and minuty2016UTC >= (T0 & 0b01111111111111111111111111111111))    //  aktualni cas se nachazi v ramci nacteneho mesicniho cyklu (vice nez cas novu, ale mene nez cas pristiho novu)
          {
            stari_mesice = minuty2016UTC - T0;             // stari Mesice od posledniho novu v minutach
            stari_mesice_D = stari_mesice / 1440.0;        // stari Mesice od posledniho novu ve dnech a desetinach dne
            priste_prohledavat_od = index_databaze - 4;
            break;
          }
      }
  
  
    byte ikonazatmeni=0;
    byte zatmeniM  = (T2  >> 31);  // pokud pri uplnku dojde k  zatmeni Mesice, ulozi se informace o zatmeni do promenne
    byte zatmeniS2 = (T0n >> 31);  // pokud pri nasledujicim novu dojde k  zatmeni Slunce, ulozi se informace o zatmeni do promenne
    // pripadne informace o zatmeni se vymazou a v T2 a T0n zustane jen obycejny cas
    T2  = T2  & 0b01111111111111111111111111111111;
    T0n = T0n & 0b01111111111111111111111111111111;

  
    if (minuty2016UTC < T1)  // od aktualniho casu bude prvni udalosti 1. ctvrt
      {
        typ_pristi_faze = 1;
        cas_pristi_faze = T1;
        pristifaze_txt = EEPROM_text_read(47);                 // Prvni ctvrt   \0
        pristifaze(2, T1);
        
      }
  
    if (minuty2016UTC >= T1 and minuty2016UTC < T2)  // od aktualniho casu bude prvni udalosti uplnek
      {
        typ_pristi_faze = 2;
        cas_pristi_faze = T2;
        if (zatmeniM > 0)
          {
            pristifaze_txt = EEPROM_text_read(49);               // Zatmeni Mesice\0
            pristifaze(4 , T2);         // kdyz pri tomto uplnku nastane zaroven zatmeni Mesice, zobrazi se specialni miniikona
            ikonazatmeni = 4;
          }
        else
          {
            pristifaze_txt = EEPROM_text_read(48);               // Uplnek        \0
            pristifaze(3 , T2);               // kdyz k zatmeni Mesice nedojde, zobrazuje se obycejna miniikona uplnku
          }
      }
  
    if (minuty2016UTC >= T2 and minuty2016UTC < T3)  // od aktualniho casu bude prvni udalosti posledni ctvrt
      {
        typ_pristi_faze = 3;
        cas_pristi_faze = T3;
        pristifaze_txt = EEPROM_text_read(50);        // Posledni ctvrt\0
        pristifaze(5, T3);
      }
  
    if (minuty2016UTC >= T3 and minuty2016UTC < T0n)  // od aktualniho casu bude prvni udalosti nov z nasledujiciho Mesicniho cyklu
      {
        typ_pristi_faze = 0;
        cas_pristi_faze = T0n;
        if (zatmeniS2 > 0)
          {
            pristifaze_txt = EEPROM_text_read(46);          // Zatmeni Slunce\0
            pristifaze(1 , T0n);                  // kdyz pri tomto novu dojde k zatmeni Slunce, zobrazi se specialni miniikona
            ikonazatmeni = 1;
          }
        else
          {
            pristifaze_txt = EEPROM_text_read(45);              // Nov           \0
            pristifaze( 0, T0n);               // kdyz k zatmeni Slunce nedojde, zobrazuje se obycejna miniikona novu
          }
      }
  


    // vypocet colongituda kvuli vyberu aktualni ikony Mesice
    astro_LOC_rok = LOC_rok;
    astro_LOC_mes = LOC_mes;
    astro_LOC_den = LOC_den;
    astro_LOC_hod = LOC_hod;
    astro_LOC_min = LOC_min;
    z_Astro_LOC_na_Astro_UTC(casova_zona);

    M_souradnice (1, casova_zona, GeoLon, GeoLat );

    index_iko = (int)(Mes_colongitudo / 3);
    
    if (minuly_index_iko != index_iko)          // k prekresleni dochazi jen v pripade, ze je nejaka zmena
      {
        zobraziko(index_iko);  // zobrazi velkou ikonu Mesice
      }
    minuly_index_iko = index_iko;


    if (screen == 0)
      {
        if (ikonazatmeni == 0)
          {
            Serial2.print("xstr 391,143,0,0,3,63488,65504,1,1,1,\"\"");
            SerialFFF();  
          }

        if (ikonazatmeni == 1)
          {
            Serial2.print("xstr 391,143,66,26,3,63488,65504,1,1,1,\"Zat.S.\"");
            SerialFFF();  
          }
        if (ikonazatmeni == 4)
          {
            Serial2.print("xstr 391,143,66,26,3,63488,65504,1,1,1,\"Zat.M.\"");
            SerialFFF();  
          }
        
      }
  
  
  }  // konec vypoctu ohledne fazi Mesice



// v kazde 15. smycce se otestuje, jestli nenastal cas na ukonceni mereni jednoho bloku rekordu a prepnuti na dalsi
void prepinac_rekordu(void)
  {
    unsigned int cas = hodmin_to_int(LOC_hod, LOC_min);
    byte posledni_prepnuti = RAM1_read(EERAM_ADDR_dennoc);
  
  
    if (cas >= 360 and cas < 1080 and posledni_prepnuti == 18)      // cas je mezi 6:00 a 17:59 a k poslednimu prepnuti doslo v 18:00
      {
        presun_blok_rekordu();
        RAM1_write(EERAM_ADDR_dennoc, 6);
      }
  
    if ((cas >= 1080 or cas < 360) and posledni_prepnuti == 6)      // cas je mezi 18:00 a 05:59 a k poslednimu prepnuti doslo v 6:00
      {
        presun_blok_rekordu();
        RAM1_write(EERAM_ADDR_dennoc, 18);
      }
  
    if (posledni_prepnuti != 6 and posledni_prepnuti != 18)      // pro pripad, ze by se nejak porusila pamet posledniho prepnuti
      {
        if (cas >= 360 and cas < 1080)                      // pri casu mezi 6:00 az 17:59
          {
            RAM1_write(EERAM_ADDR_dennoc, 6);                // se nastavi takovy cas prepnuti, aby se hodnota v poslednim ukoncenem bloku neprepsala
          }
        else                                                // to same pro cas od 18:00 do 5:59
          {
            RAM1_write(EERAM_ADDR_dennoc, 18);               // taky se nastavavi cas prepnuti tak, aby se stary blok neprepsal
          }
      
      }
  
  }


// presune hodnoty minima a maxima (vcetne casu) z aktualniho bloku do minuleho bloku
void presun_blok_rekordu(void)
  {
  
    // do ukonceneho bloku se zapisou data z aktualniho bloku
    RAM1_write_int(EERAM_ADDR_tep_min2  , RAM1_read_int(EERAM_ADDR_tep_min1));
    RAM1_write_int(EERAM_ADDR_hhmm_min2 , RAM1_read_int(EERAM_ADDR_hhmm_min1));
    RAM1_write_int(EERAM_ADDR_tep_max2  , RAM1_read_int(EERAM_ADDR_tep_max1));
    RAM1_write_int(EERAM_ADDR_hhmm_max2 , RAM1_read_int(EERAM_ADDR_hhmm_max1));
  
    tep_min2 = RAM1_read_int(EERAM_ADDR_tep_min2);
    tep_max2 = RAM1_read_int(EERAM_ADDR_tep_max2);
  
    // aktualni blok se vynuluje
    tep_max1 = 0;
    RAM1_write_int(EERAM_ADDR_tep_max1  , tep_max1);
    RAM1_write_int(EERAM_ADDR_hhmm_max1 , hodmin_to_int(LOC_hod, LOC_min));
  
    tep_min1 = 14994;
    RAM1_write_int(EERAM_ADDR_tep_min1  , tep_min1);
    RAM1_write_int(EERAM_ADDR_hhmm_min1 , hodmin_to_int(LOC_hod, LOC_min));
  
  
  }




void displej_page0(void)
  {
    minuly_index_iko = 200;                      // pri prvnim zobrazeni hlavni stranky se vzdycky prekresli i ikona Mesice
    Serial2.print("page 21");                   // "page 21"
    SerialFFF();
    screen = 0;
    if (stav_cidla1 == 0 and stav_cidla2 == 0)         // kdyz je cidlo teploty v poradku, zobrazi se teplota
      {
        if (zobraz_out < 100 and zobraz_out > -50)
          {
            dis_N (zobraz_cislo(suma_OUT), 'o');
            zobraz_index_cidla();      
          }        
      }
    else                         // pri chybe cidla se zobrazi "---,-"
      {
        Serial2.print(EEPROM_text_read(121));         // "to0.pic=31"
        SerialFFF();
        Serial2.print(EEPROM_text_read(122));         // "to1.pic=31"
        SerialFFF();
        Serial2.print(EEPROM_text_read(123));         // "to2.pic=31"
        SerialFFF();
        Serial2.print(EEPROM_text_read(124));         // "to3.pic=31"
        SerialFFF();

        akt_cidlo = 0;
        zobraz_index_cidla();
        
      }
    dis_N (zobraz_cislo(suma_IN), 'i');
    dis_V(vlhkost);
    zobrazeni_minigrafu();
    dis_pulrekordy();                   
  }







//==========================================================



void flush_read_buffer2(void)
  {
    delay(5);
    while (Serial2.available())
      {
        Serial2.read();
      }
    delay(5);
  }



void zobraz_6_zatmeni(byte horni_index)
  {
    for (byte radka=1;radka<7; radka++)
      {
        unsigned int tab_index = horni_index + radka;



        T0n = EEPROM_read_longZ((tab_index * 5) + 20000);     // do T0n se ulozi cas nejakeho zatmeni podle aktualne nalistovaneho indexu
        info_zatmeni = EEPROM_read((tab_index * 5) + 20004); // informace o typu a velikosti zatmeni
        pristifaze(99, T0n);

        Serial2.print("ic");          // ikona podle toho, jestli se jedna o zatmeni Slunce, nebo Mesice
        Serial2.print(radka);
        Serial2.print(".pic=");
        if (info_zatmeni <= 100)      // nalistovane zatmeni je zatmeni Slunce
          {
            Serial2.print("37");      // "Zatmeni Slunce:"
          }
        else
          {
            Serial2.print("40");      // "Zatmeni Mesice"
          }
        SerialFFF();


        // zobrazeni datumu a casu na prislusne radce
        String utcdatcas = String(UTC_den) + "." + String(UTC_mes) + "." + String(UTC_rok) + " " + String(UTC_hod) + ":" + String(UTC_min) + " UTC";

        Serial2.print("zat");         
        Serial2.print(radka);
        Serial2.print(EEPROM_text_read(227));                   // ".txt=\""
        Serial2.print(utcdatcas);
        Serial2.print("  \"");
        SerialFFF();

        // do posledniho sloupce jeste informace o velikosti zatmeni
        Serial2.print("po");
        Serial2.print(radka);
        Serial2.print(EEPROM_text_read(227));                   // ".txt=\""
        Serial2.print(velikost_zatmeni(T0n));
        Serial2.print('\"');
        SerialFFF();
        
      }

  }





// odesle jednu radku aktualnich dat v CSV formatu
void posli_CSV_blok(void)
  {
    Serial.print(dat_str);                    // datum  (uvodni nuly nahrazeny mezerami)  priklad: " 6. 3.2018", nebo "13. 8.2020"
    print_strednik();                         // oddelovac ';'
    Serial.print(tim_str);                    // cas (hodiny maji uvodni nulu nahrazenou mezerou, minuty a sekundy jsou vcetne uvodnich nul) priklad: " 6:05:08"
    print_strednik();                         // oddelovac ';'
    Serial.print(prumer_teplota_OUT);         // venkovni teplota v desetinach 'C + 500  (751 = 25,1'C)
    print_strednik();                         // oddelovac ';'
    Serial.print(prumer_teplota_IN);          // vnitrni teplota v desetinach 'C + 500  (726 = 22,6'C)
    print_strednik();                         // oddelovac ';'
    Serial.print(vlhkost);                    // vlhkost v procentech
    print_strednik();                         // oddelovac ';'
    Serial.print(prumer_tlak - 60000);        // atmosfericky tlak snizeny o 60000Pa    (35000 = 95000Pa = 950hPa = 95kPa)
    print_strednik();                         // oddelovac ';'
  
    Serial.print(na_binar(status_byte));
    print_strednik();                         // oddelovac ';'

    Serial.print(index_iko);                  // index aktualni ikony Mesice
    print_strednik();                         // oddelovac ';'
    Serial.println(stari_mesice);             // stari Mesice v minutach od posledniho novu
    //  Serial.print(stari_mesice_D);             // jako varianta se da odesilat stari Mesice ve dnech a desetinach dne od posledniho novu (napr "12.65")
  }







String na_binar(byte cislo)
  {
    String vysledek = "0b";
    for (byte index_bit = 8; index_bit > 0 ; index_bit--)
      {
        vysledek = vysledek + bitRead (cislo, index_bit - 1);
      }
    return vysledek;
  }






//----------------------------------
// kvuli uspore mista v ProgMEM
void nuluj_rekordy(void)
  {

    loguj(7);     // logovani mazani rekordu
    
    tep_max =  1;                      // cislo     1 = -49.99'C
    tep_min =  24999;                  // cislo 24999 = 199.99'C
    tep_rst_datcas = DMhn;
    tep_max_datcas = tep_rst_datcas;
    tep_min_datcas = tep_rst_datcas;

    RAM1_write_int(EERAM_ADDR_min_teplota, tep_min);
    RAM1_write_int(EERAM_ADDR_max_teplota, tep_max);
  
    for (i = EERAM_ADDR_rst_rekordy; i < EERAM_ADDR_rst_rekordy + 12; i++)
      {
        char znak = DMhn.charAt(i - EERAM_ADDR_rst_rekordy);         // zapis datumu a casu pri nulovani rekordu
        byte bznak = znak;
        RAM1_write(i, bznak);
        RAM1_write(i + EERAM_ADDR_min_datum - EERAM_ADDR_rst_rekordy , bznak);
        RAM1_write(i + EERAM_ADDR_max_datum - EERAM_ADDR_rst_rekordy, bznak);
      }
  
    byte cislo_rek_souboru = RAM1_read(EERAM_ADDR_rec_file);
    cislo_rek_souboru ++;
    RAM1_write(EERAM_ADDR_rec_file, cislo_rek_souboru);
    
    RAM1_write_int(EERAM_ADDR_stop_RG      , 1440);          // po vynulovani hlavnich rekordu se na 24 hodin zablokuji R/G LEDky 
  
  }


//----------------------------------
// kvuli uspore mista v ProgMEM
void zhasni_vsechny_LED(void)
  {
    zhasni_RG_LED();                  // zhasinani LED, ktere signalizuji rekordy
    RAM1_write(EERAM_ADDR_modra, 0);  // zhasinani modre LED
    zhasni_B();
  }


//----------------------------------
// kvuli uspore mista v ProgMEM
float citelna_teplota(int necitelna_teplota)
  {
    return (necitelna_teplota - 500) / 10.0;
  }


//----------------------------------
// kvuli uspore mista v ProgMEM
float citelna_teplota2(int necitelna_teplota)
  {
    return (necitelna_teplota - 5000) / 100.0;
  }


//----------------------------------
// kvuli uspore mista v ProgMEM
void print_strednik(void)
  {
    Serial.print(';');
  }



//----------------------------------
// kvuli uspore mista v ProgMEM (96 Bajtu) se casto opakovana pauza vola jako podprogram bez parametru
void pauza100(void)
  {
    delay(100);
  }
  
//------------------------------------------------
// podprogramy pro RS485 komunikaci
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

void rs485_get_temp(byte MODadresa)
  {
    uint16_t CRC16_mod;
    zadost[0] = MODadresa;               // adresa SLAVE zarinei na sbernici
    CRC16_mod = ModRTU_CRC(zadost,1);    // vypocet "kontrolniho souctu" z prvniho bajtu v poli zadost[] (z adresy)
    zadost[1] = (CRC16_mod >> 8) & 255;  // doplneni pole zadost[] o kontrolni soucet (MSB)
    zadost[2] = CRC16_mod & 255;         // doplneni pole zadost[] o kontrolni soucet (LSB)

    delay(20);   

    while (Serial1.available())         // uplne vymazani  prijimaciho bufferu z predchozi komunikace
      {
        Serial1.read();
      }
    delay(5);                           // Kratka pauza pred pred prepnutim DERE pinu na vysilani
    
    // Odesli pripravenou zadost
    digitalWrite(pin_DERE,HIGH);        // Prepnuti pinu pro rizeni smeru RS485 na vysilani.
    delay(5);                           // Kratka pauza pred zacatkem vysilani.
    Serial1.write(zadost,3);            // Cela pripravena sekvence v poli zadost[] (3 bajty) se odesle do seriove linky.
    delay(10);                          // Pauza pro buffer.
    digitalWrite(pin_DERE,LOW);         // Prepnuti pinu pro rizeni smeru RS485 zpatky do defaultniho stavu - na prijem.

    delay(80);                          // chvilku pauza, aby si SLAVE uvedomil, ze je zadost urcena pro nej a pripravil si odpoved

//  ----------------------------
//   Tady v tom trochu plavu. Obcas se stavalo, ze posledni prijaty bajt z ATtiny byl poskozeny. Tento bajt obsahoval LSB z kontrolniho souctu.
//   Poskozeni pak zpusobilo, ze kontrolni soucet nesouhlasil.
//   Proto ATtiny ted odesila po kontrolnim souctu vzdycky jeden bajt navic.  Tento bajt se pak uz ani nekontroluje. Kdyz se poskodi, vubec to nevadi.
//
//   Druhou zvlastnosti, ktere nerozumim, je to, ze po prepnuti komunikace na prijem se do nulte pozice prijimaciho bufferu ulozi cislo "0".
//   Vsechny ostatni data se pak prijmou o jednu pozici posunute. Protoze k tomuto jevu dochazi vzdycky pri prepnuti sbernice na prijem, resim to tak,
//    ze prvni bajt z bufferu prectu naprazdno.
//
//  Mozna by to chtelo v budoucnu trochu predelat.
//  ----------------------------

    unsigned int timeout = 1000;        // kdyz bude nejaky problem s komunikaci, tak se bude cekat maximalne 1 sekundu
    while (timeout > 0 and Serial1.available() < 10)        // ceka se na prijem 10 bajtu 
      {                                                          // (posledni bajt je navic - nevim proc, ale pri prenosu se obcas poskodi)
        timeout --;                                              //  proto se z ATtiny odesila 1 bajt na zaver navic
        delay (1);
      }

    if (timeout > 0)
      {
        Serial1.read();             // !!! TOHLE JE NESMYSL, ALE Z NEJAKEHO NEZNAMEHO DUVODU SE MI PO PREPNUTI NA PRIJEM ULOZI DO NULTE POZICE V BUFFERU CISLO 0
                                    //   A OSTATNI DATA SE PAK PRIJMOU O JEDNU POZICI POSUNUTE. TIMHLE PRIKAZEM SE TEN NULTY BAJT JEDNODUSE ODSTRANI.
        
        i=0;
        while (Serial1.available())         // precteni dat, ktera prisla ze seriove linky
          {
            odpoved[i] = Serial1.read();
            i++;
          }
        
        CRC16_mod = ModRTU_CRC(odpoved,7);    // vypocet "kontrolniho souctu" z prvnich 7 bajtu v poli odpoved[] 
    
        if (CRC16_mod != ((odpoved[7] * 256) + odpoved[8]))  // porovnani vypocteneho a prijateho CRC
          {
            
            // CRC error - Kontrolni soucet nesouhlasi, nastala nejaka chyba pri komunikaci.
            Serial.println("CRCerr");
//            Serial.println(CRC16_mod);
//            Serial.println((odpoved[7] * 256) + odpoved[8]);
//            Serial.println("------");
//            for (i=0;i<9;i++)
//              {
//                Serial.print(i);
//                Serial.print(" ... ");
//                Serial.println(odpoved[i]);
//              }
//            Serial.println("======");
            teplota_1 = 2510;
            teplota_2 = 2520;   
          } 
        else  // kdyz CRC souhlasi, poskladaji se ze 4 datovych bajtu dve teploty ve formatu unsigned int v rozsahu 0 az 1500 pro -50'C az 100'C
          {
            teplota_1 = (((odpoved[1] * 256) + odpoved[2]) / 10) - 100 + RAM1_read(EERAM_ADDR_korekce_OUT);  // pripoctou se jeste korekce
            teplota_2 = (((odpoved[3] * 256) + odpoved[4]) / 10) - 100 + RAM1_read(EERAM_ADDR_korekce_OUT2);
          }
        
      }
    else     // kdyz vyprsi timeout, nastavi se nesmyslne vysoke teploty
      {
        teplota_1 = 2530;
        teplota_2 = 2540;        
      }


    if (teplota_1 >= 2500 or teplota_2 >= 2500)     // pri chybe cidel (prijata teplota je 200'C = 2500 +/- nejake korekce)
      {
        chyba_cidla1 ++;
        if (chyba_cidla1 > 100) chyba_cidla1 = 100;
      }
    else
      {
        chyba_cidla1 = 0;
      }
    
  }



// nastaveni SLAVE adresy na desce s teplomery
//                                                           0x02 , 0x05 , 0xFF , 0xF9
//                aktualni adresa ____________________________|      |      |      |
//                nova adresa     ___________________________________|      |      |
//                65536 - puvodni adresa - nova adresa   ___________________|______|
//
//                                                           0x05 , 0x02 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0xFF , 0xF9 , 0xXX
//                nova adresa ________________________________|      |      |      |      |      |      |      |      |      |
//                puvodni adresa ____________________________________|      |      |      |      |      |      |      |      |
//                5 x 0x00 _________________________________________________|______|______|______|______|      |      |      |
//                65536 - puvodni adresa - nova adresa ________________________________________________________|______|      |
//                jeden bajt navic kvuli nejake chybe, ktera poskozuje posledni bajt pri prepinani sbernice _________________|  


boolean rs485_set_addr(byte MODadresa,byte nova_addr)
  {
   
    uint16_t CRC16_mod;
    zadost[0] = MODadresa;               // adresa SLAVE zarinei na sbernici
    zadost[1] = nova_addr;
    CRC16_mod = ModRTU_CRC(zadost,2);    // vypocet "kontrolniho souctu" z prvnich dvou bajtu v poli zadost[] (ze stare a nove adresy)
    zadost[2] = (CRC16_mod >> 8) & 255;  // doplneni pole zadost[] o kontrolni soucet (MSB)
    zadost[3] = CRC16_mod & 255;         // doplneni pole zadost[] o kontrolni soucet (LSB)

    delay(20);   

    while (Serial1.available())         // uplne vymazani  prijimaciho bufferu z predchozi komunikace
      {
        Serial1.read();
      }
    delay(5);                           // Kratka pauza pred pred prepnutim DERE pinu na vysilani
    
    // Odesli pripravenou zadost
    digitalWrite(pin_DERE,HIGH);        // Prepnuti pinu pro rizeni smeru RS485 na vysilani.
    delay(5);                           // Kratka pauza pred zacatkem vysilani.
    Serial1.write(zadost,4);            // Cela pripravena sekvence v poli zadost[] (4 bajty) se odesle do seriove linky.
    delay(10);                          // Pauza pro buffer.
    digitalWrite(pin_DERE,LOW);         // Prepnuti pinu pro rizeni smeru RS485 zpatky do defaultniho stavu - na prijem.

    delay(80);                          // chvilku pauza, aby si SLAVE uvedomil, ze je zadost urcena pro nej a pripravil si odpoved

    unsigned int timeout = 1000;        // kdyz bude nejaky problem s komunikaci, tak se bude cekat maximalne 1 sekundu
    while (timeout > 0 and Serial1.available() < 10)
      {
        timeout --;
        delay (1);
      }

    if (timeout > 0)
      {
        Serial1.read();             // !!! TOHLE JE NESMYSL, ALE Z NEJAKEHO NEZNAMEHO DUVODU SE MI PO PREPNUTI NA PRIJEM ULOZI DO NULTE POZICE V BUFFERU CISLO 0
                                    //   A OSTATNI DATA SE PAK PRIJMOU O JEDNU POZICI POSUNUTE. TIMHLE PRIKAZEM SE TEN NULTY BAJT JEDNODUSE ODSTRANI.

        
        i=0;
        while (Serial1.available())         // precteni dat, ktera prisla ze seriove linky
          {
            odpoved[i] = Serial1.read();
            i++;
          }
        
        CRC16_mod = ModRTU_CRC(odpoved,7);    // vypocet "kontrolniho souctu" z prvnich 7 bajtu v poli odpoved[] (indexy 0 az 6)
    
        if (CRC16_mod == ((odpoved[7] * 256) + odpoved[8]))  // porovnani vypocteneho a prijateho CRC
          {
            return true;
          }
        else
          {
            Serial.println("Chyba CRC pri pokusu o prenastaveni MOD adresy");
            return false;
          }
      }
    else
      {
        Serial.println("Chyba komunikace pri pokusu o prenastaveni MOD adresy");
      }
    return false;
  
  }











//========================================================================================================
// jen odecteni vsech dat do jednoho unsigned int cisla
unsigned int  ModRTU_CRC(byte pole[], byte pocet)
  {
    uint16_t crc = 0;
    byte i;
    for (byte pos = 0; pos < pocet; pos++)
      {
        crc = crc - pole[pos];    
      }
    return crc;  
  }

//========================================================================================================
  


void aktualizuj_soubor_s_rekordy(void)
  {

    // zobrazeni rekordu na hlavnim displeji
    dis_pulrekordy();

    if (SD_aktivni == 1)
      {
        char filename_rekordu[] = "rec000.txt";
        byte cislo_rek_souboru = RAM1_read(EERAM_ADDR_rec_file);
        filename_rekordu[3]  = 48 + (cislo_rek_souboru /  100 );             // stovky
        filename_rekordu[4]  = 48 + ((cislo_rek_souboru % 100)   /   10 );   // desitky 
        filename_rekordu[5]  = 48 + ((cislo_rek_souboru % 10))           ;   // jednotky
        
        
        if (tep_min_datcas_S != tep_min_datcas)    // kdyz se od minuleho minutoveho ulozeni rekordu rekord zmenil...
          {
            sd.begin(pin_SD_CS,SD_SCK_HZ(F_CPU/4));       //inicializace SD karty pro pripad, ze by byla predtim vytazena     
            SdFile::dateTimeCallback(dateTime);
            open_OK = soubor.open(filename_rekordu, O_WRITE | O_APPEND | O_CREAT);
            
            if (open_OK)
              {
                soubor.print (tep_min_datcas);
                soubor.print (' ');
                soubor.print(EEPROM_text_read(36));        // "Minimalni teplota"
                soubor.println(citelna_teplota2(tep_min));   // tep_min je v setinach 'C + 5000  (7256 = 22,56'C)
                tep_min_datcas_S = tep_min_datcas;
                ikona5(false);                // ikona vykricniku po uspesnem zapisu zmizi
              }
            else
              {
                ikona5(true);                 // pri chybe karty se zobrazi ikona vykricniku
              }
            soubor.close();
    
          }
    
        if (tep_max_datcas_S != tep_max_datcas)    // kdyz se od minuleho minutoveho ulozeni rekordu rekord zmenil...
          {
            sd.begin(pin_SD_CS,SD_SCK_HZ(F_CPU/4));       //inicializace SD karty pro pripad, ze by byla predtim vytazena     
            SdFile::dateTimeCallback(dateTime);
            open_OK = soubor.open(filename_rekordu, O_WRITE | O_APPEND | O_CREAT);
            
            if (open_OK)
              {
                soubor.print (tep_max_datcas);
                soubor.print (' ');
                soubor.print(EEPROM_text_read(37));        // "Maximalni teplota"
                soubor.println(citelna_teplota2(tep_max));   // tep_min je v setinach 'C + 5000  (7256 = 22,56'C)
                tep_max_datcas_S = tep_max_datcas;
                ikona5(false);                // ikona vykricniku po uspesnem zapisu zmizi
              }
            else
              {
                ikona5(true);                 // pri chybe karty se zobrazi ikona vykricniku
              }
            soubor.close();
          }
      }      
  }







// Pripravi ze souctu klouzavych prumeru zaokrouhleny retezec urceny k tisku na displeji
//      0 .... -50.0'C
//      1 .... -50.0'C
//      5 .... -50.0'C
//      6 .... -49.9'C
//     20 .... -49.8'C
//   5000 .... + 0.0'C
//   5058 .... + 0.6'C
//   6084 .... +10.8'C
//  10000 .... +50.0'C
//  14994 .... +99.9'C
//


String zobraz_cislo(unsigned long cislo)           // verze podprogramu bez zadaneho oddelovace
  {
    return zobraz_cislo(cislo , '.');
  }


String zobraz_cislo(unsigned long cislo, char oddelovac)
  {
    String vystup = "-28.3";
  
    if (cislo < 5000)          // podle kladneho, nebo zaporneho cisla se nastavi znamenko
      {
        vystup[0] = '-';
        cislo = 5000 - cislo;
      }
    else
      {
        cislo = cislo - 5000;   // pri kladne teplote se odecita 5000 
        vystup[0] = ' ';
      }
  
    cislo = cislo + 5;        // zaokrouhleni na predposledni misto (na desetiny 'C)
    cislo = cislo / 10;       // posledni misto se odsekne
  
    // priklad: z puvodniho 5058 vznikne 6 (to je 0.6'C)
  
    if (cislo >= 100)                       // pro teploty vetsi nez 10'C (nebo mensi, nez -10)
      {
        vystup[1] = 48 + (cislo / 100);
        cislo = cislo % 100;                // odseknuti radu desitek 'C
      }
    else                                    // pro teploty mensi nez 10'C (9.9 az -9.9)
      {
        vystup[1] = ' ';                    // se na rad desitek doplni mezera
      }
  
  
    vystup[2] = 48 + (cislo / 10);         // rad jednotek se zobrazuje vzdycky (i kdyby to mela byt 0)
    vystup[3] = oddelovac;                 // desetinna tecka je vzdycky a pozici 3
  
    vystup[4] = 48 + cislo % 10;           // rad jednotek ze vstupniho cisla se zapise za deseinnou tecku, jako desetiny 'C
  
    return vystup;
  
  }



// prevod hodin a minut na INT cislo, ktere se bude ukladat do pameti
unsigned int hodmin_to_int(byte hodiny , byte minuty)
  {
    return (hodiny * 60) + minuty;
  }


// prevod INT cisla na retezec hodin a minut ktere se budou zobrazovat
String int_to_hodmin(unsigned int hodmin)
  {
    String vysledek = "HH:mm";
  
    byte hodiny = hodmin / 60;
    vysledek[0] = 48 + (hodiny / 10);
    vysledek[1] = 48 + (hodiny % 10);
    if (vysledek [0] == '0') vysledek [0] = ' ';
  
    byte minuty = hodmin % 60;
    vysledek[3] = 48 + (minuty / 10);
    vysledek[4] = 48 + (minuty % 10);
  
    return vysledek;
  }
  

// prevod INT cisla na retezec hodin a minut ktere se budou zobrazovat (hodiny mohou mit 3 rady)
String int2_to_hodmin(unsigned int hodmin)
  {
    String vysledek = "HHH hod. MM min.";
  
    byte hodiny = hodmin / 60;
    if (hodiny == 0)                // kdyz je zbyvajici cas kratsi, nez hodina, hodiny se vubec nezobrazuji
      {
        for (i=0; i<8 ; i++)
          {
            vysledek [i] = ' '; 
          }
      }
    else
      {
        vysledek[0] = 48 + (hodiny / 100);
        hodiny = (hodiny % 100);
        vysledek[1] = 48 + (hodiny / 10);
        vysledek[2] = 48 + (hodiny % 10);
        if (vysledek [0] == '0') vysledek [0] = ' ';        // uvodni nuly se u hodin nezobrazuji
        if (vysledek [1] == '0') vysledek [1] = ' ';
      }
  
    byte minuty = hodmin % 60;
    vysledek[9] = 48 + (minuty / 10);
    vysledek[10] = 48 + (minuty % 10);

     if (vysledek[9] == '0')   // u minut se take maze uvodni '0'
       {
         vysledek[9] = ' ';
       }

  
    return vysledek;
  }



// podprogram pro nastavovani datumu a casu u souboru na SD karte:
// zdroj:  https://forum.arduino.cc/index.php?topic=348562.0
void dateTime(uint16_t* date, uint16_t* time)
  {
    *date = FAT_DATE(LOC_rok, LOC_mes, LOC_den);
    *time = FAT_TIME(LOC_hod, LOC_min, LOC_sek);
  }


boolean test_periferii_1(void)
  {
    boolean uspech = true;
    byte pomprom_test;


    Serial2.print("dim=80");
    SerialFFF();

    Serial2.print("vis t0,0");
    SerialFFF();



    // ------------  Displej ------------
    Serial2.print("get j0.val");
    SerialFFF();
    delay(20);

    pomprom_test = Serial2.read();
    flush_read_buffer2();
    if (pomprom_test = 0x71)
      {
        Serial2.print("j0.val=100");
        SerialFFF();
      }
    else
      {
        return false;         // kdyz je chyba v displeji, nema cenu dalsi testovani
      }


    // ------------  SD karta ------------
    sd.begin(pin_SD_CS,SD_SCK_HZ(F_CPU/4));       //inicializace SD karty pro pripad, ze by byla predtim vytazena
    pomprom_test = soubor.open("log.txt", O_WRITE | O_APPEND | O_CREAT);
    if (pomprom_test == true)
      {
        soubor.close();
        Serial2.print("j9.val=100");
      }
    else
      {
        Serial2.print("j9.bco=YELLOW");   // SD karta nemusi byt zasadni problem tak se jenom oznaci jako varovani
                                          //   ale nezpusobuje to zastaveni programu
        RAM1_write(EERAM_ADDR_SDaktiv,0); //   Aktivni operace s SD kartou se vypnou
        // uspech = false;                // v tomto pripade se z podprogramu nevraci "false"
      }
    SerialFFF();


    // ------------  RTC ------------
    RTC_default();

    RTC2016();     // test, jestli je v RTC nastaveny rok v intervalu 2016 az 2070. Kdyz ne, nastavi se na 1.1.2016

    
    Wire.beginTransmission(I2C_ADDR_DS3231);
    Wire.write(0); // set DS3231 register pointer to 00h
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR_DS3231, 1);
    pomprom_test = Wire.read();
    delay(1010);                                            // pockat alespon sekundu
    Wire.beginTransmission(I2C_ADDR_DS3231);
    Wire.write(0); // set DS3231 register pointer to 00h
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR_DS3231, 1);

    if (Wire.read() != pomprom_test)                        // sekundy se zmenily
      {
        Serial2.print("j8.val=100");
      }
    else
      {
        uspech = false;
        Serial2.print("j8.bco=RED");
      }
    SerialFFF();
    
        



    // ------------  EERAM 1 ------------
    pomprom_test = RAM1_read(EERAM_ADDR_testper1);
    pomprom_test ++;
    RAM1_write(EERAM_ADDR_testper1,pomprom_test);
    delay(10);
    if (pomprom_test == RAM1_read(EERAM_ADDR_testper1))
      {
        Serial2.print("j2.val=100");
      }
    else
      {
        uspech = false;
        Serial2.print("j2.bco=RED");
      }
    SerialFFF();


    // ------------  EERAM 2 ------------
    pomprom_test = RAM2_read(EERAM_ADDR_testper2);
    pomprom_test ++;
    RAM2_write(EERAM_ADDR_testper2,pomprom_test);
    delay(10);
    if (pomprom_test == RAM2_read(EERAM_ADDR_testper2))
      {
        Serial2.print("j3.val=100");
      }
    else
      {
        uspech = false;
        Serial2.print("j3.bco=RED");
      }
    SerialFFF();

    // ------------  EEPROM ------------
    pomprom_test = EEPROM_read(EEPROM_ADDR_testper3);
    pomprom_test ++;
    EEPROM_write(EEPROM_ADDR_testper3,pomprom_test);
    delay(10);
    if (pomprom_test == EEPROM_read(EEPROM_ADDR_testper3))
      {
        Serial2.print("j1.val=100");
      }
    else
      {
        uspech = false;
        Serial2.print("j1.bco=RED");
      }
    SerialFFF();





    return uspech;
  }


// test cidel - nevraci chyby, ale jen nastavuje barvy (zelena/zluta) u jednotlivych polozek
void test_periferii_2(void)
  {

    // -------------- cidlo tlaku -------------------
    if (bmp.begin())
      {
        Serial2.print("j4.val=100");
      }
    else
      {
        Serial2.print("j4.bco=YELLOW");
      }
    SerialFFF();
    delay(500);
    
    
    // -------------- cidlo teploty DS18B20 -------------------

    typ_out_teplomeru = RAM1_read(EERAM_ADDR_typ_teplomeru);
    byte uspech = true;
    if (typ_out_teplomeru < 3)    // jedno nebo dve cidla DS18B20
      {
        DS18B20.begin();
        delay(500);
        DS18B20.requestTemperatures();
        delay(500);
        teplota_ds = DS18B20.getTempCByIndex(0);  // zjisteni teploty z cidla teploty
    
        if (teplota_ds > 100 or teplota_ds < -50)      // kdyz je teplota mimo realitu
          {
            uspech = false;
          }

        if (typ_out_teplomeru == 2)    // dve cidla DS18B20
          {
            DS18B20.requestTemperatures();
            delay(1000);
            teplota_ds = DS18B20.getTempCByIndex(1);  // zjisteni teploty z cidla teploty
        
            if (teplota_ds > 100 or teplota_ds < -50)      // kdyz je teplota mimo realitu
              {
                uspech = false;
              }
            
          }


        if (typ_out_teplomeru == 3)    // dalkova cidla pres RS485
          {
            rs485_get_temp(mod_addr);
            if (teplota_1 > 1500 or teplota_2 > 1500)      // kdyz je teplota mimo realitu
              {
                uspech = false;
              }
            
          }


          
          
        if (uspech == true)
          {
            Serial2.print("j5.val=100");
          }
        else
          {
            Serial2.print("j5.bco=YELLOW");
          }
        SerialFFF();
        
      }


    // -------------- cidlo vlhkosti -------------------
    vlhkomerDHT.begin();
    delay(500);
    cidlo_vlhkosti = vlhkomerDHT.readHumidity();

    if (isnan(cidlo_vlhkosti))
      {
        Serial2.print("j7.bco=YELLOW");
      }
    else
      {
        Serial2.print("j7.val=100");
      }
    SerialFFF();


    // -------------- cidlo vnitrni teploty -------------------
    float pomprom = vlhkomerDHT.readTemperature();
    if (pomprom > 0 and pomprom < 40)
      {
        Serial2.print("j6.val=100");
      }
    else
      {
        Serial2.print("j6.bco=YELLOW");
      }
    SerialFFF();
      

    
  }



// test dostupnosti datovych souboru na SD karte pro prvni zapnuti
boolean test_periferii_3(void)
  {
    boolean uspech = true;
    boolean pomprom_test;

    // ------------  SD karta ------------
    sd.begin(pin_SD_CS,SD_SCK_HZ(F_CPU/4));       //inicializace SD karty pro pripad, ze by byla predtim vytazena
    pomprom_test = soubor.open("faze.txt", O_READ);
    if (pomprom_test == true)        soubor.close();
    else                             uspech = false;  
    

    pomprom_test = soubor.open("zatmeni.txt", O_READ);
    if (pomprom_test == true)        soubor.close();
    else                             uspech = false;  

    pomprom_test = soubor.open("texty.txt", O_READ);
    if (pomprom_test == true)        soubor.close();
    else                             uspech = false;  

    if (uspech == false)
      {
        Serial2.print("j9.bco=RED");
        SerialFFF();
        Serial2.print("j9.val=0");
        SerialFFF();
      }

    
    return uspech;

  }



//---------------------------------------------------------------------------------------------
// nastaveni defaultnich hodnot do EERAM (korekce, GEO souradnice ...)
void tovarni_reset(void)
  {
    Serial.println("Tovarni RESET");
    
    RAM1_write(EERAM_ADDR_timezone,     2 - pcf.read(4));       //casova zona podle aktualni polohy prepinace
    RAM1_write(EERAM_ADDR_korekce_IN,          100);            //Korekce teplomeru z cidla vlhkosti
    RAM1_write(EERAM_ADDR_korekce_tlak,        100);            //Korekce tlaku
    RAM1_write(EERAM_ADDR_korekce_vlhkost,     100);            //Korekce vlhkosti
    RAM1_write(EERAM_ADDR_korekce_OUT,         100);            //Korekce prvniho teplomeru DS18B20
    RAM1_write(EERAM_ADDR_korekce_OUT2,        100);            //Korekce druheho teplomeru DS18B20 
    RAM1_write(EERAM_ADDR_typ_teplomeru,         1);            //Typ venkovniho teplomeru: 1x DS18B20       
    RAM1_write(EERAM_ADDR_zhasinat_podsvet,      1);            //zhasinat podsvet
    RAM1_write(EERAM_ADDR_SDaktiv ,              1);            //Aktivni operace s SD kartou
    RAM1_write(EERAM_ADDR_LEDblok ,              1);            //Blokovani LED
    RAM1_write_int(EERAM_ADDR_GeoLat ,        5000);            //GeoLat * 100  (50.00 stupnu)
    RAM1_write_int(EERAM_ADDR_GeoLon ,        1500);            //GeoLat * 100  (15.00 stupnu)
    RAM1_write_int(EERAM_ADDR_GeoAlt ,           0);            //nadmorska vyska 0m (tlak se nebude prepocitavat)
    RAM1_write_int(EERAM_ADDR_prepinaci_uroven1, 0);            //prvni prepinaci uroven na -50'C
    RAM1_write_int(EERAM_ADDR_prepinaci_uroven2, 0);            //druha prepinaci uroven na -50'C
    RAM1_write_int(EERAM_ADDR_dis_on,            0);            //cas rozsviceni displeje v minutach od pulnoci
    RAM1_write_int(EERAM_ADDR_dis_off,           0);            //cas zhasnuti displeje v minutach od pulnoci
    RAM1_write(EERAM_ADDR_dis_maska,             0);            //ve kterych dnech se ma budik zapinat
    RAM1_write(EERAM_ADDR_adr_mod,               1);            //SLAVE adresa desky s teplomery, ze ktere se maji cist teploty pri komunikaci pres RS485

    nuluj_rekordy();
    
  }


void rozsvit_R(void)
  {
    if (LED_blok == 0)               // LED se rozsvecuje jen kdyz neni blokovana
      {
        pcf.write(1, LOW);
      }
    if (screen == 0)
      {
        Serial2.print(EEPROM_text_read(168));           // "ik1.pic=74"
        SerialFFF();
      }  
    status_byte = status_byte | 0b00100000;
    
  }

void zhasni_R(void)
  {
    pcf.write(1, HIGH);

    if (screen == 0)
      {
        Serial2.print(EEPROM_text_read(169));            // "ik1.pic=81"
        SerialFFF();
      }  
    status_byte = status_byte & 0b11011111;


  }

void rozsvit_G(void)
  {
    if (LED_blok == 0)               // LED se rozsvecuje jen kdyz neni blokovana
      {
        pcf.write(2, LOW);
      }

    if (screen == 0)
      {
        Serial2.print(EEPROM_text_read(170));          // "ik2.pic=75"
        SerialFFF();
      }
    status_byte = status_byte | 0b00010000;

  }

void zhasni_G(void)
  {
    pcf.write(2, HIGH);

    if (screen == 0)
      {
        Serial2.print(EEPROM_text_read(171));           // "ik2.pic=81"
        SerialFFF();
      }

    status_byte = status_byte & 0b11101111;

  }

void rozsvit_B(void)
  {
    if (LED_blok == 0)               // LED se rozsvecuje jen kdyz neni blokovana
      {
        pcf.write(0, LOW);
      }
     
    if (screen == 0)
      {
        Serial2.print(EEPROM_text_read(172));             // "ik3.pic=76"
        SerialFFF();
      }      
    status_byte = status_byte | 0b10000000;
    
  }

void zhasni_B(void)
  {
    pcf.write(0, HIGH);

    if (screen == 0)
      {
        Serial2.print(EEPROM_text_read(173));             // "ik3.pic=80"
        SerialFFF();
      }  
    status_byte = status_byte & 0b01111111;

  }





//-----------------------------------------------------------------
// podprogramy pro rozsvecovani a zhasinani InfoLED
// kdyz je cidlo teploty v poruse, LED trvale sviti, ale kratkym pohasnutim signalizuje napriklad priblizeni, prijem Infra prikazu
// kdyz je cidlo teploty v poradku, je LED trvale zhasnuta, ale pri priblizeni, nebo pro prijmu infra signalu se kratce rozsviti
void zhasni_info(void)
  {
    if (stav_cidla1 == 0 and stav_cidla2 == 0 )
      {
        pinMode(pin_LED_info, INPUT);    
      }
    else
      {
        pinMode(pin_LED_info, OUTPUT);
        digitalWrite(pin_LED_info, LOW);
        delayMicroseconds(40000);
      }
  }


void rozsvit_info(void)
  {
    if (stav_cidla1 == 0 and stav_cidla2 == 0 )
      {
        pinMode(pin_LED_info, OUTPUT);
        digitalWrite(pin_LED_info, LOW);
      }
    else
      {
        pinMode(pin_LED_info, INPUT);
      }
  
  }
//-----------------------------------------------------------------




//-----------------------------------------------------------
// kdyz infracidlo pohybu zaznamena zmenu, blikne LED
void pohybINT(void)
  {
    detachInterrupt(digitalPinToInterrupt(pin_infra));        // zrusit interrupt od dalkoveho ovladani
    
    byl_pohyb=false;

    if (InfraIndex > 98)         // pohyb je vyhodnocen jen v pripade, ze nedoslo zaroven k prijmu nejakeho kodu z DO
      {
        byl_pohyb=true;

        if (zpozdeni_podsvetu == 0 and blokuj_priblizeni == 0)      // LEDka blikne jen v pripade, ze je displej zhasnuty a neni docasne zablokovany
          {
            rozsvit_info();  
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            delayMicroseconds(10000);
            zhasni_info(); 
          }
        else     // kdyz je cidlo priblizeni deaktivovane, nebo kdyz je displej rozsviceny, tak se znova zapne infracidlo dalkoveho ovladani
          {
            attachInterrupt(digitalPinToInterrupt(pin_infra)  , infraINT  , FALLING);        
          }
      }
  }



//-------------------------------------------------------
// obsluha preruseni pri prijmu nejakeho infra tlacitka v rezimu uceni infrakodu
void infraINT_nastav(void)
  {
    unsigned int pole_impulzu[36];
    unsigned int sirka;
    byte i;    
    detachInterrupt(digitalPinToInterrupt(pin_infra));     // zruseni interruptu (aby se vyhodnotil jen jeden kod z dalkoveho ovladace)
  
    for (i = 0; i < 36 ; i++)
      {
        sirka = pulseIn(pin_infra, HIGH, 10000) ;                   // bude se testovat jen sirka HIGH impulzu (predpoklada se, ze LOW impulzy jsou stejne siroke)
        
        if (sirka >  400 and sirka <  700) pole_impulzu[i] = 0;     // sirka impulzu je kratka
        if (sirka > 1400 and sirka < 1800) pole_impulzu[i] = 1;     // sirka impulzu je dlouha
      }
    InfraHexaKod_uceni = 0;

    for (i = 0; i < 32 ; i++)
      {
        if (pole_impulzu[i] == 1)
          {
            InfraHexaKod_uceni = InfraHexaKod_uceni + ( 1 << (31 - i));
          }
      }
     

    for (i = 0; i < 36 ; i++)
      {
        pole_impulzu[i] = 0;
      }
      
  }



//-------------------------------------------------------
// obsluha preruseni pri prijmu nejakeho infra tlacitka
void infraINT(void)
  {
    unsigned int pole_impulzu[36];
    unsigned int sirka;
    boolean nalezeno = false;
    byte i;    
    detachInterrupt(digitalPinToInterrupt(pin_infra));     // zruseni interruptu (aby se vyhodnotil jen jeden kod z dalkoveho ovladace)
                                                           //         po vyhodnoceni se preruseni na konci podporgramu 'infraswitch()' zase zapne

  
    for (i = 0; i < 36 ; i++)
      {
        sirka = pulseIn(pin_infra, HIGH, 10000) ;
        if (sirka >  400 and sirka <  700) pole_impulzu[i] = 0;     // sirka impulzu je kratka
        if (sirka > 1400 and sirka < 1800) pole_impulzu[i] = 1;     // sirka impulzu je dlouha
      }

    InfraIndex = 100;
    unsigned long InfraHexaKod = 0;

    for (i = 0; i < 32 ; i++)
      {
        if (pole_impulzu[i] == 1)
          {
            InfraHexaKod = InfraHexaKod + ( 1 << (31 - i));
          }
      }
     
    for (i = 0; i<22 ; i++)   // k bliknuti LED dojde jen v pripade, ze je rozpoznan jeden z naucenych kodu 
      {
        if (InfraHexaKod == infra_btn[i])
          {
            InfraIndex = i;
            rozsvit_info();
            delayMicroseconds(10000);
            zhasni_info();
            nalezeno = true;
          }
      }

    if (nalezeno == false)
      {
        InfraIndex = 100;
      }


    for (i = 0; i < 36 ; i++)
      {
        pole_impulzu[i] = 0;
      }

  }



//============================================================================
//   DISPELJ TJC4832 (Nextion)
//============================================================================




//====================================================================================
// Odeslani datumu na NEXTION displej
// priklad:
// D_ser(3 ,"12. 5. 2019")
//    dvt.pic=45 #FF#FF#FF
//    d0.txt="12. 5.2019" #FF#FF#FF
void D_ser(byte den_v_tydnu, String data)
  {
    if (screen==0)
      {
        Serial2.print(EEPROM_text_read(0));               // "dvt.pic="
        Serial2.print(den_v_tydnu + 42);
        SerialFFF();
    
       
        Serial2.print(EEPROM_text_read(1));               // "d0.txt=\""
        Serial2.print(data);
        Serial2.print('\"');
        SerialFFF();
        
      }
  }










//====================================================================================
// Prevod casu na 6 obrazku (h0) az (h5) na NEXTION displeji
// priklad:
// " 2:45:14"
//    h0.pic=35 #FF#FF#FF
//    h1.pic=22 #FF#FF#FF
//    h2.pic=24 #FF#FF#FF
//    h3.pic=25 #FF#FF#FF
//    h4.pic=11 #FF#FF#FF
//    h5.pic=14 #FF#FF#FF

void T_ser(String data)
  {
    byte index_znaku;
    if (screen==0)
      {
        for (byte n_objekt = 0 ; n_objekt < 6 ; n_objekt ++ )
          {
            Serial2.print('h');
            Serial2.print(n_objekt);        
            Serial2.print(".pic=");
            switch (n_objekt)
              {
                case 0:                       // desitky hodin
                  index_znaku = 0;
                  break;
    
                case 1:                       // jednotky hodin
                  index_znaku = 1;
                  break;
    
                case 2:                       // desitky minut
                  index_znaku = 3;
                  break;
    
                case 3:                       // jednotky minut
                  index_znaku = 4;
                  break;
    
                case 4:                      // desitky sekund (mensi zluty font)
                  index_znaku = 6;
                  break;
    
                case 5:                      // jednotky sekund (mensi zluty font)
                  index_znaku = 7;
                  break;
    
              }
    
            if (index_znaku < 6)             // hodiny a minuty velkym fontem
              {
                if (data[index_znaku] == ' ') Serial2.print("35");                         // 35 = velka mezera v resources
                else                          Serial2.print(data[index_znaku]-48 + 20);    // 48 = ASCII '0';   20 = zacatek velkeho digifontu v resources
              }
            else                             // sekundy malym fontem
              {
                Serial2.print(data[index_znaku]-48 + 10);    // 48 = ASCII '0';   10 = zacatek maleho zluteho digifontu v resources
              }
    
            SerialFFF();        
          }
        
      }

    
  }




//====================================================================================
// Prevod cisla na 4 obrazky (i0) az (i3) nebo (o0) az (o3) na NEXTION displeji
// priklad:
// "-25.4" , 'o'
//    to0.pic=31 #FF#FF#FF
//    to1.pic=2  #FF#FF#FF
//    to2.pic=5  #FF#FF#FF
//    to3.pic=4  #FF#FF#FF
void dis_N(String vstup_N, char objekt)
  {
    if (screen==0)
      {
        Serial2.print('t');
        // znamenko nebo mezera na indexu 0
        Serial2.print(objekt);
        if (vstup_N[0] == '-') Serial2.print(EEPROM_text_read(2));                   // "0.pic=31"     // zelene '-'
        else                   Serial2.print(EEPROM_text_read(3));                   // "0.pic=34"     // prazdne pole
    
        SerialFFF();
    
        // rad desitek nebo mezera na indexu 1
        Serial2.print('t');
        Serial2.print(objekt);
        if (vstup_N[1] == ' ') Serial2.print(EEPROM_text_read(129));          // "1.pic=34"         // prazdne pole
        else
          {
            Serial2.print("1.pic=");         // priprava prikazu
            Serial2.print(vstup_N[1]-48);      // znak posunuty na prislusny index v resource ('0' => 0)
          }
    
        SerialFFF();
    
        // rad jednotek (jednotky se zobrazuji vzdycky, i kdyby to mela byt 0)
        Serial2.print('t');
        Serial2.print(objekt);
        Serial2.print("2.pic=");         // priprava prikazu
        Serial2.print(vstup_N[2]-48);      // znak posunuty na prislusny index v resource ('0' => 0)
    
        SerialFFF();
    
        // rad desetin (desetiny se zobrazuji vzdycky, i kdyby to mela byt 0)
        Serial2.print('t');
        Serial2.print(objekt);
        Serial2.print(EEPROM_text_read(228));                   // "3.pic="         // priprava prikazu
        Serial2.print(vstup_N[4]-48);      // znak posunuty na prislusny index v resource ('0' => 0)
    
        SerialFFF();
           
      }

  }




//====================================================================================
// Prevod cisla 0 az 99 na 2 obrazky (v0) a (v1) na NEXTION displeji
// v pripade, ze je cislo mensi, nez 10, zobrazuje se na prvni pozici mezera
// priklad:
// 59 
//    v0.pic=5 #FF#FF#FF
//    v1.pic=9 #FF#FF#FF
void dis_V(byte vstup_V)
  {
    if (screen==0)
      {
        // desitky nebo mezera na indexu 0
        Serial2.print(EEPROM_text_read(130));        // "v0.pic="
        if (vstup_V < 10) Serial2.print(34);         // prazdne pole
        else
          {
            Serial2.print(vstup_V/10);
          }
            
        SerialFFF();
    
    
        // jednotky na indexu 1
        Serial2.print(EEPROM_text_read(217));        // "v1.pic="
          {
            Serial2.print(vstup_V % 10);
          }
            
        SerialFFF();       
      }

  }


void dis_pulrekordy(void)
  {
    if (screen == 0)
      {

        if (RAM1_read(EERAM_ADDR_dennoc) == 6)
          {
            //  rh.txt="Aktualne od 6:00"
            Serial2.print(EEPROM_text_read(4));                  // "rh.pic=71" 
            SerialFFF();
          }
        else
          {
            //  rh.txt="Aktualne od 18:00"
            Serial2.print(EEPROM_text_read(5));                  //"rh.pic=72" 
            SerialFFF();
          }
        

        if (tep_max1 > tep_min1)
          {

            Serial2.print(EEPROM_text_read(6));                   // "rx1.xcen=2"
            SerialFFF();
            Serial2.print(EEPROM_text_read(7));                   // "ri1.xcen=2"
            SerialFFF();
            
            //  rx1.txt="max: 25.6 (19:45)"
            Serial2.print(EEPROM_text_read(8));                   // "rx1.txt=\"max: "
            Serial2.print(zobraz_cislo(tep_max1));
            Serial2.print(" (");
            Serial2.print(int_to_hodmin(RAM1_read_int(EERAM_ADDR_hhmm_max1)));
            Serial2.print(")\"");
            SerialFFF();
            
            //  ri1.txt="min: 18.1 (20:20)"
            Serial2.print(EEPROM_text_read(9));                    // "ri1.txt=\"min: "
            Serial2.print(zobraz_cislo(tep_min1));
            Serial2.print(" (");
            Serial2.print(int_to_hodmin(RAM1_read_int(EERAM_ADDR_hhmm_min1)));
            Serial2.print(")\"");
            SerialFFF();

          }
        else
          {
            
            Serial2.print(EEPROM_text_read(10));                    // "rx1.xcen=0"
            SerialFFF();
            Serial2.print(EEPROM_text_read(11));                    // "ri1.xcen=0");
            SerialFFF();
            Serial2.print(EEPROM_text_read(12));                    // "rx1.txt=\"max: ---\""
            SerialFFF();
            Serial2.print(EEPROM_text_read(13));                    // "ri1.txt=\"min: ---\"");
            SerialFFF();
          }


        
        
        
        
        if (LOC_hod >= 6 and LOC_hod < 18)
          {
            Serial2.print(EEPROM_text_read(176));                    // "rd.pic=70"
          }
        else
          {
            Serial2.print(EEPROM_text_read(177));                    // "rd.pic=69"
          }
        SerialFFF();
        
        if (tep_max2 > tep_min2)
          { 
            Serial2.print(EEPROM_text_read(178));                     // "rx2.xcen=2"
            SerialFFF();
            Serial2.print(EEPROM_text_read(179));                     // "ri2.xcen=2"
            SerialFFF();
            
            //  rx2.txt="max: 12.6 (14:10)"
            Serial2.print(EEPROM_text_read(180));                     // "rx2.txt=\"max: "
            Serial2.print(zobraz_cislo(tep_max2));
            Serial2.print(" (");
            Serial2.print(int_to_hodmin(RAM1_read_int(EERAM_ADDR_hhmm_max2)));
            Serial2.print(")\"");
            SerialFFF();
            
            //  ri2.txt="min: 11.4 (8:56)"
            Serial2.print(EEPROM_text_read(181));                     // "ri2.txt=\"min: "
            Serial2.print(zobraz_cislo(tep_min2));
            Serial2.print(" (");
            Serial2.print(int_to_hodmin(RAM1_read_int(EERAM_ADDR_hhmm_min2)));
            Serial2.print(")\"");
            SerialFFF();        
          }
        else
          {
            Serial2.print(EEPROM_text_read(182));                      // "rx2.xcen=0"
            SerialFFF();
            Serial2.print(EEPROM_text_read(183));                      // "ri2.xcen=0"
            SerialFFF();

            Serial2.print(EEPROM_text_read(184));                      // "rx2.txt=\"max: ---\""
            SerialFFF();
            Serial2.print(EEPROM_text_read(185));                      // "ri2.txt=\"min: ---\""
            SerialFFF();            
          }

        // aktualizace ikon nad puldennimi rekordy
          

        if (SD_aktivni == 0)      Serial2.print(EEPROM_text_read(186));   // "ik5.pic=80"       // aktualizace ikony SD
        else                      Serial2.print(EEPROM_text_read(187));   // "ik5.pic=78"
        SerialFFF();            
      
        if (LED_blok == 0)        Serial2.print(EEPROM_text_read(188));   // "ik7.pic=82"       // aktualizace ikony pro povolene LED
        else                      Serial2.print(EEPROM_text_read(189));   // "ik7.pic=80"
        SerialFFF();            


        if (stav_BT == 0)         Serial2.print(EEPROM_text_read(175));      // "ik4.pic=80     // aktualizace ikony pro BT
        else                      Serial2.print(EEPROM_text_read(174));      // "ik4.pic=77"
        SerialFFF();            

        if (SD_aktivni == 0)      ikona5(false);                            // smazani vykricniku
     
      }

  }


void aktualizuj_graf_urovni(void)
  {
    if (prepinaci_uroven1 == 0 and prepinaci_uroven2 == 1500)
      {
        Serial2.print(EEPROM_text_read(190));         // "gr.pic=98"          // sviti v celem rozsahu
        SerialFFF();
        return; 
      }

    if (prepinaci_uroven1 == prepinaci_uroven2)
      {
        Serial2.print(EEPROM_text_read(191));         // "gr.pic=99"         // nesviti vubec
        SerialFFF();  
        return; 
      }
    

    if (prepinaci_uroven1 == 0 and prepinaci_uroven2 < 1500)
      {
        Serial2.print(EEPROM_text_read(192));         // "gr.pic=86"         // sviti od zacatku do urovne 2
        SerialFFF();  
        return; 
      }

    if (prepinaci_uroven1 > 0 and prepinaci_uroven2 == 1500)
      {
        Serial2.print(EEPROM_text_read(193));         // "gr.pic=87"         // sviti od urovne 1 do konce
        SerialFFF();  
        return; 
      }


    if (prepinaci_uroven1 == 1500 or prepinaci_uroven2 == 0)
      {
        Serial2.print(EEPROM_text_read(194));         // "gr.pic=99"         // nesmyslne zadani - porad zhasnuto
        SerialFFF();  
        return; 
      }


    if (prepinaci_uroven2 > prepinaci_uroven1)
      {
        Serial2.print(EEPROM_text_read(195));         // "gr.pic=84"          // normalni pripad (LED sviti mezi urovnemi)
        SerialFFF();  
        return; 
      }


    if (prepinaci_uroven2 < prepinaci_uroven1)
      {
        Serial2.print(EEPROM_text_read(196));         // "gr.pic=85"          // opacny pripad (LED nesviti mezi urovnemi)
        SerialFFF();  
        return; 
      }
  }


void dis_rekordy(void)
  {

    Serial2.print(EEPROM_text_read(197));            // "rrd.txt=\""
    Serial2.println(tep_rst_datcas);                 // datum a cas posledniho resetu rekordu
    Serial2.print("\"");
    SerialFFF();

    Serial2.print(EEPROM_text_read(198));            // "rxt.txt=\""
    Serial2.print(citelna_teplota2(tep_max));
    Serial2.print("'C\"");
    SerialFFF();    
    
    Serial2.print(EEPROM_text_read(199));            // "rxd.txt=\""
    Serial2.print(tep_max_datcas);
    Serial2.print("\"");
    SerialFFF();

    Serial2.print(EEPROM_text_read(200));            // "rit.txt=\""
    Serial2.print(citelna_teplota2(tep_min));
    Serial2.print("'C\"");
    SerialFFF();    
    
    Serial2.print(EEPROM_text_read(201));            // "rid.txt=\""
    Serial2.print(tep_min_datcas);
    Serial2.print("\"");
    SerialFFF();
        
  }

//nastaveni hodin a datumu podle policek v nastavovaci obrazovce casu na displeji 
void nastav_dis_hodiny(void)
  {
    byte odpoved_dis;
    // hodiny
    Serial2.print(EEPROM_text_read(202));            // "get eh.val"
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        LOC_hod = Serial2.read();
      }                
    flush_read_buffer2();

    // minuty
    Serial2.print(EEPROM_text_read(203));            // "get ei.val"
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        LOC_min = Serial2.read();
      }                
    flush_read_buffer2();


    // sekundy
    Serial2.print(EEPROM_text_read(204));            // "get es.val"
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        LOC_sek = Serial2.read();
      }                
    flush_read_buffer2();


    // dny
    Serial2.print(EEPROM_text_read(205));            // "get ed.val"
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        LOC_den = Serial2.read();
      }                
    flush_read_buffer2();


    // mesice
    Serial2.print(EEPROM_text_read(206));            // "get em.val"
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        LOC_mes = Serial2.read();
      }                
    flush_read_buffer2();


    // roky
    Serial2.print(EEPROM_text_read(207));            // "get er.val"
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        LOC_rok = Serial2.read() + (Serial2.read()*256);
      }                
    flush_read_buffer2();


    // den v tydnu
    Serial2.print(EEPROM_text_read(208));            // "get va0.val"
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        LOC_dvt = Serial2.read();
      }                
    flush_read_buffer2();

    setDS3231time(LOC_sek, LOC_min, LOC_hod, LOC_den, LOC_mes, LOC_rok, LOC_dvt);  // knihovni funkce pro zapis do RTC obvodu
    priste_prohledavat_od = 0;
    najdi_VZM();

    
  }



void edit_hodiny(void)
  {
    Serial2.print(EEPROM_text_read(209));         // "eh.val="
    Serial2.print(LOC_hod);
    SerialFFF();

    Serial2.print(EEPROM_text_read(210));         // "ei.val="
    Serial2.print(LOC_min);
    SerialFFF();
    
    Serial2.print(EEPROM_text_read(211));         // "es.val="
    Serial2.print(LOC_sek);
    SerialFFF();

    Serial2.print(EEPROM_text_read(212));         // "ed.val="
    Serial2.print(LOC_den);
    SerialFFF();

    Serial2.print(EEPROM_text_read(213));         // "em.val="
    Serial2.print(LOC_mes);
    SerialFFF();
    
    Serial2.print(EEPROM_text_read(214));         // "er.val="
    Serial2.print(LOC_rok);
    SerialFFF();

    Serial2.print(EEPROM_text_read(215));         // "t9.txt=\""
    Serial2.print(dny[LOC_dvt]);
    Serial2.print("\"");
    SerialFFF();
    
    Serial2.print(EEPROM_text_read(216));         // "va0.val="
    Serial2.print(LOC_dvt);
    SerialFFF();



  }



// zobrazi (/nezobrazi) ikonu budiku, podle toho, jestli nastane dalsi cas buzeni drive, nez za 24 hodin
// dovnitr vstupuji globalni promenne pro aktualni cas: 'LOC_hod', 'LOC_min', 'LOC_dvt'
//                               a pro nastaveny budik: 'budik_on', 'budik_maska'
// vystupem je odeslani retezce do displeje pro zobrazeni ikony budiku: "ik9.pic=270"
//                                       nebo pro smazani ikony budiku: "ik9.pic=80"
void budik_iko(void)
  {
    zbyva_do_budiku = 30000;      // nikdy nebude do pristiho budiku delsi doba, nez 10080 minut (7 dni * 24 hodin * 60 minut)
    unsigned int aktualni_week_minuty = akt_LOC_hodmin + ((LOC_dvt - 1) * 1440);     // aktualni pocet minut od pondelni pulnoci
    unsigned int budik_week_minuty;            // pro nalezeny budik se vypocte jeho cas od zacatku tydne

    if (bitRead(budik_maska, 8 - LOC_dvt) == true)          // kdyz je pro dnesek nastaveny budik ...
      {
        if (akt_LOC_hodmin <= budik_on)                     // ... a zaroven je aktualni cas mensi, nez nastaveny budik
          {
            zbyva_do_budiku = budik_on - akt_LOC_hodmin;    // nastane budik za jednoduche odecteni tech dvou casu
            dvt_pro_budik = LOC_dvt + 42;
          }
      }

    if (zbyva_do_budiku == 30000)
      {


        if (LOC_dvt < 7)                                          // pro aktualni den pondeli az sobota ...
          {
            for (i = (LOC_dvt + 1) ; i < 8 ; i++)                 // ... budou se prohledavat jednotlive dny od zitrka az do nedele
              {
                if (bitRead(budik_maska, 8 - i) == true)          // kdyz je pro den 'i' nastaveny budik ...
                  {
                    budik_week_minuty = (i-1) * 1440 + budik_on;  //  ...vypocte se rozdil casu budiku a aktualniho casu
                    zbyva_do_budiku = budik_week_minuty - aktualni_week_minuty;
                    dvt_pro_budik = i + 42;
                    break;
                  }
              }
            if (zbyva_do_budiku == 30000)                         // Kdyz do konce tydne nebyl nalezeny budik ...
              {
                for (i = 1 ; i < (LOC_dvt + 1) ; i++)             // .. zacne se prohledavat jeste usek od pondeli do dnesniho dne
                  {
                    if (bitRead(budik_maska, 8 - i) == true)      // Kdyz je pro den 'i' nastaveny budik ...
                      {
                        budik_week_minuty = (i-1) * 1440 + budik_on;  //  ...vypocte se rozdil casu budiku a aktualniho casu
                        zbyva_do_budiku = 10080 - aktualni_week_minuty + budik_week_minuty;       // (s prenosem pres konec tydne)
                        dvt_pro_budik = i + 42;
                        break;
                      }
                  }
              }
          }
        else                                                  // kdyz je aktualni den nedele ...
          {
            for (i = 1 ; i < 8 ; i++)                         // .. zacne se rovnou prohledavat usek od pondeli do dnesniho dne (do nedele)
              {
                if (bitRead(budik_maska, 8 - i) == true)      // Kdyz je pro den 'i' nastaveny budik ...
                  {
                    budik_week_minuty = (i-1) * 1440 + budik_on;  //  ...vypocte se rozdil casu budiku a aktualniho casu
                    zbyva_do_budiku = 10080 - aktualni_week_minuty + budik_week_minuty;       // (s prenosem pres konec tydne)
                    dvt_pro_budik = i + 42;
                    break;
                  }
              }
          }
      }


    
    if ( screen == 0)           // podprogram se vykonava jen pri zobrazene hlavni obrazovce
      {
        boolean zobraz_budik = false;

        if (zbyva_do_budiku < 1440)           // kdyz do pristiho budiku zbyva mene nez 24 hodin
          {
            zobraz_budik = true;             // muze se zobrazit ikona budiku
          }
        else
          {
            zobraz_budik = false;            // kdyz do pristiho budiku zbyva vice nez 24 hodin, ikona budiku se nezobrazi
          }



        if (zobraz_budik == true)
          {
            if (pcf.read(5) == LOW)
              {
                Serial2.print("ik9.pic=400");      // skrtnuty zvonek
              }
            else
              {
                Serial2.print("ik9.pic=270");
              }
          }
        else
          {
            Serial2.print("ik9.pic=80");
          }
        SerialFFF();


      }
                                // pomoci pinu P5 je monzne vypnout zvuk hardwerove
    if (pcf.read(5) == LOW)     // kdyz je zvuk vypnuty, ikona se skrtne
      {
        if (screen == 25)      // na strance s detailem budiku se skrta velka ikona budiku tlustym krizem
          {
            for (i = 0; i< 7 ; i++)
              {
                Serial2.print("line 369,");
                Serial2.print(95+i);
                Serial2.print(",470,");
                Serial2.print(5+i);
                Serial2.print(",RED");
                SerialFFF();

                Serial2.print("line 369,");
                Serial2.print(5+i);
                Serial2.print(",470,");
                Serial2.print(95+i);
                Serial2.print(",RED");
                SerialFFF();
              }
          }
      }
    
  }

void edit_budik(void)
  {
    Serial2.print("bh1.val=");         // hodiny pro rozsviceni displeje
    Serial2.print(budik_on / 60);
    SerialFFF();

    Serial2.print("bm1.val=");         // minuty pro rozsviceni displeje
    Serial2.print(budik_on % 60);
    SerialFFF();

    Serial2.print("bh0.val=");         // hodiny pro zhasnuti displeje
    Serial2.print(budik_off / 60);
    SerialFFF();

    Serial2.print("bm0.val=");         // minuty pro zhasnuti displeje
    Serial2.print(budik_off % 60);
    SerialFFF();



    for (i = 0; i< 7 ; i ++)          // zaskrtnuti 7 policek pro pondeli az nedeli  ("c0.val=1")
      {
        Serial2.print('c'); 
        Serial2.print(i);
        Serial2.print(".val=");

        if (bitRead(budik_maska, 7-i) == true)
          {
            Serial2.print(1);        
          }
        else
          {
            Serial2.print(0);        
          }      
        SerialFFF();

      }

    


  }
        

// Zmeri teplotu na prvnim cidle DS18B20
//  Kdyz je vsechno v poradku, je vystupem upravena teplota vcetne korekce pro zapis do klouzaku (500 = 0'C)
//  Pri chybe vraci -50'C (cislo 0) a zaroven nastavuje promennou 'chyba_cidla1' na nejakou nenulovou hodnotu (maximalne 100)
unsigned int ds18b20_1(void)
  {
    unsigned int teplota1;                           // navratova promenna
    float teplota_ds1 = DS18B20.getTempCByIndex(0);  // zjisteni teploty z prvniho cidla teploty se provadi vzdycky
    if (teplota_ds1 > 100 or teplota_ds1 <= -50)     // kdyz je teplota mimo realitu, je to chyba cidla a nic se nezaznamenava ani se nepocitaji zadne prumery
      {
        teplota1=1500;                                  // pri chybe cidla vraci +100'C
        chyba_cidla1 ++;
        if (chyba_cidla1 > 100) chyba_cidla1 = 100;
      }
    else
      {
        chyba_cidla1 = 0;
        teplota1 = 500 + (10 * teplota_ds1) - 100 + RAM1_read(EERAM_ADDR_korekce_OUT);         // korekce zobrazovane teploty
      }
    return teplota1;
  }


// Zmeri teplotu na druhem cidle DS18B20
//  Kdyz je vsechno v poradku, je vystupem upravena teplota vcetne korekce pro zapis do klouzaku (500 = 0'C)
//  Pri chybe vraci -50'C (cislo 0) a zaroven nastavuje promennou 'chyba_cidla2' na nejakou nenulovou hodnotu (maximalne 100)
unsigned int ds18b20_2(void)
  {
    unsigned int teplota2;                           // navratova promenna
    float teplota_ds2 = DS18B20.getTempCByIndex(1);  // zjisteni teploty z druheho cidla teploty
    if (teplota_ds2 > 100 or teplota_ds2 <= -50)     // kdyz je teplota mimo realitu, je to chyba cidla a nic se nezaznamenava ani se nepocitaji zadne prumery
      {
        teplota2=1500;                               // pri chybe cidla vraci +100'C
        chyba_cidla2 ++;
        if (chyba_cidla2 > 100) chyba_cidla2 = 100;
      }
    else
      {
        chyba_cidla2 = 0;                             // cidlo je v poradku (vraci nejakou realnou hodnotu)
        teplota2 = 500 + (10 * teplota_ds2) - 100 + RAM1_read(EERAM_ADDR_korekce_OUT2);         // korekce zobrazovane teploty
      }
    return teplota2;
  }






void nastav_servis(boolean konec)
  {
    byte odpoved_dis;
    unsigned int geo_pomprom;

    flush_read_buffer2();

    Serial2.print("get koro.val");  // zjisteni korekce prvniho venkovniho cidla
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        odpoved_dis = Serial2.read();
        RAM1_write(EERAM_ADDR_korekce_OUT, odpoved_dis);
        log_pomprom = odpoved_dis;
        loguj(4);     // logovani zmeny korekce venkovni teploty        
      }                
    flush_read_buffer2();

    Serial2.print("get koro2.val");  // zjisteni korekce druheho venkovniho cidla
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        odpoved_dis = Serial2.read();
        RAM1_write(EERAM_ADDR_korekce_OUT2, odpoved_dis);
        log_pomprom = odpoved_dis;
        loguj(15);     // logovani zmeny korekce venkovni teploty        
      }                
    flush_read_buffer2();
    

    Serial2.print("get kori.val");  // zjisteni korekce vnitrniho teplomeru
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        odpoved_dis = Serial2.read();
        RAM1_write(EERAM_ADDR_korekce_IN, odpoved_dis);
        log_pomprom = odpoved_dis;
        loguj(3);     // logovani zmeny korekce vnitrni teploty        
      }                
    flush_read_buffer2();

    
    Serial2.print("get korv.val");  // zjisteni korekce vlhkomeru
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        odpoved_dis = Serial2.read();
        RAM1_write(EERAM_ADDR_korekce_vlhkost, odpoved_dis);
        log_pomprom = odpoved_dis;
        loguj(6);     // logovani zmeny korekce vlhkomeru       
      }                
    flush_read_buffer2();

    Serial2.print("get kort.val");  // zjisteni korekce tlaku
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        odpoved_dis = Serial2.read();
        RAM1_write(EERAM_ADDR_korekce_tlak, odpoved_dis);
        log_pomprom = odpoved_dis;
        loguj(5);     // logovani zmeny korekce vlhkomeru       
      }                
    flush_read_buffer2();



    Serial2.print("get lon100.val");  // zjisteni zemepisne delky
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        odpoved_dis = Serial2.read();
        geo_pomprom = odpoved_dis + (256 * Serial2.read());
        RAM1_write_int(EERAM_ADDR_GeoLon, geo_pomprom);
      }                
    flush_read_buffer2();


    Serial2.print("get lat100.val");  // zjisteni zemepisne sirky
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        odpoved_dis = Serial2.read();
        geo_pomprom = odpoved_dis + (256 * Serial2.read());
        RAM1_write_int(EERAM_ADDR_GeoLat, geo_pomprom);
      }                
    flush_read_buffer2();

    loguj(16);

    Serial2.print("get alt.val");  // zjisteni nadmorske vysky
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        odpoved_dis = Serial2.read();
        geo_pomprom = odpoved_dis + (256 * Serial2.read());
        RAM1_write_int(EERAM_ADDR_GeoAlt, geo_pomprom);
      }                
    flush_read_buffer2();



    Serial2.print("get typtep.val");  // zjisteni typu teplomeru
    SerialFFF();
    delay(20);
    odpoved_dis = Serial2.read();
    if (odpoved_dis == 0x71)
      {
        odpoved_dis = Serial2.read();
        RAM1_write(EERAM_ADDR_typ_teplomeru, odpoved_dis);
      }                
    flush_read_buffer2();


    if (konec == true)  // kdyz je nastavena znacka 'konec', dojde po ulozeni k resetu
      {
        Serial2.print("page 0");  // na displeji zobrazit bootovací obrazovku
        SerialFFF();
        delay(100);
    
        iwdg_init(IWDG_PRE_256, 10);                     // nastaveni WD na 50 milisekund  (10*5)
        iwdg_feed();                                     // a okamzite obcerstveni WD
        delay(1000);                                     // behem pauzy dojde k aktivaci WD a resetu procesoru 
      }
    // kdyz znacka 'konec' nastavena neni, znamena to jen vstup do nastaveni infrakodu, takze se parametry sice ulozi, ale neresetuje se    
  }



// vstup na servisni obrazovku
void ukaz_servis(void)
  {
    Serial2.print("page 23");
    SerialFFF();
    delay(100);

    iwdg_feed();                                     // obcerstveni WD

    
    Serial2.print("koro.val=");
    Serial2.print(RAM1_read(EERAM_ADDR_korekce_OUT));                
    SerialFFF();
    
    Serial2.print("ko.val=");
    Serial2.print(RAM1_read(EERAM_ADDR_korekce_OUT)-100);                
    SerialFFF();
    
    Serial2.print("koro2.val=");
    Serial2.print(RAM1_read(EERAM_ADDR_korekce_OUT2));                
    SerialFFF();
    
    Serial2.print("ko2.val=");
    Serial2.print(RAM1_read(EERAM_ADDR_korekce_OUT2)-100);                
    SerialFFF();
    
    Serial2.print("kori.val=");
    Serial2.print(RAM1_read(EERAM_ADDR_korekce_IN));                
    SerialFFF();
    
    Serial2.print("ki.val=");
    Serial2.print(RAM1_read(EERAM_ADDR_korekce_IN)-100);                
    SerialFFF();
    
    Serial2.print("korv.val=");
    Serial2.print(RAM1_read(EERAM_ADDR_korekce_vlhkost));                
    SerialFFF();
    
    Serial2.print("kv.val=");
    Serial2.print(RAM1_read(EERAM_ADDR_korekce_vlhkost)-100);                
    SerialFFF();
    
    Serial2.print("kort.val=");
    Serial2.print(RAM1_read(EERAM_ADDR_korekce_tlak));                
    SerialFFF();
    
    Serial2.print("kt.val=");
    Serial2.print((RAM1_read(EERAM_ADDR_korekce_tlak)-100)*10);                
    SerialFFF();
    
    Serial2.print("glat.val=");
    Serial2.print(RAM1_read_int(EERAM_ADDR_GeoLat));                
    SerialFFF();
    
    Serial2.print("lat100.val=");
    Serial2.print(RAM1_read_int(EERAM_ADDR_GeoLat));                
    SerialFFF();
    
    Serial2.print("glon.val=");
    Serial2.print(RAM1_read_int(EERAM_ADDR_GeoLon));                
    SerialFFF();

    Serial2.print("lon100.val=");
    Serial2.print(RAM1_read_int(EERAM_ADDR_GeoLon));                
    SerialFFF();

    
    Serial2.print("alt.val=");
    Serial2.print(RAM1_read_int(EERAM_ADDR_GeoAlt));                
    SerialFFF();
    
    Serial2.print("galt.val=");
    Serial2.print(RAM1_read_int(EERAM_ADDR_GeoAlt));
    SerialFFF();
    
    Serial2.print("t9.txt=\"RS485: A");
    Serial2.print(RAM1_read(EERAM_ADDR_adr_mod));                
    Serial2.print('\"');
    SerialFFF();


    
    Serial2.print("typtep.val=");
    Serial2.print(RAM1_read(EERAM_ADDR_typ_teplomeru));                
    SerialFFF();
    
    Serial2.print("r0.val=0");
    SerialFFF();
    Serial2.print("r1.val=0");
    SerialFFF();
    Serial2.print("r2.val=0");
    SerialFFF();
    
    
    Serial2.print('r');
    Serial2.print(RAM1_read(EERAM_ADDR_typ_teplomeru) -1);
    Serial2.print(".val=1");
    SerialFFF();                    


    // ze servisni obrazovky uz neni uniku mimo resetu
    float data_cidlo ;
    byte smycka = 0;
    byte koro, koro2, kori, korv, kort, typtep;
    byte odpoved_dis;
    int kora;
    boolean mer_tlak;                              // kdyz je nastavena nejaka nadmorska vyska, ale prvni teplotni cidlo je vadne, nezobrazuje se tlak
    oneWire_teplomer.reset();
    delay(300);
    DS18B20.setResolution(11);                     // nastaveni prumerovani hodnot teplomeru DS18B20 na 11-bitove rozliseni
    delay(1000);
    DS18B20.requestTemperatures();

    
    rs485_get_temp(mod_addr);


 // nekonecna smycka zjistovani hodnot z cidel, prepocet pres korekce a jejich zobrazeni
    while (true)       
      {

        iwdg_feed();                                     // obcerstveni WD

        flush_read_buffer2();
    
        Serial2.print("get koro.val");  // zjisteni korekce prvniho venkovniho cidla
        SerialFFF();
        delay(20);
        odpoved_dis = Serial2.read();
        if (odpoved_dis == 0x71)     koro = Serial2.read();
        flush_read_buffer2();
    
        Serial2.print("get koro2.val");  // zjisteni korekce druheho venkovniho cidla
        SerialFFF();
        delay(20);
        odpoved_dis = Serial2.read();
        if (odpoved_dis == 0x71)     koro2 = Serial2.read();
        flush_read_buffer2();
        
    
        Serial2.print("get kori.val");  // zjisteni korekce vnitrniho teplomeru
        SerialFFF();
        delay(20);
        odpoved_dis = Serial2.read();
        if (odpoved_dis == 0x71)     kori = Serial2.read();
        flush_read_buffer2();
    
        
        Serial2.print("get korv.val");  // zjisteni korekce vlhkomeru
        SerialFFF();
        delay(20);
        odpoved_dis = Serial2.read();
        if (odpoved_dis == 0x71)     korv = Serial2.read();
        flush_read_buffer2();
    
        Serial2.print("get kort.val");  // zjisteni korekce tlaku
        SerialFFF();
        delay(20);
        odpoved_dis = Serial2.read();
        if (odpoved_dis == 0x71)
          {
            kort = Serial2.read();
            RAM1_write(EERAM_ADDR_korekce_tlak,kort);
          }
        flush_read_buffer2();

        Serial2.print("get alt.val");  // zjisteni nadmorske vysky (kvuli aktualnimu prepoctu tlaku)
        SerialFFF();
        delay(20);
        odpoved_dis = Serial2.read();
        if (odpoved_dis == 0x71)
          {
            kora = Serial2.read() + (Serial2.read() * 256) ;
            RAM1_write_int(EERAM_ADDR_GeoAlt,kora);
          }
        flush_read_buffer2();



        Serial2.print("get typtep.val");  // zjisteni typu teplomeru
        SerialFFF();
        delay(20);
        odpoved_dis = Serial2.read();
        if (odpoved_dis == 0x71)     typtep = Serial2.read();
        flush_read_buffer2();


        iwdg_feed();                                     // obcerstveni WD




        smycka ++;
        if (smycka > 5) smycka = 0;




        
        if (smycka == 0)
          {

            Serial2.print("t9.txt=\"RS485: A");
            Serial2.print(RAM1_read(EERAM_ADDR_adr_mod));                
            Serial2.print('\"');
            SerialFFF();


            
            Serial2.print("ao.txt=\"");
            if (typtep < 3)
              {
                data_cidlo = DS18B20.getTempCByIndex(0);  // zjisteni teploty z prvniho cidla teploty
              }
            else           // pri komunikaci RS485
              {
                data_cidlo = (teplota_1 - 500) / 10.0 ;
              }
          
            if (data_cidlo >= 100 or data_cidlo <= -50)     // kdyz je teplota mimo realitu, zobrazi se chyba
              {
                Serial2.print("---,-");
                mer_tlak = false;                           // kdyz je zaroven nastavena nejaka nadmorska vyska, nebude se mereit tlak
              }
            else
              {
                Serial2.print(data_cidlo + ((koro - 100.0)/10.0),1);
                suma_OUT = ((data_cidlo * 10) + 500) * 10;
                mer_tlak = true;

              }
            Serial2.print("\""); 
            SerialFFF();
          }  

          
        if (smycka == 1)
          {
            Serial2.print("ao2.txt=\"");
            if (typtep < 3)
              {
                data_cidlo = DS18B20.getTempCByIndex(1);  // zjisteni teploty z druheho cidla teploty
              }
            else           // pri komunikaci RS485
              {
                data_cidlo = (teplota_2 - 500) / 10.0 ;
              }
            
            if (data_cidlo >= 100 or data_cidlo <= -50)     // kdyz je teplota mimo realitu, zobrazi se chyba
              {
                Serial2.print("---,-"); 
              }
            else
              {
                Serial2.print(data_cidlo + ((koro2 - 100.0)/10.0),1);
              }
            Serial2.print("\""); 
            SerialFFF();
          }

        if (smycka == 2)
          {
            if (typtep < 3)
              {
                DS18B20.requestTemperatures();   // kazdou 2. smycku se spusti mereni teploty pak nasleduji smycky 3, 4 a 5 pri kterych se obsluhuji dalsi cidla,
                                                 // takze je dostatek casu pro DS18B20, aby teplotu zmerily
              }
            else                                 // v pripade RS485 se stahnou data ze sbernice
              {
                rs485_get_temp(mod_addr);   
              }
              
            delay(100);
          }


  
        if (smycka == 3)
          {
            Serial2.print("ai.txt=\"");
            data_cidlo = vlhkomerDHT.readTemperature();  // zjisteni teploty z vnitrniho cidla teploty
            if (data_cidlo > 100 or data_cidlo <= -50)     // kdyz je teplota mimo realitu, zobrazi se chyba
              {
                Serial2.print("---,-"); 
              }
            else
              {
                Serial2.print(data_cidlo + ((kori - 100.0)/10.0),1);
              }
            Serial2.print("\""); 
            SerialFFF();
          }
          
          
        if (smycka == 4)
          {
            Serial2.print("av.txt=\"");
            data_cidlo = vlhkomerDHT.readHumidity();  // zjisteni vlhkosti
            if (data_cidlo > 200 or data_cidlo <= -50)     // kdyz je teplota mimo realitu, je to chyba cidla a nic se nezaznamenava ani se nepocitaji zadne prumery
              {
                Serial2.print("---"); 
              }
            else
              {
                Serial2.print((int)data_cidlo + korv - 100);
              }
            Serial2.print("\""); 
            SerialFFF();
          }


        if (smycka == 5)
          {
            Serial2.print("at.txt=\"");
            if ( mer_tlak == false and kora > 0)        // kdyz nefunguje 1. teplomer a zaroven je nastavena nejaka nadmorska vyska
              {
                Serial2.print("T:Err\"");
              }
            else
              {
                data_cidlo = prepocet_tlaku();             // zjisteni tlaku a jeho pripadny prepocet z absolutni hodnoty na hladinu more
                Serial2.print((int)data_cidlo);
                Serial2.print("\""); 
              }
            SerialFFF();
          }
         

        
         




        Serial2.print("get klik.val");  // zjisteni jestli bylo kliknuto na tlacitko "Nastavit" (v displeji se nahazuje promenna 'klik' na 1)
        SerialFFF();
        delay(20);
        odpoved_dis = Serial2.read();
        if (odpoved_dis == 0x71)
          {
            odpoved_dis = Serial2.read();
            if (odpoved_dis == 1)             nastav_servis(true);         // servis konci aktivaci watchdogu a resetem procesoru            
            if (odpoved_dis == 2)
              {
                nastav_servis(false);        // servis konci ulozenim promennych, ale na konci ukladani nedojde k resetu
                nastav_infra();              // uceni infrakodu konci aktivaci watchdogu a resetem procesoru            
              }
          }
        flush_read_buffer2();
        
        iwdg_feed();                                     // obcerstveni WD
        

       delay(300);

      }

  }


void zobraz_index_cidla(void)
  {
    if (screen == 0)
      {
        Serial2.print("cd2.pic=");
        Serial2.print(akt_cidlo + 271);
        SerialFFF();
      }
  }

//-----------------------------------------------------
// zobrazeni stranky s pristim budikem
void pristi_budik(void)
  {
    Serial2.print("page 25");
    SerialFFF();
    screen = 25;

    budik_iko();             // vypocty pristiho budiku

    Serial2.print("t1.txt=\"");
    if (zbyva_do_budiku < 30000)
      {
        Serial2.print(int2_to_hodmin(zbyva_do_budiku));                
      }
    else
      {
        Serial2.print("---"); 
      }
    Serial2.print('\"');
    SerialFFF();


    Serial2.print("t0.txt=\"");
    if (zbyva_do_budiku < 30000)
      {
        Serial2.print(int_to_hodmin(budik_on));                
      }
    else
      {
        Serial2.print("---"); 
      }
    Serial2.print('\"');
    SerialFFF();


    Serial2.print("p2.pic=");
    if (zbyva_do_budiku < 30000)
      {
        Serial2.print(dvt_pro_budik);                 // Znacka "Po" az "Ne" na indexech 43 az 49
      }
    else
      {
        Serial2.print(50);                  // prazdny obrazek misto znacky "Po", "Ut" , .... "Ne"
      }
    SerialFFF();

  }




// ukoncovaci trojznak #FF#FF#FF
void SerialFFF(void)
  {
    Serial2.write(255);
    Serial2.write(255);
    Serial2.write(255);

  }


void rtcINT(void)
  {
    aktualizuj_cas = true;
  }




// podprogram pro prepocet absolutniho tlaku z cidla na relativni tlak na hladine more
// do programu vstupuje posledni zprumerovana hodnota venkovni teploty, zadana nadmorska vyska a rucne nastavena korekce tlaku (-10kPa az +15,5kPa)
// kdyz je nadmorska vyska nastavena na 0m, zadne prepocty se neprovadeji, jen se absolutni tlak upravi podle korekce
unsigned long prepocet_tlaku(void)
  {
    unsigned long absolutni_tlak = bmp.readPressure();
    int           korekce_tlaku = (RAM1_read(EERAM_ADDR_korekce_tlak) - 100) * 10;
    unsigned int  nadmorska_vyska = RAM1_read_int(EERAM_ADDR_GeoAlt);
    
    if (nadmorska_vyska == 0)                      // nadmorska vyska je nastavena na 0, takze se vraci absolutni tlak +/- korekce
      {
        return (absolutni_tlak + korekce_tlaku);
      }
    else                                           // pri zadane nadmorske vysce se tlak prepocte
      {
        return (absolutni_tlak * 9.80665 * nadmorska_vyska) / (287 * (273 + ((suma_OUT - 5000.0) / 100.0) + (nadmorska_vyska / 400))) + absolutni_tlak + korekce_tlaku;
      }
  }


