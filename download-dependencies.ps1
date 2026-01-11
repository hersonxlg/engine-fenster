# =============================================================================
#   C GAME ENGINE SETUP - ULTIMATE EDITION (Windows)
#   Autor: Gemini & User
#   Descripción: Descarga librerías single-header y assets.
#                Incluye enlaces de respaldo por si las URLs cambian.
# =============================================================================

# --- CONFIGURACIÓN BÁSICA ---
$BaseDir = $PSScriptRoot
$LibDir  = Join-Path $BaseDir "libs"
$AssetDir = Join-Path $BaseDir "assets"
$FontDir  = Join-Path $AssetDir "fonts"
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

# --- BASE DE DATOS DE RECURSOS ---
# Estructura: Name (Archivo), Url (Raw directo), Home (Web oficial por si falla)

$Libraries = @(
    # --- GUI & WINDOWS ---
    @{ Name="fenster.h";         Url="https://raw.githubusercontent.com/zserge/fenster/main/fenster.h";         Home="https://github.com/zserge/fenster" },
    @{ Name="microui.c";         Url="https://raw.githubusercontent.com/rxi/microui/master/src/microui.c";     Home="https://github.com/rxi/microui" },
    @{ Name="microui.h";         Url="https://raw.githubusercontent.com/rxi/microui/master/src/microui.h";     Home="https://github.com/rxi/microui" },
    
    # --- STB SUITE (Gráficos y Texto) ---
    @{ Name="stb_truetype.h";    Url="https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h";    Home="https://github.com/nothings/stb" },
    @{ Name="stb_image.h";       Url="https://raw.githubusercontent.com/nothings/stb/master/stb_image.h";       Home="https://github.com/nothings/stb" },
    @{ Name="stb_image_write.h"; Url="https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h"; Home="https://github.com/nothings/stb" },
    @{ Name="stb_ds.h";          Url="https://raw.githubusercontent.com/nothings/stb/master/stb_ds.h";          Home="https://github.com/nothings/stb" },

    # --- AUDIO & UTILIDADES ---
    @{ Name="miniaudio.h";       Url="https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h";   Home="https://github.com/mackron/miniaudio" },
    @{ Name="tinydir.h";         Url="https://raw.githubusercontent.com/cxong/tinydir/master/tinydir.h";         Home="https://github.com/cxong/tinydir" },
    @{ Name="sokol_time.h";      Url="https://raw.githubusercontent.com/floooh/sokol/master/sokol_time.h";      Home="https://github.com/floooh/sokol" },

    # --- MATEMÁTICAS & FÍSICA ---
    @{ Name="HandmadeMath.h";    Url="https://raw.githubusercontent.com/HandmadeMath/HandmadeMath/master/HandmadeMath.h"; Home="https://github.com/HandmadeMath/HandmadeMath" },
    @{ Name="cute_c2.h";         Url="https://raw.githubusercontent.com/RandyGaul/cute_headers/master/cute_c2.h";         Home="https://github.com/RandyGaul/cute_headers" }
)

$Fonts = @(
    @{ Name="Roboto-Medium.ttf";     Url="https://raw.githubusercontent.com/ocornut/imgui/master/misc/fonts/Roboto-Medium.ttf"; Home="https://github.com/ocornut/imgui/tree/master/misc/fonts" },
    @{ Name="JetBrainsMono-Bold.ttf"; Url="https://raw.githubusercontent.com/JetBrains/JetBrainsMono/master/fonts/ttf/JetBrainsMono-Bold.ttf"; Home="https://github.com/JetBrains/JetBrainsMono" },
    @{ Name="PressStart2P.ttf";      Url="https://raw.githubusercontent.com/google/fonts/main/ofl/pressstart2p/PressStart2P-Regular.ttf"; Home="https://fonts.google.com/specimen/Press+Start+2P" },
    @{ Name="VT323.ttf";             Url="https://raw.githubusercontent.com/google/fonts/main/ofl/vt323/VT323-Regular.ttf"; Home="https://fonts.google.com/specimen/VT323" },
    @{ Name="Orbitron-Bold.ttf";     Url="https://raw.githubusercontent.com/theleagueof/orbitron/master/Orbitron%20Bold.ttf"; Home="https://github.com/theleagueof/orbitron" },
    @{ Name="MedievalSharp.ttf";     Url="https://raw.githubusercontent.com/google/fonts/main/ofl/medievalsharp/MedievalSharp.ttf"; Home="https://fonts.google.com/specimen/MedievalSharp" }
)

# --- FUNCIONES VISUALES ---
function Print-Header {
    Clear-Host
    Write-Host "============================================================" -ForegroundColor Cyan
    Write-Host "      🚀  INICIALIZADOR DE MOTOR DE JUEGOS EN C  🚀" -ForegroundColor White
    Write-Host "============================================================" -ForegroundColor Cyan
    Write-Host "  Modo: Seguro (Con enlaces de respaldo manuales)" -ForegroundColor DarkGray
    Write-Host "============================================================`n" -ForegroundColor Cyan
}

function Download-Item {
    param ($Item, $DestFolder)
    $DestPath = Join-Path $DestFolder $Item.Name
    
    # Formato de columna alineado
    $NamePadded = $Item.Name.PadRight(25)
    
    if (Test-Path $DestPath) {
        Write-Host " [EXISTE] " -ForegroundColor DarkGray -NoNewline
        Write-Host $NamePadded -ForegroundColor Gray -NoNewline
        Write-Host "(Saltando)" -ForegroundColor DarkGray
        return
    }

    Write-Host " [BAJANDO] " -ForegroundColor Yellow -NoNewline
    Write-Host $NamePadded -ForegroundColor White -NoNewline
    
    try {
        Invoke-WebRequest -Uri $Item.Url -OutFile $DestPath -UseBasicParsing -ErrorAction Stop
        Write-Host " OK " -ForegroundColor Green
    } catch {
        Write-Host " ERROR " -ForegroundColor Red
        Write-Host "`n    [!] Falló el enlace directo." -ForegroundColor Red
        Write-Host "    [i] Descarga manual en: " -ForegroundColor Yellow
        Write-Host "        $($Item.Home)" -ForegroundColor Cyan
        Write-Host ""
    }
}

# --- EJECUCIÓN PRINCIPAL ---
Print-Header

# 1. Crear Estructura
Write-Host "1. VERIFICANDO SISTEMA DE ARCHIVOS..." -ForegroundColor Magenta
$Folders = @($LibDir, $AssetDir, $FontDir)
foreach ($f in $Folders) {
    if (!(Test-Path $f)) {
        New-Item -ItemType Directory -Path $f | Out-Null
        Write-Host "   [+] Carpeta creada: $(Split-Path $f -Leaf)" -ForegroundColor Green
    }
}
Write-Host ""

# 2. Descargar Librerías
Write-Host "2. INSTALANDO LIBRERÍAS (HEADERS C)..." -ForegroundColor Magenta
foreach ($lib in $Libraries) { Download-Item -Item $lib -DestFolder $LibDir }
Write-Host ""

# 3. Descargar Fuentes
Write-Host "3. OBTENIENDO ASSETS (FUENTES)..." -ForegroundColor Magenta
foreach ($font in $Fonts) { Download-Item -Item $font -DestFolder $FontDir }

Write-Host "`n============================================================" -ForegroundColor Cyan
Write-Host "                   ✅  PROCESO TERMINADO  ✅" -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host " CONSEJOS PARA EL FUTURO:" -ForegroundColor Yellow
Write-Host " 1. Si un archivo falta, abre este script y mira la propiedad 'Home'."
Write-Host " 2. Guarda la carpeta 'libs' en tu propio repositorio (Git)."
Write-Host " 3. Compila con: gcc main.c -o game.exe -I./libs -lgdi32 -lwinmm"
Write-Host "============================================================" -ForegroundColor Cyan
