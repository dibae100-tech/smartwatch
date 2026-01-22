// ============================================
// TTGO Watch Gauge - Main Sketch
// Version: 2.4 - NTP Time Sync Support
// ============================================
// 
// Features:
//   - Animated clock face with rotating elements
//   - NTP time synchronization via WiFi
//   - Battery monitoring
//   - Touch-based menu navigation
//   - Auto sleep for power saving
//
// Menu:
//   - Double tap: Toggle Menu/Clock
//   - Single tap on menu items: Select
//
// WiFi Setup:
//   Edit WIFI_SSID and WIFI_PASSWORD in config.h
//
// ============================================

#include "config.h"
#include "fonts.h"
#include "UI-main.h"

// Global watch instance
WatchUICLASS twatch;

void setup() {
    Serial.begin(115200);
    delay(100);
    
    Serial.println();
    Serial.println("=============================");
    Serial.println("TTGO Watch v2.4 - NTP Support");
    Serial.println("=============================");
    
    // 시계 초기화 (NTP 동기화 포함)
    twatch.initializeWatch();
    
    // 디스플레이 설정
    twatch.setupDisplay();
    
    Serial.println("Setup complete!");
    Serial.println("=============================");
}

void loop() {
    // 상태 체크 (터치, 슬립, IRQ)
    twatch.checkStatus();
    
    // UI 업데이트
    twatch.updateUI();
    
    // CPU 부하 감소
    delay(20);
}
