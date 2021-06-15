#include <Arduino.h>

#include <ESP_EEPROM.h>
uint32_t resetRegister, bloccoMotoreRegister, eventRegister, overflowImpulsiRegister;
#define resetRegisterAddress 0
#define eventRegisterAddress 8
#define bloccoMotoreRegisterAddress 16
#define overflowImpulsiAddress 24
//Valori di inizializzazione dopo aggiornamento software
#define initResetRegister 0
#define initBloccoMotoreRegister 0
#define initEventRegister 0
#define initOverflowRegister 0

//=====================================================================
//SaLe per 
//FabFactory
//
//   BOX ROSA
//
//=====================================================================
/*
Copyright <2021> <SaLe>

Permission is hereby granted, free of charge, to any person obtaining 
a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software 
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


void Spy();
void bloccoMotore(uint8_t suono);
void avvia();
void StatusSelect(uint8_t status);

#define DebugStatus 0
#define DebugSpeed 115200
#define DebugPort Serial

#define CicliDiAvviso 12
#define CicloDiPausa 3000
#define SleepTime 15000

#define AnalogPin A0
#define AnalogResolution 1023
#define AnalogFS 3.3
#define AnalogKConvertion (AnalogFS / AnalogResolution)
#define SpyLevelCome 40
#define SpyLevelGo 60
#define SpyOffset 27

#define BuzzerAllarm D2
#define MotorPin D5
#define CammaSwitch D1
#define LedR D6
#define LedG D7
#define LedB D8
uint8_t ledPin[3] = {LedR, LedG, LedB};

int16_t analogCount = 0;
float analogValue = 0;
//bit 0  Set misura analogica fuori range
//bit 1  
//bit 2  set ospite vicino al sensore
//bit 3
//bit 4
//bit 5
//bit 6 posizione camma set camma in movimento
//bit 7 stato motore set motore in moto
uint8_t spyFlag = 0;
//Temporizzatore per attendere fra un evento e il prossimo
uint32_t spyTimeCome = 0;
//tempoo di attesa fra due eventi
#define spyEventPause 10000
#define MotorTimeOut 7000
#define MotorStartTime 3500
#define MotorImpulse 50

//                         3.0,  2.9,  2.8,  2.7,  2.6,  2.5,  2.4,  2.3,  2.2,  2.1,
//                         2.0,  1.9,  1.8,  1.7,  1.6,  1.5,  1.4,  1.3,  1.2,  1.1,
//                         1.0,  0.9,  0.8,  0.7,  0.6,  0.5,  0.4,  0.3
float distanceConv[28] = {   7,  7.5,  7.7,    8,  8.5,    9,  9.5,   10,   11, 11.5,
                            12, 12.5,   13,   14,   15,   17,   18,   20,   21,   25,
                            27,   30,   37,   40,   50,   60,   70,   80
                         };
float distance = 0;

uint16_t eventiCorrenti = 0;

uint8_t impulso = 0;

#include <WebManagement.cpp>

//==============================================================
//                  SETUP
//==============================================================
void setup() 
{
  EEPROM.begin(32);//Inizializza lo spazio per l'area dati
  //Carica i valori degli accumulatori
  EEPROM.get(resetRegisterAddress,resetRegister);
  if (resetRegister < initResetRegister)
  {
    resetRegister = initResetRegister;
  }
  EEPROM.get(eventRegisterAddress,eventRegister);
  if (eventRegister < initEventRegister)
  {
    eventRegister = initEventRegister;
  }
  EEPROM.get(bloccoMotoreRegisterAddress,bloccoMotoreRegister);
  if (bloccoMotoreRegister < initBloccoMotoreRegister)
  {
    bloccoMotoreRegister = initBloccoMotoreRegister;
  }
  EEPROM.get(overflowImpulsiAddress, overflowImpulsiRegister);
  if (overflowImpulsiRegister < initOverflowRegister)
  {
    overflowImpulsiRegister = initOverflowRegister;
  }
  resetRegister++;//Aggiorna il contatore dei reset
  EEPROM.put(resetRegisterAddress,resetRegister);//predispone l'aggiornamento dell'accumulatore
  EEPROM.commit();//Aggiorna l'accumulatore dei reset

  DebugPort.begin(DebugSpeed);
  delay(500);
  if(DebugStatus==1)
  {  
    DebugPort.println("00 - Porta seriale inizializzata");
    DebugPort.print("----------------Reset compreso il presente ");
    DebugPort.println(resetRegister);
    DebugPort.print("----------------Intervento del blocco motore ");
    DebugPort.println(bloccoMotoreRegister);
    DebugPort.print("----------------Eventi ");
    DebugPort.println(eventRegister);
  }

  pinMode(BuzzerAllarm, OUTPUT);
  digitalWrite(BuzzerAllarm, LOW);

  pinMode(MotorPin, OUTPUT);
  digitalWrite(MotorPin, LOW);

  pinMode(LedR, OUTPUT);
  digitalWrite(LedR, LOW);
  pinMode(LedG, OUTPUT);
  digitalWrite(LedG, LOW);
  pinMode(LedB, OUTPUT);
  digitalWrite(LedB, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(CammaSwitch, INPUT_PULLUP);

  if(DebugStatus==1)
  {  
    DebugPort.println("01 - Inizializzo canale Web");
  }
  //Attiva il server Web
  webSetup();

  if(DebugStatus==1)
  {  
    DebugPort.println("07 - Inizializzo pagina Web");
  }
  //Fa il primo lancio della pagina Web
  for (uint8_t n=0; n<100; n++)
  {
   StatusSelect(0);
    webLoop();
  }

  //Esegue i controlli e il ciclo di avviamento
  avvia();
}

void loop() 
{
  Spy();
  analogWrite(LedR,random(0,1023));
  analogWrite(LedG,random(0,1023));
  analogWrite(LedB,random(0,1023));
  if(DebugStatus==1)
  {  
    DebugPort.print(analogValue);
    DebugPort.print(" - ");
    DebugPort.println(distance);
  }

  digitalWrite(LED_BUILTIN, bitRead(spyFlag,2));

  //Aggiorna la pagina Web
  StatusSelect(0);
  webLoop();

  delay(300);
}


void Spy()
{
  //legge la posizione della camma
  bitWrite(spyFlag,6,digitalRead(CammaSwitch));
  if(DebugStatus==1)
  {  
    DebugPort.print("20 - camma ");
    DebugPort.print(digitalRead(CammaSwitch));
    DebugPort.print(" - ");
  }
  if(digitalRead(CammaSwitch))
  {
    StatusSelect(20);
  }
  else
  {
    StatusSelect(21);
  }
  webLoop();
  //se la camma è in movimento controlla il time out del motore
  if (bitRead(spyFlag,6)==1)
  {
    //se ha superato il time out del motore lo spegne e non si muove più
    if ((millis() - spyTimeCome) > MotorTimeOut)
    {
      if(DebugStatus==1)
      {  
        DebugPort.println("90 - Motore in blocco per Time Out Giro");
      }
      StatusSelect(90);
      bloccoMotore(1);
    }
  }
  //Se la camma non è in movimento
  else
  {
    StatusSelect(40);
    //Legge l'ADC
    analogCount = analogRead(AnalogPin);
    //Lo converte in volt
    analogValue = float(analogCount) * AnalogKConvertion;
    //controlla se è fuori dal range di funzionamento del sensore
    if ((analogValue<=3.0) & (analogValue>=0.3))
    {
      bitClear(spyFlag, 0);//Dice che è nel range
      distance = distanceConv[SpyOffset - int((analogValue * 10) - 3) ]; 
////      var2 = distance;
    }
    else
    {
      bitSet(spyFlag, 0);//Dice che è fuori range
    }
    webLoop();
    //controlla se si è avvicinato qualcuno
    //se non è fuori range
    //se non ha già visto qualcuno vicino
    if ((distance <= SpyLevelCome) & ((spyFlag & B00000101) == 0))
    {
      if(DebugStatus==1)
      {
        DebugPort.println("30 - Pollo in vista");
      }
      StatusSelect(30);
      webLoop();
      eventiCorrenti++;//Aggiorna contatore eventi nella sessione corrente
      eventRegister++;//Aggiorna l'accumulatore degli eventi
      EEPROM.put(eventRegisterAddress, eventRegister);
      EEPROM.commit();//e lo registra
      bitSet(spyFlag,2);//Dice che bisogna colpire
      bitSet(spyFlag,7);//segna il motore come attivo
      digitalWrite(MotorPin,HIGH);//attiva il motore
      digitalWrite(LED_BUILTIN,LOW);//accende il LED locale
      digitalWrite(LedR,HIGH);//accende il LED ROSSO

      delay(500);//Dopo l'allungamento del braccetto sensore della camma garantiamo che alla partenza la camma sia letta

      spyTimeCome = millis();//e segna quando ha rilevato il curioso
      //Aspetta per il tempo MotorStartTime per vedere se libera la camma
      //altrimenti blocca tutto
      while (digitalRead(CammaSwitch) == 0)
      {
        delay(1);
        //se ha superato il tempo di avviamento e non ha mosso la camma
        //blocca il motore
        if((millis()-spyTimeCome) > MotorStartTime)
        {
          if(DebugStatus==1)
          {  
            DebugPort.println("91 - Blocco motore in avviamento");
          }
          StatusSelect(91);
          bloccoMotore(2);
        }
      }
      if(DebugStatus==1)
      {
        DebugPort.print("93 - Camma fuori posizione");
      }
      StatusSelect(93);
      webLoop();
      //Il motore è avviato e la camma si è mossa
      //Fino a quando la camma è aperta e il motore è in movimento 
      //continua a girare
      uint8_t appo = digitalRead(CammaSwitch);
      for (uint8_t iiii=0; appo == 1; iiii++)
      {
        //aggiorna lo stato della camma
        delay(1);
        appo = digitalRead(CammaSwitch);
        //se ha superato il time out del motore lo spegne e non si muove più
        if ((millis() - spyTimeCome) > MotorTimeOut)
        {
          if(DebugStatus==1)
          {  
            DebugPort.print("92 - Motore in blocco per Time Out Giro - ");
          }
          StatusSelect(92);
          bloccoMotore(1);
        }
      }        
      //Spegne il motore
      //Spegne i LED di segnalazione
      digitalWrite(MotorPin,LOW);//disattiva il motore
      digitalWrite(LED_BUILTIN,HIGH);//Spegne il LED locale
      digitalWrite(LedR,LOW);//spegne il LED ROSSO
      if(DebugStatus==1)
      {
        DebugPort.print("Camma in posizione");
      }
    }
    //controlla se si è allontanota abbastanza
    else if ((distance >SpyLevelGo) & ((millis() - spyTimeCome) > SleepTime))
    {
      bitClear(spyFlag,2);//dice che è non è nel range
    }
  }
}





void bloccoMotore(uint8_t suono)
{
      digitalWrite(MotorPin, LOW);//Spegne il motore
      //suona all'infinito
      webLoop();
      bloccoMotoreRegister++;//Aggiorna l'accumulatore dei blocchi motore
      DebugPort.print(bloccoMotoreRegister);
      DebugPort.println(" <-----");
      EEPROM.put(bloccoMotoreRegisterAddress, bloccoMotoreRegister);
      EEPROM.commit();//E lo registra
      while (1==1)
      {
        //Suona e segnala sulla piastra e nella black box
        digitalWrite(BuzzerAllarm,LOW);
        digitalWrite(LED_BUILTIN,LOW);
        digitalWrite(LedR,HIGH);
        delay(1000*suono);
        digitalWrite(BuzzerAllarm, HIGH);
        digitalWrite(LED_BUILTIN,HIGH);
        digitalWrite(LedR,LOW);
        delay(500*suono);
      }
}


//Realizza il ciclo di avviamento sicuro
void avvia()
{
  if(DebugStatus==1)
  {
    DebugPort.println("08 - Inizia il ciclo di avviamento");
  }
  StatusSelect(8);
  webLoop();
  //Avvisa che il sistema è on
  if(DebugStatus==1)
  {  
    DebugPort.println("09 - Il sistema sta per andare ON");
  }
  StatusSelect(9);
  webLoop();
  for (uint8_t iii=0; iii < CicliDiAvviso; iii++)
  {
  //Suona e segnala sulla piastra e nella black box
    StatusSelect(99);
    webLoop();
    digitalWrite(BuzzerAllarm,LOW);
    digitalWrite(LED_BUILTIN,LOW);
    digitalWrite(LedR,HIGH);
    delay(200);
    StatusSelect(9);
    webLoop();
    digitalWrite(BuzzerAllarm, HIGH);
    digitalWrite(LED_BUILTIN,HIGH);
    digitalWrite(LedR,LOW);
    delay(1000);
    //Aggiorna la pagina Web
  }

  StatusSelect(99);
  webLoop();
  delay(CicloDiPausa);

  if(DebugStatus==1)
  {
    DebugPort.println("10 - Controlla la posizione della camma");
  }
  StatusSelect(10);
  webLoop();
  //controlla la posizione della camma
  //Se la camma è aperta avvia il ciclo di riposizionamento
  if(digitalRead(CammaSwitch)==1)
  {
    if(DebugStatus==1)
    {
      DebugPort.println("11 - Camma fuori posizione");
      DebugPort.println("12 - Inizia cilo di allineamento camma");
    }
    StatusSelect(12);
    webLoop();        
    //Avvisa con suono prolungato che sta per muoversi la camma
    digitalWrite(BuzzerAllarm,LOW);
    digitalWrite(LED_BUILTIN,LOW);
    digitalWrite(LedR,HIGH);
    delay(CicloDiPausa);
    //Pausa breve
    digitalWrite(BuzzerAllarm, HIGH);
    digitalWrite(LED_BUILTIN,HIGH);
    digitalWrite(LedR,LOW);
    delay(2000);

    //Aggiorna la pagina Web
    webLoop();

    if(DebugStatus==1)
    {
      DebugPort.println("13 - Sto per avviarmi ad impulsi");
    }
    StatusSelect(13);
    webLoop();        
 
    //Prova a portare in posizione la camma
    for(impulso = 0; digitalRead(CammaSwitch) == 1; impulso++)
    {
      //Impulso breve motore
      if(DebugStatus==1)
      {
        DebugPort.print("14 - Impulso motore ");
        DebugPort.print(impulso+1);
        DebugPort.print(" di ");
        DebugPort.println(MotorTimeOut/MotorImpulse);
      }
      StatusSelect(14);
      webLoop();        

      digitalWrite(MotorPin, HIGH);//Accende il motore
      digitalWrite(BuzzerAllarm,LOW);
      digitalWrite(LED_BUILTIN,LOW);
      digitalWrite(LedR,HIGH);
      delay(MotorImpulse);//alimenta per 4 cicili della tensione di alimentazione
      digitalWrite(MotorPin, LOW);//Spegne il motore

      StatusSelect(0);
      webLoop();        

      digitalWrite(BuzzerAllarm, HIGH);
      digitalWrite(LED_BUILTIN,HIGH);
      digitalWrite(LedR,LOW);
      delay(2000);//Attende per un tempo lungo il prossimo impulso
      //Se ha superto il nomero di tentivi per il riposizionamento della camma
      //Si blocca con suono continuo
      while (impulso > (MotorTimeOut/MotorImpulse))
      {
        digitalWrite(BuzzerAllarm,HIGH);
        digitalWrite(LED_BUILTIN,LOW);
        digitalWrite(LedR,HIGH);
        if((DebugStatus==1) & (impulso == (MotorTimeOut/MotorImpulse+1)))
        {
          DebugPort.print("98 - Blocco  motore per overflow impulsi");
          impulso++;
          overflowImpulsiRegister++;
          EEPROM.put(overflowImpulsiAddress, overflowImpulsiRegister);
          EEPROM.commit();
        }
        StatusSelect(98);
        webLoop();
        delay(1);
      }

      //Aggiorna la pagina Web
      webLoop();
    
    }    
  }
  delay(2000);
  //Camma in posizione
  //segnala la fine del ciclo di avviamento
  if(DebugStatus==1)
  {
    DebugPort.println("15 - Camma in posizione");
    DebugPort.println("16 - Sto per diventare attivo");
  }
  for (uint8_t iii=0; iii < CicliDiAvviso; iii++)
  {
    //Suona e segnala sulla piastra e nella black box
    digitalWrite(BuzzerAllarm,HIGH);
    digitalWrite(LED_BUILTIN,LOW);
    digitalWrite(LedG,HIGH);
    delay(200);
    digitalWrite(BuzzerAllarm, LOW);
    digitalWrite(LED_BUILTIN,HIGH);
    digitalWrite(LedG,LOW);
    delay(200);
    if(DebugStatus==1)
    {
      DebugPort.println(iii);
    }

    //Aggiorna la pagina Web
    webLoop();
  }
  delay(2000);
  if(DebugStatus==1)
  {
    DebugPort.println("17 - Macchina attiva");
  }

  //Aggiorna la pagina Web
  webLoop();
  //Macchina On
}