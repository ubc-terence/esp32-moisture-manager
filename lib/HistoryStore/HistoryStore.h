#pragma once
#include "HistoryBuffer.h"

namespace HistoryStore {

// Loads persisted samples from LittleFS into buf. Returns true if a
// previous history file was found and loaded.
bool load(HistoryBuffer &buf);

// Overwrites the on-flash history file with buf's current contents.
void save(const HistoryBuffer &buf);

}
