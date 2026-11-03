#pragma once
// DIAGNOSTICO solo lectura. Modo "solo cambios":
//  1a pulsacion: guarda la referencia (baseline) y muestra "referencia guardada".
//  siguientes:   muestra SOLO las lineas cuyo valor cambio respecto a la referencia anterior.
#include <cstdio>

// Carpeta donde el juego ya guarda sus archivos (escribible sin permisos especiales).
static const char *DiagFilePath = "/sdcard/Android/data/com.mojang.minecraftpe/files/vanth_diag.txt";
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>

// Lectura que NO crashea: si la direccion no es valida, pread devuelve error en vez de SIGSEGV.
static inline bool SafeRead(const void *addr, void *out, size_t len) {
    static int memFd = -1;
    if (memFd < 0) memFd = open("/proc/self/mem", O_RDONLY);
    if (memFd < 0) return false;
    ssize_t n = pread(memFd, out, len, (off_t) (uintptr_t) addr);
    return n == (ssize_t) len;
}


namespace Diag {

static bool dumpRequested = false;
static int dumpSpan = 0x800;
static std::vector<std::string> lines;
static std::vector<unsigned long long> prevVals;
static std::vector<unsigned long long> prevL2;
static const unsigned long long NOREAD = 0xDEADDEADDEADDEADULL;

static inline void dumpPlayer(void *player) {
    lines.clear();
    if (!player) { lines.push_back("player = NULL"); return; }
    std::vector<unsigned long long> vals;
    vals.reserve(dumpSpan / 8);
    for (int off = 0; off < dumpSpan; off += 8) {
        unsigned long long v = NOREAD;
        if (!SafeRead((char *) player + off, &v, 8)) v = NOREAD;
        vals.push_back(v);
    }
    char buf[96];
    bool firstPress = prevVals.size() != vals.size();
    if (firstPress) lines.push_back("Referencia guardada. Pulsa otra vez tras cambiar el tótem.");
    if (!firstPress) {
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
    }
    prevVals = vals; // la siguiente pulsacion compara contra esta

    // NIVEL 2: por cada puntero tipo heap (b4...) dentro del jugador, volcar 0x100 bytes del objeto.
    // Asi una sola pulsacion cubre el inventario aunque este detras de un puntero.
    lines.push_back("--- nivel 2 (punteros b4...) ---  (* = cambio vs referencia)");
    std::vector<unsigned long long> curL2; // valores en orden, para comparar con la referencia
    for (size_t i = 0; i < vals.size(); i++) {
        unsigned long long p = vals[i];
        if ((p >> 56) != 0xB4ULL || p == NOREAD) continue;
        char hb[96];
        snprintf(hb, sizeof(hb), "[%04x] -> %016llx", (unsigned) (i * 8), p);
        lines.push_back(hb);
        for (int off = 0; off < 0x100; off += 8) {
            unsigned long long v = NOREAD;
            if (!SafeRead((void *) (p + off), &v, 8)) v = NOREAD;
            curL2.push_back(v);
            size_t k = curL2.size() - 1;
            bool chg = !firstPress && k < prevL2.size() && prevL2[k] != v;
            char lb[120];
            if (v == NOREAD) snprintf(lb, sizeof(lb), "   +%03x: <no legible>%s", off, chg ? " *" : "");
            else snprintf(lb, sizeof(lb), "   +%03x: %016llx%s", off, v, chg ? " *" : "");
            lines.push_back(lb);
        }
    }

    prevL2 = curL2;

    // Guardar tambien en archivo (el portapapeles no funciona en esta build de ImGui en Android)
    FILE *f = fopen(DiagFilePath, "w");
    if (f) {
        for (const std::string &ln : lines) { fputs(ln.c_str(), f); fputc('\n', f); }
        fclose(f);
    }
}

} // namespace Diag
