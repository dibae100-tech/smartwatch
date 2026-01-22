// ============================================
// TTGO Watch UI Implementation
// Version: 2.4 - NTP Sync Support
// ============================================

#include "UI-main.h"
#include "fonts.h"

// ============================================
// 이전 상태 저장용 (깜빡임 방지)
// ============================================
static String prevSec = "";
static String prevMin = "";
static String prevHr = "";
static int prevAngle = -1;
static int prevRAngle = -1;
static int prevBrightness = -1;
static int prevBatteryPct = -1;
static bool firstDraw = true;

// ============================================
// Constructor / Destructor
// ============================================
WatchUICLASS::WatchUICLASS() {
    lastActivityTime = 0;
    lastUpdateTime = 0;
    lastNtpSyncTime = 0;
    sleepTimeout = AUTO_SLEEP_TIME;
    
    brightness = FULL_BRIGHTNESS;
    displayInitialized = false;
    
    currentState = AppState::CLOCK;
    touch.reset();
    selectedMenuItem = 0;
    
    rAngle = 359;
    lastAngle = 0;
    angle = 0;
    circle = 100;
    circleDir = false;
    
    // NTP 상태
    ntpStatus = NtpStatus::IDLE;
    ntpSyncedToday = false;
}

WatchUICLASS::~WatchUICLASS() {
}

// ============================================
// Initialization
// ============================================
void WatchUICLASS::initializeWatch() {
    _PL("=================================");
    _PL("Initializing Watch v2.4 (NTP)...");
    _PL("=================================");
    
    wakeUpWatch();
    
    lastActivityTime = millis();
    lastUpdateTime = millis();
    
    // ✅ 시작 시 NTP 동기화 시도
    _PL("Attempting initial NTP sync...");
    if (performNtpSync()) {
        _PL("Initial NTP sync successful!");
    } else {
        _PL("Initial NTP sync failed - using RTC time");
    }
    
    _PL("Watch initialized successfully");
}

void WatchUICLASS::wakeUpWatch() {
    _PL("Waking up watch...");
    
    ttgo = TTGOClass::getWatch();
    if (ttgo == nullptr) {
        _PL("ERROR: TTGOClass failed!");
        return;
    }
    _PL("TTGOClass OK");
    
    ttgo->begin();
    _PL("ttgo->begin() OK");
    
    // TFT 초기화
    ttgo->tft->init();
    ttgo->tft->setRotation(2);
    ttgo->tft->fillScreen(TFT_BLACK);
    ttgo->tft->setSwapBytes(true);
    _PL("TFT initialized (rotation=2)");
    
    ttgo->openBL();
    _PL("Backlight ON");
    
    // 전원 설정
    power = ttgo->power;
    if (power != nullptr) {
        power->adc1Enable(
            AXP202_VBUS_VOL_ADC1 | 
            AXP202_VBUS_CUR_ADC1 | 
            AXP202_BATT_CUR_ADC1 | 
            AXP202_BATT_VOL_ADC1, 
            true
        );
        _PL("Power ADC enabled");
    }
    
    // PWM 백라이트
    ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
    ledcAttachPin(12, pwmLedChannelTFT);
    setBrightness(FULL_BRIGHTNESS);
    
    // 인터럽트
    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, []() { irq = true; }, FALLING);
    
    if (power != nullptr) {
        power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
        power->clearIRQ();
    }
    
    // 상태 초기화
    firstDraw = true;
    prevSec = "";
    prevMin = "";
    prevHr = "";
    prevAngle = -1;
    prevRAngle = -1;
    
    lastActivityTime = millis();
    _PL("Wake up complete!");
}

void WatchUICLASS::goToSleep() {
    _PL("Going to sleep...");
    
    // WiFi 확실히 끄기
    disconnectWiFi();
    
    ttgo->closeBL();
    ttgo->displaySleep();
    ttgo->powerOff();
    
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
}

// ============================================
// WiFi Connection
// ============================================
bool WatchUICLASS::connectWiFi() {
    _PL("Connecting to WiFi...");
    _PP("SSID: ");
    _PL(WIFI_SSID);
    
    ntpStatus = NtpStatus::CONNECTING_WIFI;
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int timeout = WIFI_TIMEOUT_SEC * 10;  // 100ms 단위
    int count = 0;
    
    while (WiFi.status() != WL_CONNECTED && count < timeout) {
        delay(100);
        count++;
        
        // 화면에 진행 상황 표시
        if (count % 5 == 0) {
            _PP(".");
        }
    }
    _PL("");
    
    if (WiFi.status() == WL_CONNECTED) {
        _PL("WiFi Connected!");
        _PP("IP: ");
        _PL(WiFi.localIP().toString());
        ntpStatus = NtpStatus::WIFI_CONNECTED;
        return true;
    } else {
        _PL("WiFi Connection Failed!");
        ntpStatus = NtpStatus::FAILED_WIFI;
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        return false;
    }
}

void WatchUICLASS::disconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect(true);
    }
    WiFi.mode(WIFI_OFF);
    _PL("WiFi Disconnected");
}

// ============================================
// NTP Sync
// ============================================
bool WatchUICLASS::syncNtpTime() {
    _PL("Syncing NTP time...");
    ntpStatus = NtpStatus::SYNCING;
    
    // NTP 서버 설정
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
    
    // 시간 가져오기 대기
    struct tm timeinfo;
    int retry = 0;
    const int maxRetry = 10;
    
    while (!getLocalTime(&timeinfo) && retry < maxRetry) {
        _PL("Waiting for NTP time...");
        delay(500);
        retry++;
    }
    
    if (retry >= maxRetry) {
        _PL("Failed to get NTP time!");
        ntpStatus = NtpStatus::FAILED_NTP;
        return false;
    }
    
    _PF("NTP Time: %04d-%02d-%02d %02d:%02d:%02d\n",
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec);
    
    return true;
}

void WatchUICLASS::updateRtcFromNtp() {
    struct tm timeinfo;
    
    if (!getLocalTime(&timeinfo)) {
        _PL("Failed to get local time for RTC update");
        return;
    }
    
    // RTC 업데이트 - TTGO Watch 라이브러리 API 사용
    uint16_t year = timeinfo.tm_year + 1900;
    uint8_t month = timeinfo.tm_mon + 1;
    uint8_t day = timeinfo.tm_mday;
    uint8_t hour = timeinfo.tm_hour;
    uint8_t minute = timeinfo.tm_min;
    uint8_t second = timeinfo.tm_sec;
    
    // setDateTime(year, month, day, hour, minute, second)
    ttgo->rtc->setDateTime(year, month, day, hour, minute, second);
    
    _PF("RTC Updated: %04d-%02d-%02d %02d:%02d:%02d\n",
        year, month, day, hour, minute, second);
    
    lastNtpSyncTime = millis();
    ntpSyncedToday = true;
    ntpStatus = NtpStatus::SUCCESS;
}

bool WatchUICLASS::performNtpSync() {
    _PL("=== Starting NTP Sync ===");
    
    // 1. WiFi 연결
    if (!connectWiFi()) {
        return false;
    }
    
    // 2. NTP 시간 동기화
    if (!syncNtpTime()) {
        disconnectWiFi();
        return false;
    }
    
    // 3. RTC 업데이트
    updateRtcFromNtp();
    
    // 4. WiFi 끄기 (배터리 절약)
    disconnectWiFi();
    
    _PL("=== NTP Sync Complete ===");
    return true;
}

String WatchUICLASS::getLastSyncTimeStr() {
    if (!ntpSyncedToday) {
        return "Never";
    }
    
    unsigned long elapsed = (millis() - lastNtpSyncTime) / 1000;
    
    if (elapsed < 60) {
        return String(elapsed) + "s ago";
    } else if (elapsed < 3600) {
        return String(elapsed / 60) + "m ago";
    } else {
        return String(elapsed / 3600) + "h ago";
    }
}

// ============================================
// Display Setup
// ============================================
void WatchUICLASS::setupDisplay() {
    _PL("Setting up display...");
    
    if (displayInitialized) {
        return;
    }
    
    ttgo->tft->setTextDatum(MC_DATUM);
    initCoordinates();
    
    displayInitialized = true;
    firstDraw = true;
    _PL("Display setup complete");
}

void WatchUICLASS::initCoordinates() {
    const double rad = 0.01745329252;
    const int r = 104;
    
    int hourIdx = 0;
    int minIdx = 0;
    
    for (int i = 0; i < 360; i++) {
        double cosVal = cos(rad * i);
        double sinVal = sin(rad * i);
        
        x[i] = (r * cosVal) + sx;
        y[i] = (r * sinVal) + sy;
        
        px[i] = ((r - 16) * cosVal) + sx;
        py[i] = ((r - 16) * sinVal) + sy;
        lx[i] = ((r - 26) * cosVal) + sx;
        ly[i] = ((r - 26) * sinVal) + sy;
        
        if (i % 30 == 0 && hourIdx < 12) {
            start[hourIdx] = i;
            hourIdx++;
        }
        
        if (i % 6 == 0 && minIdx < 60) {
            startP[minIdx] = i;
            minIdx++;
        }
    }
}

// ============================================
// Brightness Control
// ============================================
void WatchUICLASS::setBrightness(int level) {
    brightness = constrain(level, 0, 255);
    ledcWrite(pwmLedChannelTFT, brightness);
}

// ============================================
// Touch Handling
// ============================================
void WatchUICLASS::updateTouchState() {
    int16_t tx, ty;
    
    touch.wasPressed = touch.isPressed;
    touch.isPressed = ttgo->getTouch(tx, ty);
    
    if (touch.isPressed) {
        touch.x = tx;
        touch.y = ty;
        
        if (!touch.wasPressed) {
            touch.pressStartTime = millis();
        }
    }
}

bool WatchUICLASS::detectSingleTap() {
    if (touch.wasPressed && !touch.isPressed) {
        unsigned long pressDuration = millis() - touch.pressStartTime;
        if (pressDuration < 500) {
            return true;
        }
    }
    return false;
}

bool WatchUICLASS::detectDoubleTap() {
    unsigned long currentTime = millis();
    
    if (touch.wasPressed && !touch.isPressed) {
        unsigned long pressDuration = currentTime - touch.pressStartTime;
        
        if (pressDuration < 500) {
            if (currentTime - touch.lastTapTime < DOUBLE_TAP_INTERVAL) {
                touch.tapCount++;
                
                if (touch.tapCount >= 2) {
                    touch.tapCount = 0;
                    touch.lastTapTime = 0;
                    return true;
                }
            } else {
                touch.tapCount = 1;
            }
            
            touch.lastTapTime = currentTime;
        }
    }
    
    if (currentTime - touch.lastTapTime > DOUBLE_TAP_INTERVAL * 2) {
        touch.tapCount = 0;
    }
    
    return false;
}

// ============================================
// Status Check
// ============================================
void WatchUICLASS::checkStatus() {
    unsigned long currentTime = millis();
    
    updateTouchState();
    
    if (touch.isPressed) {
        lastActivityTime = currentTime;
        setBrightness(FULL_BRIGHTNESS);
    }
    
    if (irq) {
        handleIRQ();
    }
    
    switch (currentState) {
        case AppState::CLOCK:
            if (detectDoubleTap()) {
                _PL("Double tap - Menu");
                switchToState(AppState::MENU);
            }
            break;
            
        case AppState::MENU:
            if (detectDoubleTap()) {
                _PL("Double tap - Clock");
                switchToState(AppState::CLOCK);
            } else if (detectSingleTap()) {
                // 메뉴 항목 선택 (Y 좌표 기준)
                if (touch.y >= 55 && touch.y < 90) {
                    _PL("NTP Sync Selected");
                    switchToState(AppState::NTP_SYNC);
                } else if (touch.y >= 90 && touch.y < 125) {
                    _PL("Battery Info Selected");
                    switchToState(AppState::BATTERY_INFO);
                } else if (touch.y >= 125 && touch.y < 160) {
                    _PL("Settings Selected");
                } else if (touch.y >= 160 && touch.y < 195) {
                    _PL("Exit Selected");
                    switchToState(AppState::CLOCK);
                }
            }
            break;
            
        case AppState::NTP_SYNC:
            if (detectSingleTap()) {
                // 동기화 버튼 영역 (Y: 130~180)
                if (touch.y >= 130 && touch.y < 180) {
                    _PL("Starting NTP Sync...");
                    firstDraw = true;  // 화면 갱신
                    performNtpSync();
                    firstDraw = true;  // 결과 표시
                } else if (touch.y >= 200) {
                    // 뒤로가기
                    switchToState(AppState::MENU);
                }
            } else if (detectDoubleTap()) {
                switchToState(AppState::CLOCK);
            }
            break;
            
        case AppState::BATTERY_INFO:
            if (detectDoubleTap() || detectSingleTap()) {
                switchToState(AppState::MENU);
            }
            break;
            
        default:
            break;
    }
    
    unsigned long idleTime = currentTime - lastActivityTime;
    
    if (idleTime > (unsigned long)sleepTimeout) {
        goToSleep();
    } else if (idleTime > (unsigned long)(sleepTimeout / 2)) {
        if (brightness > DIM_BRIGHTNESS) {
            setBrightness(DIM_BRIGHTNESS);
        }
    }
}

void WatchUICLASS::handleIRQ() {
    irq = false;
    
    if (power != nullptr) {
        power->readIRQ();
        
        if (power->isPEKShortPressIRQ()) {
            _PL("Button press");
            lastActivityTime = millis();
            setBrightness(FULL_BRIGHTNESS);
        }
        
        power->clearIRQ();
    }
}

void WatchUICLASS::switchToState(AppState newState) {
    currentState = newState;
    lastActivityTime = millis();
    
    ttgo->tft->fillScreen(TFT_BLACK);
    firstDraw = true;
    
    _PP("State: ");
    _PL((int)newState);
}

// ============================================
// UI Update
// ============================================
void WatchUICLASS::updateUI() {
    if (!displayInitialized) {
        setupDisplay();
    }
    
    switch (currentState) {
        case AppState::CLOCK:
            drawClockFace();
            break;
        case AppState::MENU:
            drawMenuScreen();
            break;
        case AppState::BATTERY_INFO:
            drawBatteryInfoScreen();
            break;
        case AppState::NTP_SYNC:
            drawNtpSyncScreen();
            break;
        default:
            drawClockFace();
            break;
    }
}

// ============================================
// Clock Face - 부분 업데이트
// ============================================
void WatchUICLASS::drawClockFace() {
    TFT_eSPI *tft = ttgo->tft;
    
    String currentTime = String(ttgo->rtc->formatDateTime());
    String dateStr = String(ttgo->rtc->formatDateTime(PCF_TIMEFORMAT_DD_MM_YYYY));
    
    String sec = currentTime.substring(6, 8);
    String minStr = currentTime.substring(3, 5);
    String hr = currentTime.substring(0, 2);
    
    String d1 = dateStr.substring(0, 1);
    String d2 = dateStr.substring(1, 2);
    String m1 = dateStr.substring(3, 4);
    String m2 = dateStr.substring(4, 5);
    
    int day = dateStr.substring(0, 2).toInt();
    int month = dateStr.substring(3, 5).toInt();
    int year = dateStr.substring(6, 10).toInt();
    
    int newAngle = sec.toInt() * 6;
    if (newAngle >= 360) newAngle = 0;
    
    // 첫 번째 그리기
    if (firstDraw) {
        tft->fillScreen(TFT_BLACK);
        
        tft->drawCircle(sx, sy, 124, COLOR_GRAY1);
        
        tft->fillRect(70, 86, 12, 20, color3);
        tft->fillRect(84, 86, 12, 20, color3);
        tft->fillRect(150, 86, 12, 20, color3);
        tft->fillRect(164, 86, 12, 20, color3);
        
        tft->setTextDatum(MC_DATUM);
        tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
        tft->drawString("MONTH", 84, 78);
        tft->drawString("DAY", 162, 78);
        
        tft->setTextColor(COLOR_ORANGE, COLOR_BLACK);
        tft->drawString("System Control", 120, 174);
        tft->drawString("***", 120, 104);
        
        tft->fillTriangle(sx - 1, sy - 70, sx - 5, sy - 56, sx + 4, sy - 56, COLOR_ORANGE);
        
        tft->drawRect(186, 8, 36, 14, COLOR_CYAN);
        tft->fillRect(222, 12, 4, 6, COLOR_CYAN);
        
        tft->setTextSize(2);
        tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
        tft->drawString(getWeekday(day, month, year), 30, 15);
        tft->setTextSize(1);
        
        tft->setTextColor(COLOR_WHITE, color3);
        tft->drawString(m1, 77, 96, 2);
        tft->drawString(m2, 91, 96, 2);
        tft->drawString(d1, 157, 96, 2);
        tft->drawString(d2, 171, 96, 2);
        
        tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
        tft->drawString("FTH-KOREA.co", 120, 120, 2);
        
        // ✅ NTP 동기화 상태 아이콘
        if (ntpSyncedToday) {
            tft->fillCircle(10, 230, 4, COLOR_GREEN);  // 녹색 = 동기화됨
        } else {
            tft->fillCircle(10, 230, 4, COLOR_ORANGE); // 주황 = 미동기화
        }
        
        firstDraw = false;
        prevAngle = -1;
        prevRAngle = -1;
    }
    
    // 이전 회전 요소 지우기
    if (prevAngle >= 0 && prevAngle != newAngle) {
        tft->setTextColor(COLOR_BLACK, COLOR_BLACK);
        for (int i = 0; i < 12; i++) {
            int idx = (start[i] + prevAngle) % 360;
            tft->drawString(clockNumbers[i], (int)x[idx], (int)y[idx], 2);
            tft->drawLine((int)px[idx], (int)py[idx], (int)lx[idx], (int)ly[idx], COLOR_BLACK);
        }
        
        for (int i = 0; i < 60; i++) {
            int idx = (startP[i] + prevAngle) % 360;
            tft->fillCircle((int)px[idx], (int)py[idx], 2, COLOR_BLACK);
        }
    }
    
    // 이전 빨간 점 지우기
    if (prevRAngle >= 0 && prevRAngle != rAngle) {
        tft->fillCircle((int)px[prevRAngle], (int)py[prevRAngle], 7, COLOR_BLACK);
    }
    
    // 새 회전 요소 그리기
    if (prevAngle != newAngle) {
        tft->setTextDatum(MC_DATUM);
        tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
        for (int i = 0; i < 12; i++) {
            int idx = (start[i] + newAngle) % 360;
            tft->drawString(clockNumbers[i], (int)x[idx], (int)y[idx], 2);
            tft->drawLine((int)px[idx], (int)py[idx], (int)lx[idx], (int)ly[idx], color1);
        }
        
        for (int i = 0; i < 60; i++) {
            int idx = (startP[i] + newAngle) % 360;
            tft->fillCircle((int)px[idx], (int)py[idx], 1, color1);
        }
        
        prevAngle = newAngle;
    }
    
    // 빨간 점 애니메이션
    rAngle -= 2;
    if (rAngle <= 0) rAngle = 359;
    
    tft->fillCircle((int)px[rAngle], (int)py[rAngle], 6, COLOR_RED);
    prevRAngle = rAngle;
    
    // 초 업데이트
    if (sec != prevSec) {
        tft->setFreeFont(&DSEG7_Modern_Bold_20);
        tft->setTextDatum(MC_DATUM);
        tft->setTextColor(COLOR_BLACK, COLOR_BLACK);
        if (prevSec.length() > 0) {
            tft->drawString(prevSec, sx, sy - 36);
        }
        
        tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
        tft->drawString(sec, sx, sy - 36);
        tft->setTextFont(0);
        
        prevSec = sec;
    }
    
    // 시:분 업데이트
    if (hr != prevHr || minStr != prevMin) {
        String prevTime = prevHr + ":" + prevMin;
        String newTime = hr + ":" + minStr;
        
        tft->setFreeFont(&DSEG7_Classic_Regular_28);
        tft->setTextDatum(MC_DATUM);
        
        if (prevHr.length() > 0) {
            tft->setTextColor(COLOR_BLACK, COLOR_BLACK);
            tft->drawString(prevTime, sx, sy + 28);
        }
        
        tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
        tft->drawString(newTime, sx, sy + 28);
        tft->setTextFont(0);
        
        prevHr = hr;
        prevMin = minStr;
    }
    
    // 배터리 업데이트
    int battPct = (int)getBatteryPercentage();
    if (abs(battPct - prevBatteryPct) >= 5 || prevBatteryPct < 0) {
        tft->setTextDatum(MC_DATUM);
        tft->setTextSize(2);
        
        tft->setTextColor(COLOR_BLACK, COLOR_BLACK);
        tft->fillRect(185, 5, 35, 20, COLOR_BLACK);
        
        tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
        tft->drawString(String(battPct), 204, 15);
        tft->setTextSize(1);
        
        prevBatteryPct = battPct;
    }
    
    // 밝기 업데이트
    if (brightness != prevBrightness) {
        tft->setTextDatum(MC_DATUM);
        tft->setTextSize(2);
        
        tft->fillRect(15, 220, 80, 20, COLOR_BLACK);
        
        tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
        tft->drawString("B:" + String(brightness), 50, 230);
        tft->setTextSize(1);
        
        prevBrightness = brightness;
    }
}

// ============================================
// Menu Screen - NTP 동기화 추가
// ============================================
void WatchUICLASS::drawMenuScreen() {
    if (!firstDraw) return;
    
    TFT_eSPI *tft = ttgo->tft;
    
    tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->setFreeFont(&DSEG7_Classic_Regular_28);
    tft->drawString("MENU", 120, 25);
    tft->setFreeFont(NULL);
    
    tft->setTextFont(2);
    
    const char* menuItems[] = {
        "1. NTP Sync",       // ✅ 추가
        "2. Battery Info", 
        "3. Settings",
        "4. Exit"
    };
    
    int yPos = 70;
    for (int i = 0; i < 4; i++) {
        tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
        tft->drawString(menuItems[i], 120, yPos);
        yPos += 35;
    }
    
    // NTP 상태 표시
    tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
    tft->setTextFont(1);
    if (ntpSyncedToday) {
        tft->drawString("Last sync: " + getLastSyncTimeStr(), 120, 200);
    } else {
        tft->drawString("Not synced today", 120, 200);
    }
    
    tft->drawString("Double tap to exit", 120, 220);
    
    firstDraw = false;
}

// ============================================
// NTP Sync Screen
// ============================================
void WatchUICLASS::drawNtpSyncScreen() {
    TFT_eSPI *tft = ttgo->tft;
    
    if (firstDraw) {
        tft->fillScreen(TFT_BLACK);
        firstDraw = false;
    }
    
    tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->setFreeFont(&DSEG7_Classic_Regular_28);
    tft->drawString("NTP", 120, 25);
    tft->setFreeFont(NULL);
    
    tft->setTextFont(2);
    
    // WiFi 정보
    tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
    tft->drawString("WiFi:", 50, 60);
    tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft->drawString(WIFI_SSID, 150, 60);
    
    // 마지막 동기화
    tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
    tft->drawString("Last:", 50, 85);
    tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft->drawString(getLastSyncTimeStr(), 150, 85);
    
    // 상태 표시
    tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
    tft->drawString("Status:", 50, 110);
    
    String statusStr;
    uint16_t statusColor;
    
    switch (ntpStatus) {
        case NtpStatus::IDLE:
            statusStr = "Ready";
            statusColor = COLOR_WHITE;
            break;
        case NtpStatus::CONNECTING_WIFI:
            statusStr = "Connecting WiFi...";
            statusColor = COLOR_YELLOW;
            break;
        case NtpStatus::WIFI_CONNECTED:
            statusStr = "WiFi OK";
            statusColor = COLOR_GREEN;
            break;
        case NtpStatus::SYNCING:
            statusStr = "Syncing...";
            statusColor = COLOR_YELLOW;
            break;
        case NtpStatus::SUCCESS:
            statusStr = "Sync OK!";
            statusColor = COLOR_GREEN;
            break;
        case NtpStatus::FAILED_WIFI:
            statusStr = "WiFi Failed";
            statusColor = COLOR_RED;
            break;
        case NtpStatus::FAILED_NTP:
            statusStr = "NTP Failed";
            statusColor = COLOR_RED;
            break;
    }
    
    // 이전 상태 지우기
    tft->fillRect(100, 100, 140, 25, COLOR_BLACK);
    tft->setTextColor(statusColor, COLOR_BLACK);
    tft->drawString(statusStr, 170, 110);
    
    // Sync 버튼
    tft->drawRect(60, 140, 120, 35, COLOR_CYAN);
    tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
    tft->drawString("[ SYNC NOW ]", 120, 157);
    
    // 뒤로가기
    tft->setTextColor(COLOR_GRAY1, COLOR_BLACK);
    tft->setTextFont(1);
    tft->drawString("Tap button to sync", 120, 190);
    tft->drawString("Tap here to go back", 120, 220);
}

// ============================================
// Battery Info
// ============================================
void WatchUICLASS::drawBatteryInfoScreen() {
    if (!firstDraw) return;
    
    TFT_eSPI *tft = ttgo->tft;
    
    float voltage = power->getBattVoltage() / 1000.0;
    float percentage = getBatteryPercentage();
    bool isCharging = power->isChargeing();
    
    tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->setFreeFont(&DSEG7_Classic_Regular_28);
    tft->drawString("BATT", 120, 30);
    tft->setFreeFont(NULL);
    
    tft->setTextFont(2);
    
    tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
    tft->drawString("Voltage:", 60, 80);
    tft->setTextColor(COLOR_WHITE, COLOR_BLACK);
    tft->drawString(String(voltage, 2) + " V", 170, 80);
    
    tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
    tft->drawString("Level:", 60, 110);
    tft->setTextColor(percentage > 20 ? COLOR_WHITE : COLOR_RED, COLOR_BLACK);
    tft->drawString(String((int)percentage) + " %", 170, 110);
    
    tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
    tft->drawString("Status:", 60, 140);
    tft->setTextColor(isCharging ? TFT_GREEN : COLOR_WHITE, COLOR_BLACK);
    tft->drawString(isCharging ? "Charging" : "Discharge", 170, 140);
    
    int battWidth = 100;
    int battHeight = 40;
    int battX = (240 - battWidth) / 2;
    int battY = 170;
    
    tft->drawRect(battX, battY, battWidth, battHeight, COLOR_WHITE);
    tft->fillRect(battX + battWidth, battY + 10, 8, 20, COLOR_WHITE);
    
    int fillWidth = (int)((battWidth - 4) * percentage / 100);
    uint16_t fillColor = percentage > 50 ? TFT_GREEN : 
                         percentage > 20 ? COLOR_ORANGE : COLOR_RED;
    tft->fillRect(battX + 2, battY + 2, fillWidth, battHeight - 4, fillColor);
    
    tft->setTextColor(COLOR_CYAN, COLOR_BLACK);
    tft->setTextFont(1);
    tft->drawString("Tap to go back", 120, 225);
    
    firstDraw = false;
}

// ============================================
// Utility Functions
// ============================================
float WatchUICLASS::getBatteryPercentage() {
    if (power == nullptr) return 0;
    
    float voltage = power->getBattVoltage() / 1000.0;
    const float minVoltage = 3.3;
    const float maxVoltage = 4.2;
    
    float percentage = (voltage - minVoltage) / (maxVoltage - minVoltage) * 100;
    
    return constrain(percentage, 0, 100);
}

String WatchUICLASS::getWeekday(int day, int month, int year) {
    int m = month;
    int y = year;
    
    if (m < 3) {
        m += 12;
        y -= 1;
    }
    
    int K = y % 100;
    int J = y / 100;
    
    int h = (day + (13 * (m + 1)) / 5 + K + K / 4 + J / 4 - 2 * J) % 7;
    
    if (h < 0) h += 7;
    
    return dayNames[h];
}
