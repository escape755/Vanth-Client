#pragma once
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

} // namespace PopDetect
