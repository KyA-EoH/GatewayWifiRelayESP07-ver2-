/*************************************************************
  Download latest ERa library here:
    https://github.com/eoh-jsc/era-lib/releases/latest
    https://www.arduino.cc/reference/en/libraries/era
    https://registry.platformio.org/libraries/eoh-ltd/ERa/installation

    ERa website:                https://e-ra.io
    ERa blog:                   https://iotasia.org
    ERa forum:                  https://forum.eoh.io
    Follow us:                  https://www.fb.com/EoHPlatform
 *************************************************************/

// Enable debug console
#define ERA_DEBUG
// #define ERA_SERIAL Serial

/* Select ERa host location (VN: Viet Nam, SG: Singapore) */
#define ERA_LOCATION_VN
// #define ERA_LOCATION_SG

/* Define setting button */
#define BUTTON_PIN              12

bool state = false;

#if defined(BUTTON_PIN)
    // Active low (false), Active high (true)
    #define BUTTON_INVERT       false
    #define BUTTON_HOLD_TIMEOUT 5000UL

    // This directive is used to specify whether the configuration should be erased.
    // If it's set to true, the configuration will be erased.
    #define ERA_ERASE_CONFIG    true
#endif

#include <Arduino.h>
#include <ERa.hpp>
#if defined(BUTTON_PIN)
    #include <Ticker.h>
    #include <ERa/ERaButton.hpp>
#endif

#if defined(BUTTON_PIN)
    Ticker ticker;
    ERaButton button;

    static void handlerButton() {
        button.run();
    }

#if ERA_VERSION_NUMBER >= ERA_VERSION_VAL(1, 2, 0)
    static void eventButton(uint8_t pin, ButtonEventT event) {
        if (event != ButtonEventT::BUTTON_ON_FALLING) {
            return;
        }
        ERA_LOG(ERA_PSTR("ERa"), ERA_PSTR("Button pressed!"));
        // ERa.switchToConfig(ERA_ERASE_CONFIG);
        int d4value = digitalRead(4);
        digitalWrite(4, !d4value);
        ERa.virtualWrite(V1, !d4value);
        state = !d4value;
        ERa.virtualWrite(V0, 0);
        ERa.getFlash().writeFlash("pin_state", &state, sizeof(state));
        (void)pin;
    }
#else
    static void eventButton(ButtonEventT event) {
        if (event != ButtonEventT::BUTTON_ON_HOLD) {
            return;
        }
        ERa.switchToConfig(ERA_ERASE_CONFIG);
    }
#endif

    void initButton() {
        pinMode(BUTTON_PIN, INPUT);
        // button.setButton(BUTTON_PIN, digitalRead, eventButton,
        //                 BUTTON_INVERT).onHold(BUTTON_HOLD_TIMEOUT);
        button.setButton(BUTTON_PIN, digitalRead, eventButton,
                BUTTON_INVERT).onFalling();
        ticker.attach_ms(100, handlerButton);
    }
#endif

/*Write state to flash after each control from app*/
ERA_PIN_WRITE(4) {
    state = param.getInt();
    ERa.getFlash().writeFlash("pin_state", &state, sizeof(state));
    ERA_LOG(ERA_PSTR("ERa"), ERA_PSTR("Reset flag of control"));
    ERa.virtualWrite(V0, 0);
    return false;
}

/* This function will run every time ERa is connected */
ERA_CONNECTED() {
    ERA_LOG(ERA_PSTR("ERa"), ERA_PSTR("ERa connected!"));
}

/* This function will run every time ERa is disconnected */
ERA_DISCONNECTED() {
    ERA_LOG(ERA_PSTR("ERa"), ERA_PSTR("ERa disconnected!"));
}

/* This function print uptime every second */
void timerEvent() {
    ERA_LOG(ERA_PSTR("Timer"), ERA_PSTR("Uptime: %d"), ERaMillis() / 1000L);
}

void setup() {
    /* Setup debug console */
#if defined(ERA_DEBUG)
    Serial.begin(115200);
#endif

#if defined(BUTTON_PIN)
    /* Initializing button. */
    initButton();
#endif

    /* Set board id */
    // ERa.setBoardID("Board_1");

    /* In some cases, it is necessary to disable
    the Pull-Up mode of the RX GPIO */
    /*
    #if defined(SerialMB)
        SerialMB.enableRxGPIOPullUp(false);
    #endif
    */

    /* White labeling App (use this ONLY if you have a branded ERa App) */
    // ERa.setVendorName("MyORG");
    // ERa.setVendorPrefix("MyPrefix");

    /* Set scan WiFi. If activated, the board will scan
       and connect to the best quality WiFi. */
    // ERa.setScanWiFi(true);

    /* Initializing the ERa library. */
    ERa.begin();

    /*Get state from flash after startup*/
    ERa.getFlash().begin();
    ERa.getFlash().readFlash("pin_state", &state, sizeof(state));
    // digitalWrite(4, state);
    if (state) {
      ERa.virtualWrite(V0, 1);
      ERA_LOG(ERA_PSTR("ERa"), ERA_PSTR("Control has changed after startup!"));
    }

    /* Setup timer called function every second */
    ERa.addInterval(1000L, timerEvent);
}

void loop() {
    ERa.run();
}