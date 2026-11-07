#pragma once
// Captura de paquetes enviados al servidor (incluida la transaccion de inventario del tótem).
// Engancha LoopbackPacketSender::send (vtable slot [2]), direccion relativa 0x9c871c4
// tomada de vanth_offsets.txt. Mientras esta armado, registra la cabecera de cada paquete:
// vtable del paquete + primeros bytes. Reenvia los argumentos sin cambios.
#include <cstdint>
#include <cstdio>
#include "diag.h"   // SafeRead

namespace TxCapture {

static const char *OutPath = "/sdcard/Android/data/com.mojang.minecraftpe/files/vanth_tx_capture.txt";
static const uintptr_t SendRva = 0x9c871c4;   // LoopbackPacketSender::send (vtable [2])
static const int MaxPackets = 300;            // paquetes a registrar tras armar
static const int HeaderBytes = 0x60;          // bytes de cabecera por paquete

static bool armed = false;      // true = registrar paquetes
static bool installed = false;  // hook ya instalado
static int captured = 0;        // paquetes registrados en esta sesion

typedef uintptr_t (*SendFn)(uintptr_t, uintptr_t, uintptr_t, uintptr_t,
                            uintptr_t, uintptr_t, uintptr_t, uintptr_t);
static SendFn orig = nullptr;

static void logPacket(uintptr_t self, uintptr_t packet) {
    FILE *f = fopen(OutPath, captured == 0 ? "w" : "a");
    if (!f) return;
    unsigned long long vptr = 0;
    SafeRead((const void *) packet, &vptr, 8);
    fprintf(f, "#%03d this=%llx packet=%llx vptr=%llx\n", captured + 1,
            (unsigned long long) self, (unsigned long long) packet, vptr);
    unsigned char row[16];
    for (int k = 0; k < HeaderBytes; k += 16) {
        if (!SafeRead((const void *) (packet + k), row, 16)) {
            fprintf(f, "  %02x: <no legible>\n", k);
            continue;
        }
        fprintf(f, "  %02x:", k);
        for (int j = 0; j < 16; j++) fprintf(f, " %02x", row[j]);
        fprintf(f, "\n");
    }
    fclose(f);
}

// Hook: registra mientras esta armado y reenvia TODOS los argumentos a la original.
static uintptr_t hook(uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3,
                      uintptr_t a4, uintptr_t a5, uintptr_t a6, uintptr_t a7) {
    if (armed && captured < MaxPackets) {
        logPacket(a0, a1);
        captured++;
        if (captured >= MaxPackets) armed = false;
    }
    return orig(a0, a1, a2, a3, a4, a5, a6, a7);
}

// Instala el hook una vez. addr = direccion absoluta de LoopbackPacketSender::send.
static void install(uintptr_t addr) {
    if (installed || !addr) return;
    DobbyHook(reinterpret_cast<void *>(addr), reinterpret_cast<void *>(hook),
              reinterpret_cast<void **>(&orig));
    installed = orig != nullptr;
}

} // namespace TxCapture
