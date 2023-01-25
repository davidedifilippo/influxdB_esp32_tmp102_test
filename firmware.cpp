#include <Arduino.h>


#include <Wire.h>                                                   
#include <SparkFunTMP102.h> // Used to send and recieve specific information from our sensor
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define WIFI_SSID "<Network Name>"                                                                                        //Network Name
#define WIFI_PASSWORD "<Network Password>"                                                                                //Network Password
#define INFLUXDB_URL "<INFLUXDB_URL>"                                                                                     //InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_TOKEN "<INFLUXDB_TOKEN>"                                                                                 //InfluxDB v2 server or cloud API token (Use: InfluxDB UI -> Data -> API Tokens -> <select token>)
#define INFLUXDB_ORG "<INFLUXDB_ORG>"                                                                                     //InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_BUCKET "<INFLUXDB_BUCKET>"                                                                               //InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define TZ_INFO "AEDT+11"                                                                                                 //InfluxDB v2 timezone

TMP102 temp_sensor_tmp102;

int temp = 0;                                                       //Variables to store sensor readings
int humid = 0;
int pressure = 0;

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);     //InfluxDB client instance with preconfigured InfluxCloud certificate
 

Point sensor("weather");                                            //Data point

void setup() 
{
  Serial.begin(115200);    
  Wire.begin();                                       

  if(!temp_sensor_tmp102.begin())
  {
        Serial.println("Il TMP102 non risponde.");
        Serial.println("Controllare la connessione al bus I2C. STOP ESECUZIONE.");
        while(1); 
  }
 

  WiFi.mode(WIFI_STA);                                              //Setup wifi connection
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");                               //Connect to WiFi
  while (wifiMulti.run() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  sensor.addTag("device", DEVICE);                                   //Add tag(s) - repeat as required
  sensor.addTag("SSID", WIFI_SSID);

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");                 //Accurate time is necessary for certificate validation and writing in batches

  if (client.validateConnection())                                   //Check server connection
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } 
  else 
  {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop()                                                          //Loop function
{
  temp_sensor_tmp102.wakeup();
  temp = temp_sensor_tmp102.readTempC();                            //Record temperature
  temp_sensor_tmp102.sleep();                            
  

  sensor.clearFields();                                              //Clear fields for reusing the point. Tags will remain untouched

  sensor.addField("temperature", temp);                              // Store measured value into point
 

    
  if (wifiMulti.run() != WL_CONNECTED)                               //Check WiFi connection and reconnect if needed
    Serial.println("Wifi connection lost");

  if (!client.writePoint(sensor))                                    //Write data point
  {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  
  Serial.print("Temp: ");                                            //Display readings on serial monitor
  Serial.println(temp);
  
  delay(10000);                                                      //Wait 10 seconds
}
