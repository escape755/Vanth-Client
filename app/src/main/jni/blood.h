#pragma once
// Tema demoniaco. Todo se dibuja en la capa de FONDO (detras de la ventana) para no tapar la GUI.
// - Dos bolitas abajo al centro (cuernos pequeños con brillo).
// - Dos calaveras de perfil arriba en las esquinas, mirando hacia afuera, con sombras, grietas y dientes.
// - Al ABRIR la GUI los ojos se encienden; al CERRARLA sueltan sangre hasta abajo de la pantalla.
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
    if (prevOpen && !open) bleedTimer = 1.6f;
    prevOpen = open;
    if (bleedTimer > 0) bleedTimer -= dt;
    float target = open ? 1.0f : 0.0f;
    eyeGlow += (target - eyeGlow) * (dt * 10.0f > 1.0f ? 1.0f : dt * 10.0f);
}

// Cuerno en media luna (como la referencia): base ancha abajo, punta arriba, borde grueso y brillo.
// dir = -1 curva hacia la izquierda, +1 hacia la derecha. c = centro de la base, r = escala.
static inline void drawHorn(ImDrawList *dl, ImVec2 c, float r, float dir) {
    auto P = [&](float x, float y) { return ImVec2(c.x + dir * x * r, c.y + y * r); };
    ImVec2 body[] = {
        P(0.10f, 0.00f), P(0.55f, -0.10f), P(0.85f, -0.45f), P(0.70f, -0.90f), P(0.35f, -1.25f),
        P(0.25f, -1.20f), P(0.25f, -0.80f), P(0.20f, -0.45f), P(0.00f, -0.15f),
    };
    dl->AddConvexPolyFilled(body, 9, IM_COL32(28, 3, 5, 255));
    dl->AddPolyline(body, 9, IM_COL32(170, 12, 16, 255), ImDrawFlags_Closed, 2.2f);
    // Brillo: linea corta a lo largo del cuerpo
    dl->AddLine(P(0.55f, -0.30f), P(0.62f, -0.65f), IM_COL32(190, 60, 60, 200), 1.6f);
}

// Calavera de perfil (referencia): craneo alto, orbita profunda, pomulo, mandibula atras.
// dir = -1 mira a la izquierda, +1 a la derecha. c = centro del craneo, r = escala.
static inline void drawSkullProfile(ImDrawList *dl, ImVec2 c, float r, float glow, float dir) {
    auto P = [&](float x, float y) { return ImVec2(c.x - dir * x * r, c.y + y * r); };
    ImU32 bone = IM_COL32(214, 200, 192, 255);
    ImU32 boneShade = IM_COL32(140, 118, 112, 255);
    ImU32 dark = IM_COL32(30, 4, 6, 255);
    ImU32 rim = IM_COL32(110, 10, 14, 255);

    // Contorno de perfil: craneo alto, frente, orbita, nariz, maxilar y mandibula.
    ImVec2 outline[] = {
        P(0.95f, -0.30f), P(0.90f, -0.80f), P(0.50f, -1.15f), P(-0.05f, -1.20f),
        P(-0.55f, -1.00f), P(-0.85f, -0.65f), P(-0.98f, -0.35f), P(-1.00f, -0.12f), // frente y orbita
        P(-1.10f, 0.05f), P(-1.14f, 0.22f), P(-0.98f, 0.30f),                       // nariz
        P(-0.92f, 0.46f), P(-0.80f, 0.60f), P(-0.62f, 0.88f), P(-0.30f, 1.02f),     // maxilar y menton
        P(0.10f, 0.92f), P(0.42f, 0.60f), P(0.70f, 0.35f),                          // mandibula atras
    };
    int n = 18;
    dl->AddConvexPolyFilled(outline, n, bone);
    dl->AddPolyline(outline, n, rim, ImDrawFlags_Closed, 1.6f);

    // Sombras: parte trasera del craneo, debajo del pomulo, y mandibula
    ImVec2 back[] = { P(0.95f, -0.30f), P(0.90f, -0.80f), P(0.50f, -1.15f), P(0.40f, -0.60f), P(0.70f, 0.35f), P(0.80f, 0.00f) };
    dl->AddConvexPolyFilled(back, 6, boneShade);
    ImVec2 jaw[] = { P(0.42f, 0.60f), P(0.10f, 0.92f), P(-0.30f, 1.02f), P(-0.62f, 0.88f), P(-0.30f, 0.62f), P(0.10f, 0.62f) };
    dl->AddConvexPolyFilled(jaw, 6, boneShade);

    // Sutura del craneo (como la foto)
    dl->AddLine(P(0.20f, -1.10f), P(0.02f, -0.70f), rim, 1.0f);
    dl->AddLine(P(0.02f, -0.70f), P(-0.10f, -0.42f), rim, 0.9f);

    // Orbita profunda y ojo encendido cuando la GUI esta abierta
    ImVec2 eye = P(-0.50f, -0.22f);
    dl->AddCircleFilled(eye, r * 0.40f, dark, 20);
    if (glow > 0.01f) {
        int a = (int) (glow * 255);
        dl->AddCircleFilled(eye, r * 1.00f, IM_COL32(255, 20, 10, a / 6), 24);
        dl->AddCircleFilled(eye, r * 0.62f, IM_COL32(255, 30, 20, a / 3), 20);
        dl->AddCircleFilled(eye, r * 0.30f, IM_COL32(255, 90, 60, a), 16);
        dl->AddCircleFilled(eye, r * 0.10f, IM_COL32(255, 230, 220, a), 8);
    }

    // Nariz cavernosa
    dl->AddTriangleFilled(P(-1.02f, -0.05f), P(-0.86f, 0.18f), P(-0.96f, 0.30f), dark);

    // Dientes superiores e inferiores (fila)
    for (int i = 0; i < 5; i++) {
        float x0 = -0.92f + i * 0.13f, x1 = x0 + 0.11f;
        dl->AddRectFilled(P(x0, 0.40f), P(x1, 0.54f), IM_COL32(242, 236, 222, 255), 1.5f);
        dl->AddRect(P(x0, 0.40f), P(x1, 0.54f), rim, 1.5f, 0, 0.7f);
    }
    for (int i = 0; i < 5; i++) {
        float x0 = -0.80f + i * 0.13f, x1 = x0 + 0.11f;
        dl->AddRectFilled(P(x0, 0.56f), P(x1, 0.68f), IM_COL32(232, 224, 210, 255), 1.5f);
    }

    // Brillo sobre el craneo
    dl->AddCircleFilled(P(0.35f, -0.85f), r * 0.10f, IM_COL32(255, 255, 255, 100), 12);
}

static inline void draw() {
    if (!haveRect) return;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    float r = wSize.x * 0.05f;

    // Bolitas (cuernos) abajo al centro, pegadas al borde inferior de la ventana.
    float cx = wPos.x + wSize.x * 0.5f, bottom = wPos.y + wSize.y;
    drawHorn(dl, ImVec2(cx - r * 0.3f, bottom), r * 0.9f, -1.0f);
    drawHorn(dl, ImVec2(cx + r * 0.3f, bottom), r * 0.9f, 1.0f);

    // Calaveras de perfil en las esquinas superiores, fuera de la ventana, mirando hacia afuera.
    ImVec2 skL(wPos.x - r * 1.0f, wPos.y - r * 0.6f);
    ImVec2 skR(wPos.x + wSize.x + r * 1.0f, wPos.y - r * 0.6f);
    drawSkullProfile(dl, skL, r, eyeGlow, -1.0f);
    drawSkullProfile(dl, skR, r, eyeGlow, 1.0f);

    // Sangre: al cerrar, sale de las calaveras y cae hasta abajo de la pantalla.
    if (bleedTimer > 0) {
        for (int i = 0; i < 3; i++) {
            ImVec2 src = (rnd01() < 0.5f) ? skL : skR;
            Drop d;
            d.p = ImVec2(src.x + (rnd01() - 0.5f) * r * 0.8f, src.y + r * 0.8f);
            d.vy = 120.0f + rnd01() * 120.0f;
            d.len = 8.0f + rnd01() * 12.0f;
            d.w = 2.0f + rnd01() * 2.0f;
            drops.push_back(d);
        }
    }
    float dt = 1.0f / 60.0f;
    for (size_t i = 0; i < drops.size();) {
        Drop &d = drops[i];
        d.vy += 600.0f * dt;
        d.p.y += d.vy * dt;
        ImU32 col = IM_COL32(170 + (int) (rnd01() * 50), 6, 10, 230);
        dl->AddLine(d.p, ImVec2(d.p.x, d.p.y - d.len), col, d.w);
        dl->AddCircleFilled(d.p, d.w, col);
        if (d.p.y > screen.y + 20) { drops[i] = drops.back(); drops.pop_back(); }
        else i++;
    }
}

} // namespace Blood
