// ============================================
// TTGO Watch UI Header
// Version: 2.5 - Splash Screen Support
// ============================================
#ifndef UI_MAIN_H
#define UI_MAIN_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <esp_sleep.h>
#include <math.h>
#include "config.h"

// ============================================
// Global Variables (extern)
// ============================================
extern TTGOClass *ttgo;
extern AXP20X_Class *power;
extern volatile bool irq;
extern float x[360], y[360];

// ============================================
// Clock Number Labels
// ============================================
const String clockNumbers[12] = {"45", "40", "35", "30", "25", "20", "15", "10", "05", "0", "55", "50"};
const String dayNames[7] = {"SAT", "SUN", "MON", "TUE", "WED", "THU", "FRI"};

// ============================================
// Application States
// ============================================
enum class AppState {
    CLOCK,
    MENU,
    SETTINGS,
    BATTERY_INFO,
    WATCH_FACE,
    NTP_SYNC,       // ✅ NTP 동기화 화면 추가
    WIFI_SETTINGS
};

// ============================================
// NTP Sync Status
// ============================================
enum class NtpStatus {
    IDLE,
    CONNECTING_WIFI,
    WIFI_CONNECTED,
    SYNCING,
    SUCCESS,
    FAILED_WIFI,
    FAILED_NTP
};

// ============================================
// Touch State Structure
// ============================================
struct TouchState {
    bool isPressed;
    bool wasPressed;
    int16_t x;
    int16_t y;
    unsigned long pressStartTime;
    unsigned long lastTapTime;
    int tapCount;
    
    void reset() {
        isPressed = false;
        wasPressed = false;
        x = 0;
        y = 0;
        pressStartTime = 0;
        lastTapTime = 0;
        tapCount = 0;
    }
};

// ============================================
// WatchUICLASS
// ============================================
class WatchUICLASS {
private:
    // PWM Configuration
    static const int pwmFreq = 5000;
    static const int pwmResolution = 8;
    static const int pwmLedChannelTFT = 0;
    
    // Screen center
    static const int sx = CENTER_X;
    static const int sy = CENTER_Y;
    
    // Coordinate arrays
    float px[360], py[360];
    float lx[360], ly[360];
    int start[12];
    int startP[60];
    
    // State variables
    unsigned long lastActivityTime;
    unsigned long lastUpdateTime;
    unsigned long lastNtpSyncTime;   // ✅ 마지막 NTP 동기화 시간
    int sleepTimeout;
    int brightness;
    bool displayInitialized;
    
    // Animation variables
    int rAngle;
    int lastAngle;
    int angle;
    float circle;
    bool circleDir;
    
    // Application state
    AppState currentState;
    TouchState touch;
    int selectedMenuItem;
    
    // ✅ NTP 관련 변수
    NtpStatus ntpStatus;
    bool ntpSyncedToday;
    
    // Private methods
    void initCoordinates();
    void wakeUpWatch();
    void goToSleep();
    
    // Touch handling
    void updateTouchState();
    bool detectSingleTap();
    bool detectDoubleTap();
    
    // IRQ handling
    void handleIRQ();
    
    // State management
    void switchToState(AppState newState);
    
    // Drawing methods
    void drawSplashScreen(const char* status, int progress);  // 🆕 스플래시 화면
    void drawClockFace();
    void drawMenuScreen();
    void drawBatteryInfoScreen();
    void drawNtpSyncScreen();    // ✅ NTP 동기화 화면
    
    // ✅ NTP/WiFi methods
    bool connectWiFi();
    void disconnectWiFi();
    bool syncNtpTime();
    void updateRtcFromNtp();
    String getLastSyncTimeStr();
    
    // Utility
    float getBatteryPercentage();
    String getWeekday(int day, int month, int year);

    int lastMinute = -1;              // 🆕 추가
    uint16_t circleColor = COLOR_RED; // 🆕 추가
    uint16_t getRandomColor();     // 🆕 랜덤 색상 생성
    
public:
    WatchUICLASS();
    ~WatchUICLASS();
    
    // Main interface
    void initializeWatch();
    void setupDisplay();
    void checkStatus();
    void updateUI();
    
    // Brightness control
    void setBrightness(int level);
    int getBrightness() const { return brightness; }
    
    // ✅ NTP Sync - 외부에서 호출 가능
    bool performNtpSync();
    bool isNtpSynced() const { return ntpSyncedToday; }
    NtpStatus getNtpStatus() const { return ntpStatus; }
};

#endif // UI_MAIN_H
