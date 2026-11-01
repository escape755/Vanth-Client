#pragma once
// Rotaciones del Aura de Vanth (basadas en las de Abysm/Miu: Snap, Closest, Quintuplet, Smooth,
// FrontsX, Astral). Todas devuelven vec2_t(pitch, yaw) en grados, igual que CalcPlayerAngle.
// Convencion: la posicion de un Actor viene a nivel de ojos (eyeHeight abajo es el offset a los pies).
#include <cmath>
#include <chrono>
#include <unordered_map>
#include "Vector.h"

namespace AuraRots {

enum Mode { Default = 0, Snap, Closest, Quintuplet, Smooth, FrontsX, Astral };
static const char *Names[] = {"Default", "Snap", "Closest", "Quintuplet", "Smooth", "FrontsX", "Astral"};

static float eyeHeight = 1.62f; // ajustable si el aim queda alto o bajo
static float lockDeg = 4.0f;    // con menos error que esto se clava exacto
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
    if (now - t.t >= 30) {
        float dt = (float) (now - t.t) / 50.0f; // en ticks de 50 ms
        if (dt > 4.0f) { t.vx = t.vy = t.vz = 0; }
        else { t.vx = (x - t.px) / dt; t.vy = (y - t.py) / dt; t.vz = (z - t.pz) / dt; }
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

// eye: tus ojos. tpos: posicion del objetivo (a nivel de ojos). target/self: punteros para rastrear velocidad.
static vec2_t compute(int mode, const vec3_t &eye, const vec3_t &tpos, const void *target, const void *self,
                      float width, float height, const vec2_t &cur) {
    if (mode <= Default || mode > Astral) return eye.CalcPlayerAngle(tpos);

    if (mode != sMode || target != sTarget) { sMode = mode; sTarget = target; sPitch = cur.x; sYaw = cur.y; }

    float tvx, tvy, tvz, evx, evy, evz;
    velocity(target, tpos.x, tpos.y, tpos.z, tvx, tvy, tvz);
    velocity(self, eye.x, eye.y, eye.z, evx, evy, evz);

    float fx = tpos.x, fy = tpos.y - eyeHeight, fz = tpos.z; // pies del objetivo
    float dist = std::sqrt((eye.x - fx) * (eye.x - fx) + (eye.y - fy) * (eye.y - fy) + (eye.z - fz) * (eye.z - fz));
    float wp = 0, wy = 0;

    switch (mode) {
        case Snap:
            angleTo(fx - eye.x, fy + height * 0.6f - eye.y, fz - eye.z, wp, wy);
            sPitch = wp; sYaw = wy;
            break;
        case Closest:
        case Quintuplet: {
            float cx = fx, cy = fy, cz = fz, margin = 0.0f;
            if (mode == Quintuplet) { // 1 tick adelante + margen hacia adentro
                cx += tvx - evx; cy += tvy - evy; cz += tvz - evz; margin = 0.05f;
            }
            float hw = clampf(width * 0.5f - margin, 0.05f, 5.0f);
            float px = clampf(eye.x, cx - hw, cx + hw);
            float py = clampf(eye.y, cy + margin, cy + height - margin);
            float pz = clampf(eye.z, cz - hw, cz + hw);
            bool inside = (px == eye.x && py == eye.y && pz == eye.z);
            if (inside) angleTo(cx - eye.x, cy + height * 0.6f - eye.y, cz - eye.z, wp, wy);
            else angleTo(px - eye.x, py - eye.y, pz - eye.z, wp, wy);
            if (mode == Closest) { sPitch = wp; sYaw = wy; }
            else { float old = lockDeg; lockDeg = 15.0f; chase(wp, wy, 0.99f); lockDeg = old; }
            break;
        }
        case Smooth: {
            float k = dist * 0.1f;
            angleTo(fx + (tvx - evx) * k - eye.x, fy + (tvy - evy) * k + height * 0.75f - eye.y,
                    fz + (tvz - evz) * k - eye.z, wp, wy);
            chase(wp, wy, clampf(14.0f / (dist + 2.0f), 0.7f, 0.95f));
            break;
        }
        case FrontsX: {
            float tk = (1.0f + dist * 0.1f) * 1.2f;
            angleTo(fx + (tvx - evx) * tk - eye.x, fy + (tvy - evy) * tk + height * 0.5f + 0.3f - eye.y,
                    fz + (tvz - evz) * tk - eye.z, wp, wy);
            chase(wp, wy, clampf(16.0f / (dist + 1.0f), 0.5f, 0.95f));
            break;
        }
        case Astral: {
            float tk = (1.0f + dist * 0.1f) * 1.2f;
            angleTo(fx + (tvx - evx) * tk - eye.x, fy + (tvy - evy) * tk + height * 0.55f + 0.3f - eye.y,
                    fz + (tvz - evz) * tk - eye.z, wp, wy);
            chase(wp, wy, 0.97f);
            break;
        }
    }
    return vec2_t(sPitch, sYaw);
}

} // namespace AuraRots
