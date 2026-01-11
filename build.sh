#!/bin/bash

# ==============================================================================
# SCRIPT DE COMPILACIÓN INTELIGENTE (Linux / macOS)
# ==============================================================================

# --- 1. DETECCIÓN DEL SISTEMA OPERATIVO Y FLAGS ---
OS="$(uname -s)"
COMPILER="gcc"
ENGINE_FILE="engine.c"

# Definir colores para salida bonita
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
GRAY='\033[0;90m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GRAY}--------------------------------------------------${NC}"
echo -e "${GRAY}Sistema detectado: $OS${NC}"

# Configuración de librerías según el SO
if [ "$OS" == "Darwin" ]; then
    # macOS (Necesita Cocoa para ventana y AudioToolbox para sonido)
    LIBS="-I./libs -framework Cocoa -framework AudioToolbox -framework CoreAudio"
else
    # Linux (Necesita X11 para ventana y matemáticas/hilos para sonido)
    LIBS="-I./libs -lX11 -lm -lpthread -ldl"
fi

# --- 2. DETERMINAR ARCHIVOS A COMPILAR ---
FILES=()

if [ $# -eq 0 ]; then
    echo -e "${CYAN}Modo Automático: Buscando todos los archivos .c...${NC}"
    # Busca todos los .c en el directorio actual
    for f in *.c; do
        [ -e "$f" ] || continue # Evitar error si no hay archivos
        FILES+=("$f")
    done
else
    echo -e "${CYAN}Modo Manual: Verificando archivos...${NC}"
    FILES=("$@")

    # --- LÓGICA DE INCLUSIÓN IMPLÍCITA ---
    # Verificar si engine.c existe en disco
    if [ -f "$ENGINE_FILE" ]; then
        HAS_ENGINE=false
        # Revisar si ya está en la lista de argumentos
        for f in "${FILES[@]}"; do
            if [[ "$f" == "$ENGINE_FILE" ]]; then
                HAS_ENGINE=true
                break
            fi
        done
        
        # Si no está, agregarlo
        if [ "$HAS_ENGINE" = false ]; then
            FILES+=("$ENGINE_FILE")
            echo -e "${GRAY}  -> Nota: Se agregó '$ENGINE_FILE' automáticamente.${NC}"
        fi
    fi
fi

# Validación de seguridad
if [ ${#FILES[@]} -eq 0 ]; then
    echo -e "${RED}Error: No se encontraron archivos .c para compilar.${NC}"
    exit 1
fi

# --- 3. DETERMINAR NOMBRE DEL EJECUTABLE ---
OUTPUT_NAME="game" # Nombre por defecto si falla la lógica

# Intentar encontrar main.c
FOUND_MAIN=false
for f in "${FILES[@]}"; do
    if [[ "$f" == "main.c" ]]; then
        OUTPUT_NAME="main"
        FOUND_MAIN=true
        break
    fi
done

# Si no hay main.c, usar el nombre del primer archivo
if [ "$FOUND_MAIN" = false ]; then
    FIRST_FILE="${FILES[0]}"
    BASENAME=$(basename "$FIRST_FILE" .c)
    # Evitar llamar al ejecutable "engine" si hay otros archivos
    if [[ "$BASENAME" == "engine" && ${#FILES[@]} -gt 1 ]]; then
         SECOND_FILE="${FILES[1]}"
         OUTPUT_NAME=$(basename "$SECOND_FILE" .c)
    else
         OUTPUT_NAME="$BASENAME"
    fi
fi

# --- 4. COMPILAR ---
# Convertir array a string separado por espacios
FILES_STR="${FILES[*]}"

echo -e "${YELLOW}Compilando: ./$OUTPUT_NAME${NC}"
echo -e "${GRAY}Inputs:     $FILES_STR${NC}"
echo -e "${GRAY}--------------------------------------------------${NC}"

# Ejecutar GCC
$COMPILER $FILES_STR -o $OUTPUT_NAME $LIBS

# Verificar resultado (Exit Code 0 = Éxito)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✔ COMPILACIÓN EXITOSA${NC}"
    # Opcional: Ejecutar inmediatamente si se desea
    # ./$OUTPUT_NAME
else
    echo -e "${RED}✘ ERROR DE COMPILACIÓN${NC}"
    exit 1
fi
