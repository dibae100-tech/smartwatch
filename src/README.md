# TTGO Watch Gauge

**TTGO T-Watch 2020 V1을 위한 커스텀 시계 펌웨어**

![Version](https://img.shields.io/badge/version-2.4-blue)
![Platform](https://img.shields.io/badge/platform-ESP32-green)
![License](https://img.shields.io/badge/license-MIT-yellow)

---

## 📖 소개

TTGO Watch Gauge는 TTGO T-Watch 2020 V1을 위한 커스텀 워치페이스 프로젝트입니다. 
회전하는 숫자판과 애니메이션 효과가 적용된 독특한 디자인의 시계 UI를 제공하며, 
NTP를 통한 자동 시간 동기화, 배터리 모니터링, 터치 기반 메뉴 시스템을 지원합니다.

### ✨ 주요 특징

- 🎨 **애니메이션 시계 화면** - 회전하는 숫자판과 초침 효과
- 🌐 **NTP 시간 동기화** - WiFi를 통한 자동 시간 맞춤
- 🔋 **배터리 모니터링** - 전압, 퍼센트, 충전 상태 표시
- 👆 **터치 인터페이스** - 더블탭 메뉴, 싱글탭 선택
- 💤 **자동 슬립** - 배터리 절약을 위한 딥슬립 모드
- ⚡ **깜빡임 없는 디스플레이** - 부분 업데이트 방식

---

## 🛠️ 하드웨어 요구사항

| 항목 | 사양 |
|------|------|
| **보드** | TTGO T-Watch 2020 V1 |
| **MCU** | ESP32 |
| **디스플레이** | 1.54" IPS LCD (240x240) |
| **터치** | Capacitive Touch |
| **RTC** | PCF8563 |
| **전원** | AXP202 PMU |

---

## 📁 프로젝트 구조

```
ttgo-watch-gauge/
├── ttgoWatchGauge_v24.ino   # 메인 스케치
├── config.h                  # 설정 (WiFi, NTP, 색상 등)
├── UI-main.h                 # UI 클래스 헤더
├── UI-main.cpp               # UI 구현 (시계, 메뉴, NTP)
├── globals.cpp               # 전역 변수
├── fonts.h                   # DSEG7 폰트 데이터
└── README.md                 # 이 문서
```

---

## ⚙️ 설치 방법

### 1. 필수 라이브러리 설치

Arduino IDE에서 다음 라이브러리를 설치하세요:

- **TTGO TWatch Library** (LilyGoWatch)
  - GitHub: https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library
- **TFT_eSPI** (TWatch 라이브러리에 포함)

### 2. Arduino IDE 보드 설정

```
Tools → Board → ESP32 Dev Module
Tools → Partition Scheme → Huge APP (3MB No OTA/1MB SPIFFS)
Tools → Upload Speed → 921600
Tools → Port → (해당 COM 포트)
```

### 3. WiFi 설정 (config.h)

```cpp
#define WIFI_SSID       "YOUR_WIFI_SSID"      // WiFi 이름
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"  // WiFi 비밀번호
```

### 4. 시간대 설정 (선택사항)

기본값은 한국 시간(GMT+9)입니다. 다른 시간대를 사용하려면:

```cpp
// config.h
#define GMT_OFFSET_SEC      32400   // 초 단위 (9시간 = 32400초)
#define DAYLIGHT_OFFSET_SEC 0       // 서머타임 (한국은 0)
```

| 지역 | GMT_OFFSET_SEC |
|------|----------------|
| 한국/일본 | 32400 (GMT+9) |
| 중국 | 28800 (GMT+8) |
| 미국 동부 | -18000 (GMT-5) |
| 미국 서부 | -28800 (GMT-8) |
| 영국 | 0 (GMT+0) |

### 5. 업로드

Arduino IDE에서 업로드 버튼 클릭

---

## 📱 사용 방법

### 시계 화면 (메인)

```
┌─────────────────────────────┐
│  MON          [배터리 %]    │
│         45                  │
│      50    40               │
│                             │
│    55   [MM:DD]   35        │
│          **                 │
│     0    [초]    30         │
│         [시:분]             │
│    05  System   25          │
│      10   15   20           │
│ 🟢 B:255                    │
└─────────────────────────────┘
```

- **회전 숫자판**: 초에 따라 회전
- **빨간 점**: 애니메이션 효과
- **동기화 상태**: 🟢 동기화됨 / 🟠 미동기화

### 터치 조작

| 동작 | 기능 |
|------|------|
| **더블탭** | 메뉴 ↔ 시계 전환 |
| **싱글탭** | 메뉴 항목 선택 |
| **화면 터치** | 밝기 최대로 복원 |

### 메뉴 구조

```
MENU
├── 1. NTP Sync      → NTP 시간 동기화 화면
├── 2. Battery Info  → 배터리 상세 정보
├── 3. Settings      → (예정)
└── 4. Exit          → 시계 화면으로
```

### NTP 동기화

1. 메뉴에서 **1. NTP Sync** 선택
2. **[SYNC NOW]** 버튼 터치
3. WiFi 연결 → NTP 동기화 → 자동 WiFi 끄기

```
NTP
─────────────────
WiFi:    YOUR_SSID
Last:    5m ago
Status:  Sync OK!

    [ SYNC NOW ]

  Tap button to sync
```

---

## 🔧 설정 옵션

### config.h 주요 설정

```cpp
// 타이밍
#define AUTO_SLEEP_TIME     30000   // 자동 슬립 (30초)
#define DOUBLE_TAP_INTERVAL 400     // 더블탭 간격 (ms)

// 밝기
#define FULL_BRIGHTNESS     255     // 최대 밝기
#define DIM_BRIGHTNESS      100     // 감소 밝기
#define MIN_BRIGHTNESS      30      // 최소 밝기

// NTP 서버
#define NTP_SERVER1     "pool.ntp.org"
#define NTP_SERVER2     "time.google.com"
#define NTP_SERVER3     "time.nist.gov"

// WiFi 타임아웃
#define WIFI_TIMEOUT_SEC    15      // 연결 대기 시간
```

---

## 🔋 전원 관리

### 배터리 정보

| 전압 | 충전 상태 |
|------|----------|
| 4.2V | 100% (완충) |
| 3.9V | ~65% |
| 3.7V | ~35% |
| 3.3V | 0% (방전) |

### 슬립 모드

- **15초 후**: 밝기 감소 (DIM_BRIGHTNESS)
- **30초 후**: 딥슬립 진입
- **깨우기**: 측면 버튼 누르기

### WiFi 전력 절약

NTP 동기화 완료 후 WiFi가 자동으로 꺼집니다:
```cpp
// 동기화 완료 후 자동 실행
WiFi.disconnect(true);
WiFi.mode(WIFI_OFF);
```

---

## 📊 시스템 아키텍처

```
┌──────────────────────────────────────────┐
│              Main Loop                    │
│  ┌────────────┐    ┌─────────────────┐   │
│  │checkStatus │ →  │    updateUI     │   │
│  │ - Touch    │    │ - drawClockFace │   │
│  │ - IRQ      │    │ - drawMenu      │   │
│  │ - Sleep    │    │ - drawNtpSync   │   │
│  └────────────┘    └─────────────────┘   │
└──────────────────────────────────────────┘
          ↓                    ↓
┌─────────────────┐  ┌─────────────────────┐
│   State Machine │  │  Partial Update     │
│ - CLOCK         │  │ - prevSec/Min/Hr    │
│ - MENU          │  │ - prevAngle         │
│ - NTP_SYNC      │  │ - firstDraw flag    │
│ - BATTERY_INFO  │  └─────────────────────┘
└─────────────────┘
```

---

## 🐛 트러블슈팅

### 화면이 켜지지 않음

1. **Partition Scheme** 확인 → `Huge APP (3MB No OTA/1MB SPIFFS)`
2. 배터리 충전 상태 확인
3. 시리얼 모니터로 로그 확인

### NTP 동기화 실패

1. WiFi SSID/비밀번호 확인
2. WiFi 신호 강도 확인
3. 방화벽에서 NTP 포트(123) 차단 여부 확인

### 시간이 맞지 않음

1. `GMT_OFFSET_SEC` 값 확인
2. NTP 동기화 수동 실행
3. RTC 배터리 상태 확인

### 터치가 반응하지 않음

1. 화면 보호 필름 제거 또는 터치용 필름 사용
2. 슬립 모드에서는 측면 버튼으로 깨우기

---

## 📜 버전 히스토리

| 버전 | 날짜 | 변경 사항 |
|------|------|----------|
| v2.4 | 2025.01 | NTP 시간 동기화 추가 |
| v2.3 | 2025.01 | 화면 방향 수정, 깜빡임 제거 |
| v2.2 | 2025.01 | 스프라이트 → 직접 렌더링 전환 |
| v2.1 | 2025.01 | TFT 초기화 수정 |
| v2.0 | 2025.01 | 클래스 기반 리팩토링 |
| v1.0 | 2025.03 | 최초 버전 |

---

## 🔮 향후 계획

- [ ] 다양한 워치페이스 테마
- [ ] 알람 기능
- [ ] 스톱워치/타이머
- [ ] 걸음 수 측정 (가속도계)
- [ ] 날씨 정보 표시
- [ ] BLE 스마트폰 연동
- [ ] OTA 펌웨어 업데이트

---

## 📄 라이선스

MIT License

```
Copyright (c) 2025 FTH-KOREA

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

---

## 🎬 참고 자료

### 원본 프로젝트
- 🔗 **[VolosR/TwatchWatch](https://github.com/VolosR/TwatchWatch)** - 원본 워치페이스 프로젝트
  - 이 프로젝트의 시계 UI 디자인과 애니메이션 컨셉의 기반이 되었습니다.

### YouTube 튜토리얼
- 📺 [TTGO Watch Gauge 튜토리얼](https://www.youtube.com/watch?v=UUzywu2RrHc) - VolosR

---

## 🔄 원본 대비 변경 사항

이 프로젝트는 [VolosR/TwatchWatch](https://github.com/VolosR/TwatchWatch)를 기반으로 다음 기능을 추가/개선했습니다:

| 항목 | 원본 | 이 프로젝트 |
|------|------|------------|
| **렌더링** | 스프라이트 기반 | 직접 TFT 렌더링 (메모리 최적화) |
| **화면 업데이트** | 전체 갱신 | 부분 업데이트 (깜빡임 제거) |
| **NTP 동기화** | ❌ | ✅ WiFi 기반 자동 시간 맞춤 |
| **메뉴 시스템** | 기본 | 확장된 터치 메뉴 |
| **배터리 화면** | ❌ | ✅ 상세 배터리 정보 |
| **코드 구조** | 단일 파일 | 클래스 기반 모듈화 |
| **슬립 모드** | 기본 | 개선된 전력 관리 |

---

## 🙏 감사

- **[VolosR](https://github.com/VolosR)** - 원본 TwatchWatch 프로젝트 및 YouTube 튜토리얼
- [LilyGo](https://github.com/Xinyuan-LilyGO) - TTGO Watch 하드웨어 및 라이브러리
- [Bodmer](https://github.com/Bodmer) - TFT_eSPI 라이브러리
- DSEG7 폰트 제작자

---

## 📞 문의

프로젝트 관련 문의나 버그 리포트는 이슈를 등록해 주세요.

**FTH-KOREA.co**
