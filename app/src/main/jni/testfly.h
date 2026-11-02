#pragma once
// TestFly (portado de Abysm/KiwiMods Fly2Module). Vuelo por empuje: cada tick fija la velocidad
// del jugador (posDelta) segun tu input (WASD, saltar, agacharse) y la camara.
//
// Offsets (libminecraftpe 1.21.111, ARM64), sacados de la funcion de PlayerAuthInput:
//   paquete + 48        yaw (float)           (Apollon ya escribe aqui el yaw del aura)
//   paquete + 96 / +100 MoveVector x (strafe) / y (adelante)
//   paquete + 136       InputData (bitset, bit i = PlayerAuthInputData[i])
//   posicion (getPosition()) + 24  posDelta: StateVectorComponent { pos, posOld, posDelta }
#include <cmath>
#include <cstdint>
#include "Vector.h"

namespace TestFly {

static bool enabled = false;
static float hSpeedBps = 46.0f, upBps = 19.8f, downBps = 46.0f;
static float glide = -0.02f, upH = 0.55f, downH = 0.55f;
static bool hover = true;

// Ultimo input visto en PlayerAuthInput
static float moveX = 0, moveY = 0, yawDeg = 0;
static uint64_t inputBits = 0;
static bool haveInput = false;

static float lastX = 0, lastY = 0, lastZ = 0;
static bool haveLast = false;

enum { B_JUMPING = 6, B_SNEAKING = 8, B_WANT_UP = 16, B_WANT_DOWN = 17 };

// Llamar desde el hook de PlayerAuthInput (a1 = paquete). Solo ARM64: ahi se verificaron los offsets.
static inline void captureInput(void *packet) {
#if defined(__aarch64__)
    if (!packet) return;
    char *p = (char *) packet;
    yawDeg = *(float *) (p + 48);
    moveX = *(float *) (p + 96);
    moveY = *(float *) (p + 100);
    inputBits = *(uint64_t *) (p + 136);
    haveInput = true;
#endif
}

static inline bool bit(int i) { return (inputBits >> i) & 1ULL; }

static inline void reset() { haveLast = false; haveInput = false; }

// Llamar al inicio de NormalTick del jugador local, antes de old_NormalTick. pos = getPosition().
static inline void apply(vec3_t *pos) {
#if defined(__aarch64__)
    if (!enabled || !pos || !haveInput) return;
    float *f = (float *) pos;
    // Comprobacion de seguridad: en un StateVectorComponent real, posOld esta cerca de pos y posDelta
    // es pequeno y finito. Si no, ese puntero no es lo que se espera: no se escribe nada.
    for (int i = 0; i < 9; i++) if (!std::isfinite(f[i])) return;
    if (std::fabs(f[3] - f[0]) > 6.0f || std::fabs(f[5] - f[2]) > 6.0f ||
        std::fabs(f[6]) > 12.0f || std::fabs(f[7]) > 12.0f || std::fabs(f[8]) > 12.0f) return;

    float dx = f[0] - lastX, dy = f[1] - lastY, dz = f[2] - lastZ;
    bool teleported = !haveLast || std::sqrt(dx * dx + dy * dy + dz * dz) > 4.5f;
    lastX = f[0]; lastY = f[1]; lastZ = f[2]; haveLast = true;
    if (teleported) return; // salto grande (teletransporte/correccion): no empujar este tick

    bool up = bit(B_WANT_UP) || bit(B_JUMPING);
    bool down = bit(B_WANT_DOWN) || bit(B_SNEAKING);

    if (!up && !down && moveX == 0.0f && moveY == 0.0f && glide == 0.0f) {
        if (hover) { f[6] = 0; f[7] = 0; f[8] = 0; }
        return;
    }

    float hSpeed = hSpeedBps / 20.0f;
    float cap = std::sqrt(5.99f);
    if (hSpeed > cap) hSpeed = cap;
    if (up) hSpeed *= upH;
    if (down) hSpeed *= downH;

    float vertical = glide;
    if (up) vertical = upBps / 20.0f;
    else if (down) vertical = -(downBps / 20.0f) + glide;

    double rad = (double) yawDeg * 3.14159265358979 / 180.0;
    double s = std::sin(rad), c = std::cos(rad);
    double fwd = moveY, strafe = moveX;
    float mx = (float) (-s * fwd + c * strafe);
    float mz = (float) (c * fwd + s * strafe);
    float len = std::sqrt(mx * mx + mz * mz);
    float outX = 0, outZ = 0;
    if (len > 0.001f) { float k = hSpeed / len; outX = mx * k; outZ = mz * k; }

    f[6] = outX; f[7] = vertical; f[8] = outZ; // posDelta
#endif
}

} // namespace TestFly
