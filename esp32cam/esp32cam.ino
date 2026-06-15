#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TM1637Display.h>
#include "esp_camera.h"
#include "driver/i2s.h"
#include <base64.h>


const char* WIFI_SSID     = "----";
const char* WIFI_PASSWORD = "----";
const char* SERVER_URL    = "http://192.168.29.179:8000/rate-outfit";


// TM1637 Seven Segment Display
#define TM1637_CLK  14
#define TM1637_DIO  15

// INMP441 Microphone (I2S)
#define I2S_WS      2
#define I2S_SCK     4
#define I2S_SD      12

// LEDs
#define LED_GREEN   16   // Ready + Mic ON indicator
#define LED_RED     13   // Error (inverted logic: LOW = ON)


// ESP32-CAM CAMERA PINS (AI Thinker)

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


TM1637Display display(TM1637_CLK, TM1637_DIO);



void greenON()  { digitalWrite(LED_GREEN, HIGH); }
void greenOFF() { digitalWrite(LED_GREEN, LOW);  }
void redON()    { digitalWrite(LED_RED, HIGH);  }  
void redOFF()   { digitalWrite(LED_RED, LOW); }  

void allOFF() {
  greenOFF();
  redOFF();
}

// Green blink = microphone is listening
void blinkGreen(int times, int ms) {
  for (int i = 0; i < times; i++) {
    greenON();
    delay(ms);
    greenOFF();
    delay(ms);
  }
}

// Red blink = error occurred
void blinkRed(int times, int ms) {
  for (int i = 0; i < times; i++) {
    redON();
    delay(ms);
    redOFF();
    delay(ms);
  }
}


void setup() {
  Serial.begin(115200);

  // Init LED pins
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED,   OUTPUT);
  allOFF();

  // Startup test — blink both LEDs once
  greenON(); redON();
  delay(500);
  allOFF();
  delay(200);

  // Init display
  display.setBrightness(7);
  display.showNumberDecEx(0, 0, true); // show 0000

  // Connect WiFi
  // Green blinks fast while connecting
  connectWiFi();

  // Init Camera
  initCamera();

  // Init Microphone
  initMicrophone();

  // Ready state:
  // Green LED slow blink = mic is ON and listening
  // Display shows ----
  display.setSegments((const uint8_t[]){
    SEG_G, SEG_G, SEG_G, SEG_G
  });

  Serial.println("Smart Mirror Ready! Listening...");
}


void loop() {
  // Green LED blinking = microphone is ON and listening
  greenON();
  delay(300);
  greenOFF();
  delay(300);

  // Listen for wake word
  if (detectWakeWord()) {
    Serial.println("Wake word detected!");

    // Green LED solid ON = processing
    greenON();

    // Show ---- on display while processing
    display.setSegments((const uint8_t[]){
      SEG_G, SEG_G, SEG_G, SEG_G
    });

    // Capture and send image to server
    float rating = captureAndSend();

    if (rating > 0) {
      // Show rating on TM1637
      // e.g. 8.5 → displayed as 8.5
      int displayVal = (int)(rating * 10);
      display.showNumberDecEx(displayVal, 0b01000000, false);

      Serial.print("Rating: ");
      Serial.println(rating);

      // Green blink 3 times = success, rating received
      blinkGreen(3, 200);

      // Green solid = back to ready/listening
      greenON();

    } else {
      // Red LED ON = error
      greenOFF();
      redON();

      // Show Err on display
      display.setSegments((const uint8_t[]){
        SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, 
        SEG_E | SEG_G,                           
        SEG_E | SEG_G,                           
        0                                        
      });

      Serial.println("Error getting rating");

      // Red blink 3 times
      blinkRed(3, 300);

      // After 3 seconds reset to ready
      delay(3000);
      redOFF();

      // Reset display to ----
      display.setSegments((const uint8_t[]){
        SEG_G, SEG_G, SEG_G, SEG_G
      });

      Serial.println("Back to listening...");
    }
  }
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    // Fast green blink while connecting
    greenON();
    delay(250);
    greenOFF();
    delay(250);
    Serial.print(".");
    dots++;
  }

  greenOFF();
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}


void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_SVGA; // 800x600
  config.jpeg_quality = 12;
  config.fb_count     = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    redON();
    return;
  }

  sensor_t* s = esp_camera_sensor_get();
  s->set_whitebal(s, 1);
  s->set_exposure_ctrl(s, 1);
  s->set_gain_ctrl(s, 1);
  s->set_brightness(s, 1);

  Serial.println("Camera ready!");
}


void initMicrophone() {
  i2s_config_t i2s_config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = 16000,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 4,
    .dma_buf_len          = 1024,
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num   = I2S_SCK,
    .ws_io_num    = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  Serial.println("Microphone ready!");
}


// WAKE WORD DETECTION
// Green LED blinks = mic actively listening
// Speak loudly or clap to trigger
// Adjust 3000 threshold if needed

bool detectWakeWord() {
  int16_t buffer[1024];
  size_t bytesRead;

  i2s_read(
    I2S_NUM_0,
    buffer,
    sizeof(buffer),
    &bytesRead,
    portMAX_DELAY
  );

  long energy = 0;
  int samples = bytesRead / 2;
  for (int i = 0; i < samples; i++) {
    energy += abs(buffer[i]);
  }
  energy /= samples;

  // Increase 3000 if background noise triggers it
  // Decrease 3000 if voice not detected
  if (energy > 3000) {
    delay(500);
    return true;
  }
  return false;
}

// CAPTURE IMAGE AND SEND TO SERVER

float captureAndSend() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return -1;
  }

  String encoded = base64::encode(fb->buf, fb->len);
  size_t frameLen = fb->len;       // ← save it first
  esp_camera_fb_return(fb);        // ← now safe to return

  DynamicJsonDocument doc(frameLen * 2 + 2048);  // ← use saved value

  doc["image"] = encoded;
  String payload;
  serializeJson(doc, payload);

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(30000);

  Serial.println("Sending image to server...");
  int responseCode = http.POST(payload);

  if (responseCode == 200) {
    String response = http.getString();
    Serial.println("Response: " + response);

    StaticJsonDocument<256> responseDoc;
    deserializeJson(responseDoc, response);
    float rating = responseDoc["rating"];
    http.end();
    return rating;
  } else {
    Serial.printf("HTTP Error: %d\n", responseCode);
    http.end();
    return -1;
  }
}