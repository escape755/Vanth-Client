#pragma once
// DIAGNOSTICO solo lectura. Modo "solo cambios":
//  1a pulsacion: guarda la referencia (baseline) y muestra "referencia guardada".
//  siguientes:   muestra SOLO las lineas cuyo valor cambio respecto a la referencia anterior.
#include <cstdio>

// Carpeta donde el juego ya guarda sus archivos (escribible sin permisos especiales).
static const char *DiagFilePath = "/sdcard/Android/data/com.mojang.minecraftpe/files/vanth_diag.txt";
#include <string>
#include <vector>
#include "KittyMemory/KittyMemory.h"

namespace Diag {

static bool dumpRequested = false;
static int dumpSpan = 0x800;
static std::vector<std::string> lines;
static std::vector<unsigned long long> prevVals;
static const unsigned long long NOREAD = 0xDEADDEADDEADDEADULL;

static inline void dumpPlayer(void *player) {
    lines.clear();
    if (!player) { lines.push_back("player = NULL"); return; }
    std::vector<unsigned long long> vals;
    vals.reserve(dumpSpan / 8);
    for (int off = 0; off < dumpSpan; off += 8) {
        unsigned long long v = NOREAD;
        if (KittyMemory::memRead(&v, (char *) player + off, 8) != KittyMemory::SUCCESS) v = NOREAD;
        vals.push_back(v);
    }
    char buf[96];
    if (prevVals.size() != vals.size()) {
        lines.push_back("Referencia guardada. Pulsa otra vez tras cambiar el tótem.");
        prevVals = vals;
        return;
    }
    snprintf(buf, sizeof(buf), "Cambios vs referencia (objeto %p):", player);
    lines.push_back(buf);
    int n = 0;
    for (size_t i = 0; i < vals.size(); i++) {
        if (prevVals[i] == vals[i]) continue;
        n++;
        if (vals[i] == NOREAD)
            snprintf(buf, sizeof(buf), "%04x: <no legible>", (unsigned) (i * 8));
        else
            snprintf(buf, sizeof(buf), "%04x: %016llx  (antes %016llx)", (unsigned) (i * 8), vals[i], prevVals[i]);
        lines.push_back(buf);
    }
    if (n == 0) lines.push_back("(ningun cambio)");
    prevVals = vals; // la siguiente pulsacion compara contra esta

    // Guardar tambien en archivo (el portapapeles no funciona en esta build de ImGui en Android)
    FILE *f = fopen(DiagFilePath, "w");
    if (f) {
        for (const std::string &ln : lines) { fputs(ln.c_str(), f); fputc('\n', f); }
        fclose(f);
    }
}

} // namespace Diag
