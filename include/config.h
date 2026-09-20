#pragma once

// Build-time configuration for ESP32 Moisture Manager.
//
// These are the defaults that ship in the repo. To use your own values without
// touching tracked files, copy config.local.example.h to config.local.h (which
// is git-ignored) and define any of the MM_* macros there.

#if __has_include("config.local.h")
#include "config.local.h"
#endif

// Wi-Fi access point the device hosts for the dashboard.
#ifndef MM_AP_SSID
#define MM_AP_SSID "MoistureManager"
#endif

// WPA2 password for the access point. This default is public (it is in the
// repo), so override it in config.local.h for any device you actually deploy.
#ifndef MM_AP_PASSWORD
#define MM_AP_PASSWORD "plant1234"
#endif

// WPA2 requires 8-63 characters; sizeof includes the terminating NUL.
static_assert(sizeof(MM_AP_PASSWORD) - 1 >= 8, "MM_AP_PASSWORD must be at least 8 characters (WPA2)");
static_assert(sizeof(MM_AP_PASSWORD) - 1 <= 63, "MM_AP_PASSWORD must be at most 63 characters (WPA2)");
