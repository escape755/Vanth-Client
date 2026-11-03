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
    // write() desde memoria invalida devuelve EFAULT en vez de crashear. Leemos el resultado por un pipe.
    // (/proc/self/mem suele estar bloqueado en Android para apps, por eso no se usa.)
    static int pfd[2] = {-1, -1};
    if (pfd[0] < 0) {
        if (pipe(pfd) != 0) return false;
    }
    uintptr_t a = (uintptr_t) addr & 0x00FFFFFFFFFFFFFFULL;
    ssize_t w = write(pfd[1], (const void *) a, len);
    if (w != (ssize_t) len) {
        // vaciar lo que haya quedado en el pipe si fue parcial
        if (w > 0) { char tmp[64]; ssize_t r = read(pfd[0], tmp, (size_t) w); (void) r; }
        return false;
    }
    ssize_t r = read(pfd[0], out, len);
    return r == (ssize_t) len;
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

    // SONDA (solo lectura): candidato del offhand segun getOffhandSlot -> this+0x2F0 (752).
    // Se lee el puntero y 0x40 bytes de lo que apunta, para comparar con tótem en mano principal vs offhand.
    {
        unsigned long long cand = NOREAD;
        if (SafeRead((char *) player + 0x2F0, &cand, 8)) {
            char pb[120];
            snprintf(pb, sizeof(pb), "--- sonda: player+0x2F0 = %016llx ---", cand);
            lines.push_back(pb);
            if (cand != 0) {
                for (int off = 0; off < 0x40; off += 8) {
                    unsigned long long v = NOREAD;
                    if (!SafeRead((void *) (cand + off), &v, 8)) v = NOREAD;
                    char sb[96];
                    if (v == NOREAD) snprintf(sb, sizeof(sb), "   sonda+%02x: <no legible>", off);
                    else snprintf(sb, sizeof(sb), "   sonda+%02x: %016llx", off, v);
                    lines.push_back(sb);
                }
            }
        } else {
            lines.push_back("--- sonda: player+0x2F0 no legible ---");
        }
    }

    // Guardar tambien en archivo (el portapapeles no funciona en esta build de ImGui en Android)
    FILE *f = fopen(DiagFilePath, "w");
    if (f) {
        for (const std::string &ln : lines) { fputs(ln.c_str(), f); fputc('\n', f); }
        fclose(f);
    }
}

} // namespace Diag
