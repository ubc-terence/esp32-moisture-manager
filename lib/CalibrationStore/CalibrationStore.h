#pragma once
#include "Calibration.h"

namespace CalibrationStore {

// Loads persisted dry/wet raw values from NVS into cal. If none are
// stored yet, cal keeps its constructor defaults.
void load(Calibration &cal);

// Persists a new dry calibration point (also updates cal in place).
void saveDry(Calibration &cal, int raw);

// Persists a new wet calibration point (also updates cal in place).
void saveWet(Calibration &cal, int raw);

}
