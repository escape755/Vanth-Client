#pragma once
// Captura de transacciones de inventario REALES enviadas por el juego.
// Sirve para medir el layout de InventoryAction / InventoryTransaction en 1.21.111 (ARM64).
//
// Seguridad:
//  - El hook se instala solo cuando pulsas "Capturar transaccion" en el menu.
//  - Reenvia los 8 registros de argumento (x0..x7) a la funcion original sin cambios,
//    asi que el juego recibe exactamente los mismos argumentos y devuelve lo mismo.
//  - Solo lee memoria (SafeRead) y escribe un archivo. No modifica nada del juego.
#include <cstdint>
#include <cstdio>
#include "diag.h"   // SafeRead

namespace TxCapture {

static const char *OutPath = "/sdcard/Android/data/com.mojang.minecraftpe/files/vanth_tx_capture.txt";
static const int MaxCaptures = 3;     // transacciones a registrar tras armar
static const int DumpBytes = 0x200;   // bytes volcados del argumento

static bool armed = false;            // true = la siguiente transaccion se registra
static bool installed = false;        // hook ya instalado
static int captured = 0;              // transacciones registradas en esta sesion

typedef uintptr_t (*SendFn)(uintptr_t, uintptr_t, uintptr_t, uintptr_t,
                            uintptr_t, uintptr_t, uintptr_t, uintptr_t);
static SendFn orig = nullptr;

static void dumpArgs(uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3) {
    FILE *f = fopen(OutPath, captured == 0 ? "w" : "a");
    if (!f) return;
    fprintf(f, "--- tx #%d  a0=%llx a1=%llx a2=%llx a3=%llx\n", captured + 1,
            (unsigned long long) a0, (unsigned long long) a1,
            (unsigned long long) a2, (unsigned long long) a3);
    // Volcado del argumento a1 (la transaccion)
    for (int k = 0; k < DumpBytes; k += 16) {
        unsigned char row[16];
        if (!SafeRead((const void *) (a1 + k), row, 16)) {
            fprintf(f, "%03x: <no legible>\n", k);
            continue;
        }
        fprintf(f, "%03x:", k);
        for (int j = 0; j < 16; j++) fprintf(f, " %02x", row[j]);
        fprintf(f, "\n");
    }
    fprintf(f, "--- fin tx #%d\n", captured + 1);
    fclose(f);
}

// Hook: registra si esta armado y reenvia TODOS los argumentos a la funcion original.
static uintptr_t hook(uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3,
                      uintptr_t a4, uintptr_t a5, uintptr_t a6, uintptr_t a7) {
    if (armed && captured < MaxCaptures) {
        dumpArgs(a0, a1, a2, a3);
        captured++;
        if (captured >= MaxCaptures) armed = false;
    }
    return orig(a0, a1, a2, a3, a4, a5, a6, a7);
}

// Instala el hook una sola vez. addr = LocalPlayer::sendInventoryTransaction (absoluta).
// Comprueba el prologo antes de enganchar (sub sp,sp,#imm ; stp x29,x30 ...).
static void install(uintptr_t addr) {
    if (installed || !addr) return;
    unsigned int pro = 0;
    if (!SafeRead((const void *) addr, &pro, 4)) return;
    if ((pro & 0xFF000000u) != 0xD1000000u) return;   // sub sp,sp,#imm (64-bit)
    DobbyHook(reinterpret_cast<void *>(addr), reinterpret_cast<void *>(hook),
              reinterpret_cast<void **>(&orig));
    installed = orig != nullptr;
}

} // namespace TxCapture
