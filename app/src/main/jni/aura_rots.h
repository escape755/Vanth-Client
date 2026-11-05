#pragma once
// Rotaciones del Aura de Vanth (basadas en las de Abysm/Miu: Snap, Closest, Quintuplet, Smooth,
// FrontsX, Astral). Todas devuelven vec2_t(pitch, yaw) en grados, igual que CalcPlayerAngle.
// Convencion: la posicion de un Actor viene a nivel de ojos (eyeHeight abajo es el offset a los pies).
#include <cmath>
#include <chrono>
#include <unordered_map>
#include "Vector.h"

namespace AuraRots {

enum Mode { Default = 0, Snap, Closest, Quintuplet, Smooth, FrontsX, Astral, Predictor, Halcon };
static const char *Names[] = {"Default", "Snap", "Closest", "Quintuplet", "Smooth", "FrontsX", "Astral", "Predictor", "Halcon"};

static float eyeHeight = 1.62f; // ajustable si el aim queda alto o bajo
static float lockDeg = 6.0f;    // con menos error que esto se clava exacto
static float turbo = 1.0f;      // multiplica la ganancia de Smooth/FrontsX/Astral

static const float R2D = 57.29577951f;

struct Track { float px, py, pz, vx, vy, vz; long long t; };
static std::unordered_map<const void *, Track> g_tracks;
static float sPitch = 0, sYaw = 0;
static const void *sTarget = nullptr;
static int sMode = -1;

static long long nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
}

static float wrap180(float a) {
    if (!std::isfinite(a)) return 0.0f;
    float r = std::fmod(a + 180.0f, 360.0f);
    if (r < 0.0f) r += 360.0f;
    return r - 180.0f;
}

// Velocidad por tick estimada con el cambio de posicion (~cada 30 ms). 0 si el dato esta viejo.
static void velocity(const void *key, float x, float y, float z, float &vx, float &vy, float &vz) {
    long long now = nowMs();
    if (g_tracks.size() > 256) g_tracks.clear();
    auto it = g_tracks.find(key);
    if (it == g_tracks.end()) {
        g_tracks[key] = {x, y, z, 0, 0, 0, now};
        vx = vy = vz = 0;
        return;
    }
    Track &t = it->second;
    if (now - t.t >= 10) {
        float dt = (float) (now - t.t) / 50.0f; // en ticks de 50 ms
        if (dt > 4.0f) { t.vx = t.vy = t.vz = 0; }
        else {
            // promedio suave (EMA): usa varias muestras, no solo la ultima
            float nvx = (x - t.px) / dt, nvy = (y - t.py) / dt, nvz = (z - t.pz) / dt;
            t.vx = t.vx * 0.5f + nvx * 0.5f; t.vy = t.vy * 0.5f + nvy * 0.5f; t.vz = t.vz * 0.5f + nvz * 0.5f;
        }
        t.px = x; t.py = y; t.pz = z; t.t = now;
    }
    vx = t.vx; vy = t.vy; vz = t.vz;
}

static void angleTo(float dx, float dy, float dz, float &pitch, float &yaw) {
    float d = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (d < 0.001f) { pitch = sPitch; yaw = sYaw; return; }
    float s = dy / d; if (s > 1) s = 1; if (s < -1) s = -1;
    pitch = -std::asin(s) * R2D;
    yaw = -std::atan2(dx, dz) * R2D;
    if (pitch > 90) pitch = 90; if (pitch < -90) pitch = -90;
}

static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

static void chase(float wp, float wy, float gain) {
    float g = clampf(gain * turbo, 0.0f, 1.0f);
    float dy = wrap180(wy - sYaw), dp = wp - sPitch;
    if (std::fabs(dy) < lockDeg && std::fabs(dp) < lockDeg) { sYaw = wy; sPitch = clampf(wp, -90, 90); return; }
    sYaw = wrap180(sYaw + g * dy);
    sPitch = clampf(sPitch + g * dp, -90, 90);
}

// Punto del hitbox con el menor giro desde la mira actual (cur = pitch/yaw actuales).
static void pickAimPoint(float cx, float cy, float cz, float hw, float height,
                         const vec3_t &eye, const vec2_t &cur, float &wp, float &wy) {
    float bestErr = 1e9f; wp = 0; wy = 0;
    for (int iy = 0; iy < 3; iy++) {
        float py = cy + height * (0.2f + 0.3f * iy); // pies, pecho, cabeza
        for (int ix = -1; ix <= 1; ix++) {
            float px = cx + ix * hw * 0.8f;
            float p, y;
            angleTo(px - eye.x, py - eye.y, cz - eye.z, p, y);
            float err = std::fabs(wrap180(y - cur.y)) + std::fabs(p - cur.x);
            if (err < bestErr) { bestErr = err; wp = p; wy = y; }
        }
    }
}

// eye: tus ojos. tpos: posicion del objetivo (a nivel de ojos). target/self: punteros para rastrear velocidad.
static vec2_t compute(int mode, const vec3_t &eye, const vec3_t &tpos, const void *target, const void *self,
                      float width, float height, const vec2_t &cur) {
    if (mode <= Default || mode > Halcon) return eye.CalcPlayerAngle(tpos);

    if (mode != sMode || target != sTarget) { sMode = mode; sTarget = target; sPitch = cur.x; sYaw = cur.y; }

    float tvx, tvy, tvz, evx, evy, evz;
    velocity(target, tpos.x, tpos.y, tpos.z, tvx, tvy, tvz);
    velocity(self, eye.x, eye.y, eye.z, evx, evy, evz);

    float fx = tpos.x, fy = tpos.y - eyeHeight, fz = tpos.z; // pies del objetivo
    float dist = std::sqrt((eye.x - fx) * (eye.x - fx) + (eye.y - fy) * (eye.y - fy) + (eye.z - fz) * (eye.z - fz));
    float hw = clampf(width * 0.5f, 0.05f, 5.0f);
    float wp = 0, wy = 0;
    // Velocidad relativa del objetivo respecto a ti (lo que importa para apuntar)
    float rvx = tvx - evx, rvy = tvy - evy, rvz = tvz - evz;

    switch (mode) {
        case Snap: {
            // Apunta al centro del pecho, sin giro intermedio.
            angleTo(fx - eye.x, fy + height * 0.6f - eye.y, fz - eye.z, wp, wy);
            sPitch = wp; sYaw = wy;
            break;
        }
        case Closest: {
            // Punto del hitbox mas cercano a la mira real (por distancia).
            float px = clampf(eye.x, fx - hw, fx + hw);
            float py = clampf(eye.y, fy + 0.05f, fy + height - 0.05f);
            float pz = clampf(eye.z, fz - hw, fz + hw);
            bool inside = (px == eye.x && py == eye.y && pz == eye.z);
            if (inside) angleTo(fx - eye.x, fy + height * 0.6f - eye.y, fz - eye.z, wp, wy);
            else angleTo(px - eye.x, py - eye.y, pz - eye.z, wp, wy);
            sPitch = wp; sYaw = wy;
            break;
        }
        case Quintuplet: {
            // Como Closest, con 1 tick de prediccion y clavado casi instantaneo.
            float cx = fx + rvx, cy = fy + rvy, cz = fz + rvz;
            float px = clampf(eye.x, cx - hw + 0.05f, cx + hw - 0.05f);
            float py = clampf(eye.y, cy + 0.05f, cy + height - 0.05f);
            float pz = clampf(eye.z, cz - hw + 0.05f, cz + hw - 0.05f);
            angleTo(px - eye.x, py - eye.y, pz - eye.z, wp, wy);
            chase(wp, wy, 0.99f);
            break;
        }
        case Smooth: {
            // Prediccion corta + giro proporcional a la distancia: rapido lejos, firme cerca.
            float k = dist * 0.1f;
            pickAimPoint(fx + rvx * k, fy + rvy * k, fz + rvz * k, hw, height, eye, cur, wp, wy);
            chase(wp, wy, clampf(14.0f / (dist + 2.0f), 0.9f, 1.0f));
            break;
        }
        case FrontsX: {
            // Adelanto que crece con la distancia, punto de menor giro.
            float tk = (1.0f + dist * 0.1f) * 1.2f;
            pickAimPoint(fx + rvx * tk, fy + rvy * tk, fz + rvz * tk, hw, height, eye, cur, wp, wy);
            chase(wp, wy, clampf(16.0f / (dist + 1.0f), 0.8f, 1.0f));
            break;
        }
        case Predictor: {
            // Adelanto corto + punto de menor giro; ganancia alta para no quedarse atras.
            float tk = 1.0f + dist * 0.05f;
            pickAimPoint(fx + rvx * tk, fy + rvy * tk, fz + rvz * tk, hw, height, eye, cur, wp, wy);
            chase(wp, wy, 0.95f);
            break;
        }
        case Halcon: {
            // Persecucion: ganancia maxima cuando el error es grande, firme cuando ya esta encima.
            float tk = 0.6f + dist * 0.05f;
            pickAimPoint(fx + rvx * tk, fy + rvy * tk, fz + rvz * tk, hw, height, eye, cur, wp, wy);
            float errNow = std::fabs(wrap180(wy - sYaw)) + std::fabs(wp - sPitch);
            chase(wp, wy, errNow > 10.0f ? 1.0f : 0.85f);
            break;
        }
        case Astral: {
            // Adelanto amplio y giro maximo: la mas agresiva de las de precision.
            float tk = (1.0f + dist * 0.1f) * 1.2f;
            pickAimPoint(fx + rvx * tk, fy + rvy * tk, fz + rvz * tk, hw, height, eye, cur, wp, wy);
            chase(wp, wy, 1.0f);
            break;
        }
    }
    return vec2_t(sPitch, sYaw);
}

} // namespace AuraRots
