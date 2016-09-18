#include <Arduino.h>
#include <EEPROM.h>
#include <WS2812FX.h>      //https://github.com/kitesurfer1404/WS2812FX
#include <Button.h>        //https://github.com/t3db0t/Button


#define LED_COUNT 144
#define LED_PIN 2


struct Config {
  char name[10];
  unsigned int version;
  unsigned int R;
  unsigned int G;
  unsigned int B;
  unsigned int brightness;
  unsigned int speed;
  unsigned int mode;
};
#define CONFIG_VERSION 1

#define DOWN_PIN 3
#define UP_PIN 5
#define LEFT_PIN 4
#define RIGHT_PIN 7
#define CENTER_PIN 6

#define DEBOUNCE true
#define DEBOUNCE_MS 50     //A debounce time of 50 milliseconds usually works well for tactile button switches.

const unsigned int HOLD_TIME = 500;

Button btnUp = Button(UP_PIN, BUTTON_PULLUP_INTERNAL, DEBOUNCE, DEBOUNCE_MS);
Button btnDown = Button(DOWN_PIN, BUTTON_PULLUP_INTERNAL, DEBOUNCE, DEBOUNCE_MS);
Button btnLeft = Button(LEFT_PIN, BUTTON_PULLUP_INTERNAL, DEBOUNCE, DEBOUNCE_MS);
Button btnRight = Button(RIGHT_PIN, BUTTON_PULLUP_INTERNAL, DEBOUNCE, DEBOUNCE_MS);
Button btnCenter = Button(CENTER_PIN, BUTTON_PULLUP_INTERNAL, DEBOUNCE, DEBOUNCE_MS);

#define brightnessSensor A0  // Analog input pin that the brightness potentiometer is attached to
int previousBrightnessSensorValue = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

String cmd = "";               // String to store incoming serial commands
boolean cmd_complete = false;  // whether the command string is complete

enum {RUN, SETUP_COLOR};       // Setup state machine possible states
uint8_t STATE;                 // The current setup state machine state
enum {ALL = 0, RED = 1, GREEN = 2, BLUE = 3};       // Color choosing state machine
uint8_t CHANNEL = ALL;                 // The curent color being modified

Config config;

/*uint32_t getColor() {
  int color = 0;
  if (config.R > 225 || config.G > 255 || config.B > 255) {
    return 0xFFFFFF;
  }
  color = config.R << 16 + config.G << 8 + config.B;
  return color;

}*/

void cb_storeConfig(Button& b) {
  Serial.println(F("Store config"));
  EEPROM.put(0, config);
}

void changeChannel(uint8_t channel) {
  switch (channel) {
    case ALL:
      ws2812fx.setColor(config.R, config.G, config.B);
      break;
    case RED:
      ws2812fx.setColor(config.R, 0, 0);
      break;
    case GREEN:
      ws2812fx.setColor(0, config.G, 0);
      break;
    case BLUE:
      ws2812fx.setColor(0, 0, config.B);
      break;
  }
  CHANNEL = channel;

}

void cb_nextChannel(Button& b) {
  if(CHANNEL != BLUE)
    changeChannel(CHANNEL+1);
}

void cb_previousChannel(Button& b) {
  if(CHANNEL != ALL)
    changeChannel(CHANNEL-1);
}

void cb_increaseChannelValue(Button& b) {
  switch (CHANNEL) {
    case ALL:
      ws2812fx.setColor(config.R, config.G, config.B);
      break;
    case RED:
      config.R = min(config.R+10, 255);
      ws2812fx.setColor(config.R, 0, 0);
      Serial.print(F("Set red to: "));
      Serial.println(ws2812fx.getColor(), HEX);
      break;
    case GREEN:
      config.G = min(config.G+10, 255);
      ws2812fx.setColor(0, config.G, 0);
      Serial.print(F("Set green to: "));
      Serial.println(ws2812fx.getColor(), HEX);
      break;
    case BLUE:
      config.B = min(config.B+10, 255);
      ws2812fx.setColor(0, 0, config.B);
      Serial.print(F("Set blue to: "));
      Serial.println(ws2812fx.getColor(), HEX);
      break;
  }
}

void cb_decreaseChannelValue(Button& b) {
  switch (CHANNEL) {
    case ALL:
      ws2812fx.setColor(config.R, config.G, config.B);
      break;
    case RED:
      config.R = config.R < 10 ? 0 : config.R - 10;
      ws2812fx.setColor(config.R, 0, 0);
      Serial.print(F("Set red to: "));
      Serial.println(ws2812fx.getColor(), HEX);
      break;
    case GREEN:
      config.G = config.G < 10 ? 0 : config.G - 10;
      ws2812fx.setColor(0, config.G, 0);
      Serial.print(F("Set green to: "));
      Serial.println(ws2812fx.getColor(), HEX);
      break;
    case BLUE:
      config.B = config.B < 10 ? 0 : config.B - 10;
      ws2812fx.setColor(0, 0, config.B);
      Serial.print(F("Set blue to: "));
      Serial.println(ws2812fx.getColor(), HEX);
      break;
  }
}

void setBrightness(int brightness) {
  ws2812fx.setBrightness(brightness);
  config.brightness = ws2812fx.getBrightness();
  Serial.println(config.brightness);
}

void cb_increaseBrightness(Button& b) {
  ws2812fx.increaseBrightness(25);
  Serial.print(F("Increased brightness by 25 to: "));
  config.brightness = ws2812fx.getBrightness();
  Serial.println(config.brightness);
}

void cb_decreaseBrightness(Button& b) {
  ws2812fx.decreaseBrightness(25);
  Serial.print(F("Decreased brightness by 25 to: "));
  config.brightness = ws2812fx.getBrightness();
  Serial.println(config.brightness);
}

void cb_increaseSpeed(Button& b) {
  ws2812fx.increaseSpeed(10);
  Serial.print(F("Increased speed by 10 to: "));
  config.speed = ws2812fx.getSpeed();
  Serial.println(config.speed);
}

void cb_decreaseSpeed(Button& b) {
  ws2812fx.decreaseSpeed(10);
  Serial.print(F("Decreased speed by 10 to: "));
  config.speed = ws2812fx.getSpeed();
  Serial.println(config.speed);
}

void cb_nextMode(Button& b) {
  config.mode++;
  if (config.mode >= ws2812fx.getModeCount())
    config.mode = 0;

  ws2812fx.setMode(config.mode);
  Serial.print(F("Set mode to: "));
  Serial.println(config.mode);
}

void cb_previousMode(Button& b) {
  if (config.mode == 0)
    config.mode = ws2812fx.getModeCount();

  config.mode--;

  ws2812fx.setMode(config.mode);
  Serial.print(F("Set mode to: "));
  Serial.println(config.mode);
}

void cb_toggleSetup(Button& b) {
  switch (STATE) {
    case RUN:
      Serial.println("Switching to SETUP_COLOR state");
      STATE = SETUP_COLOR;
      changeChannel(ALL);

      btnUp.clickHandler(cb_increaseChannelValue);
      btnDown.clickHandler(cb_decreaseChannelValue);
      btnLeft.clickHandler(cb_previousChannel);
      btnRight.clickHandler(cb_nextChannel);
      ws2812fx.setMode(FX_MODE_STATIC);
      break;

    case SETUP_COLOR:
      Serial.println("Switching to RUN state");
      STATE = RUN;
      btnUp.clickHandler(cb_increaseSpeed);
      btnDown.clickHandler(cb_decreaseSpeed);
      btnLeft.clickHandler(cb_previousMode);
      btnRight.clickHandler(cb_nextMode);
      ws2812fx.setColor(config.R, config.G, config.B);
      ws2812fx.setMode(config.mode);
      break;
  }
}

void processBrightness(){
  int sensorValue = analogRead(brightnessSensor);
  if (previousBrightnessSensorValue != sensorValue) {
    setBrightness(map(sensorValue, 0, 1023, 0, 255));
    previousBrightnessSensorValue = sensorValue;
  }
}


void setup() {
  Serial.begin(115200);
  cmd.reserve(50);
  delay(2000);
  EEPROM.get(0, config);
  if (strcmp(config.name, "Luzimino ") != 0 || config.version != CONFIG_VERSION) {
    Serial.println("Conf not in EEPROM");
    strcpy(config.name, "Luzimino ");
    config.version = CONFIG_VERSION;
    config.R = 0xFF;
    config.G = 0x28;
    config.B = 0x00;
    config.brightness = 30;
    config.speed = 200;
    config.mode = FX_MODE_COLOR_SWEEP_RANDOM;
    EEPROM.put(0, config);
  } else {
    Serial.println("Got conf from EEPROM");
    Serial.print("config.name: ");
    Serial.println(config.name);
    Serial.print("config.version: ");
    Serial.println(config.version);
    Serial.print("config.R: 0x");
    Serial.println(config.R, HEX);
    Serial.print("config.G: 0x");
    Serial.println(config.G, HEX);
    Serial.print("config.B: 0x");
    Serial.println(config.B, HEX);
    Serial.print("config.brightness: ");
    Serial.println(config.brightness);
    Serial.print("config.speed: ");
    Serial.println(config.speed);
    Serial.print("config.mode: ");
    Serial.println(config.mode);
  }

  // Assign buttons callback functions
  btnUp.clickHandler(cb_increaseSpeed);
  btnDown.clickHandler(cb_decreaseSpeed);
  //btnUp.holdHandler(cb_increaseBrightness,500);
  //btnDown.holdHandler(cb_decreaseBrightness,500);
  btnLeft.clickHandler(cb_previousMode);
  btnRight.clickHandler(cb_nextMode);
  btnCenter.holdHandler(cb_toggleSetup, 500);
  btnCenter.clickHandler(cb_storeConfig);

  ws2812fx.init();
  ws2812fx.setBrightness(config.brightness);
  ws2812fx.setSpeed(config.speed);
  ws2812fx.setColor(config.R, config.G, config.B);
  //ws2812fx.setColor(0xFF2800);
  ws2812fx.setMode(config.mode);
  ws2812fx.start();
}

void loop() {
  ws2812fx.service();

  btnUp.process();
  btnDown.process();
  btnLeft.process();
  btnRight.process();
  btnCenter.process();

  processBrightness();
}
