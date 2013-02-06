#include <FastSPI_LED.h>

#define ROWS 13
#define COLS 13
#define NUM_LEDS ROWS*COLS

// Sometimes chipsets wire in a backwards sort of way
struct CRGB { unsigned char g; unsigned char r; unsigned char b; };
// struct CRGB { unsigned char r; unsigned char g; unsigned char b; };
struct CRGB *leds;

#define PIN 4

static unsigned char state[ROWS][COLS];
static unsigned char next_state[ROWS][COLS];

static void set_led(int row, int col, unsigned char life) {
  struct CRGB *led;
  if ((row % 2) == 1) {
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
    case 255:
      led->r = 160;
      led->g = 160;
      led->b = 160;
      break;
    default:
      led->r = 0;
      led->g = 0;
      led->b = 160;
      break;
  }
}

static void gol_init(){
  for(int row = 0; row < ROWS; ++row) {
    for(int col = 0; col < COLS; ++col) {
      int r = random(0,2);
      if (r%2)
        state[row][col] = random(1,255);
      else
        state[row][col] = 0;
    }
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
  
  randomSeed(analogRead(1));
  
  memset(state, 0, NUM_LEDS);
  memset(next_state, 0, NUM_LEDS);
  gol_init();
}

static int gol_num_neighbors(int row, int col){
  int count = 0;
  for(int i = row-1; i <= row+1; ++i){
    for(int j = col-1; j <= col+1; j++){
      if (i < 0 || j < 0)
        continue;
      if (i >= ROWS || j >= COLS)
        continue;
      if (i == row && j == col)
        continue;
      if (state[i][j])
        ++count;
    }
  }
  return count;
}
                          
static void gol_handle(int row, int col){
  int neighbor_count = gol_num_neighbors(row, col);
  if (!state[row][col]){
    if (neighbor_count == 3)
      next_state[row][col] = 1;
    else
      next_state[row][col] = 0;
    return;
  }

  if (neighbor_count < 2 || neighbor_count > 3)
    next_state[row][col] = 0;
  else {
    next_state[row][col] = state[row][col]+1;
    if(!next_state[row][col])
      next_state[row][col] = 255;
  }
}

static void gol_draw(){
  for(int row = 0; row < ROWS; ++row) {
    for(int col = 0; col < COLS; ++col) {
      set_led(row, col, state[row][col]);
    }
  }
  FastSPI_LED.show();
  memcpy(state, next_state, NUM_LEDS);
  delay(analogRead(0));
}

void loop() { 
  for(int row = 0; row < ROWS; ++row) {
    for(int col = 0; col < COLS; ++col) {
      gol_handle(row, col);
    }
  }
  gol_draw();
}

