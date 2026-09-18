#include "CalibrationStore.h"
#include <Preferences.h>

namespace {
constexpr const char *NAMESPACE = "cal";
constexpr const char *KEY_DRY = "dry";
constexpr const char *KEY_WET = "wet";
}

namespace CalibrationStore {

void load(Calibration &cal) {
    Preferences prefs;
    prefs.begin(NAMESPACE, true);
    int dry = prefs.getInt(KEY_DRY, cal.dryRaw());
    int wet = prefs.getInt(KEY_WET, cal.wetRaw());
    prefs.end();
    cal.setDry(dry);
    cal.setWet(wet);
}

void saveDry(Calibration &cal, int raw) {
    cal.setDry(raw);
    Preferences prefs;
    prefs.begin(NAMESPACE, false);
    prefs.putInt(KEY_DRY, raw);
    prefs.end();
}

void saveWet(Calibration &cal, int raw) {
    cal.setWet(raw);
    Preferences prefs;
    prefs.begin(NAMESPACE, false);
    prefs.putInt(KEY_WET, raw);
    prefs.end();
}

}
