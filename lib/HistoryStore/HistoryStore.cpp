#include "HistoryStore.h"
#include <LittleFS.h>

namespace {
constexpr const char *HISTORY_PATH = "/history.bin";
constexpr const char *HISTORY_TMP_PATH = "/history.tmp";
}

namespace HistoryStore {

bool load(HistoryBuffer &buf) {
    if (!LittleFS.exists(HISTORY_PATH)) {
        return false;
    }

    File f = LittleFS.open(HISTORY_PATH, "r");
    if (!f) {
        return false;
    }

    std::vector<HistorySample> records;
    HistorySample sample;
    while (f.read(reinterpret_cast<uint8_t *>(&sample), sizeof(sample)) == sizeof(sample)) {
        records.push_back(sample);
    }
    f.close();

    buf.restoreFrom(records);
    return true;
}

void save(const HistoryBuffer &buf) {
    // Write to a temp file and rename over the real path so a power loss
    // mid-write can't truncate/corrupt the existing history file — the
    // worst case is losing the not-yet-renamed temp file, not the whole
    // persisted history.
    File f = LittleFS.open(HISTORY_TMP_PATH, "w");
    if (!f) {
        return;
    }
    for (const auto &sample : buf.records()) {
        f.write(reinterpret_cast<const uint8_t *>(&sample), sizeof(sample));
    }
    f.close();

    // LittleFS's rename() atomically replaces an existing destination file
    // on its own — do not remove() the destination first, that would open a
    // window where neither file exists if power is lost between the two calls.
    LittleFS.rename(HISTORY_TMP_PATH, HISTORY_PATH);
}

}
