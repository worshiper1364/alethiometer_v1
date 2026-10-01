/*
 * =====================================================================================
 *  arduino_alethiometer.ino
 *
 *  Purpose: Drives the divination needle of an alethiometer. An Arduino Nano (or similar
 *  clone) turns an X27.168 stepper motor whose internal end-stop has been cut out so it
 *  can spin a full 360 deg. Adapted from Myth Made's original alethiometer sketch.
 *
 *  Before powering on, rest the needle on the center of any symbol.
 *
 *  How it works: touching the sensor starts a "reading". The needle swings a random
 *  number of symbols one way or the other, pauses, sometimes quivers, and repeats 2 to 6
 *  times. Every swing is a whole number of symbols (20 motor steps each), so if the
 *  needle starts centered on a symbol and the motor completes every step, it stops on a
 *  symbol center. Nothing checks this (open-loop control), so a skipped step, a
 *  knock or a needle slipping on its shaft will throw it off until it's re-centered.
 *
 * =====================================================================================
 */


// =====================================================================================
// 1. HARDWARE: pins and the step pattern table
// =====================================================================================

// The four motor wires. D4 and D5 connect to one coil (electromagnet), D6 and D7 to the
// other. See README "Wiring".
// The motor is driven straight from these pins, with no driver chip.
const uint8_t motorPins[4] = {4, 5, 6, 7};
const uint8_t buttonPin    = 2;          // touch sensor output: HIGH while touched

// The pin patterns that turn the motor, one row per step. Each row sets D4, D5, D6, D7
// high (1) or low (0). Moving down the table turns the motor one way, moving up turns it
// the other way, and after row 3 it wraps back to row 0. From one row to the next, the
// current flips direction in just one coil. README "How the Nano drives the motor"
// explains why that makes the motor turn
// (In the notes below, + and - are the two directions of current through a coil.)
const uint8_t stepSequence[4][4] = {
  {1, 0, 0, 1},   // row 0: coil 1 +, coil 2 -
  {1, 0, 1, 0},   // row 1: coil 1 +, coil 2 +
  {0, 1, 1, 0},   // row 2: coil 1 -, coil 2 +
  {0, 1, 0, 1}    // row 3: coil 1 -, coil 2 -
};


// =====================================================================================
// 2. GEOMETRY: steps, degrees and symbols
// =====================================================================================

// The motor's internal gears turn each step into half a degree at the needle.
const int   STEPS_PER_REV  = 720;                       // steps per full turn of the needle
const int   NUM_TILES      = 36;                        // symbols on the dial
const int   STEPS_PER_TILE = STEPS_PER_REV / NUM_TILES; // 20 steps = 10 deg per symbol
const float DEG_PER_STEP   = 360.0 / STEPS_PER_REV;     // 0.5 deg


// =====================================================================================
// 3. FEEL: numbers you can tune to change how the needle moves
// =====================================================================================

const float PEAK_SPEED_DEG_S      = 110.0; // top speed mid-swing. The motor can start and stop
                                           // instantly only below about 250 deg/s, so stay well under
const unsigned long MIN_SWING_MS  = 750;   // even short hops take this long, so they glide
const int   MIN_HOP_TILES         = 3;     // shortest swing: 3 symbols = 30 deg
const int   MAX_HOP_TILES         = 18;    // longest swing: 18 symbols = 180 deg, never the long way round
const int   REVERSE_PERCENT       = 70;    // chance each new swing reverses direction
const unsigned long PAUSE_MIN_MS  = 1500;  // pause on each symbol, chosen at random
const unsigned long PAUSE_MAX_MS  = 3000;  //   between these two values
const int   QUIVER_STEPS          = 6;     // visible quiver size: 6 steps = 3 deg
const unsigned long QUIVER_MS     = 130;   // duration of each half of one quiver
const unsigned long MIN_STEP_US   = 1500;  // safety limit: never step faster than this
                                           // (1 step per 1.5 ms = 667 steps/s = 333 deg/s)


// =====================================================================================
// 4. GEAR SLACK
// =====================================================================================

// Gears have a little play between their teeth, so when the motor changes direction the
// needle lags very slightly. To land in the same spot every time, every stop
// finishes with a forward (+) move: forward swings already arrive that way, and backward
// swings go slightly past and then settle forward. This only covers slack up to
// BACKLASH_STEPS + TAKEUP_MARGIN_STEPS; it can't fix more slack than that, or a needle
// slipping on its shaft.
const int   BACKLASH_STEPS        = 1;     // rough estimate from the 'b' test (1 step = 0.5 deg)
const int   TAKEUP_MARGIN_STEPS   = 4;     // extra overshoot beyond the slack on backward moves
const unsigned long SETTLE_MS     = 350;   // time for the small settle back onto a symbol


// =====================================================================================
// 5. MOTOR STATE
// =====================================================================================

uint8_t phase   = 0;       // which row of stepSequence is being output (0 to 3)
bool    coilsOn = false;   // are the coils currently powered?


// =====================================================================================
// 6. MOTOR DRIVE: the only code that writes to the motor pins
// =====================================================================================

// Output the current row's pattern. Each coil is wired between two pins: one high and
// one low pushes current through it one way, and swapping them pushes it the other way.
// If the coils were off, pause 30 ms so the motor's magnet can settle into position
// before stepping. That settling can shift a hand-placed needle slightly. The motor has
// one magnetic "notch" for each 2 deg of needle travel, so the shift should be at most
// about 1 deg, plus a little gear slack. Nudging the needle back by hand won't stick:
// the motor pulls it to the same notch next time the coils switch on.
void energise() {
  const uint8_t *pattern = stepSequence[phase];
  for (uint8_t i = 0; i < 4; i++) digitalWrite(motorPins[i], pattern[i]);
  if (!coilsOn) { coilsOn = true; delay(30); }
}

// Set all four pins low: no current in either coil, so nothing heats up. The motor no
// longer actively holds its position. Friction in the gears and the magnet's pull on
// the motor's steel usually keep the needle still, but a knock can move it.
void release() {
  for (uint8_t i = 0; i < 4; i++) digitalWrite(motorPins[i], LOW);
  coilsOn = false;
}

// Take one step forward (dir = +1) or backward (dir = -1): move to the next or previous
// row and output it. "& 3" wraps the row number round, so after row 3 comes row 0 and
// before row 0 comes row 3. (It keeps only the last two binary digits of the number.)
void stepOnce(int dir) {
  phase = (phase + dir) & 3;
  energise();
}

// Work out how long a swing should take so its top speed is PEAK_SPEED_DEG_S. A smooth
// S-curve move reaches twice its average speed at the midpoint, so
// time = 2 x distance / top speed. (The 2000 is 2 x 1000 milliseconds per second.)
// Short hops are stretched to MIN_SWING_MS.
unsigned long swingMs(long steps) {
  float deg = labs(steps) * DEG_PER_STEP;
  unsigned long ms = (unsigned long)(2000.0 * deg / PEAK_SPEED_DEG_S);
  return (ms < MIN_SWING_MS) ? MIN_SWING_MS : ms;
}

// Move 'delta' steps (positive one way, negative the other) over 'durationMs': start
// slowly, speed up, then slow to a gentle stop. This uses a smooth sine-based
// profile (known as a cycloidal profile); plotted as position against time it makes an
// S shape, hence "S-curve". A sudden start could make the motor skip steps, and nothing
// would notice, because the code never checks where the needle really is; it just
// counts the steps it sends.
//
// Instead of pausing a fixed time between steps, the loop keeps checking the clock and
// asks "how many steps should be done by now?", taking a step whenever it's behind,
// but never faster than MIN_STEP_US. With the default settings, steps are always at
// least about 4 ms apart, so that limit never kicks in. If you raise the speeds enough
// to hit it, the needle lags behind the curve and catches up, so moves get slightly
// longer than planned.
void sCurveMove(long delta, unsigned long durationMs) {
  if (delta == 0) return;
  int  dir   = (delta > 0) ? 1 : -1;
  long total = labs(delta);              // number of steps to take
  energise();                            // make sure the coils are on

  float T = durationMs * 1000.0;         // duration in microseconds
  // micros() counts microseconds since power-up and wraps back to zero about every
  // 70 minutes. Subtracting unsigned numbers ("now - t0") still gives the right answer
  // when that happens, which is why these are unsigned long.
  unsigned long t0 = micros();
  unsigned long lastStep = t0 - MIN_STEP_US;   // allows a first step straight away
  long done = 0;

  while (done < total) {
    unsigned long now = micros();
    float u = (now - t0) / T;            // fraction of the move's time used so far, 0 to 1
    if (u > 1.0) u = 1.0;
    // The S-curve: fraction of steps done = u - sin(2 pi u) / (2 pi), scaled up to steps
    // and rounded (the + 0.5). The Nano has no hardware for decimal maths, so this is
    // worked out in software, but it's still quick compared with the gap between steps.
    long want = (long)(total * (u - sin(TWO_PI * u) / TWO_PI) + 0.5);
    // Step if behind the curve, but never faster than the safety limit.
    if (want > done && (now - lastStep) >= MIN_STEP_US) {
      stepOnce(dir);
      done++;
      lastStep = now;
    }
  }
  delay(20);  // let the final step physically finish before anything else happens
}

// Move by 'delta', always finishing with a forward move (see section 4). A forward move
// goes straight there. A backward move goes slightly past the target, pauses, then
// settles forward onto it.
void settleMove(long delta, unsigned long durationMs) {
  if (delta >= 0) { sCurveMove(delta, durationMs); return; }
  long extra = BACKLASH_STEPS + TAKEUP_MARGIN_STEPS;
  sCurveMove(delta - extra, durationMs);
  delay(120);
  sCurveMove(extra, SETTLE_MS);
}

// Swing a whole number of symbols: positive one way, negative the other. Keeping every
// move a whole number of symbols is what keeps a hand-centered needle landing on symbol
// centers, as long as the motor doesn't miss steps (README "Using it").
void swingTiles(int tiles) {
  long delta = (long)tiles * STEPS_PER_TILE;
  settleMove(delta, swingMs(delta));
}

// One small tremble on the current symbol: back a little, then forward onto it again.
// The backward part is enlarged by the gear slack so the visible size stays
// QUIVER_STEPS, and it finishes with a forward move so the needle lands exactly where it
// began.
void quiver() {
  long a = QUIVER_STEPS + BACKLASH_STEPS;
  sCurveMove(-a, QUIVER_MS);
  sCurveMove( a, QUIVER_MS);
}


// =====================================================================================
// 7. DIVINATION: choosing and performing a reading
// =====================================================================================

// How many symbols this reading visits. random(1, 101) picks a whole number from 1 to
// 100. Checking it against rising cut-offs gives each outcome a slice of 1..100 as wide
// as its chance of happening. Weights from Myth Made's original sketch.
int pickNumberOfSymbols() {
  int r = random(1, 101);
  if (r == 1)  return 6;     // 1%
  if (r <= 10) return 5;     // 9%
  if (r <= 24) return 4;     // 14%
  if (r <= 50) return 2;     // 26%
  return 3;                  // 50%
}

// How many quivers at a stop (same technique, using 0..99).
int pickQuivers() {
  int r = random(100);
  if (r < 45) return 0;      // 45%
  if (r < 75) return 1;      // 30%
  if (r < 92) return 2;      // 17%
  return 3;                  // 8%
}

// A full reading: several swings of random length, with pauses and quivers.
void divination() {
  int n   = pickNumberOfSymbols();
  int dir = random(2) ? 1 : -1;          // first swing: either direction, 50/50
  Serial.print(F("Reading, ")); Serial.print(n); Serial.println(F(" symbols:"));

  for (int i = 0; i < n; i++) {
    // After the first swing, usually reverse, so the needle swings back and forth rather
    // than circling like a clock hand.
    if (i > 0 && random(100) < REVERSE_PERCENT) dir = -dir;
    int hop = random(MIN_HOP_TILES, MAX_HOP_TILES + 1);   // how many symbols to travel
    swingTiles(dir * hop);

    int q = pickQuivers();
    for (int k = 0; k < q; k++) { delay(250); quiver(); }

    Serial.print(F("  swing ")); if (dir > 0) Serial.print('+'); Serial.print(dir * hop);
    Serial.print(F(" tiles, quivers ")); Serial.println(q);

    if (i < n - 1) delay(random(PAUSE_MIN_MS, PAUSE_MAX_MS + 1));  // pause (not after the last)
  }
  release();                             // coils off until next time
}


// =====================================================================================
// 8. BENCH TOOLS: Serial Monitor test commands (see README "Serial Monitor bench tools")
// =====================================================================================
// F("...") stores text in the Nano's program memory instead of its small 2 KB working
// memory.

void printHelp() {
  Serial.println(F("Commands: r (reading)  t (tile tour)  s (spin test)  b (backlash test)  x (coils off)  ? (help)"));
}

// A full turn, one symbol at a time, finishing on the starting symbol.
void tileTour() {
  Serial.println(F("Tile tour: every stop should be centered."));
  for (int k = 1; k <= NUM_TILES; k++) {
    swingTiles(1);
    Serial.print(F("  tile ")); Serial.println(k);
    delay(400);
  }
  Serial.println(F("Tour done. Needle should be back where it started, dead center."));
}

// Three full turns one way, then three back. If the needle doesn't return to where it
// started, look for skipped steps, something rubbing, or the needle slipping on its
// shaft. Passing is a good sign, but it doesn't prove every move is perfect.
void spinTest() {
  Serial.println(F("Spin test: 3 turns one way, 3 back..."));
  sCurveMove( 3L * STEPS_PER_REV, swingMs(3L * STEPS_PER_REV));
  delay(800);
  settleMove(-3L * STEPS_PER_REV, swingMs(3L * STEPS_PER_REV));
  Serial.println(F("Done. Needle should be back where it started. If not, look for skipped steps, rubbing or a slipping needle."));
}

// Estimate the gear slack. The quiver leaves the gears pressed forward; then the
// motor steps backward slowly and you note the step where the needle first visibly
// moves. How closely you can see that varies, so treat the result as a rough estimate;
// erring a step high is safe.
void backlashTest() {
  quiver();
  delay(1000);
  Serial.println(F("Backlash test: the motor steps backwards once every 0.8 s."));
  Serial.println(F("Watch the needle tip. Note the step number where it FIRST visibly moves."));
  for (int k = 1; k <= 20; k++) {
    stepOnce(-1);
    Serial.print(F("  step ")); Serial.println(k);
    delay(800);
  }
  sCurveMove(20, 800);   // straight back, finishing with a forward move
  Serial.println(F("Set BACKLASH_STEPS to that number, upload, done."));
}

// Read any characters typed into the Serial Monitor and act on each one.
void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'r': case 'R': divination();   break;
      case 't': case 'T': tileTour();     break;
      case 's': case 'S': spinTest();     break;
      case 'b': case 'B': backlashTest(); break;
      case 'x': case 'X': release(); Serial.println(F("Coils off.")); break;
      case '?':           printHelp();    break;
      default: break;   // ignore newlines, spaces, etc.
    }
  }
}


// =====================================================================================
// 9. SETUP AND LOOP
// =====================================================================================

// Runs once at power-up or reset.
void setup() {
  // Motor pins become outputs, starting low so both coils begin switched off.
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(motorPins[i], OUTPUT);
    digitalWrite(motorPins[i], LOW);
  }
  // The touch sensor sets its own output high or low, so a plain input is enough.
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);

  // random() isn't truly random: it follows a fixed sequence that depends on a starting
  // "seed". Reading an unconnected pin gives a rough seed from electrical noise, and
  // the timing of the first touch is mixed in later (see loop).
  randomSeed(analogRead(A0));

  Serial.println(F("Alethiometer ready. Needle should be resting on the center of a symbol."));
  printHelp();
}

// Runs over and over, forever. Each pass checks for Serial commands and whether the
// touch sensor is HIGH (see README "The touch sensor").
void loop() {
  handleSerial();

  if (digitalRead(buttonPin) == HIGH) {
    // A "static" variable keeps its value between passes of loop(). On the first touch,
    // mix the exact microsecond of the touch into the seed. That makes the readings very
    // likely to differ each time the alethiometer is switched on, though it can't
    // guarantee it.
    static bool seeded = false;
    if (!seeded) { randomSeed(micros() ^ random(0x7FFFFFFFL)); seeded = true; }

    divination();
    delay(500);  // still touching after the reading? Then another reading starts.
  }
}
