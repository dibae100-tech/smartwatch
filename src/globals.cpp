// ============================================
// Global Variables Definition
// Version: 2.4
// ============================================

#include "config.h"

// TTGO Class instance
TTGOClass *ttgo = nullptr;

// Power management
AXP20X_Class *power = nullptr;

// IRQ flag
volatile bool irq = false;

// Coordinate arrays for clock face
float x[360];
float y[360];
