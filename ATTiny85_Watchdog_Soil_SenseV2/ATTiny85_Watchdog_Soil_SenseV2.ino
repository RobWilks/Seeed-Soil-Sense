// modified for 10 minute update interval
// flash with UNO board using Arduino as ISP
// clock speed = INTERNAL 8MHz
// flash boot fuses if a new chip
// 10PU variant of ATTiny 85 can operate at lower voltage.  Standby current ~90microA

#include <avr/sleep.h>
#include <avr/wdt.h>

const int V_REQ_PIN = 0;  // PB0 (Pin 5) - Request Voltage Signal to ESP32
const int GATE_PIN = 1;   // PB1 (Pin 6) - Output to NPN Base
const int DONE_PIN = 2;   // PB2 (Pin 7) - Input from ESP32 "Finished" signal

const int SLEEP_ITERATIONS = 75; // ~10 minute interval
const int VOLT_REQ_INTERVAL = 3; // 6 cycles = 60 minutes

// const int SLEEP_ITERATIONS = 7; // ~56 sec interval
// const int VOLT_REQ_INTERVAL = 1; // every minute

volatile int watchdog_counter = 0;
volatile bool trigger_power = false;
uint16_t cycle_count = 0; // Tracks total power-on cycles for voltage timing

// Watchdog Interrupt Service Routine
ISR(WDT_vect) {
  watchdog_counter++;
  if (watchdog_counter >= SLEEP_ITERATIONS) {
    trigger_power = true;
    watchdog_counter = 0;
  }
}

void setup() {
  pinMode(GATE_PIN, OUTPUT);
  pinMode(DONE_PIN, INPUT);
  pinMode(V_REQ_PIN, OUTPUT); // Configure the request pin
  
  digitalWrite(GATE_PIN, LOW); // Ensure power is OFF initially
  digitalWrite(V_REQ_PIN, LOW); // Ensure request is LOW initially

  setup_watchdog();
}

void loop() {
  if (trigger_power) {
    // 1. Determine if this cycle requires a voltage measurement
    if (cycle_count == 0 || cycle_count >= VOLT_REQ_INTERVAL) {
      digitalWrite(V_REQ_PIN, HIGH); // Signal ESP32 to measure voltage
      cycle_count = 1; // Reset cycle counter
    } else {
      cycle_count++;
    }

    // 2. Turn on the 7.4V rail for the ESP32 
    digitalWrite(GATE_PIN, HIGH);

    // 3. Wait for ESP32 to signal "Done" via GPIO25 
    unsigned long startTime = millis();
    delay(1000); //time for sensor and esp32 GPIO output pins to settle
      while (digitalRead(DONE_PIN) == LOW && (millis() - startTime < 30000)) {
      delay(10);
    }

    // 4. Power down and reset signals
    digitalWrite(GATE_PIN, LOW);
    digitalWrite(V_REQ_PIN, LOW); // Reset voltage request for next cycle
    trigger_power = false;
  }

  enter_sleep();
}

void setup_watchdog() {
  MCUSR &= ~(1 << WDRF); // Clear Watchdog Reset Flag
  WDTCR |= (1 << WDCE) | (1 << WDE); // Enable configuration mode
  WDTCR = (1 << WDP3) | (1 << WDP0); // Set interval to 8 seconds
  WDTCR |= (1 << WDIE); // Enable WDT Interrupt mode
}

void enter_sleep() {
  ADCSRA &= ~(1 << ADEN); // Disable ADC to save power
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  
  // Disable Brown-out Detection during sleep (ATtiny85 specific)
  MCUCR |= (1 << BODS) | (1 << BODSE);
  MCUCR &= ~(1 << BODSE);
  
  sleep_cpu(); // Sleep here
  
  // --- Wakes up here ---
  sleep_disable();
  ADCSRA |= (1 << ADEN); // Re-enable ADC
}