#include <FirebaseArduino.h>
#include <dht11.h> // dht11 kütüphanesini ekliyoruz.
#include  <ESP8266WiFi.h>


/*** Adafruit Kütüphane Dosyaları ***/ 
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

/*** Firebase Bulut DB Bilgileri ***/
#define FIREBASE_HOST "iot-course-project-e647f.firebaseio.com"
#define FIREBASE_AUTH "LppUFDW04N9xcyw5EznVJstEtZp1a3gUhoPD54vF"

/*** Kablosuz Ağ Bilgileri ***/
#define WIFI_SSID "burakcalisgan" // Change the name of your WIFI
#define WIFI_PASSWORD "cpt96brk" // Change the password of your WIFI

/************************* Adafruit.io Kurulumu *********************************/
#define   AIO_SERVER      "io.adafruit.com"
#define   AIO_SERVERPORT  1883                     // MQTT Portu
#define   AIO_USERNAME    "burakcalisgan"              // Kullanıcı Adı
#define   AIO_KEY         "7ee9ccd2693e42bcbb8a95c9b2a5cbad"   // adafruit türetilen KEY

/************************* MQTT Sunucuya Bağlantı Ayarları *********************************/
WiFiClient client;     // ESP8266WiFiClient sınıfından bağlantı nesnesi
// adafruit MQTT bağlantı kurulumu 
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Publish ve Subscribe için Feed Ayarları
Adafruit_MQTT_Publish sicaklik = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature"); 
Adafruit_MQTT_Publish nem = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");


 
#define DHTPIN 14 // DHT11_pin olarak Dijital 2'yi belirliyoruz.
dht11 DHT11_sensor; // DHT11_sensor adında bir DHT11 nesnesi oluşturduk.

void WiFiSetup()
{
  //WiFi ye bağlanılıyor.
   WiFi.begin (WIFI_SSID, WIFI_PASSWORD);
   Serial.println("WiFi Connecting...");
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println ("");
  Serial.println ("WiFi Connected !");
}

void FirebaseSetup()
{
  //Firebase Authentication yapılıyor.
  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
  if (Firebase.success()) 
  {
    Serial.println("Firebase Bağlandı.");
  }

}

void ReadDataAndSendToFirebase(float t,float h)
{
  Firebase.setFloat ("Temperature",t);
  Firebase.setFloat ("Humidity",h);
   if (Firebase.failed()) {
      Serial.print("setting /data sending failed:");
      Serial.println(Firebase.error());  
      return;
  }
}

/*** MQTT Bağlantı Fonksiyonu ***/
void MQTT_connect() {
  int8_t ret;
  // Bağlantı kurulmuş ise dur
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  //uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // 0 dönerse bağlanmıştır
       Serial.println(mqtt.connectErrorString(ret));   
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void MQTTSend( float t,float h)
{
  sicaklik.publish(t);
  nem.publish(h);
}

void setup()
{

  Serial.begin(115200); // Seri iletişimi başlatıyoruz.
  //WiFi bağlantısı
  WiFiSetup();
  //Firebase bağlantısı
  FirebaseSetup();
  Serial.println("*********************");
}
 
void loop()
{
  // Sensörün okunup okunmadığını konrol ediliyor. 
 
  int chk = DHT11_sensor.read(DHTPIN);
  Serial.print("Sensor Okundu mu ?: ");
  Serial.println(chk);
  // Sensörden gelen verileri serial monitörde yazdırıyoruz.
  Serial.print("Nem Orani (%): ");
  Serial.println((float)DHT11_sensor.humidity,0);
 
  Serial.print("Sicaklik (Celcius): ");
  Serial.println((float)DHT11_sensor.temperature, 0);
 
  //Serial.print("Sicaklik (Kelvin): ");
  //Serial.println(DHT11_sensor.kelvin(), 2);
  
  //Serial.print("Sicaklik (Fahrenheit): ");
  //Serial.println(DHT11_sensor.fahrenheit(), 2);
 
  // Çiğ Oluşma Noktası
  //Serial.print("Cig Olusma Noktasi: ");
  //Serial.println(DHT11_sensor.dewPoint(), 2);
  Serial.println("------------------");

  float h = (float)DHT11_sensor.humidity;
  
  float t = (float)DHT11_sensor.temperature;  // Reading temperature as Celsius (the default)

  
  //Firebase data gönderilmesi işlemi.
  ReadDataAndSendToFirebase(t,h);
  
  // MQTT bağlanma fonksiyonu
  MQTT_connect();   
  delay(1000);

  MQTTSend(t,h);
  
  // 3 saniye bekliyoruz. 3 saniyede bir veriler ekrana yazdırılacak.
  delay(3000);
 
}
