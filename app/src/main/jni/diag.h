#pragma once
// DIAGNOSTICO solo lectura: vuelca 8-bytes del objeto LocalPlayer para encontrar el campo del offhand.
// Uso: activar "Dump Player" en el menu, con el tótem en la mano secundaria y luego sin el. Comparar en logcat
// (etiqueta "VanthDiag"). Lee con KittyMemory::memRead, que devuelve error en vez de cerrar el juego.
#include <android/log.h>
#include <cstring>
#include "KittyMemory/KittyMemory.h"

namespace Diag {

static bool dumpRequested = false;   // se pone en true desde el menu; NormalTick lo atiende una vez
static int dumpSpan = 0x800;         // bytes a volcar desde el inicio del objeto

static inline void dumpPlayer(void *player) {
    if (!player) return;
    __android_log_print(ANDROID_LOG_INFO, "VanthDiag", "=== dump player %p span %x ===", player, dumpSpan);
    for (int off = 0; off < dumpSpan; off += 8) {
        unsigned long long v = 0;
        void *addr = (char *) player + off;
        if (KittyMemory::memRead(&v, addr, 8) != KittyMemory::SUCCESS) {
            __android_log_print(ANDROID_LOG_INFO, "VanthDiag", "%04x: <no legible>", off);
            continue;
        }
        __android_log_print(ANDROID_LOG_INFO, "VanthDiag", "%04x: %016llx", off, v);
    }
    __android_log_print(ANDROID_LOG_INFO, "VanthDiag", "=== fin dump ===");
}

} // namespace Diag
