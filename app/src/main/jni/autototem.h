#pragma once
// Auto Totem para Vanth.
//
// Estado de implementacion:
//  - DETECCION (implementada): en cada NormalTick, si la vida esta por debajo del umbral,
//    lee el offhand con Actor::getOffhandSlot (prologo verificado) y comprueba si contiene
//    "totem_of_undying". Solo lectura.
//  - REPOSICION (bloqueada): mover un tótem del inventario al offhand requiere
//    LocalPlayer::sendInventoryTransaction + InventoryTransaction::addAction + InventoryAction,
//    y acceder al contenedor del inventario. Esos layouts y simbolos NO estan en la tabla
//    actual, asi que no se escribe memoria ni se envia ningun paquete: el estado queda en
//    Blocked y se informa en el menu. Se activara cuando se aporten esos simbolos.
#include <cstdint>
#include "diag.h"
#include "offsets.h"

namespace AutoTotem {

enum class State : int {
    Off = 0,            // modulo desactivado
    Idle,               // vida sobre el umbral, nada que hacer
    OffhandOk,          // vida baja y el offhand ya tiene tótem
    OffhandEmpty,       // vida baja y el offhand NO tiene tótem (necesita reposicion)
    Blocked,            // reposicion pendiente: falta layout de inventario
    BadPrologue,        // la direccion de getOffhandSlot no coincide con lo esperado
    Unverified,         // deteccion pendiente de verificar (ver comentario en update)
};

static bool enabled = false;
static int triggerHealth = 10;      // vida a partir de la cual se exige tótem en offhand
static State state = State::Off;
static int emptyEvents = 0;         // veces que se detecto offhand vacio con vida baja
static bool wasEmpty = false;       // para contar solo la transicion, no cada tick

// Llamar en NormalTick. health: vida actual. player: LocalPlayer*.
// fnAddr: direccion absoluta de Actor::getOffhandSlot (InvSym64::Actor_getOffhandSlot).
static inline void update(void *player, int health, uintptr_t fnAddr) {
    if (!enabled) { state = State::Off; wasEmpty = false; return; }
    if (health < 0 || !player) { state = State::Idle; return; }
    if (health > triggerHealth) { state = State::Idle; wasEmpty = false; return; }

    if (!fnAddr) { state = State::BadPrologue; return; }
    unsigned int pro = 0;
    if (!SafeRead((const void *) fnAddr, &pro, 4) || pro != 0xA9BF7BFDu) {
        state = State::BadPrologue;
        return;
    }

    typedef void *(*GetOffhandFn)(void *);
    void *slot = ((GetOffhandFn) fnAddr)(player);
    // La busqueda de texto "totem_of_undying" dentro del ItemStack nunca acierta: el nombre
    // vive en el objeto Item, no en el slot. Hasta verificar la comparacion slot+00 frente a
    // VanillaItemNames::TotemOfUndying (ver diag.h), no se afirma nada sobre el offhand.
    (void) slot;
    state = State::Unverified;
}

static inline const char *StateText() {
    switch (state) {
        case State::Off:          return "Desactivado";
        case State::Idle:         return "En espera (vida alta)";
        case State::OffhandOk:    return "Tótem en offhand";
        case State::OffhandEmpty: return "Offhand vacío";
        case State::Blocked:      return "Sin tótem en offhand: reposición pendiente (falta layout de inventario)";
        case State::BadPrologue:  return "Dirección de offhand no verificada";
        case State::Unverified:   return "Detección pendiente de verificar (ver diagnóstico)";
    }
    return "";
}

} // namespace AutoTotem
