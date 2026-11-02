#pragma once
// Tema demoniaco: cuernos abajo al centro (hasta la mitad de la GUI), dos calaveras arriba en las esquinas.
// Al ABRIR la GUI los ojos de las calaveras se encienden. Al CERRARLA las calaveras sueltan sangre
// que cae hasta abajo de la pantalla.
#include <vector>
#include <cmath>
#include <cstdlib>
#include <chrono>

namespace Blood {

struct Drop { ImVec2 p; float vy, len, w; };
static std::vector<Drop> drops;
static bool haveRect = false, open = true, prevOpen = true;
static ImVec2 wPos(0, 0), wSize(0, 0);
static float eyeGlow = 0.0f;
static float bleedTimer = 0.0f;
static std::chrono::steady_clock::time_point lastT = std::chrono::steady_clock::now();

static inline float rnd01() { return (rand() % 10000) / 10000.0f; }

// Llamar ANTES de ImGui::Begin de la ventana principal, con su titulo.
static inline void frame(const char *title) {
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - lastT).count();
    lastT = now;
    if (dt > 0.1f) dt = 0.1f;

    ImGuiWindow *w = ImGui::FindWindowByName(title);
    if (w && !w->Hidden) {
        wPos = w->Pos; wSize = w->Size; haveRect = true;
        open = !w->Collapsed;
    } else {
        open = false;
    }
    // Al cerrar: empieza a sangrar durante un rato.
    if (prevOpen && !open) bleedTimer = 1.6f;
    prevOpen = open;
    if (bleedTimer > 0) bleedTimer -= dt;
    eyeGlow += ((open ? 1.0f : 0.0f) - eyeGlow) * (dt * 10.0f > 1 ? 1 : dt * 10.0f);
}

static inline void drawHorn(ImDrawList *dl, ImVec2 a, ImVec2 c1, ImVec2 c2, ImVec2 b, float s) {
    // Cuerno en capas: borde rojo grueso, cuerpo oscuro y punta mas fina.
    dl->AddBezierCubic(a, c1, c2, b, IM_COL32(170, 10, 14, 255), s * 0.9f);
    dl->AddBezierCubic(a, c1, c2, b, IM_COL32(45, 4, 6, 255), s * 0.6f);
    dl->AddBezierCubic(a, c1, c2, b, IM_COL32(90, 10, 12, 255), s * 0.15f);
}

static inline void drawSkull(ImDrawList *dl, ImVec2 c, float r, float glow) {
    ImU32 bone = IM_COL32(205, 190, 185, 255), shade = IM_COL32(60, 10, 12, 255);
    dl->AddCircleFilled(c, r, bone, 32);
    dl->AddRectFilled(ImVec2(c.x - r * 0.55f, c.y + r * 0.35f), ImVec2(c.x + r * 0.55f, c.y + r * 0.95f), bone, 4.0f);
    dl->AddLine(ImVec2(c.x - r * 0.3f, c.y + r * 0.6f), ImVec2(c.x + r * 0.3f, c.y + r * 0.6f), shade, 2.0f);
    // Cuencas
    ImVec2 eL(c.x - r * 0.42f, c.y - r * 0.05f), eR(c.x + r * 0.42f, c.y - r * 0.05f);
    dl->AddCircleFilled(eL, r * 0.3f, shade, 16);
    dl->AddCircleFilled(eR, r * 0.3f, shade, 16);
    // Ojos encendidos
    if (glow > 0.01f) {
        int a = (int) (glow * 255);
        for (ImVec2 e : {eL, eR}) {
            dl->AddCircleFilled(e, r * 0.55f, IM_COL32(255, 30, 20, a / 4), 16);
            dl->AddCircleFilled(e, r * 0.28f, IM_COL32(255, 60, 40, a), 16);
        }
    }
    // Nariz
    dl->AddTriangleFilled(ImVec2(c.x, c.y + r * 0.2f), ImVec2(c.x - r * 0.12f, c.y + r * 0.45f),
                          ImVec2(c.x + r * 0.12f, c.y + r * 0.45f), shade);
}

static inline void draw() {
    if (!haveRect) return;
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    float s = wSize.x * 0.07f;

    // Cuernos: abajo al centro, suben hasta la mitad de la GUI.
    float cx = wPos.x + wSize.x * 0.5f, bottom = wPos.y + wSize.y, midY = wPos.y + wSize.y * 0.5f;
    for (int side = -1; side <= 1; side += 2) {
        ImVec2 a(cx + side * s * 0.9f, bottom);
        ImVec2 c1(cx + side * s * 3.2f, bottom - s * 0.4f);
        ImVec2 c2(cx + side * s * 3.4f, midY + s * 0.6f);
        ImVec2 b(cx + side * s * 2.0f, midY - s * 0.5f);
        drawHorn(dl, a, c1, c2, b, s);
    }

    // Calaveras: arriba en las esquinas.
    float r = wSize.x * 0.055f;
    ImVec2 skL(wPos.x - r * 0.1f, wPos.y + r * 0.2f), skR(wPos.x + wSize.x + r * 0.1f, wPos.y + r * 0.2f);
    drawSkull(dl, skL, r, eyeGlow);
    drawSkull(dl, skR, r, eyeGlow);

    // Sangre: al cerrar, las calaveras sueltan gotas durante bleedTimer.
    if (bleedTimer > 0) {
        for (int i = 0; i < 3; i++) {
            ImVec2 src = (rnd01() < 0.5f) ? skL : skR;
            Drop d;
            d.p = ImVec2(src.x + (rnd01() - 0.5f) * r * 0.8f, src.y + r * 0.6f);
            d.vy = 120.0f + rnd01() * 120.0f;
            d.len = 8.0f + rnd01() * 12.0f;
            d.w = 2.0f + rnd01() * 2.0f;
            drops.push_back(d);
        }
    }
    float dt = 1.0f / 60.0f;
    for (size_t i = 0; i < drops.size();) {
        Drop &d = drops[i];
        d.vy += 600.0f * dt; // gravedad
        d.p.y += d.vy * dt;
        ImU32 col = IM_COL32(170 + (int) (rnd01() * 50), 6, 10, 230);
        dl->AddLine(d.p, ImVec2(d.p.x, d.p.y - d.len), col, d.w);
        dl->AddCircleFilled(d.p, d.w, col);
        if (d.p.y > screen.y + 20) { drops[i] = drops.back(); drops.pop_back(); }
        else i++;
    }
}

} // namespace Blood
