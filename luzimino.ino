#include <EEPROM.h>
#include <WS2812FX.h>

#define LED_COUNT 144
#define LED_PIN 2

struct Config {
  char name[10];
  int version;
  int R;
  int G;
  int B;
  int brightness;
  int speed;
  int mode;
};
#define CONFIG_VERSION 1


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

Config config;

/*uint32_t getColor() {
  int color = 0;
  if (config.R > 225 || config.G > 255 || config.B > 255) {
    return 0xFFFFFF;
  }
  color = config.R << 16 + config.G << 8 + config.B;
  return color;
  
}*/

void setup() {
  Serial.begin(115200);
  cmd.reserve(50);
  delay(2000);
  EEPROM.get(0, config);
  if (strcmp(config.name, "Luzimino ") != 0 || config.version != CONFIG_VERSION)
  //if (config.version != CONFIG_VERSION)
  {
    Serial.println("Conf not in EEPROM");
    strcpy(config.name, "Luzimino ");
    config.version = CONFIG_VERSION;
    config.R = 0xFF;
    config.G = 0x28;
    config.B = 0x00;
    config.brightness = 30;
    config.speed = 200;
    //config.mode = FX_MODE_STATIC;
    config.mode = FX_MODE_COLOR_SWEEP_RANDOM;
    EEPROM.put(0, config);
  } else {
    Serial.println("Got conf from EEPROM");
    Serial.print("config.name: ");
    Serial.println(config.name);
    Serial.print("config.version: ");
    Serial.println(config.version);
    Serial.print("config.R: ");
    Serial.println(config.R);
    Serial.print("config.G: ");
    Serial.println(config.G);
    Serial.print("config.B: ");
    Serial.println(config.B);
    Serial.print("config.brightness: ");
    Serial.println(config.brightness);
    Serial.print("config.speed: ");
    Serial.println(config.speed);
    Serial.print("config.mode: ");
    Serial.println(config.mode);
  }
  
  ws2812fx.init();
  ws2812fx.setBrightness(config.brightness);
  ws2812fx.setSpeed(config.speed);
  //ws2812fx.setColor(config.R, config.G, config.B);
  ws2812fx.setColor(0xFF2800);
  ws2812fx.setMode(config.mode);
  ws2812fx.start();

  printModes();
  printUsage();
}

void loop() {
  ws2812fx.service();

  //On micro, call serialEvent since it's not called automatically : 
  serialEvent();

  if(cmd_complete) {
    process_command();
  }
}

/*
 * Checks received command and calls corresponding functions.
 */
void process_command() {
  if(cmd == F("b+")) { 
    ws2812fx.increaseBrightness(25);
    Serial.print(F("Increased brightness by 25 to: "));
    config.brightness = ws2812fx.getBrightness();
    Serial.println(config.brightness);
  }

  if(cmd == F("b-")) {
    ws2812fx.decreaseBrightness(25); 
    Serial.print(F("Decreased brightness by 25 to: "));
    config.brightness = ws2812fx.getBrightness();
    Serial.println(config.brightness);
  }

  if(cmd.startsWith(F("b "))) { 
    uint8_t b = (uint8_t) cmd.substring(2, cmd.length()).toInt();
    ws2812fx.setBrightness(b);
    Serial.print(F("Set brightness to: "));
    config.brightness = ws2812fx.getBrightness();
    Serial.println(config.brightness);
  }

  if(cmd == F("s+")) { 
    ws2812fx.increaseSpeed(10);
    Serial.print(F("Increased speed by 10 to: "));
    config.speed = ws2812fx.getSpeed();
    Serial.println(config.speed);
  }

  if(cmd == F("s-")) {
    ws2812fx.decreaseSpeed(10); 
    Serial.print(F("Decreased speed by 10 to: "));
    config.speed = ws2812fx.getSpeed();
    Serial.println(config.speed);
  }

  if(cmd.startsWith(F("s "))) {
    uint8_t s = (uint8_t) cmd.substring(2, cmd.length()).toInt();
    ws2812fx.setSpeed(s); 
    Serial.print(F("Set speed to: "));
    config.speed = ws2812fx.getSpeed();
    Serial.println(config.speed);
  }

  if(cmd.startsWith(F("m "))) { 
    uint8_t m = (uint8_t) cmd.substring(2, cmd.length()).toInt();
    ws2812fx.setMode(m);
    Serial.print(F("Set mode to: "));
    config.mode = ws2812fx.getMode(); 
    Serial.print(config.mode);
    Serial.print(" - ");
    Serial.println(ws2812fx.getModeName(config.mode));
  }

  if(cmd.startsWith(F("c "))) { 
    uint32_t c = (uint32_t) strtol(&cmd.substring(2, cmd.length())[0], NULL, 16);
    ws2812fx.setColor(c); 
    Serial.print(F("Set color to: "));
    Serial.println(ws2812fx.getColor(), HEX);
    // TODO: store it in the eeprom too
  }

  cmd = "";              // reset the commandstring
  cmd_complete = false;  // reset command complete
  EEPROM.put(0, config);
}

/*
 * Prints a usage menu.
 */
void printUsage() {
  Serial.println(F("Usage:"));
  Serial.println();
  Serial.println(F("m <n> \t : select mode <n>"));
  Serial.println();
  Serial.println(F("b+ \t : increase brightness"));
  Serial.println(F("b- \t : decrease brightness"));
  Serial.println(F("b <n> \t : set brightness to <n>"));
  Serial.println();
  Serial.println(F("s+ \t : increase speed"));
  Serial.println(F("s- \t : decrease speed"));
  Serial.println(F("s <n> \t : set speed to <n>"));
  Serial.println();
  Serial.println(F("c 0x007BFF \t : set color to 0x007BFF"));
  Serial.println();
  Serial.println();
  Serial.println(F("Have a nice day."));
  Serial.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
  Serial.println();
}


/*
 * Prints all available WS2812FX blinken modes.
 */
void printModes() {
  Serial.println(F("Supporting the following modes: "));
  Serial.println();
  for(int i=0; i < ws2812fx.getModeCount(); i++) {
    Serial.print(i);
    Serial.print(F("\t"));
    Serial.println(ws2812fx.getModeName(i));
  }
  Serial.println();
}


/*
 * Reads new input from serial to cmd string. Command is completed on \n
 */
void serialEvent() {
  while(Serial.available()) {
    char inChar = (char) Serial.read();
    if(inChar == '\n') {
      cmd_complete = true;
    } else {
      cmd += inChar;
    }
  }
}
