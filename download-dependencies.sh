#!/bin/bash

# =============================================================================
#   C GAME ENGINE SETUP - ULTIMATE EDITION (Linux & Mac)
#   Descripción: Descarga librerías single-header y assets.
# =============================================================================

# --- COLORES ANSI PARA LA TERMINAL ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
GRAY='\033[0;90m'
NC='\033[0m' # No Color

# --- DIRECTORIOS ---
BASE_DIR=$(pwd)
LIB_DIR="$BASE_DIR/libs"
ASSET_DIR="$BASE_DIR/assets"
FONT_DIR="$ASSET_DIR/fonts"

# --- FUNCIÓN DE DESCARGA ---
download_item() {
    local name=$1
    local url=$2
    local home=$3
    local dest_dir=$4
    local dest_path="$dest_dir/$name"

    # Verificar si ya existe
    if [ -f "$dest_path" ]; then
        printf " ${GRAY}[EXISTE] %-25s (Saltando)${NC}\n" "$name"
        return
    fi

    # Intentar descargar
    printf " ${YELLOW}[BAJANDO] %-25s${NC}" "$name"
    
    # Usamos curl: -s (silencioso), -L (seguir redirecciones), -f (fallar si hay error http), -o (output)
    if curl -s -L -f -o "$dest_path" "$url"; then
        printf "${GREEN} OK ${NC}\n"
    else
        printf "${RED} ERROR ${NC}\n"
        printf "${RED}    [!] Falló la descarga automática.${NC}\n"
        printf "${CYAN}    [i] Descarga manual en: $home${NC}\n"
        # Borrar archivo vacío si se creó
        rm -f "$dest_path"
    fi
}

# --- CABECERA ---
clear
echo -e "${CYAN}============================================================${NC}"
echo -e "${CYAN}      🚀  INICIALIZADOR DE MOTOR DE JUEGOS EN C (UNIX) 🚀${NC}"
echo -e "${CYAN}============================================================${NC}"
echo -e "${GRAY}  Sistema: Linux / macOS${NC}"
echo -e "${CYAN}============================================================\n${NC}"

# 1. CREAR ESTRUCTURA
echo -e "${BLUE}1. VERIFICANDO SISTEMA DE ARCHIVOS...${NC}"

for dir in "$LIB_DIR" "$ASSET_DIR" "$FONT_DIR"; do
    if [ ! -d "$dir" ]; then
        mkdir -p "$dir"
        echo -e "${GREEN}   [+] Carpeta creada: $dir${NC}"
    fi
done
echo ""

# 2. DESCARGAR LIBRERÍAS (HEADERS C)
echo -e "${BLUE}2. INSTALANDO LIBRERÍAS (HEADERS C)...${NC}"

# GUI & WINDOWS
download_item "fenster.h" "https://raw.githubusercontent.com/zserge/fenster/main/fenster.h" "https://github.com/zserge/fenster" "$LIB_DIR"
download_item "microui.c" "https://raw.githubusercontent.com/rxi/microui/master/src/microui.c" "https://github.com/rxi/microui" "$LIB_DIR"
download_item "microui.h" "https://raw.githubusercontent.com/rxi/microui/master/src/microui.h" "https://github.com/rxi/microui" "$LIB_DIR"

# STB SUITE
download_item "stb_truetype.h" "https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h" "https://github.com/nothings/stb" "$LIB_DIR"
download_item "stb_image.h" "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "https://github.com/nothings/stb" "$LIB_DIR"
download_item "stb_image_write.h" "https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h" "https://github.com/nothings/stb" "$LIB_DIR"
download_item "stb_ds.h" "https://raw.githubusercontent.com/nothings/stb/master/stb_ds.h" "https://github.com/nothings/stb" "$LIB_DIR"

# AUDIO & UTILIDADES
download_item "miniaudio.h" "https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h" "https://github.com/mackron/miniaudio" "$LIB_DIR"
download_item "tinydir.h" "https://raw.githubusercontent.com/cxong/tinydir/master/tinydir.h" "https://github.com/cxong/tinydir" "$LIB_DIR"
download_item "sokol_time.h" "https://raw.githubusercontent.com/floooh/sokol/master/sokol_time.h" "https://github.com/floooh/sokol" "$LIB_DIR"

# MATH & PHYSICS
download_item "HandmadeMath.h" "https://raw.githubusercontent.com/HandmadeMath/HandmadeMath/master/HandmadeMath.h" "https://github.com/HandmadeMath/HandmadeMath" "$LIB_DIR"
download_item "cute_c2.h" "https://raw.githubusercontent.com/RandyGaul/cute_headers/master/cute_c2.h" "https://github.com/RandyGaul/cute_headers" "$LIB_DIR"

echo ""

# 3. DESCARGAR FUENTES
echo -e "${BLUE}3. OBTENIENDO ASSETS (FUENTES)...${NC}"

# ROBOTO (Imgui Repo - Safe)
download_item "Roboto-Medium.ttf" "https://raw.githubusercontent.com/ocornut/imgui/master/misc/fonts/Roboto-Medium.ttf" "https://github.com/ocornut/imgui" "$FONT_DIR"

# CODE
download_item "JetBrainsMono-Bold.ttf" "https://raw.githubusercontent.com/JetBrains/JetBrainsMono/master/fonts/ttf/JetBrainsMono-Bold.ttf" "https://github.com/JetBrains/JetBrainsMono" "$FONT_DIR"

# GAMER STYLES
download_item "PressStart2P.ttf" "https://raw.githubusercontent.com/google/fonts/main/ofl/pressstart2p/PressStart2P-Regular.ttf" "https://fonts.google.com" "$FONT_DIR"
download_item "VT323.ttf" "https://raw.githubusercontent.com/google/fonts/main/ofl/vt323/VT323-Regular.ttf" "https://fonts.google.com" "$FONT_DIR"
download_item "Orbitron-Bold.ttf" "https://raw.githubusercontent.com/theleagueof/orbitron/master/Orbitron%20Bold.ttf" "https://github.com/theleagueof/orbitron" "$FONT_DIR"
download_item "MedievalSharp.ttf" "https://raw.githubusercontent.com/google/fonts/main/ofl/medievalsharp/MedievalSharp.ttf" "https://fonts.google.com" "$FONT_DIR"

echo ""
echo -e "${CYAN}============================================================${NC}"
echo -e "${GREEN}                   ✅  PROCESO TERMINADO  ✅${NC}"
echo -e "${CYAN}============================================================${NC}"
echo -e "${YELLOW} NOTAS DE COMPILACIÓN (LINUX/MAC):${NC}"
echo -e " 1. En Linux necesitas X11 y Audio:"
echo -e "    ${GRAY}sudo apt install libx11-dev libasound2-dev${NC} (Ubuntu/Debian)"
echo -e " 2. En macOS necesitas frameworks:"
echo -e "    ${GRAY}-framework Cocoa -framework IOKit -framework CoreVideo${NC}"
echo -e ""
echo -e " ${YELLOW}COMANDO EJEMPLO:${NC}"
echo -e " gcc main.c -o game -I./libs -lm -lX11 -lasound   (Linux)"
echo -e " gcc main.c -o game -I./libs -framework Cocoa ... (Mac)"
echo -e "${CYAN}============================================================${NC}"
