// ============================================
// TTGO Watch Configuration
// Version: 2.5 - Splash Screen Support
// ============================================
#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// Hardware Selection
// ============================================
#define LILYGO_WATCH_2020_V1

#include <LilyGoWatch.h>

// ============================================
// WiFi Configuration (NTP 동기화용)
// ============================================
#define WIFI_SSID       "FMC_2G"      // ⚠️ 실제 WiFi SSID로 변경
#define WIFI_PASSWORD   "dpvmxl200@@"  // ⚠️ 실제 WiFi 비밀번호로 변경

// WiFi 연결 타임아웃 (초)
#define WIFI_TIMEOUT_SEC    15

// ==========================================
// NTP Configuration
// ============================================
#define NTP_SERVER1     "pool.ntp.org"
#define NTP_SERVER2     "time.google.com"
#define NTP_SERVER3     "time.nist.gov"

// 한국 시간대: GMT+9 (초 단위: 9 * 3600 = 32400)
#define GMT_OFFSET_SEC      32400
#define DAYLIGHT_OFFSET_SEC 0      // 한국은 서머타임 없음

// NTP 동기화 주기 (밀리초) - 기본 6시간
#define NTP_SYNC_INTERVAL   (6 * 60 * 60 * 1000UL)

// ============================================
// EEPROM Configuration
// ============================================
#define EEPROM_SIZE 16

// EEPROM 주소
#define EEPROM_ADDR_BRIGHTNESS  0
#define EEPROM_ADDR_LAST_SYNC   4   // 마지막 동기화 시간 (4바이트)

// ============================================
// Timing Constants
// ============================================
#define SECOND              1000
#define DOUBLE_TAP_INTERVAL 400
#define LONG_PRESS_TIME     5000
#define AUTO_SLEEP_TIME     60000

// ============================================
// Display Constants
// ============================================
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   240
#define CENTER_X        120
#define CENTER_Y        120

// ============================================
// Brightness Levels
// ============================================
#define FULL_BRIGHTNESS 255
#define DIM_BRIGHTNESS  100
#define MIN_BRIGHTNESS  30

// ============================================
// Button Pin
// ============================================
#define BUTTON_PIN      35

// ============================================
// Debug Configuration
// ============================================
#define _DEBUG_

#ifdef _DEBUG_
    #define _PP(a)  Serial.print(a)
    #define _PL(a)  Serial.println(a)
    #define _PPH(a) Serial.print(a, HEX)
    #define _PLH(a) Serial.println(a, HEX)
    #define _PF(...)  Serial.printf(__VA_ARGS__)
#else
    #define _PP(a)
    #define _PL(a)
    #define _PPH(a)
    #define _PLH(a)
    #define _PF(...)
#endif

// ============================================
// Color Definitions
// ============================================
#define COLOR_BLACK     TFT_BLACK
#define COLOR_WHITE     TFT_WHITE
#define COLOR_RED       TFT_RED
#define COLOR_GREEN     TFT_GREEN
#define COLOR_BLUE      TFT_BLUE
#define COLOR_ORANGE    TFT_ORANGE
#define COLOR_YELLOW    TFT_YELLOW
#define COLOR_CYAN      0x35D7
#define COLOR_GRAY1     0x8410
#define COLOR_GRAY2     0x5ACB
#define COLOR_GRAY3     0x15B3

// 기존 color 호환
#define color1  COLOR_WHITE
#define color2  COLOR_GRAY1
#define color3  COLOR_GRAY2
#define color4  COLOR_GRAY3
#define color5  COLOR_BLACK

#endif // CONFIG_H
