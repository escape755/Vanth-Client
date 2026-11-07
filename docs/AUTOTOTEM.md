# Auto Totem para Vanth

## Estado

| Parte | Estado |
|---|---|
| Símbolos de inventario (`offsets.h`, bloque `InvSym64`) | Listo, 12 símbolos (arm64) |
| Detección de tótem en offhand (`autototem.h`) | Implementada, solo lectura |
| Menú y configuración (checkbox, umbral, estado) | Implementado |
| Reposición automática del tótem | **Bloqueada**: faltan datos (ver abajo) |
| Compilación `.so` (`ndk-build`) | Workflow listo, **sin verificar** (no hay NDK en el entorno de desarrollo) |
| Publicación en GitHub Releases | Workflow listo (`.github/workflows/release.yml`) |
| Instalador `vanth` para Termux | Implementado y probado contra un servidor simulado |

## Lo que falta para la reposición automática

1. `InventoryAction` y `InventoryTransaction`: tamaño de cada clase y firma de su constructor.
   Los símbolos `InventoryTransaction::addAction` y los destructores ya están, pero sin el constructor no se puede construir el objeto en la pila.
2. Acceso al contenedor del inventario del jugador y lectura de un ítem por slot
   (símbolo de `PlayerInventory`/`Container::getItem` o equivalente).
3. Identificación de tótem: confirmar si `VanillaItemNames::TotemOfUndying` es un puntero a nombre o un `ItemStack` directo.
4. Ruta donde Vanth carga `libReverseSigma.so` en el dispositivo (para el parámetro `--dest` del instalador).
5. Propietario y nombre del repositorio en GitHub (`VANTH_REPO`).

Sin estos datos, la reposición queda deshabilitada a propósito. Un paquete o escritura de memoria mal formados cierran el juego.

## Instalador

```
export VANTH_REPO=propietario/repositorio
vanth install --dest /ruta/absoluta/libReverseSigma.so
vanth status  --dest /ruta/absoluta/libReverseSigma.so
vanth uninstall --dest /ruta/absoluta/libReverseSigma.so
```

Verificación: descarga el `.so` y su `.sha256` del release, valida el SHA-256 antes de tocar el destino,
guarda una copia de respaldo y reemplaza el archivo de forma atómica.

## Publicación

Crear un tag `vX.Y.Z` y hacer push. El workflow compila `arm64-v8a` y `armeabi-v7a` con NDK r26d
y publica `libReverseSigma-<abi>.so` y `libReverseSigma-<abi>.so.sha256` en el release.
