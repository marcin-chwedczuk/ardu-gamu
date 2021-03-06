#include <openGLCD.h>


#define SNES_CLK A5
#define SNES_LATCH 12
#define SNES_DATA 13

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

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Game uses 4x4 pixel blocks and 2 pixel thick border
#define COLS 31
#define ROWS 15
#define BLOCK_SIZE 4
#define BORDER_SIZE 2
uint8_t front_buffer[COLS][ROWS] = {0};
uint8_t back_buffer[COLS][ROWS] = {0};

#define EMPTY WHITE
#define FILLED BLACK
#define BORDER 2
#define DOTTED 3

#define PALLET_WIDTH 11
#define PALLET_ROW 13
int8_t pallet_pos = 0;

#define BULLET_TRAIL 5
int8_t bullet_x[BULLET_TRAIL] = {0};
int8_t bullet_y[BULLET_TRAIL] = {0};

#define BLOCKS_BORDER 5
#define BLOCKS_H_BORDER 1
#define BLOCKS_WIDTH (COLS - 2*BLOCKS_BORDER)
#define BLOCKS_HEIGHT 4
bool blocks[BLOCKS_WIDTH][BLOCKS_HEIGHT] = {0};

int8_t bullet_x_delta = 1;
int8_t bullet_y_delta = -1;

void setup() {
  GLCD.Init();

  pinMode(SNES_CLK, OUTPUT);
  pinMode(SNES_LATCH, OUTPUT);
  pinMode(SNES_DATA, INPUT);

  digitalWrite(SNES_LATCH, LOW);
  digitalWrite(SNES_CLK, LOW);
  
  init_game();

  Serial.begin(115200);
  Serial.println("initialization completed");
}

void init_game_screen() {
  clear_buffer();
  clear_back_buffer();

  GLCD.ClearScreen();
    // render frame 2pixel thick
  GLCD.DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  GLCD.DrawRect(1, 1, SCREEN_WIDTH-2, SCREEN_HEIGHT-2);
  
}

void init_game() {
  init_game_screen();
  pallet_pos = (COLS - PALLET_WIDTH) / 2;
  
  for (int i = 0; i < BULLET_TRAIL; i++) {
    bullet_x[i] = pallet_pos + PALLET_WIDTH/2;
    bullet_y[i] = PALLET_ROW-1;
  }

  for (int i = 0; i < BLOCKS_WIDTH; i++) {
    for (int j = 0; j < BLOCKS_HEIGHT; j++) {
      blocks[i][j] = true;
    }
  }

  bullet_x_delta = 1;
  bullet_y_delta = -1;
}

void render_buffer() {
  for (uint8_t c = 0; c < COLS; c++) {
    for (uint8_t r = 0; r < ROWS; r++) {
      uint8_t f = front_buffer[c][r];
      
      if (back_buffer[c][r] != f) {
        if (f != BORDER && f != DOTTED) {
          GLCD.FillRect(
            BORDER_SIZE + BLOCK_SIZE*c,
            BORDER_SIZE + BLOCK_SIZE*r,
            BLOCK_SIZE, BLOCK_SIZE,
            f);
        }
        else if (f == BORDER) {
          GLCD.FillRect(
            BORDER_SIZE + BLOCK_SIZE*c,
            BORDER_SIZE + BLOCK_SIZE*r,
            BLOCK_SIZE, BLOCK_SIZE,
            WHITE);

          // Top line
          GLCD.DrawHLine(
            BORDER_SIZE + BLOCK_SIZE*c,
            BORDER_SIZE + BLOCK_SIZE*r,
            BLOCK_SIZE, BLACK);

          // Bottom line
          GLCD.DrawHLine(
            BORDER_SIZE + BLOCK_SIZE*c,
            BORDER_SIZE + BLOCK_SIZE*r + BLOCK_SIZE-1,
            BLOCK_SIZE, BLACK);
            
        }
        else {
          GLCD.FillRect(
            BORDER_SIZE + BLOCK_SIZE*c,
            BORDER_SIZE + BLOCK_SIZE*r,
            BLOCK_SIZE, BLOCK_SIZE,
            BLACK);

          GLCD.FillRect(
            BORDER_SIZE + BLOCK_SIZE*c + BLOCK_SIZE/4,
            BORDER_SIZE + BLOCK_SIZE*r + BLOCK_SIZE/4,
            BLOCK_SIZE/2, BLOCK_SIZE/2,
            WHITE);
        }
        
        back_buffer[c][r] = f;
      }
    }
  }
}

void render_blocks() {
  for (int i = 0; i < BLOCKS_WIDTH; i++) {
    for (int j = 0; j < BLOCKS_HEIGHT; j++) {
       front_buffer[i+BLOCKS_BORDER][j+BLOCKS_H_BORDER] = blocks[i][j] ? DOTTED : EMPTY;
    }
  }
}

void clear_buffer() {
  for (uint8_t c = 0; c < COLS; c++) {
    for (uint8_t r = 0; r < ROWS; r++) {
      front_buffer[c][r] = EMPTY;
    }
  }
}

void clear_back_buffer() {
  // TODO: Extract common function
  for (uint8_t c = 0; c < COLS; c++) {
    for (uint8_t r = 0; r < ROWS; r++) {
      back_buffer[c][r] = EMPTY;
    }
  }
}

void swap_buffer() {
  render_buffer();
  clear_buffer();
}

void render_pallete() {
  for (uint8_t c = 0, pos = pallet_pos; c < PALLET_WIDTH; c++, pos++) {
    if (pos >= COLS) break; // error - should not happen
    front_buffer[pos][PALLET_ROW] = (c & 1) ? BORDER : FILLED;
  }
}

void render_bullet() {
  for (int8_t c = 0; c < BULLET_TRAIL; c++) {
    front_buffer[bullet_x[c]][bullet_y[c]] = FILLED;
  }
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


int8_t update_bullet = 0;

#define GS_PLAYING 0
#define GS_ENDSCREEN 1
#define GS_PAUSE 2
int8_t gamestate = GS_PLAYING;

void shift_bullet_trail() {
  for (int c = BULLET_TRAIL - 1; c > 0; c--) {
    bullet_x[c] = bullet_x[c-1];
    bullet_y[c] = bullet_y[c-1];
  }
}

void render_end_text(String text1, String text2) {
    GLCD.ClearScreen();
    GLCD.DrawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5);
    //GLCD.DrawRoundRect(2, 2, SCREEN_WIDTH-4, SCREEN_HEIGHT-4, 5);
    GLCD.SelectFont(Roosewood26); 
    GLCD.DrawString(text1.c_str(), gTextfmt_center, 1);  
    GLCD.DrawString(text2.c_str(), gTextfmt_center, 23);  

    GLCD.SelectFont(Iain5x7);
    GLCD.DrawString("Press START to try again...", 8, 52);
}

void render_game_over() {
  render_end_text("GAME", "OVER");
}

void render_you_won() {
  render_end_text("YOU", "WON");
}

bool bullet_on_pallet() {
  if (bullet_y[0] != PALLET_ROW) return false;

  // on the pallet
  if (bullet_x[0] >= pallet_pos && bullet_x[0] < pallet_pos + PALLET_WIDTH) {
    return true;
  }

  // collision logic pushed ball onto the wall
  if (pallet_pos == 0 && bullet_x[0] == -1) {
    return true;
  }

  if (pallet_pos == ((COLS-1)-PALLET_WIDTH+1) && bullet_x[0] == COLS) {
    return true;
  }

  return false;
}

bool detect_collision() {
  // collision with pallete
  if (bullet_on_pallet()) {
    return true;
  }

  // collision with blocks
  int8_t x = bullet_x[0];
  int8_t y = bullet_y[0];

  // to block coords
  x -= BLOCKS_BORDER;
  y -= BLOCKS_H_BORDER;

  if (x >= BLOCKS_WIDTH || y >= BLOCKS_HEIGHT) return false;

  if (blocks[x][y]) {
    blocks[x][y] = false;
    return true;
  }

  return false;
}

bool is_winner() {
  for (int i = 0; i < BLOCKS_WIDTH; i++) {
    for (int j = 0; j < BLOCKS_HEIGHT; j++) {
      if (blocks[i][j]) return false;
    }
  }

  return true;
}

void loop() {
  delay(17);
  uint16_t keys = read_snes();

  if (gamestate == GS_ENDSCREEN) {
    if (keys & SNES_START) {
      gamestate = GS_PLAYING;
      init_game();
    }
    return;
  }

  if (gamestate == GS_PAUSE) {
    if (keys & SNES_START) {
      gamestate = GS_PLAYING;
      init_game_screen();
    }
    return;
  }
  
  int8_t pallet_boost = 0;
  if (keys & SNES_A) {
    pallet_pos = min(pallet_pos+2, (COLS-1)-PALLET_WIDTH+1);
    pallet_boost = 3;
  }
  else if (keys & SNES_Y) {
    pallet_pos = max(pallet_pos-2, 0);
    pallet_boost = -3;
  }
  else if (keys & SNES_SELECT) {
    gamestate = GS_PAUSE;
    render_end_text("PAUSE", "...");
    return;
  }

  shift_bullet_trail();
  
  bullet_x[0] += bullet_x_delta;
  bullet_y[0] += bullet_y_delta;

  if (bullet_y[0] == ROWS) {
    render_game_over();
    gamestate = GS_ENDSCREEN;
    return;
  }

  if (bullet_on_pallet()) {
    bullet_x[0] += pallet_boost;
  }
  else {
    pallet_boost = 0;
  }

  bool coll = detect_collision();
  if (bullet_y[0] < 0 || bullet_y[0] >= ROWS || coll) {
    bullet_y_delta = -bullet_y_delta;
    bullet_y[0] += bullet_y_delta;
  }

  coll = detect_collision();
  if (bullet_x[0] < 0 || bullet_x[0] >= COLS || coll) {
    bullet_x_delta = -bullet_x_delta;
    bullet_x[0] += bullet_x_delta - pallet_boost;
  }

  if (is_winner()) {
    gamestate = GS_ENDSCREEN;
    render_you_won();
    return;
  }

  
  render_blocks();
  render_pallete();
  render_bullet();
  swap_buffer();
}
