<#
.SYNOPSIS
    Script de compilación inteligente para Game Engine.
    
.DESCRIPTION
    Compila archivos C con GCC.
    - Incluye automáticamente 'engine.c' si es necesario.
    - Evita duplicados si el usuario ya incluyó 'engine.c'.
    - Detecta el nombre del ejecutable automáticamente.
    - Linkea librerías del sistema y ./libs.

.EXAMPLE
    .\build.ps1
    (Automático: Compila todo lo que encuentre)

.EXAMPLE
    .\build.ps1 main.c
    (Inteligente: Compila main.c + engine.c implícitamente)

.EXAMPLE
    .\build.ps1 main.c engine.c
    (Verificación: Nota que engine.c ya está y no lo duplica)
#>

param(
    [Parameter(Position=0, ValueFromRemainingArguments=$true)]
    [string[]]$Files,

    [Parameter(Mandatory=$false)]
    [string]$Name = ""
)

# --- CONFIGURACIÓN ---
$Compiler   = "gcc"
$Flags      = "-I./libs -lgdi32 -lwinmm"
$EngineFile = "engine.c"

# 1. DETERMINAR ARCHIVOS A COMPILAR
if ($null -eq $Files -or $Files.Count -eq 0) {
    Write-Host "Modo Automático: Buscando todos los archivos .c..." -ForegroundColor Cyan
    # Convertimos a Lista para poder manipularla si fuera necesario
    $FilesToCompile = [System.Collections.Generic.List[string]](Get-ChildItem -Filter "*.c" | Select-Object -ExpandProperty Name)
} else {
    Write-Host "Modo Manual: Verificando archivos..." -ForegroundColor Cyan
    # Creamos una lista basada en los argumentos del usuario
    $FilesToCompile = [System.Collections.Generic.List[string]]$Files

    # --- LÓGICA DE INCLUSIÓN IMPLÍCITA ---
    # Verificamos si engine.c existe en el disco primero
    if (Test-Path $EngineFile) {
        # Verificamos si el usuario YA lo incluyó en la lista (case-insensitive)
        if (-not ($FilesToCompile -contains $EngineFile)) {
            $FilesToCompile.Add($EngineFile)
            Write-Host "  -> Nota: Se agregó '$EngineFile' automáticamente." -ForegroundColor DarkGray
        }
    }
}

# 2. VALIDACIÓN DE SEGURIDAD
if ($FilesToCompile.Count -eq 0) {
    Write-Error "Error: No hay archivos para compilar."
    exit 1
}

# 3. DETERMINAR NOMBRE DEL EJECUTABLE
if ([string]::IsNullOrWhiteSpace($Name)) {
    # Busca 'main.c' para usarlo de nombre base
    $MainSource = $FilesToCompile | Where-Object { $_ -eq "main.c" } | Select-Object -First 1
    
    if ($null -ne $MainSource) {
        $BaseName = "main"
    } else {
        # Si no hay main.c, usa el nombre del primer archivo (ej: prueba.c -> prueba.exe)
        $BaseName = [System.IO.Path]::GetFileNameWithoutExtension($FilesToCompile[0])
        # Si el primer archivo resulta ser engine.c (raro), intentamos buscar otro
        if ($BaseName -eq "engine" -and $FilesToCompile.Count -gt 1) {
             $BaseName = [System.IO.Path]::GetFileNameWithoutExtension($FilesToCompile[1])
        }
    }
    $OutputExe = "$BaseName.exe"
} else {
    $OutputExe = if ($Name.EndsWith(".exe")) { $Name } else { "$Name.exe" }
}

# 4. CONSTRUIR Y EJECUTAR COMANDO
$FileListStr = $FilesToCompile -join " "
$Command = "$Compiler $FileListStr -o $OutputExe $Flags"

Write-Host "--------------------------------------------------" -ForegroundColor DarkGray
Write-Host "Compilando: $OutputExe" -ForegroundColor Yellow
Write-Host "Inputs:     $FileListStr" -ForegroundColor DarkGray
Write-Host "--------------------------------------------------" -ForegroundColor DarkGray

Invoke-Expression $Command

# 5. RESULTADO
if ($LASTEXITCODE -eq 0) {
    Write-Host "✔ COMPILACIÓN EXITOSA" -ForegroundColor Green
} else {
    Write-Host "✘ ERROR DE COMPILACIÓN" -ForegroundColor Red
    exit $LASTEXITCODE
}
