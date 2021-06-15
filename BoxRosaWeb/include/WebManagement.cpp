#include <Arduino.h>

#include <index.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WebSocketsServer.h>

WiFiServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
DNSServer dnsServer;

const char ssid[] = "PinkBoxBum";
const char pass[] = "EtHocTerrentVobis";
const int wifi_channel = 1;
const boolean wifi_hidden = false;
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 128);
const byte DNS_PORT = 53;
const char *dnsName = "Pink.Box.Bum"; // Domain name for the mDNS responder


String var1 = "";
String var2 = "";
String var3 = "";
long timeout = 2000;

void datiRandom()
{
  timeout = 10000;
}

void invioDati()
{
/*
  if (--timeout > 0)
  {
    return;
  }
*/
//  datiRandom();
  char vBuf1[50];
  char vBuf2[50];
  char vBuf3[50];
  var1.toCharArray(vBuf1,var1.length()+1);
  var2.toCharArray(vBuf2, var2.length()+1);
  var3.toCharArray(vBuf3, var3.length()+1);
  char buffer[150];
  sprintf(buffer, "%d|%d|%d|%d|%d|%d|%s|%s|%s"
                , eventiCorrenti, 
                eventRegister, 
                resetRegister,
                bloccoMotoreRegister,
                overflowImpulsiRegister,
                int(distance),
                vBuf1,
                vBuf2, 
                vBuf3);
  if(DebugStatus==1)
  {
    DebugPort.print(bloccoMotoreRegister);
    DebugPort.print(" - ");
    DebugPort.print(var1);
    DebugPort.print(var2);
    DebugPort.println(var3);
    DebugPort.println("-");
    DebugPort.println(buffer);
    DebugPort.println("-");
  }
  webSocket.broadcastTXT(buffer);//Invia i dati aggiornati alla pagina WEB
}

void startWiFi()
{ // Start a Wi-Fi access point
  WiFi.mode(WIFI_AP);
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid, pass, wifi_channel, wifi_hidden, 4) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  delay(100);

  Serial.println("\r\n");
}

void startDNS()
{
  // modify TTL associated  with the domain name (in seconds)
  // default is 60 seconds
  dnsServer.setTTL(300);
  // set which return code will be used for all other domains (e.g. sending
  // ServerFailure instead of NonExistentDomain will reduce number of queries
  // sent by clients)
  // default is DNSReplyCode::NonExistentDomain
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);

  // start DNS server for a specific domain name
  dnsServer.start(DNS_PORT, dnsName, local_IP);
}

void webSetup()
{
/*
  // Connect to a WiFi network
  if(DebugStatus==1)
  {  
    Serial.print(F("02 - Connecting to "));  
    Serial.println("ssid");
  }
  WiFi.begin(ssid,pass);

  // connection with timeout
  int count = 0;
  while ( (WiFi.status() != WL_CONNECTED) && count < 17)
  {
    if(DebugStatus==1)
    {  
      Serial.print(".");  delay(500);  count++;
    }
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    if(DebugStatus==1)
    {  
      Serial.println("");  
      Serial.print("99 - Failed to connect to ");  
      Serial.println(ssid);
    }
     while(1); //reboot!
  }

  if(DebugStatus==1)
  {  
    Serial.println("");
    Serial.println(F("03 - [CONNECTED]"));
    Serial.print("04 - [IP ");
    Serial.print(WiFi.localIP());
    Serial.println("]");
  }
*/
  startWiFi(); // Start a Wi-Fi access point
  startDNS();  // Start the mDNS responder

  // start a server
  server.begin();
  if(DebugStatus==1)
  {  
    Serial.println("05 - Server started");
  }

  webSocket.begin();
  if(DebugStatus==1)
  {  
    Serial.println("06 - WEB Socket started");
  }
}

void webLoop()
{
////  dnsServer.processNextRequest();
  invioDati();

  webSocket.loop();
  delay(1);
  
  WiFiClient client = server.available(); // Check if a client has connected
  if (!client)
  {
    return;
  }

  client.flush();
  client.print(header);
  client.print(html_1);
  if(DebugStatus==1)
  {  
    Serial.println("08 - New page served");
  }
////  delay(500);
}


//Trasforma il codice di stato in testo per la pagina Web
//Provvede anche a scrollare le tre righe di presentazione
void StatusSelect(uint8_t status)
{
  var1 = var2;//Scrolla di un verso l'alto
  var2 = var3;//le tre righe dello stato
  switch(status)
  {
    case  0:
      var3 = "0 - OFF";
      break;
    case  8:
      var3 = "8 - Inizia il ciclo di avviamento";
      break;
    case  9:
      var3 = "9 - Il sistema sta per andare ON";
      break;
    case 10:
      var3 = "10 - Controlla la posizione della camma";
      break;
    case 12:
      var3 = "12 - Inizia cilo di allineamento camma";
      break;
    case 13:
      var3 = "13 - Sto per avviarmi ad impulsi";
      break;
    case 14:
      var3 = "14 - Impulso Motore - " + String(impulso + 1);
      break;
    case 20:
      var3 = "20 - Camma in rotazione";
      break;
    case 21:
      var3 = "21 - Camma in posizione";
      break;
    case 30:
      var3 = "30 - Sogetto agganciato";
      break;
    case 40:
      var3 = "40 - Distanza";
      break;
    case 90:
      var3 = "90 - Motore in blocco per Time Out Giro";
      break;
    case 91:
      var3 = "91 - Motore in blocco in avviamento";
      break;
    case 92:
      var3 = "92 - Motore in blocco per Time Out Giro";
      break;
    case 93:
      var3 = "93 - Camma fuori posizione";
      break;
    case 98:
      var3 = "98 - Blocco  motore per overflow impulsi";
      break;
    case 99:
      var3 = "99 - Turn On";
      break;
    default:
      var3 = "out of Err Code";
      break;
  }
}
