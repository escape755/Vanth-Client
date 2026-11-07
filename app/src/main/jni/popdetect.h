#pragma once
#include "diag.h"   // SafeRead y FindTextAt (lectura sin crash)
// Detector de pop de tótem por vida. Solo detecta; NO mueve tótems (la reposicion espera el offset
// del offhand verificado). Heuristica: si la vida cae de golpe a un valor bajo, cuenta como posible pop.
namespace PopDetect {

static bool enabled = true;
static int dropThreshold = 6;   // caida en un tick que se considera golpe fuerte
static int lowHealth = 4;       // vida despues del golpe por debajo de la que se considera pop
static int prevHealth = -1;
static int pops = 0;
static int lastPopTick = -1000;
static int tick = 0;

// Llamar en NormalTick con la vida actual del jugador local.
static inline void update(int health) {
    tick++;
    if (!enabled) { prevHealth = health; return; }
    if (prevHealth >= 0 && health >= 0) {
        int drop = prevHealth - health;
        // Pop: caida fuerte que deja al jugador con vida baja, y no se repite en los siguientes 20 ticks.
        if (drop >= dropThreshold && health <= lowHealth && tick - lastPopTick > 20) {
            pops++;
            lastPopTick = tick;
        }
    }
    prevHealth = health;
}

static inline void reset() { prevHealth = -1; }


// ---- Umbral de tótem (aviso instantaneo, sin mover nada todavia) ----
static int totemAt = 10;            // a esta vida o menos, el tótem debe estar en el offhand
static bool needTotem = false;      // la vida ya pide tótem
static bool offhandHasTotem = true; // resultado de la lectura del offhand

// Llamar en NormalTick. fnAddr = direccion absoluta de Actor::getOffhandSlot (tabla de simbolos).
// Solo lee el offhand cuando la vida esta bajo el umbral, para no cargar cada tick.
static inline void checkTotem(void *player, int health, uintptr_t fnAddr) {
    needTotem = (health >= 0 && health <= totemAt);
    if (!needTotem || !player || !fnAddr) return;
    unsigned int pro = 0;
    if (!SafeRead((const void *) fnAddr, &pro, 4) || pro != 0xA9BF7BFDu) return; // prologo no coincide
    typedef void *(*GetOffhandFn)(void *);
    void *slot = ((GetOffhandFn) fnAddr)(player);
    offhandHasTotem = true; // NO VERIFICADO: ver diag.h
}

} // namespace PopDetect
