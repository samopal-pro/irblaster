#ifndef MY_CONFIG
#define MY_CONFIG


#define VER_1_0

typedef enum {
   LBT_FULL    = 3,
   LBT_MEDIUM  = 2,
   LBT_LOW     = 1
}LIGHT_BRIGHTNESS_t;

typedef enum {
   LCT_WARM    = 1,
   LCT_NEUTRAL = 2,
   LCT_COOL    = 3
}LIGHT_COLOR_t;

#if defined(VER_1_0)
#define PIN_BTN1   2
#define PIN_BTN2   1
#define PIN_BTN3   0
#define PIN_BTN4   3

#define PIN_IR_OUT 4

#define PIN_IR_IN  5

#define PIN_LED    6
#elif defined(VER_1_2)


#endif

#define ENABLE_LED_FEEDBACK  true
#define USE_DEFAULT_FEEDBACK_LED_PIN true
#define SENDING_REPEATS 0

#define IR_DELAY 500
#define TM_WIFI_CONNECT 30000

// WiFi
#define WIFI_NAME  "ASUS_58_2G"
#define WIFI_PASS  "sav59vas"

// MQTT
#define MQTT_ADDR  IPAddress(192,168,1,3)
#define MQTT_HOST  "192.168.1.3"
#define MQTT_PORT   1883
#define MQTT_USER  "esp32"
#define MQTT_PASS  "sav59vas"

#define CORE       0

#endif
