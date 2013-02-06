#include <FastSPI_LED.h>

#define ROWS 13
#define COLS 13
#define NUM_LEDS ROWS*COLS

#define WATCHDOG_ROUNDS 10
#define WATCHDOG_DELAY 10

#define MAX_LIFE 30

//#define WRAP

// Sometimes chipsets wire in a backwards sort of way
struct CRGB { unsigned char g; unsigned char r; unsigned char b; };
// struct CRGB { unsigned char r; unsigned char g; unsigned char b; };
struct CRGB *leds;

#define PIN 4

static signed char state[ROWS][COLS];
static signed char next_state[ROWS][COLS];
static signed char save_state[ROWS][COLS];

static void set_led(int row, int col, signed char life) {
  struct CRGB *led;
  if (row % 2) {
    led = &leds[row*COLS + (COLS - (col + 1))];
  } else {
    led = &leds[row*COLS + col];
  }
  
  switch(life){
    case 0:
      led->r = 0;
      led->g = 0;
      led->b = 0;
      break;
    case 1:
      led->r = 0;
      led->g = 160;
      led->b = 0;
      break;
    case 2:
      led->r = 160;
      led->g = 0;
      led->b = 0;
      break;
    case MAX_LIFE:
      led->r = 160;
      led->g = 160;
      led->b = 160;
      break;
    default:
      if (life < 0) {
        led->r = led->r/2;
        led->g = led->g/2;
        led->b = led->b/2;
        break;
      }
      if (life > MAX_LIFE-5){
        led->r = 255;
        led->g = 255;
        led->b = 255;
        break;
      }
      led->r = (160*life)/MAX_LIFE;
      led->g = (160*life)/MAX_LIFE;
      led->b = 160;
      break;
  }
}

static boolean is_dead(signed char state) {
  return (state <= 0);
}

static void gol_init(){
  for(int row = 0; row < ROWS; ++row) {
    for(int col = 0; col < COLS; ++col) {
      int r = random(0,2);
      if (r%2)
        state[row][col] = random(1,MAX_LIFE);
      else
        state[row][col] = 0;
    }
  }
  memcpy(save_state, state, NUM_LEDS);
}

static void watchdog() {
  static int diff_count = 0;
  static bool reset = false;

  if(reset) {
    static int reset_delay = 0;
    ++reset_delay;
    if(reset_delay >= WATCHDOG_DELAY){
      reset_delay = 0;
      reset = false;
      diff_count = 0;
      gol_init();
      return;
    }
  }

  if(!memcmp(save_state, next_state, NUM_LEDS)) {
    reset = true;
    return;
  }

  ++diff_count;
  if(diff_count >= WATCHDOG_ROUNDS) {
    diff_count = 0;
    memcpy(save_state, next_state, NUM_LEDS);
  }
}

void setup()
{
  FastSPI_LED.setLeds(NUM_LEDS);
  FastSPI_LED.setChipset(CFastSPI_LED::SPI_TM1809);

  FastSPI_LED.setPin(PIN);
  
  FastSPI_LED.init();
  FastSPI_LED.start();
  
  leds = (struct CRGB*)FastSPI_LED.getRGBData();
  FastSPI_LED.show();
  
  randomSeed(analogRead(0));
  
  memset(state, 0, NUM_LEDS);
  memset(next_state, 0, NUM_LEDS);
  memset(save_state, 0, NUM_LEDS);
  gol_init();
}

static int gol_num_neighbors(int row, int col){
  int count = 0;
  for(int i = row-1; i <= row+1; ++i){
    for(int j = col-1; j <= col+1; ++j){
#ifndef WRAP
      if (i < 0 || j < 0)
        continue;
      if (i >= ROWS || j >= COLS)
        continue;
#endif
      if (i == row && j == col)
        continue;
      if (!is_dead(state[i][j]))
        ++count;
    }
  }
  return count;
}
                          
static void gol_handle(int row, int col){
  int neighbor_count = gol_num_neighbors(row, col);
  if (is_dead(state[row][col])){
    if (neighbor_count == 3)
      next_state[row][col] = 1;
    else
      next_state[row][col] = state[row][col];
    return;
  }

  if (neighbor_count < 2 || neighbor_count > 3)
    next_state[row][col] = -1 * state[row][col];
  else {
    next_state[row][col] = state[row][col]+1;
    if(next_state[row][col] > MAX_LIFE)
      next_state[row][col] = MAX_LIFE;
  }
}

static void gol_draw(){
  for(int row = 0; row < ROWS; ++row) {
    for(int col = 0; col < COLS; ++col) {
      set_led(row, col, state[row][col]);
    }
  }
  FastSPI_LED.show();
  delay(analogRead(0));
  for(int row = 0; row < ROWS; ++row) {
    for(int col = 0; col < COLS; ++col) {
      set_led(row, col, state[row][col]);
    }
  }
  FastSPI_LED.show();
  delay(analogRead(0));
  for(int row = 0; row < ROWS; ++row) {
    for(int col = 0; col < COLS; ++col) {
      set_led(row, col, state[row][col]);
    }
  }
  FastSPI_LED.show();
  delay(analogRead(0));
  memcpy(state, next_state, NUM_LEDS);
  watchdog();
}

void loop() { 
  for(int row = 0; row < ROWS; ++row) {
    for(int col = 0; col < COLS; ++col) {
      gol_handle(row, col);
    }
  }
  gol_draw();
}

