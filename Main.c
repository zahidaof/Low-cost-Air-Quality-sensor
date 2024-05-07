#include <WiFi.h>
#include <MQUnifiedsensor.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//oled part
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int iteration = 0;


// wifi part
const char* ssid     = "[your wifi name]";
const char* password = "[password]";
WiFiServer server(80);

// pm25 part
#define RXD2 16 // To sensor TXD
#define TXD2 17 // To sensor RXD
// mq part
#define         Board                   ("ESP32")
#define         Pin131                   (35)  
#define         Pin7                     (33) 
#define         RatioMQ131CleanAir        (15) //RS / R0 = 10 ppm 
#define         RatioMQ7CleanAir          (27.5) //RS / R0 = 27.5 ppm  
#define         ADC_Bit_Resolution        (12) // 10 bit ADC 
#define         Voltage_Resolution        (3.3) // Volt resolution to calc the voltage
#define         TypeC                      ("MQ-7") //Board used
#define         TypeN                      ("MQ-131") //Board used

MQUnifiedsensor MQ131(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin131, TypeN);
MQUnifiedsensor MQ7(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin7, TypeC);

// Variable to store sensor value
int pm100ss;
int pm25ss;
char pm25[20];
char pm10[20];
int no2anal;


void setup() {
  // oled part
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
}

  // our debugging output
  Serial.begin(115200);
  // pm25 part
  // Set up UART connection
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  //wifi part
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
  
  // mq part
  MQ131.init();
  MQ131.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ131.setR0(26.12);
  MQ7.init();
  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ7.setR0(26.12);
}

  // pm25 part
struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};
struct pms5003data data;

 
void loop() {

  // pm25 part
  if (readPMSdata(&Serial1)) {
    // Reading data was successful
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (standard)");
    //Serial.print("PM 1.0: "); Serial.print(data.pm10_standard);
    Serial.print("PM 2.5: "); Serial.print(data.pm25_standard);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_standard);
    Serial.println("---------------------------------------");
    pm100ss = data.pm100_standard;
    pm25ss = data.pm25_standard;
    // Determine air quality based on PM10 standard concentration
    if (pm100ss >= 0 && pm100ss <= 25) {
        strcpy(pm10, "Very good quality");
    } else if (pm100ss >= 26 && pm100ss <= 50) {
        strcpy(pm10, "Good quality");
    } else if (pm100ss >= 51 && pm100ss <= 90) {
        strcpy(pm10, "Fair quality");
    } else if (pm100ss >= 91 && pm100ss <= 180) {
        strcpy(pm10, "Poor quality");
    } else {
        strcpy(pm10, "Very poor quality");
    }
        // Determine air quality based on PM25 standard concentration
    if (data.pm25_standard >= 0 && data.pm25_standard <= 15) {
        strcpy(pm25, "Very good quality");
    } else if (data.pm25_standard >= 16 && data.pm25_standard <= 30) {
        strcpy(pm25, "Good quality");
    } else if (data.pm25_standard >= 31 && data.pm25_standard <= 55) {
        strcpy(pm25, "Fair quality");
    } else if (data.pm25_standard >= 56 && data.pm25_standard <= 110) {
        strcpy(pm25, "Poor quality");
    } else {
        strcpy(pm25, "Very poor quality");
    }
  } else {
    Serial.println("Failed to read PM2.5 sensor data");
  }
  Serial.println(pm10);
  Serial.println(pm25);




  // mq part
  MQ131.update();  
  MQ7.update();
  MQ131.setA(-462.43); MQ131.setB(-2.204); //No2
  float No2 = MQ131.readSensor(); 
  
  MQ7.setA(99.042); MQ7.setB(-1.518); //CO
  float CObefore = MQ7.readSensor(); 
  float CO = CObefore * 2;
  

  Serial.print("CO ppm: ");
  Serial.println(CO);

  Serial.println("==================");
  // Read sensor pin value

  Serial.print("No2 ppm: ");
  Serial.println(No2);
  no2anal = analogRead(Pin131);
  Serial.println(no2anal);
 
 iteration++;

  // Update OLED screen based on iteration
  if (iteration == 1 || iteration == 21) {
    display.clearDisplay();
    display.setTextSize(1); // Increase text size
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(pm25); // Add static text
    display.setTextSize(2);
    display.setCursor(0, 20); // Adjust cursor position
    display.println("PM2.5:");
    display.println(String(data.pm25_standard) + " ug/m^3"); // Convert to String and add units
    display.display();
  } else if (iteration == 11 || iteration == 31) {
    display.clearDisplay();
    display.setTextSize(1); // Increase text size
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(pm10); // Add static text
    display.setTextSize(2);
    display.setCursor(0, 20); // Adjust cursor position
    display.println("PM10:");
    display.println(String(data.pm100_standard) + " ug/m^3"); // Convert to String and add units
    display.display();
  } else if (iteration == 41 || iteration == 61) {
    display.clearDisplay();
    display.setTextSize(1); // Increase text size
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Air Quality Monitor"); // Add static text
    display.setTextSize(2);
    display.setCursor(0, 20); // Adjust cursor position
    display.println("CO ppm:");
    display.println(String(CO) + " ppm"); // Convert to String and add units
    display.setTextSize(1);
    display.print(WiFi.localIP());
    display.display();
  } else if (iteration == 81 || iteration == 101) {
    display.clearDisplay();
    
    display.setTextSize(1); // Increase text size
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Air Quality Monitor"); // Add static text
    display.setTextSize(2);
    display.setCursor(0, 20); // Adjust cursor position
    display.println("NO2 ppm:");
    display.println(String(No2) + " ppm"); // Convert to String and add units
    display.setTextSize(1);
    display.print(WiFi.localIP());
    display.display();
  }

  // Reset iteration counter after every 100 iterations
  if (iteration >= 100) {
    iteration = 0;
  }
  // wifi part
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
                    // Respond to the client's HTTP request
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println("Refresh: 3"); // Refresh every 3 seconds
                    client.println();
                    client.println("<html><head><meta http-equiv=\"refresh\" content=\"3\">");
                    client.println("<style>");
                    client.println("body { font-family: Arial, sans-serif; background-color: #f0f0f0; }");
                    client.println(".container { text-align: center; margin-top: 50px; }");
                    
                    // Define CSS classes for different air quality levels
                    client.println(".very-good { background-color: green; }");
                    client.println(".good { background-color: lime; }");
                    client.println(".fair { background-color: yellow; }");
                    client.println(".poor { background-color: orange; }");
                    client.println(".very-poor { background-color: red; }");
                    client.println(".death { background-color: black; }");
        
                    
                    
                    client.println(".green-box { color: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }");
                    client.println("</style>");
                    client.println("</head><body>");
                    
                    client.println("<div class=\"container\">");
                    
                    // Determine air quality level for PM2.5
                    String pm25Color = "";
                    if (data.pm25_standard >= 0 && data.pm25_standard <= 15) {
                        pm25Color = "green";
                    } else if (data.pm25_standard >= 16 && data.pm25_standard <= 30) {
                        pm25Color = "lime";
                    } else if (data.pm25_standard >= 31 && data.pm25_standard <= 55) {
                        pm25Color = "yellow";
                    } else if (data.pm25_standard >= 56 && data.pm25_standard <= 110) {
                        pm25Color = "orange";
                    } else {
                        pm25Color = "red";
                    }
                    
                    // Determine air quality level for PM10
                    String pm10Color = "";
                    if (pm100ss >= 0 && pm100ss <= 25) {
                        pm10Color = "green";
                    } else if (pm100ss >= 26 && pm100ss <= 50) {
                        pm10Color = "lime";
                    } else if (pm100ss >= 51 && pm100ss <= 90) {
                        pm10Color = "yellow";
                    } else if (pm100ss >= 91 && pm100ss <= 180) {
                        pm10Color = "orange";
                    } else {
                        pm10Color = "red";
                    }
                    
                   client.println("<div class=\"green-box\" style=\"background-color: " + pm25Color + "\">");
                    client.print("<h1>PM2.5 Sensor Data</h1>");
                    client.print("<h2>PM 2.5 Air Quality: <span>"+String(pm25)+" ug/m^3</span></h2>");
                    client.print("<h3>PM 2.5: <span>"+String(data.pm25_standard)+" ug/m^3</span></h3>");
                    client.println("</div>"); // Close PM2.5 green-box div
                    
                    client.println("<div class=\"green-box\" style=\"background-color: " + pm10Color + "\">");
                    client.print("<h1>PM10 Sensor Data</h1>");
                    client.print("<h2>PM 10 Air Quality: <span>"+String(pm10)+" ug/m^3</span></h2>");
                    client.print("<h3>PM 10: <span>"+String(data.pm100_standard)+" ug/m^3</span></h3>");
                    client.println("</div>"); // Close PM10 green-box div
                    
                    client.println("<style>");
                    client.println("body { font-family: Arial, sans-serif; background-color: #f0f0f0; }");
                    client.println(".container { text-align: center; margin-top: 50px; }");
                    client.println(".green-box { background-color: green; color: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }");
                    client.println(".grey-box { background-color: grey; color: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }");
                    client.println(".square-box { width: 200px; height: 200px; margin: 0 auto; text-align: center; }"); // Define square-box class
                    client.println("h1, h2, h3 { color: white; }"); // Adjusted for h3 as well
                    client.println("</style>");
                    client.println("</head><body>");
                    
                    client.println("<div class=\"container\">");

                    float coPPM = CO;
                    // Determine color for CO square-box based on ppm
                    String coColor = "";
                    String coStatus = "";
                    if (coPPM >= 0 && coPPM <= 10) {
                        coColor = "very-good";
                        coStatus = "Excellent";
                    } else if (coPPM > 0 && coPPM <= 50) {
                        coColor = "good";
                        coStatus = "Good";
                    } else if (coPPM > 50 && coPPM <= 100) {
                        coColor = "poor";
                        coStatus = "Unhealthy";
                    } else if (coPPM > 100 && coPPM <= 200) {
                        coColor = "very-poor";
                        coStatus = "Very Unhealthy";
                    } else {
                        coColor = "death";
                        coStatus = "Harmful";
                    }

                    // Calculate ppm for NO2
                    float no2PPM = No2;
                    // Determine color for NO2 square-box based on ppm
                    String no2Color = "";
                    String no2Status = "";
                    if (no2PPM >= 0 && no2PPM <= 1) {
                        no2Color = "very-good";
                        no2Status = "Small-amount";
                    } else if (no2PPM > 1 && no2PPM <= 5) {
                        no2Color = "fair";
                        no2Status = "Health-risk";
                    } else if (no2PPM > 5 && no2PPM <= 10) {
                        no2Color = "poor";
                        no2Status = "High-risk";
                    } else if (no2PPM > 10 && no2PPM <= 15) {
                        no2Color = "very-poor";
                        no2Status = "Very-high-risk";
                    } else {
                        no2Color = "death";
                        no2Status = "Danger-life";
                    }

                    // CO square-box
                    client.println("<div class=\"square-box " + coColor + "\">");
                    client.println("<h1>CO Value</h1>");
                    client.println("<h2>" + String(coPPM) + " ppm</h2>"); // Display CO ppm
                    client.println("<h3>Status: " + coStatus + "</h3>");
                    client.println("</div>");

                    // NO2 square-box
                    client.println("<div class=\"square-box " + no2Color + "\">");
                    client.println("<h1>NO2 Value</h1>");
                    client.println("<h2>" + String(no2PPM) + " ppm</h2>"); // Display NO2 ppm
                    client.println("<h3>Status: " + no2Status + "</h3>");
                    client.println("</div>");

                    client.println("</div>"); // Close container div
                    client.println("<div class=\"container\">");
                    client.println("<div class=\"grey-box\">"); // Start grey-box div
                    client.println("<h1>Additional Sensor Data</h1>");
                    client.println("<h2>PM2.5 Sensor Data</h2>");
                    client.println("<h3>Concentration Units (standard)</h3>");
                    client.print("<p>PM 1.0: <span>"+String(data.pm10_standard)+"</span></p>");
                    client.print("<p>PM 2.5: <span>"+String(data.pm25_standard)+"</span></p>");
                    client.print("<p>PM 10: <span>"+String(data.pm100_standard)+"</span></p>");
                    client.println("<h3>Concentration Units (environmental)</h3>");
                    client.print("<p>PM 1.0: <span>"+String(data.pm10_env)+"</span></p>");
                    client.print("<p>PM 2.5: <span>"+String(data.pm25_env)+"</span></p>");
                    client.print("<p>PM 10: <span>"+String(data.pm100_env)+"</span></p>");
                    client.println("<h3>Particle Counts</h3>");
                    client.print("<p>Particles > 0.3um / 0.1L air: <span>"+String(data.particles_03um)+"</span></p>");
                    client.print("<p>Particles > 0.5um / 0.1L air: <span>"+String(data.particles_05um)+"</span></p>");
                    client.print("<p>Particles > 1.0um / 0.1L air: <span>"+String(data.particles_10um)+"</span></p>");
                    client.print("<p>Particles > 2.5um / 0.1L air: <span>"+String(data.particles_25um)+"</span></p>");
                    client.print("<p>Particles > 5.0um / 0.1L air: <span>"+String(data.particles_50um)+"</span></p>");
                    client.print("<p>Particles > 10.0 um / 0.1L air: <span>"+String(data.particles_100um)+"</span></p>");
                    client.println("</body></html>");
                    break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      } else {
        break;
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }

  // Delay between readings
  delay(100);
}


// pm25 part
boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
 
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }
 
  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
 
  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);
 
  // get checksum ready
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }
 /*
    //debugging
    for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
    }
    Serial.println();
  */
 
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }
 
  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);
 
  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}