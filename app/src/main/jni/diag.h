#pragma once
// DIAGNOSTICO solo lectura: vuelca 8-bytes del objeto LocalPlayer y los muestra EN EL MENU (sin logcat).
#include <cstdio>
#include <string>
#include <vector>
#include "KittyMemory/KittyMemory.h"

namespace Diag {

static bool dumpRequested = false;   // se pone en true desde el menu; NormalTick lo atiende una vez
static int dumpSpan = 0x800;
static std::vector<std::string> lines; // lo que se muestra en el menu

static inline void dumpPlayer(void *player) {
    lines.clear();
    if (!player) { lines.push_back("player = NULL"); return; }
    char buf[96];
    snprintf(buf, sizeof(buf), "player %p", player);
    lines.push_back(buf);
    for (int off = 0; off < dumpSpan; off += 8) {
        unsigned long long v = 0;
        void *addr = (char *) player + off;
        if (KittyMemory::memRead(&v, addr, 8) != KittyMemory::SUCCESS) {
            snprintf(buf, sizeof(buf), "%04x: <no legible>", off);
        } else {
            snprintf(buf, sizeof(buf), "%04x: %016llx", off, v);
        }
        lines.push_back(buf);
    }
}

} // namespace Diag
