#pragma once
// Tema "Vanth Sangre": rojo oscuro, bordes de sangre, acentos brillantes y botones con relieve.
// Se aplica una vez con embraceTheDarkness(). Colores en ImVec4(r, g, b, a) de 0 a 1.
static inline void embraceTheDarkness() {
    ImGuiStyle &st = ImGui::GetStyle();
    ImVec4 *c = st.Colors;

    const ImVec4 bgDeep   (0.06f, 0.01f, 0.02f, 0.94f);  // fondo de la ventana
    const ImVec4 bgPanel  (0.10f, 0.02f, 0.03f, 0.90f);  // paneles / hijos
    const ImVec4 bgFrame  (0.16f, 0.03f, 0.04f, 0.95f);  // campos
    const ImVec4 blood   (0.78f, 0.05f, 0.07f, 1.00f);   // rojo sangre principal
    const ImVec4 bloodHi (0.95f, 0.14f, 0.12f, 1.00f);   // rojo brillante (hover / activo)
    const ImVec4 bloodDk (0.40f, 0.02f, 0.03f, 1.00f);   // rojo oscuro (bordes internos)
    const ImVec4 bone    (0.92f, 0.87f, 0.84f, 1.00f);   // texto hueso
    const ImVec4 muted   (0.62f, 0.45f, 0.45f, 1.00f);   // texto secundario

    c[ImGuiCol_Text]                 = bone;
    c[ImGuiCol_TextDisabled]         = muted;
    c[ImGuiCol_WindowBg]             = bgDeep;
    c[ImGuiCol_ChildBg]              = bgPanel;
    c[ImGuiCol_PopupBg]              = bgPanel;
    c[ImGuiCol_Border]               = blood;
    c[ImGuiCol_BorderShadow]         = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_FrameBg]              = bgFrame;
    c[ImGuiCol_FrameBgHovered]       = ImVec4(0.26f, 0.05f, 0.06f, 1.00f);
    c[ImGuiCol_FrameBgActive]        = ImVec4(0.34f, 0.07f, 0.08f, 1.00f);
    c[ImGuiCol_TitleBg]              = ImVec4(0.12f, 0.01f, 0.02f, 1.00f);
    c[ImGuiCol_TitleBgActive]        = ImVec4(0.30f, 0.03f, 0.04f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.12f, 0.01f, 0.02f, 0.75f);
    c[ImGuiCol_MenuBarBg]            = bgPanel;
    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.06f, 0.01f, 0.02f, 0.60f);
    c[ImGuiCol_ScrollbarGrab]        = bloodDk;
    c[ImGuiCol_ScrollbarGrabHovered] = blood;
    c[ImGuiCol_ScrollbarGrabActive]  = bloodHi;
    c[ImGuiCol_CheckMark]            = bloodHi;
    c[ImGuiCol_SliderGrab]           = blood;
    c[ImGuiCol_SliderGrabActive]     = bloodHi;
    c[ImGuiCol_Button]               = ImVec4(0.30f, 0.04f, 0.05f, 1.00f);
    c[ImGuiCol_ButtonHovered]        = blood;
    c[ImGuiCol_ButtonActive]         = bloodHi;
    c[ImGuiCol_Header]               = ImVec4(0.40f, 0.05f, 0.06f, 0.80f);
    c[ImGuiCol_HeaderHovered]        = blood;
    c[ImGuiCol_HeaderActive]         = bloodHi;
    c[ImGuiCol_Separator]            = bloodDk;
    c[ImGuiCol_SeparatorHovered]     = blood;
    c[ImGuiCol_SeparatorActive]      = bloodHi;
    c[ImGuiCol_ResizeGrip]           = bloodDk;
    c[ImGuiCol_ResizeGripHovered]    = blood;
    c[ImGuiCol_ResizeGripActive]     = bloodHi;
    c[ImGuiCol_Tab]                  = ImVec4(0.22f, 0.03f, 0.04f, 1.00f);
    c[ImGuiCol_TabHovered]           = blood;
    c[ImGuiCol_TabActive]            = ImVec4(0.55f, 0.06f, 0.07f, 1.00f);
    c[ImGuiCol_PlotLines]            = bloodHi;
    c[ImGuiCol_PlotHistogram]        = blood;
    c[ImGuiCol_TextSelectedBg]       = ImVec4(0.70f, 0.05f, 0.07f, 0.35f);
    c[ImGuiCol_NavHighlight]         = bloodHi;
    c[ImGuiCol_NavWindowingHighlight]= ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    c[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.20f, 0.00f, 0.00f, 0.20f);
    c[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.10f, 0.00f, 0.00f, 0.55f);

    // Forma: esquinas redondeadas, bordes finos, espaciado comodo
    st.WindowRounding    = 10.0f;
    st.ChildRounding     = 8.0f;
    st.FrameRounding     = 6.0f;
    st.PopupRounding     = 8.0f;
    st.ScrollbarRounding = 8.0f;
    st.GrabRounding      = 6.0f;
    st.TabRounding       = 6.0f;
    st.WindowBorderSize  = 2.0f;
    st.ChildBorderSize   = 1.0f;
    st.FrameBorderSize   = 1.0f;
    st.PopupBorderSize   = 1.5f;
    st.TabBorderSize     = 1.0f;
    st.FramePadding      = ImVec2(8, 5);
    st.ItemSpacing       = ImVec2(8, 6);
    st.ItemInnerSpacing  = ImVec2(6, 4);
    st.WindowPadding     = ImVec2(12, 10);
    st.ScrollbarSize     = 12.0f;
    st.GrabMinSize       = 10.0f;
}
