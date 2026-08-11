// Newsheen Simple LED Remote — standalone IR-remote LED controller for the Newsheen board (ESP32-S3)
//   IR receiver (VS1838B) on GPIO4, 8x WS2812B on GPIO16 (via the 3V3->5V level shifter).
//   Maps the 21-key RGB remote's NEC codes to colors / white / on-off / brightness / effects.
//   Build notes: install the VS1838B with OUT in the SQUARE pad; on v2 boards tie the LED
//   level-shifter DIR pin high (+3V3) so the LEDs render. See hardware/pinout.md.
//   IRrecv OWNS GPIO4 (no manual pin ISR). 64-bit codes printed via resultToHexidecimal.
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Adafruit_NeoPixel.h>

#define IR_PIN   4
#define LED_PIN  16
#define NLED     8

IRrecv irrecv(IR_PIN, 1024, 15, true);
decode_results results;
Adafruit_NeoPixel strip(NLED, LED_PIN, NEO_GRB + NEO_KHZ800);

// ---- captured RGB-remote codes ----
#define C_ON        0xFFA25D
#define C_OFF       0xFF629D
#define C_30MIN     0xFFE21D
#define C_SMOOTH    0xFF22DD
#define C_FLASH     0xFF02FD
#define C_60MIN     0xFFC23D
#define C_UP        0xFFE01F
#define C_DOWN      0xFFA857
#define C_W         0xFF906F
#define C_R         0xFF6897
#define C_G         0xFF9867
#define C_B         0xFFB04F
#define C_REDORANGE 0xFF30CF
#define C_GREEN2    0xFF18E7
#define C_INDIGO    0xFF7A85
#define C_ORANGE    0xFF10EF
#define C_CYAN      0xFF38C7
#define C_PURPLE    0xFF5AA5
#define C_YELLOW    0xFF42BD
#define C_LTBLUE    0xFF4AB5
#define C_PINK      0xFF52AD
#define C_REPEAT    0xFFFFFFFFFFFFFFFFULL   // NEC "button held" repeat frame

enum Mode { SOLID, SMOOTH, FLASH };
Mode mode = SOLID;
bool on = true;
uint8_t bright = 160;
uint8_t curR = 255, curG = 200, curB = 120;   // boot = warm white
uint64_t lastCode = 0;
uint32_t offAtMs = 0, lastAnim = 0;
uint16_t hue = 0; bool flashPhase = false;

void setSolid(uint8_t r, uint8_t g, uint8_t b) { curR=r; curG=g; curB=b; mode=SOLID; on=true; }

void renderSolid() {
  strip.setBrightness(bright);
  uint32_t c = on ? strip.Color(curR, curG, curB) : 0;
  for (int i = 0; i < NLED; i++) strip.setPixelColor(i, c);
  strip.show();
}

const char* handle(uint64_t code) {
  switch (code) {
    case C_ON:        on=true;  renderSolid(); return "ON";
    case C_OFF:       on=false; renderSolid(); return "OFF";
    case C_W:         setSolid(255,255,255); renderSolid(); return "WHITE";
    case C_R:         setSolid(255,0,0);     renderSolid(); return "RED";
    case C_G:         setSolid(0,255,0);     renderSolid(); return "GREEN";
    case C_B:         setSolid(0,0,255);     renderSolid(); return "BLUE";
    case C_REDORANGE: setSolid(255,80,0);    renderSolid(); return "RED-ORANGE";
    case C_GREEN2:    setSolid(90,255,0);    renderSolid(); return "LIME";
    case C_INDIGO:    setSolid(50,0,255);    renderSolid(); return "INDIGO";
    case C_ORANGE:    setSolid(255,140,0);   renderSolid(); return "ORANGE";
    case C_CYAN:      setSolid(0,255,200);   renderSolid(); return "CYAN";
    case C_PURPLE:    setSolid(150,0,255);   renderSolid(); return "PURPLE";
    case C_YELLOW:    setSolid(255,210,0);   renderSolid(); return "YELLOW";
    case C_LTBLUE:    setSolid(0,150,255);   renderSolid(); return "LT-BLUE";
    case C_PINK:      setSolid(255,0,120);   renderSolid(); return "PINK";
    case C_UP:        bright = (bright>=235)?255:bright+20; on=true; renderSolid(); return "BRIGHT+";
    case C_DOWN:      bright = (bright<=25)?5:bright-20;            renderSolid(); return "BRIGHT-";
    case C_SMOOTH:    mode=SMOOTH; on=true; return "SMOOTH";
    case C_FLASH:     mode=FLASH;  on=true; return "FLASH";
    case C_30MIN:     offAtMs = millis()+30UL*60*1000; return "TIMER 30m";
    case C_60MIN:     offAtMs = millis()+60UL*60*1000; return "TIMER 60m";
    default:          return nullptr;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1200);
  Serial.println("\n\n=== Newsheen IR color control (GPIO4 rx / GPIO16 leds) ===");
  strip.begin();
  irrecv.enableIRIn();
  renderSolid();   // boot: warm white, on
}

void loop() {
  if (irrecv.decode(&results)) {
    uint64_t c = results.value;
    if (c == C_REPEAT) {                       // held button: only ramp brightness
      if (lastCode==C_UP || lastCode==C_DOWN) { const char* a=handle(lastCode); Serial.printf("  repeat -> %s (bri=%d)\n", a, bright); }
    } else {
      const char* act = handle(c);
      lastCode = c;
      Serial.printf("RX code="); Serial.print(resultToHexidecimal(&results));
      Serial.printf("  -> %s\n", act ? act : "(unmapped)");
    }
    irrecv.resume();
  }

  uint32_t now = millis();
  if (offAtMs && now > offAtMs) { on=false; offAtMs=0; renderSolid(); Serial.println("  sleep timer -> OFF"); }

  if (on && mode==SMOOTH && now-lastAnim > 18) {
    lastAnim = now; hue += 220; strip.setBrightness(bright);
    for (int i=0;i<NLED;i++) strip.setPixelColor(i, Adafruit_NeoPixel::ColorHSV(hue + i*(65536/NLED)));
    strip.show();
  } else if (on && mode==FLASH && now-lastAnim > 260) {
    lastAnim = now; flashPhase = !flashPhase; strip.setBrightness(bright);
    uint32_t c = flashPhase ? strip.Color(curR,curG,curB) : 0;
    for (int i=0;i<NLED;i++) strip.setPixelColor(i, c);
    strip.show();
  }
}
