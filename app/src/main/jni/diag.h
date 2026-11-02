#pragma once
// DIAGNOSTICO solo lectura: vuelca 8-bytes del objeto LocalPlayer y los muestra en el menu.
// Marca con "*" las lineas que cambian respecto al volcado anterior (para comparar tótem en mano vs offhand).
#include <cstdio>
#include <string>
#include <vector>
#include "KittyMemory/KittyMemory.h"

namespace Diag {

static bool dumpRequested = false;
static int dumpSpan = 0x800;
static std::vector<std::string> lines;
static std::vector<unsigned long long> prevVals;

static inline void dumpPlayer(void *player) {
    lines.clear();
    if (!player) { lines.push_back("player = NULL"); return; }
    std::vector<unsigned long long> vals;
    vals.reserve(dumpSpan / 8);
    for (int off = 0; off < dumpSpan; off += 8) {
        unsigned long long v = 0;
        if (KittyMemory::memRead(&v, (char *) player + off, 8) != KittyMemory::SUCCESS) v = 0xDEADDEADDEADDEADULL;
        vals.push_back(v);
    }
    char buf[96];
    snprintf(buf, sizeof(buf), "player %p  (* = cambio vs anterior)", player);
    lines.push_back(buf);
    for (size_t i = 0; i < vals.size(); i++) {
        bool changed = prevVals.size() == vals.size() && prevVals[i] != vals[i];
        if (vals[i] == 0xDEADDEADDEADDEADULL)
            snprintf(buf, sizeof(buf), "%04x: <no legible>%s", (unsigned) (i * 8), changed ? " *" : "");
        else
            snprintf(buf, sizeof(buf), "%04x: %016llx%s", (unsigned) (i * 8), vals[i], changed ? " *" : "");
        lines.push_back(buf);
    }
    prevVals = vals;
}

} // namespace Diag
