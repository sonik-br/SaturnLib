/*******************************************************************************
 * Sega Saturn controller input library.
 * https://github.com/sonik-br/SaturnLib
 * 
 * The library depends on greiman's DigitalIO library.
 * https://github.com/greiman/DigitalIO
 * 
 * I recommend the usage of SukkoPera's fork of DigitalIO as it supports a few more platforms.
 * https://github.com/SukkoPera/DigitalIO
 * 
 * 
 * This sketch is ready to use on a Leonardo but may work on any
 * arduino with the correct number of pins and proper setup.
*/

#include <SaturnLib.h>

#define ENABLE_SERIAL_DEBUG

//Saturn pins
#define SAT_TH A2
#define SAT_TR 7
#define SAT_TL A1
#define SAT_D0 A4
#define SAT_D1 A5
#define SAT_D2 A3
#define SAT_D3 A0

/*
#define SAT_TH 8
#define SAT_TR 9
#define SAT_TL 5
#define SAT_D0 2
#define SAT_D1 3
#define SAT_D2 4
#define SAT_D3 6
*/

SaturnPort<SAT_D0,SAT_D1,SAT_D2,SAT_D3,SAT_TH,SAT_TR,SAT_TL> saturn;

#define ENABLE_SERIAL_DEBUG

#ifdef ENABLE_SERIAL_DEBUG
#define dstart(spd) do {Serial.begin (spd); while (!Serial) {digitalWrite (LED_BUILTIN, (millis () / 500) % 2);}} while (0);
#define debug(...) Serial.print (__VA_ARGS__)
#define debugln(...) Serial.println (__VA_ARGS__)
#else
#define dstart(...)
#define debug(...)
#define debugln(...)
#endif

#define DIGITALSTATE(D) \
if(sc.digitalJustPressed(D)) { \
  debugln(F("Digital pressed: " #D)); \
} else if(sc.digitalJustReleased(D)) {\
  debugln(F("Digital released: " #D)); \
}

#define ANALOGSTATE(A) \
if(sc.analogChanged(A)) {\
  debug(F("Analog " #A ": ")); \
  debugln(sc.analog(A)); \
}

#define DEVICE(A, B) \
if(A == B) {\
  debug(#B); \
}

void printDeviceType (const SatDeviceType_Enum d) {
  DEVICE(d, SAT_DEVICE_NONE)
  DEVICE(d, SAT_DEVICE_NOTSUPPORTED)
  DEVICE(d, SAT_DEVICE_MEGA3)
  DEVICE(d, SAT_DEVICE_MEGA6)
  DEVICE(d, SAT_DEVICE_PAD)
  DEVICE(d, SAT_DEVICE_3DPAD)
  DEVICE(d, SAT_DEVICE_WHEEL)
}

void printButtons(const SaturnController& sc) {
  DIGITALSTATE(SAT_PAD_UP)
  DIGITALSTATE(SAT_PAD_DOWN)
  DIGITALSTATE(SAT_PAD_LEFT)
  DIGITALSTATE(SAT_PAD_RIGHT)
  DIGITALSTATE(SAT_B)
  DIGITALSTATE(SAT_C)
  DIGITALSTATE(SAT_A)
  DIGITALSTATE(SAT_START)
  DIGITALSTATE(SAT_Z)
  DIGITALSTATE(SAT_Y)
  DIGITALSTATE(SAT_X)
  DIGITALSTATE(SAT_R)
  DIGITALSTATE(SAT_L)
}

void printAnalog(const SaturnController& sc) {
  ANALOGSTATE(SAT_ANALOG_X)
  ANALOGSTATE(SAT_ANALOG_Y)
  ANALOGSTATE(SAT_ANALOG_L)
  ANALOGSTATE(SAT_ANALOG_R)
}

void setup() {
  //Init the library.
  saturn.begin();
  
  delay(50);

  dstart (115200);
  debugln (F("Powered on!"));
}

void loop() {
  static unsigned long idleTimer = 0;
  static uint8_t lastControllerCount = 0;
  static uint8_t lastMultitapPorts = 0;
  static SatDeviceType_Enum dtype[6] = { SAT_DEVICE_NONE, SAT_DEVICE_NONE, SAT_DEVICE_NONE, SAT_DEVICE_NONE, SAT_DEVICE_NONE, SAT_DEVICE_NONE };
  
  //uint8_t hatData;

  const unsigned long start = micros();
  
  //Call update to read the controller(s)
  saturn.update();
  
  //Time spent to read controller(s) in microseconds
  const unsigned long delta = micros() - start;
  
  //debugln(delta);

  const uint8_t multitapPorts = saturn.getMultitapPorts();

  //A multitap was connected or disconnected?
  if (lastMultitapPorts > multitapPorts) {
    debugln(F("Multitap disconnected"));
  } else if (lastMultitapPorts < multitapPorts) {
    debug(F("Multitap connected. Ports: "));
    debugln(saturn.getMultitapPorts());
  }
  
  const uint8_t joyCount = saturn.getControllerCount();
  //debugln(joyCount);
  if (lastControllerCount != joyCount) {
    debug(F("Connected devices: "));
    debugln(joyCount);
  }
  
  bool isIdle = true;
  for (uint8_t i = 0; i < joyCount; i++) {
    const SaturnController& sc = saturn.getSaturnController(i);
    if (sc.stateChanged()) {
      isIdle = false;
      const bool isAnalog = sc.isAnalog();
      //hatData = sc.hat();
      //dtype = sc.deviceType();

      //Controller just connected. Also can happen when changing mode on 3d pad
      if (sc.deviceJustChanged()) {
        debug(F("Device changed from "));
        printDeviceType(dtype[i]);
        debug(F(" to "));
        dtype[i] = sc.deviceType();
        printDeviceType(dtype[i]);
        debugln(F(""));

        if (dtype[i] > 1) {// dtype[i] != SAT_DEVICE_NONE && dtype[i] != SAT_DEVICE_NOTSUPPORTED
          debugln( isAnalog ? F("Device is analog") : F("Device is digital") );
        }
      }

      //bool isPressed = sc.digitalPressed(SAT_B);
      
      printButtons(sc);
      
      printAnalog(sc);
      
    }
    
  }

  //Controller has been disconnected?
  if (lastControllerCount > joyCount) {
    for (uint8_t i = joyCount; i < lastControllerCount; i++) {
      const SaturnController& sc = saturn.getSaturnController(i);
      if (sc.stateChanged() && sc.deviceJustChanged()) {
        debugln(F("Device disconnected"));
        dtype[i] = SAT_DEVICE_NONE;
      }
    }
  }
  
  lastControllerCount = joyCount;
  lastMultitapPorts = multitapPorts;

  if(isIdle) {
    if (millis() - idleTimer >= 3000) {
      idleTimer = millis();
      debug(F("Idle. Read time: "));
      debugln(delta);
    }
  } else {
    idleTimer = millis();
    debug(F("Read time: "));
    debugln(delta);
  }

  delay(5);
}
