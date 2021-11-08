/*
   Dario Ciceri
   Codice per controllare qualsiasi cosa tramite Telegram e una scheda esp32
   Profilo Instragram: https://www.instagram.com/_dario.ciceri_/
   Pagina GitHub: https://github.com/Dario-Ciceri
   Canale YouTube: https://www.youtube.com/channel/UCuPuHsNjWX7huiztYu9ROQA
*/

#include <WiFi.h> //libreria per il wifi (meglio non modificare)
#include <WiFiClientSecure.h> //libreria per il wifi (meglio non modificare)
#include <UniversalTelegramBot.h> //libreria per telegram (meglio non modificare)
#include <esp8266-google-home-notifier.h> //libreria per google (meglio non modificare)

#define WIFI_SSID "nome_wifi" //nome wifi (potete modificarlo)
#define WIFI_PASSWORD "password_wifi" //password wifi (potete modificarlo)
#define BOT_TOKEN "token_chatbot" //token chatbot telegram (potete modificarlo)

//gestione pin analogici sulle schede esp32
#define LEDC_CHANNEL_0     0 //canale (meglio non modificare)
#define LEDC_TIMER_13_BIT  13 //risoluzione, il massimo non è 255 ma 8191 (meglio non modificare)
#define LEDC_BASE_FREQ     5000 //frequenza (meglio non modificare)
#define LED_PIN            21 //pin di uscita analogica (potete modificarlo)

const unsigned long BOT_MTBS = 1000; //pausa per la ricezione dei messaggi (potete modificarlo)
unsigned long bot_lasttime; //gestione del millis (meglio non modificare)

WiFiClientSecure secured_client; //wifi client (meglio non modificare)
UniversalTelegramBot bot(BOT_TOKEN, secured_client); //telegram client (meglio non modificare)

GoogleHomeNotifier google; //oggetto google (meglio non modificare)

void handleNewMessages(int numNewMessages) //funzione per la gestione della chat telegram
{
  Serial.println("Prelevo messaggi dalla chat"); //debug su seriale
  Serial.println(String(numNewMessages)); //debug su seriale

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id; //id univoco della chat
    String text = bot.messages[i].text; //testo inviato in chat dall'utente

    String from_name = bot.messages[i].from_name; //chi manda il messaggio
    if (from_name == "")
      from_name = "Sconosciuto"; //se non possiamo sapere chi ci scrive, allora è uno sconosciuto

    if (text == "/on") //se il testo ricevuto è /on allora accendi la barra LED
    {
      ledcWrite(LEDC_CHANNEL_0, 255); //accendo la barra LED
    }
    if (text == "/off") //se il testo ricevuto è /off allora spegni la barra LED
    {
      ledcWrite(LEDC_CHANNEL_0, 0); //spengo la barra LED
    }
    if (text == "/google") //se il testo ricevuto è /google allora fai parlare google
    {
      if (google.notify("Ciao Mamma! Sono su Youtube!") != true) { //dico a google cosa dire (riporta vero o falso in base al risultato dell'operazione), se il risultato è false allora mando l'errore su seriale
        Serial.println(google.getLastError()); //debug su seriale
        return;
      }
    }
    if (text == "/start") //se il testo ricevuto è /start allora invia il messaggio di benvenuto
    {
      String welcome = "Benvenuto " + from_name + ", questo codice è stato creato da Dario Ciceri.\n";
      welcome += "\n";
      welcome += "Lista dei comandi:\n";
      welcome += "/on\n";
      welcome += "/off\n";
      welcome += "/google\n";
      welcome += "\n";
      welcome += "Canale YouTube:\n";
      welcome += "https://www.youtube.com/channel/UCuPuHsNjWX7huiztYu9ROQA\n";
      welcome += "\n";
      welcome += "Profilo Instagram:\n";
      welcome += "https://www.instagram.com/_dario.ciceri_/\n";
      welcome += "\n";
      welcome += "Pagina GitHub:\n";
      welcome += "https://github.com/Dario-Ciceri";
      bot.sendMessage(chat_id, welcome);
    }
  }
}

void setup()
{
  Serial.begin(115200); //inizializzo la seriale su 115200 baudrate
  Serial.println(); //debug su seriale

  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT); //inizializzo pin analogico
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0); //timer su pin analogico

  Serial.print("Connetto alla rete wifi "); //debug su seriale
  Serial.print(WIFI_SSID); //debug su seriale
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //connetto al wifi
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); //creo una connessione con Telegram utilizzando un certificato di sicurezza
  while (WiFi.status() != WL_CONNECTED) //finchè il wifi non è connesso attendi e manda dei puntini di sospensione su seriale
  {
    Serial.print("."); //debug su seriale
    delay(500);
  }
  Serial.print("\nWifi connesso. Indirizzo IP: "); //debug su seriale
  Serial.println(WiFi.localIP()); //debug su seriale

  const char displayName[] = "Laboratorio"; //inserire il nome del proprio dispositivo google, potete trovare come lo avete chiamato dall'applicazione Google Home: https://play.google.com/store/apps/details?id=com.google.android.apps.chromecast.app&hl=it&gl=US

  Serial.println("connessione a Google Home..."); //debug su seriale
  if (google.device(displayName, "it") != true) { //se trovo il dispositivo google e la lingua è italiana allora mi connetto altrimenti mando l'errore su seriale
    Serial.println(google.getLastError()); //debug su seriale
    return;
  }
  Serial.print("dispositivo Google Home trovato("); //debug su seriale
  Serial.print(google.getIPAddress()); //debug su seriale
  Serial.print(":"); //debug su seriale
  Serial.print(google.getPort()); //debug su seriale
  Serial.println(")"); //debug su seriale
}

void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS) //se il valore del timer "millis" - l'ultimo valore in millisecondi di quando abbiamo eseguito le istruzioni, è maggiore del tempo di attesa "BOT_MTBS", allora esegui le istruzioni
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1); //aggiorna la lista dei messaggi in chat

    while (numNewMessages) //finchè ci sono nuovi messaggi, processali
    {
      Serial.println("messaggio ricevuto"); //debug su seriale
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis(); //aggiorna l'ultimo periodo in millisecondi in cui sono state eseguite le istruzioni all'interno del ciclo while
  }
}
