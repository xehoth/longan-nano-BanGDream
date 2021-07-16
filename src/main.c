#include "lcd/lcd.h"
#include <string.h>
#include "utils.h"
#include "fatfs/tf_card.h"

void Inp_init();

/**
 * move to risc-v
 */
// void Inp_init() {
//   gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
// }

void Adc_init() {
  rcu_periph_clock_enable(RCU_GPIOA);
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1);
  RCU_CFG0 |= (0b10 << 14) | (1 << 28);
  rcu_periph_clock_enable(RCU_ADC0);
  ADC_CTL1(ADC0) |= ADC_CTL1_ADCON;
}

void IO_init();

/**
 * move to risc-v
 */
// void IO_init() {
//   Inp_init();  // inport init
//   Adc_init();  // A/D init
//   Lcd_Init();  // LCD init
// }

int _put_char(int ch) {
  usart_data_transmit(USART0, (uint8_t)ch);
  while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET) {
  }
  return ch;
}

const int FPS = 30;
const float FRAME_WAIT_TIME = 1000.0 / FPS;
const int MENU_SCENE = 0;
const int MAIN_GAME_SCENE = 1;
const int END_GAME_SCENE = 2;
const int NOTE_SPEED = 3;
const int PERFECT_LINE = 3;
const int GREAT_LINE = 6;
const int GOOD_LINE = 9;
const int MISS_LINE = 12;
const int JUDGE_LINE = 3;
const int PERFECT = 5;
const int GREAT = 3;
const int GOOD = 2;
const int MISS = -1;
const int MAX_NOTE_SAME_TIME = 10;
const int CHECK_COOL_DOWN = 5;
const int MAX_MUSIC_NUM = 3;

typedef union _color {
  struct {
    uint16_t b : 5;
    uint16_t a : 1;
    uint16_t g : 5;
    uint16_t r : 5;
  };
  uint16_t u16;
  uint8_t u8[2];
} Color;

typedef struct _note {
  // int16_t frame;
  int16_t type : 2;
  int16_t frame : 14;
} Note;

Color noteTextureBuffer[560];
Color longTextureBuffer[560];
Color perfectTextureBuffer[500];
Color goodTextureBuffer[350];
Color greatTextureBuffer[370];
Color missTextureBuffer[260];
Color numTextureBuffer[23 * 15];
Note noteBuffer[2][600];

int noteBufferBegin[2], noteBufferEnd[2];
FATFS sdCardFs;  // sd card
FRESULT sdCardFr;
int currentFrame, currentScene;
uint64_t frameStartTime, frameEndTime;
uint64_t globalStartTime;
bool isPressBoot0;
bool isPressButtons[2];
int hitStatus;
int hitAnimationCoolDown = 0;
int checkCoolDown[2];
int changeToScoreScene;
int score;
int perfectCount;
int greatCount;
int goodCount;
int missCount;
int combo;
bool showScoreInGame;
bool autoPlayMode;
int changeShowScoreCoolDown;
const int perfectTexture = 0;
const int noteTexture = 1;
const int goodTexture = 2;
const int greatTexture = 3;
const int missTexture = 4;
const int longTexture = 5;
const int NUMBER_TEXTURE = 300;

const char *texFile[] = {
  "perfect.bin", 
  "note.bin",
  "good.bin",
  "great.bin",
  "miss.bin",
  "long.bin"
};
const int texWidth[] = {
  50, // perfect
  40, // note
  35, // good
  37, // great
  26, // miss
  40, // long
};
const int texHeight[] = {
  10, // perfect
  14, // note
  10, // good
  10, // great
  10, // miss
  14, // long
};

FIL bgTextureFile;
FIL numberTextureFile;

unsigned char image[12800 + 640];

void setPixel(int side, int x, int y, Color color);
int readFile(void *ptr, const char *fileName, size_t size, int offset);
void readFileRaw(FIL *file, void *ptr, size_t size, int offset);
int openFile(FIL *file, const char *fileName);
void renderTexture(int side, int tex, int ox, int oy);
void drawNote(int side, int x);
void init_uart0();
void globalInit();
void showNoSdCard();
void onInit();
void onUpdate();
void onRender();
void updateControls();
void clearControls();
void mainGameStart();
void drawHitStatusAnimation(int side);
void loadNote(int side);
void checkHit();
void fillLongNote(int side, int x, int y1, int y2);
int getCurrentLongNoteStatus(int side, int cur);
void drawPressEffect(int side, int x);
void gameMenu();
void scoreScene();
void showLonganLogo();
void onRelease();
void initTextures();
void drawCombo(int side);
void drawScore(int side);
void mainGameLoop();
void mountSdCard();
void updateAutoPlay();
const char *longanLogoFile = "logo.bin";
char fileNameBuffer[32];
int musicId;
int changeMusicCoolDown = 0;

void mainFuncRiscv();

int main() {
  /**
   * move to risc-v
   */
  mainFuncRiscv();
  // globalInit();
  // for (;;) {
  //   mountSdCard();
  //   if (!sdCardFr) {
  //     onInit();
  //     for (;;) {  
  //       gameMenu();
  //       mainGameLoop();
  //       scoreScene();
  //     }
  //   } else {
  //     showNoSdCard();
  //   }
  // }
  // onRelease();
  return 0;
}

/**
 * move to risc-v
 */
// void globalInit() {
//   IO_init();  // init OLED
//   init_uart0();
// }

/**
 * move to risc-v
 */
const char *NO_CARD_FOUND = "no card found!";
void showNoSdCardC() {
  LCD_ShowString(24, 0, (u8 *)("no card found!"), BLACK);
  LCD_ShowString(24, 16, (u8 *)("no card found!"), BLUE);
  LCD_ShowString(24, 32, (u8 *)("no card found!"), BRED);
  LCD_ShowString(24, 48, (u8 *)("no card found!"), GBLUE);
  LCD_ShowString(24, 64, (u8 *)("no card found!"), RED);
}

void onInitRiscvBefore();
void onInitRiscvAfter();

void onInit() {
  /**
   * move to riscv
   */
  // currentFrame = 0;
  // currentScene = MENU_SCENE;
  // clearControls();
  // LCD_Clear(BLACK);

  // // show longan nano logo
  // showLonganLogo();
  onInitRiscvBefore();

  initTextures();

  printf("reset;\n");

  /**
   * move to risc-v
   */
  // delay_1ms(1500);
  onInitRiscvAfter();
}

void gameMenuRiscv();
void checkAutoPlayMode();
void changeMusic();

void gameMenu() {
  musicId = 0;
  currentFrame = 0;
  currentScene = MENU_SCENE;
  changeMusicCoolDown = 0;
  clearControls();
  printf("reset;\n");
  for (int side = 0; side <= 1; ++side) {
    readFile(image, "menu.bin", 12800, 12800 * side);
    renderFrameBuffer(side);
  }
  printf("bgm0;\n");
  autoPlayMode = FALSE;
  /**
   * move to risc-v
   */
  gameMenuRiscv();
  // delay_1ms(1000);
  // for (;;) {
  //   updateControls();
  //   checkAutoPlayMode();
  //   if (isPressBoot0) {
  //     // start main game
  //     currentScene = MAIN_GAME_SCENE;
  //     currentFrame = 0;
  //     mainGameStart();
  //     break;
  //   }
  //   changeMusic();
  //   clearControls();
  // }
}

void scoreScene() {
  // readFile(image, "menu.bin", sizeof(image), 0);
  // drawCombo(0);
  // LCD_ShowPicture(0, 0, 159, 39);
  // renderFrameBuffer(0);
  // readFile(image, "menu.bin", sizeof(image), 12800);
  // drawCombo(1);
  // LCD_ShowPicture(0, 40, 159, 79);
  // renderFrameBuffer(1);
  // for (;;) {
  //   updateControls();
  //   if (isPressBoot0) {
  //     break;
  //   }
  //   clearControls();
  // }
}

void onUpdate() {
  if (currentScene == MAIN_GAME_SCENE) {
    // printf("frame: %d;\n", currentFrame);
    if (noteBufferBegin[0] == noteBufferEnd[0] && noteBufferBegin[1] == noteBufferEnd[1]) {
      --changeToScoreScene;
    } else {
      changeToScoreScene = 180;
    }
    if (changeShowScoreCoolDown > 0) {
      --changeShowScoreCoolDown;
    }
    updateControls();
    if (isPressBoot0 && changeShowScoreCoolDown <= 0) {
      changeShowScoreCoolDown = 20;
      showScoreInGame = !showScoreInGame;
    }
    if (autoPlayMode)
      updateAutoPlay();
    checkHit();
  }
  if (!autoPlayMode)
    clearControls();
}

void onRender() {
  if (currentScene == MAIN_GAME_SCENE) {
    for (int side = 0; side <= 1; ++side) {
      // readFile(image, "bg.bin", 12800, 12800 * side);
      readFileRaw(&bgTextureFile, image, 12800, 12800 * side);
      if (!autoPlayMode)
        updateControls();
      if (side == 1 && isPressButtons[0]) {
        drawPressEffect(side, 20);
      }
      if (side == 0 && isPressButtons[1]) {
        drawPressEffect(side, 60);
      }
      if (side == 1) {
        drawNote(side, 20);
      } else if (side == 0) {
        drawNote(side, 60);
      }
   
      drawHitStatusAnimation(side);
      drawCombo(side);
      if (showScoreInGame || changeToScoreScene < 150) {
        drawScore(side);
      }
      renderFrameBuffer(side);
      // reader->read(image, "bmp.bin", sizeof(image), currentFrame * 25600);
      // LCD_ShowPicture(0, 0, 159, 79);
    }
    // printf("frame: %d;\n", currentFrame);
    if (autoPlayMode) clearControls();
  }
}

void updateControls() {
  isPressBoot0 = isPressBoot0 || Get_BOOT0();
  isPressButtons[0] = isPressButtons[0] || Get_Button(0);
  isPressButtons[1] = isPressButtons[1] || Get_Button(1);
}

/**
 * move to risc-v
 */
// void clearControls() {
//   isPressBoot0 = FALSE;
//   isPressButtons[0] = isPressButtons[1] = FALSE;
// }

void mainGameStartRiscv();

void mainGameStart() {
  /**
   * move to risc-v
   */
  // perfectCount = 0;
  // greatCount = 0;
  // goodCount = 0;
  // missCount = 0;
  // combo = 0;
  // score = 0;
  // changeToScoreScene = 180;
  // noteBufferBegin[0] = noteBufferBegin[1] = 0;
  // checkCoolDown[0] = checkCoolDown[1] = 0;
  // loadNote(0);
  // loadNote(1);
  mainGameStartRiscv();
  showScoreInGame = FALSE;
  changeShowScoreCoolDown = 30;
  printf("reset;\n");
  globalStartTime = get_timer_value();
  uint64_t tmpTime;
  tmpTime = get_timer_value();
  do {
    frameStartTime = get_timer_value();
  } while (globalStartTime == tmpTime);
  // printf("bad apple;\n");
  printf("bgm%d;\n", musicId);
}

int readFile(void *ptr, const char *fileName, size_t size, int offset) {
  FIL file;
  FRESULT fr = f_open(&file, fileName, FA_READ);
  if (fr) {
    printf("open %s error;\n", fileName);
    return -1;
  }
  if (offset) f_lseek(&file, offset);
  UINT br;
  int ret = file.obj.objsize;
  f_read(&file, ptr, size, &br);
  f_close(&file);
  return ret;
}

void setPixel(int side, int x, int y, Color color) {
  if (!side) {
    if (color.a && 80 - x - 1 < 40 && y < 160 && x >= 0 && y >= 0) {
      image[((80 - x - 1) * 160 + (160 - 1 - y)) * 2 + 0] = color.u8[0];
      image[((80 - x - 1) * 160 + (160 - 1 - y)) * 2 + 1] = color.u8[1];
      // ((uint16_t *)image)[(80 - x - 1) * 160 + (160 - 1 - y)] = color.u16;
    }
  } else {
    if (color.a && x >= 0 && x < 40 && y < 160 && y >= 0) {
      image[((40 - x - 1) * 160 + (160 - 1 - y)) * 2 + 0] = color.u8[0];
      image[((40 - x - 1) * 160 + (160 - 1 - y)) * 2 + 1] = color.u8[1];
    }
  }
}

void renderTexture(int side, int tex, int ox, int oy) {
  // readFile(textureBuffer, texFile[tex], texWidth[tex] * texHeight[tex] * sizeof(Color), 0);
  int width, height;
  if (tex < sizeof(texWidth) / sizeof(texWidth[0])) {
    width = texWidth[tex];
    height = texHeight[tex];
  } else {
    width = 15;
    height = 23;
  }
  Color *buf = perfectTextureBuffer;
  if (tex == perfectTexture) {
    buf = perfectTextureBuffer;
  } else if (tex == goodTexture) {
    buf = goodTextureBuffer;
  } else if (tex == greatTexture) {
    buf = greatTextureBuffer;
  } else if (tex == missTexture) {
    buf = missTextureBuffer;
  } else if (tex >= NUMBER_TEXTURE) {
    tex -= NUMBER_TEXTURE;
    readFileRaw(&numberTextureFile, numTextureBuffer, 23 * 15 * sizeof(Color), tex * 23 * 15 * sizeof(Color));
    buf = numTextureBuffer;
  }
  for (int x = 0; x < width; ++x) {
    if (!side && ox - width / 2 + x < 40) {
      x = 40 - (ox - width / 2);
      if (x >= width) break; 
    }
    if (side && ox - width / 2 + x >= 40) break;
    for (int y = 0; y < height; ++y) {
      Color tmp = buf[(height - 1 - y) * width + x];
      setPixel(side, ox - width / 2 + x, oy - height / 2 + y, tmp);
    }
  }
}

void drawHitStatusAnimation(int side) {
  if (hitAnimationCoolDown <= 0) return;
  --hitAnimationCoolDown;
  switch (hitStatus) {
    case PERFECT:
      renderTexture(side, perfectTexture, 40, 50);
      break;
    case GREAT:
      renderTexture(side, greatTexture, 40, 50);
      break;
    case GOOD:
      renderTexture(side, goodTexture, 40, 50);
      break;
    case MISS:
      renderTexture(side, missTexture, 40, 50);
      break;
  }
}

void drawNote(int side, int x) {
  // loadNote(side);
  // readFile(textureBuffer, "note.bin", 40 * 14 * sizeof(Color), 0);
  // readFileRaw(&noteTextureFile, textureBuffer, 40 * 14 * sizeof(Color), 0);
  // for (int x = 0; x < 40; ++x) {
  //   for (int y = 0; y < 14; ++y) {
  //     transformedTextureBuffer[(40 - x - 1) * 14 + (14 - 1 - y)] = textureBuffer[(14 - 1 - y) * 40 + x];
  //   }
  // }
  side = !side;
  int end = noteBufferEnd[side];
  if (noteBufferBegin[side] + MAX_NOTE_SAME_TIME < end) end = noteBufferBegin[side] + MAX_NOTE_SAME_TIME;
  for (int i = noteBufferBegin[side]; i < end; ++i) {
    int delta = noteBuffer[side][i].frame - currentFrame;
    int ox = x, oy = (delta + JUDGE_LINE) * NOTE_SPEED;
    if (oy > 180) break;
    if (noteBuffer[side][i].type == 1) continue;
    // for (int x = 0; x < 40; ++x) {
    //   for (int y = 0; y < 14; ++y) {
    //     Color tmp = noteTextureBuffer[(14 - 1 - y) * 40 + x];
    //     setPixel(!side, ox - 40 / 2 + x, oy - 14 / 2 + y, tmp);
    //   }
    // }
    int x1 = (80 - (ox - 40 / 2 + 39) - 1);
    int y1 = (160 - (oy - 14 / 2 + 13) - 1);
    if (!side) {
      x1 = (40 - (ox - 40 / 2 + 39) - 1);
    }
    int y2 = y1 + 13 < 160 ? y1 + 13 : 159;
    if (y2 < 0) continue;
    int start = y1 < 0 ? 0 : y1;
    int texStart = y1 < 0 ? -y1 : 0;
    for (int x = x1; x < 40; ++x) {
      memcpy(&image[((x) * 160 + start) * 2], &noteTextureBuffer[(x - x1) * 14 + texStart], sizeof(Color) * (y2 - start + 1));
    }
  }
  for (int i = noteBufferBegin[side]; i < end; ++i) {
    if (i > 0 && noteBuffer[side][i].type == 1 && noteBuffer[side][i - 1].type == 1) {
      if (getCurrentLongNoteStatus(side, i) == 0) continue;
      int delta = noteBuffer[side][i - 1].frame - currentFrame;
      int ox = x;
      int oy1 = (delta + JUDGE_LINE) * NOTE_SPEED;
      if (i == noteBufferBegin[side] && oy1 < 0) oy1 = 0;
      delta = noteBuffer[side][i].frame - currentFrame;
      int oy2 = (delta + JUDGE_LINE) * NOTE_SPEED;
      fillLongNote(!side, ox, oy1, oy2);
      if (oy2 > 180) break;
    }
  }
  
  for (int i = noteBufferBegin[side]; i < end; ++i) {
    if (noteBuffer[side][i].type == 1) {
      int delta = noteBuffer[side][i].frame - currentFrame;
      int ox = x, oy = (delta + JUDGE_LINE) * NOTE_SPEED;
      if (oy > 180) break;
      int x1 = (80 - (ox - 40 / 2 + 39) - 1);
      int y1 = (160 - (oy - 14 / 2 + 13) - 1);
      if (!side) {
        x1 = (40 - (ox - 40 / 2 + 39) - 1);
      }
      int y2 = y1 + 13 < 160 ? y1 + 13 : 159;
      if (y2 < 0) continue;
      int start = y1 < 0 ? 0 : y1;
      int texStart = y1 < 0 ? -y1 : 0;
      for (int x = x1; x < 40; ++x) {
        memcpy(&image[((x)*160 + start) * 2],
               &longTextureBuffer[(x - x1) * 14 + texStart],
               sizeof(Color) * (y2 - start + 1));
      }
      // for (int x = 0; x < 40; ++x) {
      //   for (int y = 0; y < 14; ++y) {
      //     Color tmp = longTextureBuffer[(14 - 1 - y) * 40 + x];
      //     setPixel(!side, ox - 40 / 2 + x, oy - 14 / 2 + y, tmp);
      //   }
      // }
    }
  }
}

void loadNote(int side) {
  if (side == 0) {
    sprintf(fileNameBuffer, "songs/%dl.bin", musicId);
    noteBufferEnd[0] = readFile(noteBuffer[0], fileNameBuffer, 1000, 0) / 2;
    // noteBufferEnd[0] = readFile(noteBuffer[0], "normall.bin", 1000, 0) / 2;
  } else if (side == 1) {
    sprintf(fileNameBuffer, "songs/%dr.bin", musicId);
    noteBufferEnd[1] = readFile(noteBuffer[1], fileNameBuffer, 1000, 0) / 2;
    // noteBufferEnd[1] = readFile(noteBuffer[1], "normalr.bin", 1000, 0) / 2;
  }
  // for (int i = 0; i <= 1; ++i) {
  //   for (int j = 0; j < noteBufferEnd[i]; ++j) {
  //     noteBuffer[i][j].frame *= 4.0 / 3.0;
  //   }
  // }
}

void checkHit() {
  for (int i = 0; i <= 1; ++i) {
    if (checkCoolDown[i] > 0) --checkCoolDown[i];
    if (isPressButtons[i]) {
      if (noteBufferBegin[i] < noteBufferEnd[i] && noteBuffer[i][noteBufferBegin[i]].type == 1 && checkCoolDown[i] <= 0) {
        if (getCurrentLongNoteStatus(i, noteBufferBegin[i]) == 0) {
          int absFrame = currentFrame - noteBuffer[i][noteBufferBegin[i]].frame;
          if (absFrame < 0) absFrame = -absFrame;
          if (absFrame < PERFECT_LINE) {
            perfectCount++;
            hitStatus = PERFECT;
            hitAnimationCoolDown = 30;
            printf("perfect;\n");
            noteBufferBegin[i]++;
            checkCoolDown[i] = CHECK_COOL_DOWN;
            ++combo;
            score += 5;
            score += combo;
          } else if (absFrame < GREAT_LINE) {
            hitStatus = GREAT;
            hitAnimationCoolDown = 30;
            printf("great;\n");
            greatCount++;
            noteBufferBegin[i]++;
            checkCoolDown[i] = CHECK_COOL_DOWN;
            ++combo;
            score += 3;
            score += combo;
          } else if (absFrame < GOOD_LINE) {
            hitStatus = GOOD;
            hitAnimationCoolDown = 30;
            printf("good;\n");
            goodCount++;
            noteBufferBegin[i]++;
            checkCoolDown[i] = CHECK_COOL_DOWN;
            ++combo;
            score += 1;
            score += combo;
          } else if (absFrame < MISS_LINE) {
            hitStatus = MISS;
            hitAnimationCoolDown = 30;
            printf("miss;\n");
            missCount += 2;
            noteBufferBegin[i] += 2;
            checkCoolDown[i] = CHECK_COOL_DOWN;
            combo = 0;
          }
        } else {
          int frames = currentFrame - noteBuffer[i][noteBufferBegin[i]].frame;
          if (frames > JUDGE_LINE * 2) {
            hitStatus = MISS;
            hitAnimationCoolDown = 30;
            printf("miss;\n");
            missCount++;
            noteBufferBegin[i]++;
            checkCoolDown[i] = CHECK_COOL_DOWN;
            combo = 0;
          }
        }
      }
      if (noteBufferBegin[i] < noteBufferEnd[i] && checkCoolDown[i] <= 0 && noteBuffer[i][noteBufferBegin[i]].type == 0) {
        int absFrame = currentFrame - noteBuffer[i][noteBufferBegin[i]].frame;
        if (absFrame < 0) absFrame = -absFrame;
        if (absFrame < PERFECT_LINE) {
          perfectCount++;
          hitStatus = PERFECT;
          hitAnimationCoolDown = 30;
          printf("perfect;\n");
          noteBufferBegin[i]++;
          checkCoolDown[i] = CHECK_COOL_DOWN;
          ++combo;
          score += 5;
          score += combo;
        } else if (absFrame < GREAT_LINE) {
          hitStatus = GREAT;
          hitAnimationCoolDown = 30;
          printf("great;\n");
          greatCount++;
          noteBufferBegin[i]++;
          checkCoolDown[i] = CHECK_COOL_DOWN;
          ++combo;
          score += 3;
          score += combo;
        } else if (absFrame < GOOD_LINE) {
          hitStatus = GOOD;
          hitAnimationCoolDown = 30;
          printf("good;\n");
          goodCount++;
          noteBufferBegin[i]++;
          checkCoolDown[i] = CHECK_COOL_DOWN;
          ++combo;
          score += 1;
          score += combo;
        } else if (absFrame < MISS_LINE) {
          hitStatus = MISS;
          hitAnimationCoolDown = 30;
          printf("miss;\n");
          missCount++;
          noteBufferBegin[i]++;
          checkCoolDown[i] = CHECK_COOL_DOWN;
          combo = 0;
        }
      }
    } else {
      if (noteBufferBegin[i] < noteBufferEnd[i] && noteBuffer[i][noteBufferBegin[i]].type == 1 && checkCoolDown[i] <= 0) {
        if (getCurrentLongNoteStatus(i, noteBufferBegin[i]) == 1) {
          int absFrame = currentFrame - noteBuffer[i][noteBufferBegin[i]].frame;
          if (absFrame < 0) absFrame = -absFrame;
          if (absFrame < PERFECT_LINE) {
            perfectCount++;
            hitStatus = PERFECT;
            hitAnimationCoolDown = 30;
            printf("perfect;\n");
            noteBufferBegin[i]++;
            checkCoolDown[i] = CHECK_COOL_DOWN;
            ++combo;
            score += 5;
            score += combo;
          } else if (absFrame < GREAT_LINE) {
            hitStatus = GREAT;
            hitAnimationCoolDown = 30;
            printf("great;\n");
            greatCount++;
            noteBufferBegin[i]++;
            checkCoolDown[i] = CHECK_COOL_DOWN;
            ++combo;
            score += 3;
            score += combo;
          } else if (absFrame < GOOD_LINE) {
            hitStatus = GOOD;
            hitAnimationCoolDown = 30;
            printf("good;\n");
            goodCount++;
            noteBufferBegin[i]++;
            checkCoolDown[i] = CHECK_COOL_DOWN;
            ++combo;
            score += 1;
            score += combo;
          } else if (absFrame < MISS_LINE) {
            hitStatus = MISS;
            hitAnimationCoolDown = 30;
            printf("miss;\n");
            missCount++;
            noteBufferBegin[i]++;
            checkCoolDown[i] = CHECK_COOL_DOWN;
            combo = 0;
          }
        }
      }
    }
    while (noteBufferBegin[i] < noteBufferEnd[i] &&
           noteBuffer[i][noteBufferBegin[i]].frame + JUDGE_LINE * 2 <
               currentFrame) {
      if (noteBuffer[i][noteBufferBegin[i]].type == 1) {
        if (getCurrentLongNoteStatus(i, noteBufferBegin[i]) == 0) {
          noteBufferBegin[i] += 2;
          missCount += 2;
        } else {
          noteBufferBegin[i]++;
          missCount++;
        }
      } else {
        noteBufferBegin[i]++;
        missCount++;
      }
      if (autoPlayMode) {
        combo++;
        score += 3 + combo;
        hitStatus = PERFECT;
        hitAnimationCoolDown = 30;
        printf("perfect;\n");
      } else {
        combo = 0;
        hitStatus = MISS;
        hitAnimationCoolDown = 30;
        printf("miss;\n");
      }
    }
  }
}

void fillLongNote(int side, int ox, int y1, int y2) {
  // printf("fill %d %d %d;\n", ox, y1, y2);
  Color outSide;
  outSide.r = 14;
  outSide.g = 28;
  outSide.b = 16;
  outSide.a = 1;
  Color inSide;
  inSide.r = 22;
  inSide.g = 29;
  inSide.b = 24;
  inSide.a = 1;
  side = !side;
  int start = (160 - y2 - 1);
  if (start < 0) start = 0;
  int x1 = (80 - (ox + 9) - 1);
  y2 = (160 - y1 - 1);
  if (!side) {
    x1 = (40 - (ox + 9) - 1);
  }
  if (y2 >= 160) y2 = 159;
  if (y2 < 0) return;
  for (int x = x1; x <= x1 + 18; ++x) {
    for (int y = start; y <= y2; ++y) {
      ((Color *)image)[x * 160 + y] = inSide;
    }
  }
  x1 = (80 - (ox - 10) - 1);
  if (!side) {
    x1 = (40 - (ox - 10) - 1);
  }
  for (int x = x1; x <= x1 + 2; ++x) {
    for (int y = start; y <= y2; ++y) {
      ((Color *)image)[x * 160 + y] = outSide;
    }
  }
  x1 = (80 - (ox + 12) - 1);
  if (!side) {
    x1 = (40 - (ox + 12) - 1);
  }
  for (int x = x1; x <= x1 + 2; ++x) {
    for (int y = start; y <= y2; ++y) {
      ((Color *)image)[x * 160 + y] = outSide;
    }
  }
  // for (int x = ox - 12; x <= ox + 12; ++x) {
  //   for (int y = y1; y <= y2; ++y) {
  //     if (x <= ox - 10 || x >= ox + 10) {
  //       setPixel(side, x, y, outSide);
  //     } else {
  //       // setPixel(side, x, y, inSide);
  //     }
  //   } 
  // }
}

int getCurrentLongNoteStatus(int side, int cur) {
  int cnt = 0;
  for (int i = cur - 1; i >= 0; --i) {
    if (noteBuffer[side][i].type == 0) break;
    cnt = !cnt;
  }
  return cnt;
}

/**
 * move to risc-v
 */
void drawPressEffect(int side, int ox) {
  Color color;
  color.a = 1;
  color.r = 18;
  color.g = 31;
  color.b = 31;
  side = !side;
  int start = (160 - (JUDGE_LINE * 2 + 8) - 1);
  if (start < 0) start = 0;
  int x1 = (80 - (ox + 17) - 1);
  int y2 = (160 - (3) - 1);
  if (!side) {
    x1 = (40 - (ox + 17) - 1);
  }
  if (y2 >= 160) y2 = 159;
  if (y2 < 0) return;
  for (int x = x1; x <= x1 + 34; ++x) {
    for (int y = start; y <= y2; ++y) {
      ((Color *)image)[x * 160 + y] = color;
    }
  }
  // for (int x = ox - 17; x <= ox + 17; ++x) {
  //   for (int y = 3; y <= JUDGE_LINE * 2 + 8; ++y) {
  //     setPixel(side, x, y, color);
  //   }
  // }
}

/**
 * move to risc-v
 */
// void init_uart0() {
//   /* enable GPIO clock */
//   rcu_periph_clock_enable(RCU_GPIOA);
//   /* enable USART clock */
//   rcu_periph_clock_enable(RCU_USART0);

//   /* connect port to USARTx_Tx */
//   gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
//   /* connect port to USARTx_Rx */
//   gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

//   /* USART configure */
//   usart_deinit(USART0);
//   usart_baudrate_set(USART0, 128000U);
//   usart_word_length_set(USART0, USART_WL_8BIT);
//   usart_stop_bit_set(USART0, USART_STB_1BIT);
//   usart_parity_config(USART0, USART_PM_NONE);
//   usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
//   usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
//   usart_receive_config(USART0, USART_RECEIVE_ENABLE);
//   usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
//   usart_enable(USART0);

//   usart_interrupt_enable(USART0, USART_INT_RBNE);
// }

/**
 * move to risc-v
 */
void showLonganLogo() {
  readFile(image, "logo.bin", sizeof(image), 0);
  LCD_ShowPicture(0, 0, 159, 39);
  readFile(image, "logo.bin", sizeof(image), 12800);
  LCD_ShowPicture(0, 40, 159, 79);
}

void readFileRaw(FIL *file, void *ptr, size_t size, int offset) {
  f_lseek(file, offset);
  UINT br;
  f_read(file, ptr, size, &br);
}

int openFile(FIL *file, const char *fileName) {
  FRESULT fr = f_open(file, fileName, FA_READ);
  if (fr) {
    printf("open %s error;\n", fileName);
    return -1;
  }
  return 0;
}

/**
 * move to risc-v
 */
void onReleaseRiscv();

void onRelease() {
  /**
   * move to riscv
   */
  // f_close(&bgTextureFile);
  // f_close(&numberTextureFile);
  onReleaseRiscv();
}

void initTextures() {
  openFile(&bgTextureFile, "bg.bin");
  openFile(&numberTextureFile, "num.bin");


  readFile(image, "note.bin", 40 * 14 * sizeof(Color), 0);
  for (int x = 0; x < 40; ++x) {
    for (int y = 0; y < 14; ++y) {
      noteTextureBuffer[(40 - x - 1) * 14 + (14 - 1 - y)] = ((Color *)image)[(14 - 1 - y) * 40 + x];
    }
  }
  readFile(image, "long.bin", 40 * 14 * sizeof(Color), 0);
  for (int x = 0; x < 40; ++x) {
    for (int y = 0; y < 14; ++y) {
      longTextureBuffer[(40 - x - 1) * 14 + (14 - 1 - y)] = ((Color *)image)[(14 - 1 - y) * 40 + x];
    }                                                 
  }
  readFile(perfectTextureBuffer, "perfect.bin", 50 * 10 * sizeof(Color), 0);
  readFile(goodTextureBuffer, "good.bin", 35 * 10 * sizeof(Color), 0);
  readFile(greatTextureBuffer, "great.bin", 37 * 10 * sizeof(Color), 0);
  readFile(missTextureBuffer, "miss.bin", 26 * 10 * sizeof(Color), 0);
}

void drawCombo(int side) {
  int left = combo / 100;
  int mid = (combo % 100) / 10;
  int right = combo % 10;
  if (side == 1)
    renderTexture(side, NUMBER_TEXTURE + left, 27, 100);
  renderTexture(side, NUMBER_TEXTURE + mid, 40, 100);
  if (side == 0)
    renderTexture(side, NUMBER_TEXTURE + right, 53, 100);
}

/**
 * move to risc-v
 */
// void drawScore(int side) {
//   int left0 = score / 10000;
//   int left1 = (score % 10000) / 1000;
//   int mid = (score % 1000) / 100;
//   int right0 = (score % 100) / 10;
//   int right1 = (score % 10);
//   if (side == 1) {
//     renderTexture(side, NUMBER_TEXTURE + left1, 27, 140);
//     renderTexture(side, NUMBER_TEXTURE + left0, 14, 140);
//   }
//   renderTexture(side, NUMBER_TEXTURE + mid, 40, 140);
//   if (side == 0) {
//     renderTexture(side, NUMBER_TEXTURE + right0, 53, 140);
//     renderTexture(side, NUMBER_TEXTURE + right1, 66, 140);
//   }
// }

void mainGameLoop() {
  for (;;) {
    uint64_t tmpTime = get_timer_value();
    do {
      frameStartTime = get_timer_value();
    } while (frameStartTime == tmpTime);
    // logic update
    onUpdate();
    if (changeToScoreScene <= 0) {
      currentScene = END_GAME_SCENE;
      break;
    }
    tmpTime = get_timer_value() - globalStartTime;
    if (tmpTime >= SystemCoreClock / 4000.0 * (currentFrame + 1) * FRAME_WAIT_TIME) {
      ++currentFrame;
      continue;
    }
    // render
    onRender();
    tmpTime = get_timer_value() - globalStartTime;
    if (tmpTime >= SystemCoreClock / 4000.0 * (currentFrame + 1) * FRAME_WAIT_TIME) {
      ++currentFrame;
      continue;
    }
    frameEndTime = get_timer_value();
    do {
      updateControls();
      tmpTime = get_timer_value() - frameStartTime;
    } while (tmpTime < SystemCoreClock / 4000.0 * FRAME_WAIT_TIME);
    ++currentFrame;
  }
}

void mountSdCard() {
  sdCardFr = f_mount(&sdCardFs, "", 1);
}

void checkAutoPlayMode() {
  if (isPressBoot0 && isPressButtons[0] && isPressButtons[1]) {
    autoPlayMode = TRUE;
  }
}

void updateAutoPlay() {
  for (int i = 0; i <= 1; ++i) {
    if (noteBufferBegin[i] < noteBufferEnd[i] && noteBuffer[i][noteBufferBegin[i]].type == 1) {
      if (getCurrentLongNoteStatus(i, noteBufferBegin[i]) == 0) {
        int absFrame = currentFrame - noteBuffer[i][noteBufferBegin[i]].frame;
        if (absFrame < 0) absFrame = -absFrame;
        isPressButtons[i] = absFrame == 0;
      } else {
        isPressButtons[i] = TRUE;
        int frames = currentFrame - noteBuffer[i][noteBufferBegin[i]].frame;
        if (frames == 0) isPressButtons[i] = FALSE;
      }
    } else if (noteBufferBegin[i] < noteBufferEnd[i] && noteBuffer[i][noteBufferBegin[i]].type == 0) {
      int absFrame = currentFrame - noteBuffer[i][noteBufferBegin[i]].frame;
      if (absFrame < 0) absFrame = -absFrame;
      isPressButtons[i] = absFrame == 0;
    } else {
      isPressButtons[i] = FALSE;
    }
  }
}

void changeMusic() {
  if (changeMusicCoolDown > 1) {
    --changeMusicCoolDown;
    return;
  }
  if (isPressButtons[1]) {
    if (++musicId == MAX_MUSIC_NUM) musicId = 0;
    printf("bgm%d;\n", musicId);
    sprintf(fileNameBuffer, "songs/%dmenu.bin", musicId);
    for (int i = 0; i <= 1; ++i) {
      readFile(image, fileNameBuffer, 12800, 12800 * i);
      renderFrameBuffer(i);
    }
    changeMusicCoolDown = 100000;
  } else if (isPressButtons[0]) {
    if (--musicId == -1) musicId = MAX_MUSIC_NUM - 1;
    printf("bgm%d;\n", musicId);
    sprintf(fileNameBuffer, "songs/%dmenu.bin", musicId);
    for (int i = 0; i <= 1; ++i) {
      readFile(image, fileNameBuffer, 12800, 12800 * i);
      renderFrameBuffer(i);
    }
    changeMusicCoolDown = 100000;
  }
}