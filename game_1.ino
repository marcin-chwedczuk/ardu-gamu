
#define SNES_CLK 4
#define SNES_LATCH 2
#define SNES_DATA 3

#define SNES_B        0b0000000000000001u
#define SNES_Y        0b0000000000000010u
#define SNES_SELECT   0b0000000000000100u
#define SNES_START    0b0000000000001000u
#define SNES_UP       0b0000000000010000u
#define SNES_DOWN     0b0000000000100000u
#define SNES_LEFT     0b0000000001000000u
#define SNES_RIGHT    0b0000000010000000u

#define SNES_A        0b0000000100000000u
#define SNES_X        0b0000001000000000u
#define SNES_L        0b0000010000000000u
#define SNES_R        0b0000100000000000u

#define SNES_ERROR    0b1111111111111111u

void setup() {
  // put your setup code here, to run once:

  pinMode(SNES_CLK, OUTPUT);
  pinMode(SNES_LATCH, OUTPUT);
  pinMode(SNES_DATA, INPUT);

  digitalWrite(SNES_LATCH, LOW);
  digitalWrite(SNES_CLK, LOW);

  Serial.begin(115200);
  Serial.println("initialization completed");
}

uint16_t read_snes() {
  const int PULSE_WIDTH_MS = 4;
  const uint16_t ZEROS_BLOCK = 0b1111000000000000u;

  uint16_t state = 0;

  // Pulse latch line
  digitalWrite(SNES_LATCH, HIGH);
  delay(PULSE_WIDTH_MS);
  digitalWrite(SNES_LATCH, LOW);

  // Read data
  for (uint16_t i = 0, pos = 1; i < 16; i++, pos <<= 1) {
    int bit = digitalRead(SNES_DATA);
    state |= (bit ? pos : 0);

    // Pulse clock line
    digitalWrite(SNES_CLK, HIGH);
    delay(PULSE_WIDTH_MS);
    digitalWrite(SNES_CLK, LOW);
  }

  // Reverse bits to 1 signify pushed button
  state = ~state;

  // 4 MSB bits have to be zeros
  if ((state & ZEROS_BLOCK) != 0) {
    return SNES_ERROR;
  }

  return state;
}

String nes_state(uint16_t value) {
  static uint16_t bits[] = {
    SNES_B, SNES_Y, SNES_START, SNES_SELECT,
    SNES_UP, SNES_DOWN, SNES_LEFT, SNES_RIGHT,
    SNES_A, SNES_X, SNES_L, SNES_R
    
  };

  static String descriptions[] = {
    "B", "Y", "START", "SELECT",
    "UP", "DOWN", "LEFT", "RIGHT",
    "A", "X", "L", "R"
  };

  String state_str = "";

  for (int i = 0; i < sizeof(bits) / sizeof(*bits); i++) {
    if (value & bits[i]) {
      state_str += " " + descriptions[i];
    }
  }

  return state_str;
}

String to_bin16(uint16_t value) {
  String s = "";

  uint16_t bit = 1u << 15;
  while (bit != 0) {
    s += (value & bit) ? "1" : "0";
    bit >>= 1;
  }

  return s;
}

void loop() {

  // put your main code here, to run repeatedly:
  uint16_t prev = 0;

  while (true) {
    uint16_t new_state = read_snes();
    if (prev != new_state) {
      prev = new_state;
      //String bits = to_bin16(new_state);
      Serial.print("RECV: "); Serial.println(nes_state(new_state));
    }

    //delay(100);
  }
}
