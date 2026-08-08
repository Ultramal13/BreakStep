# BreakStep → App de macOS (paso a paso desde cero)

Objetivo: convertir tu sequencer en una app `.app` nativa de Mac, sin firmar, para usar en tus equipos.

Tiempo estimado: 30-40 min la primera vez (la mayoría es esperar instalaciones).

---

## Lo que vas a tener al final

- Una app **BreakStep.app** que abres con doble clic, sin navegador, sin internet.
- Un instalador **BreakStep.dmg** que copias a tu otro Mac y arrastras a Aplicaciones.

---

## Parte 1 — Instalar lo necesario (una sola vez)

Abre la **Terminal** (Cmd+Espacio, escribe "Terminal", Enter) y pega cada bloque.

### 1.1 Homebrew (gestor de paquetes)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Al terminar, te dirá que ejecutes dos líneas para agregar `brew` al PATH. **Cópialas y ejecútalas** (varían según tu Mac, hazle caso al mensaje).

Verifica:
```bash
brew --version
```

### 1.2 Node.js

```bash
brew install node
node --version
```

Debe mostrar v20 o superior.

### 1.3 Rust (el motor de Tauri)

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

Cuando pregunte, presiona **Enter** para la opción por defecto (1). Al terminar:

```bash
source "$HOME/.cargo/env"
rustc --version
```

### 1.4 Herramientas de compilación de Apple (Xcode CLI)

```bash
xcode-select --install
```

Se abre una ventana, dale **Instalar**. Espera a que termine (puede tardar varios minutos).

---

## Parte 2 — Crear el proyecto Tauri

### 2.1 Ir a donde quieres el proyecto

```bash
cd ~/Downloads
```

### 2.2 Crear el proyecto con el asistente oficial

```bash
npm create tauri-app@latest
```

Te hará preguntas. Responde **exactamente** así:

| Pregunta | Respuesta |
|----------|-----------|
| Project name | `breakstep` |
| Identifier | `cl.dgtl.breakstep` |
| Choose which language for frontend | **TypeScript / JavaScript** |
| Choose your package manager | **npm** |
| Choose your UI template | **Vanilla** |
| Choose your UI flavor | **JavaScript** |

> Si alguna pregunta no aparece igual, elige siempre la opción más simple (Vanilla / JavaScript / npm).

### 2.3 Entrar e instalar dependencias

```bash
cd breakstep
npm install
```

---

## Parte 3 — Meter tu sequencer

El asistente creó una carpeta `src/` con un demo. Vas a **reemplazarla** por la tuya.

### 3.1 Borrar el contenido demo de src

```bash
rm -rf src/*
```

### 3.2 Copiar tu frontend

Copia los 4 archivos de la carpeta `breakstep_frontend/` que te entregué (index.html + los 3 .js) **dentro de la carpeta `src/`** del proyecto.

Por Finder: arrastra los 4 archivos a `~/Downloads/breakstep/src/`.

O por terminal (ajusta la ruta de origen a donde descargaste los archivos):

```bash
cp ~/Downloads/breakstep_frontend/* ~/Downloads/breakstep/src/
```

### 3.3 Verificar

```bash
ls src/
```

Debe mostrar: `index.html`, `react.production.min.js`, `react-dom.production.min.js`, `tone.js`.

---

## Parte 4 — Ajustar la config (2 cambios mínimos)

El proyecto Vanilla espera un servidor de desarrollo; tu app es estática, así que hay que decirle a Tauri que cargue los archivos directo.

### 4.1 Abrir el archivo de config

```bash
open -e src-tauri/tauri.conf.json
```

Se abre en TextEdit.

### 4.2 Buscar la sección `"build"` y dejarla así

Borra cualquier `beforeDevCommand` o `beforeBuildCommand` y deja:

```json
"build": {
  "frontendDist": "../src",
  "devUrl": "http://localhost:1420"
}
```

> Lo único imprescindible es `"frontendDist": "../src"`. Eso apunta a tu carpeta.

### 4.3 (Opcional) Tamaño de ventana

Busca la sección `"windows"` y ajusta para que abra cómoda:

```json
"windows": [
  {
    "title": "BreakStep",
    "width": 1240,
    "height": 760,
    "resizable": true
  }
]
```

Guarda (Cmd+S) y cierra.

---

## Parte 5 — Probar antes de compilar

```bash
npm run tauri dev
```

La primera vez compila Rust y tarda unos minutos. Luego se abre una **ventana nativa** con tu BreakStep. Pruébalo. Para cerrar: cierra la ventana o Ctrl+C en la terminal.

> Si ves la ventana en blanco: revisa que los 4 archivos estén en `src/` y que `frontendDist` sea `"../src"`.

---

## Parte 6 — Compilar la app final

```bash
npm run tauri build
```

Tarda varios minutos (compila optimizado). Al terminar, tus archivos están en:

```
src-tauri/target/release/bundle/
```

Dentro encuentras:
- **`macos/BreakStep.app`** → la app, doble clic para abrir.
- **`dmg/BreakStep_0.1.0_aarch64.dmg`** → el instalador para copiar a otro Mac.

---

## Parte 7 — Usar en tu otro Mac

1. Copia el **`.dmg`** (por AirDrop, USB, o nube) al otro Mac.
2. Doble clic al `.dmg`, arrastra **BreakStep** a la carpeta Aplicaciones.
3. **Primera apertura**: como no está firmada, macOS la bloqueará. Haz esto:
   - Clic derecho sobre la app → **Abrir** → en el diálogo, **Abrir** otra vez.
   - O si insiste: Ajustes del Sistema → Privacidad y Seguridad → baja hasta el aviso de BreakStep → **Abrir de todos modos**.
   - Solo hace falta la primera vez.

---

## Notas honestas

- **Sin firmar = ese aviso de seguridad.** Es normal y esperado para apps personales. Quitarlo requiere cuenta Apple Developer (99 USD/año); no vale la pena para uso propio.
- **Arquitectura:** el `.dmg` que generes sirve para Macs con el mismo chip (Apple Silicon M1/M2/M3/M4/M5). Si algún Mac tuyo es Intel, avísame y te paso cómo hacer un build universal.
- **Esto es la base.** Time-stretch real (Rubber Band) e interconexión entre apps (MIDI/BlackHole) se construyen sobre este mismo proyecto, en el lado Rust. Cuando quieras, lo seguimos.

---

## Si algo falla

Copia el mensaje de error de la terminal y mándamelo. Los tropiezos típicos:
- `command not found: npm` → reinstala Node (paso 1.2) o reinicia la terminal.
- `command not found: cargo` → ejecuta `source "$HOME/.cargo/env"` y reintenta.
- Ventana en blanco → revisa Parte 4.2 (`frontendDist`).
