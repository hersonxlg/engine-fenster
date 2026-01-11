/*
    ============================================================================
    EJEMPLO 3: SPRITES Y SISTEMA DE AUDIO
    ============================================================================
    
    DESCRIPCIÓN:
    Este ejemplo demuestra la capacidad multimedia del motor. Carga imágenes
    (sprites) desde el disco y las renderiza en pantalla aplicando un color
    de tinte (tint) neutro. También carga música de fondo y efectos de sonido.

    CÓMO USARLO:
    1. Asegúrate de tener los archivos listados en la sección "CONFIGURACIÓN DE ASSETS".
    2. Ejecuta el programa. Escucharás música de fondo.
    3. Mueve el mouse: La nave seguirá tu cursor.
    4. CLICK IZQUIERDO: Reproduce un sonido de disparo y la nave tiembla.
    5. FLECHAS ARRIBA/ABAJO: Subir o bajar volumen de la música.

    NOTA TÉCNICA:
    La función GE_DrawSprite requiere un color de "Tinte". Usamos 0xFFFFFFFF
    (Blanco) para que la imagen se vea con sus colores originales. Si usaras
    0xFFFF0000 (Rojo), la imagen se vería teñida de rojo.
    ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include "engine.h"

// ============================================================================
// CONFIGURACIÓN DE ASSETS (RUTAS GLOBALES)
// ============================================================================

// Imágenes
const char* SPRITE_FONDO     = "assets/images/fondo_espacio.png";
const char* SPRITE_PERSONAJE = "assets/images/nave.png";
const char* SPRITE_CURSOR    = "assets/images/mirilla.png";

// Sonidos
const char* AUDIO_MUSICA     = "assets/sounds/musica_fondo.mp3";
const char* AUDIO_DISPARO    = "assets/sounds/laser.wav";

// Fuente
const char* FUENTE_TEXTO     = "assets/fonts/Roboto-Medium.ttf";

// Configuración de Pantalla
#define SCREEN_W 800
#define SCREEN_H 600

int main() {
    // ------------------------------------------------------------------------
    // 1. INICIALIZACIÓN
    // ------------------------------------------------------------------------
    GE_Context* ctx = GE_Init("Ejemplo 3: Sprites y Audio", SCREEN_W, SCREEN_H);
    if (!ctx) {
        printf("Error: No se pudo iniciar el motor gráfico.\n");
        return -1;
    }

    // Ocultamos el cursor del sistema para usar uno propio (sprite)
    GE_ShowCursor(false);

    // Cargamos la fuente
    GE_Font* font = GE_LoadFont(FUENTE_TEXTO, 20.0f);

    // ------------------------------------------------------------------------
    // 2. CARGA DE RECURSOS (SPRITES & AUDIO)
    // ------------------------------------------------------------------------
    printf("--- Cargando Assets ---\n");

    // A. Cargar Imágenes
    GE_Sprite* bg = GE_LoadSprite(SPRITE_FONDO);
    GE_Sprite* ship = GE_LoadSprite(SPRITE_PERSONAJE);
    GE_Sprite* cursor = GE_LoadSprite(SPRITE_CURSOR);

    // B. Cargar Audio
    GE_Sound* music = GE_LoadSound(AUDIO_MUSICA);
    GE_Sound* sfx = GE_LoadSound(AUDIO_DISPARO);

    // Validaciones básicas de carga (Muestra error en consola si falta algo)
    if (!bg) printf("[Advertencia] No se encontro: %s\n", SPRITE_FONDO);
    if (!ship) printf("[Advertencia] No se encontro: %s\n", SPRITE_PERSONAJE);
    if (!music) printf("[Advertencia] No se encontro: %s\n", AUDIO_MUSICA);

    // ------------------------------------------------------------------------
    // 3. CONFIGURACIÓN DE AUDIO INICIAL
    // ------------------------------------------------------------------------
    float volume = 0.5f; // Volumen inicial (50%)

    if (music) {
        GE_SetLooping(music, true); // Configurar para que se repita
        GE_SetVolume(music, volume);
        GE_PlaySound(music);        // Iniciar reproducción
    }

    // ------------------------------------------------------------------------
    // 4. BUCLE PRINCIPAL (GAME LOOP)
    // ------------------------------------------------------------------------
    while (GE_PollEvents(ctx)) {
        // Limpiamos pantalla con negro (por si el fondo no carga)
        GE_Clear(ctx, 0xFF000000); 

        // --- INPUT ---
        GE_Point m = GE_GetMousePosition(ctx);

        // Control de Volumen
        if (GE_IsKeyPressed(ctx, GE_KEY_UP)) {
            volume += 0.05f;
            if (volume > 1.0f) volume = 1.0f;
            if (music) GE_SetVolume(music, volume);
        }
        if (GE_IsKeyPressed(ctx, GE_KEY_DOWN)) {
            volume -= 0.05f;
            if (volume < 0.0f) volume = 0.0f;
            if (music) GE_SetVolume(music, volume);
        }

        // Disparo (Click Izquierdo)
        bool isShooting = false;
        if (GE_IsKeyPressed(ctx, GE_MOUSE_LEFT)) {
            if (sfx) {
                GE_SetVolume(sfx, 0.8f); // Volumen alto para el disparo
                GE_PlaySound(sfx);       // Reproducir efecto
            }
            isShooting = true;
        }

        // --- RENDERIZADO (CAPAS) ---

        // CAPA 1: Fondo
        if (bg) {
            // CORRECCIÓN APLICADA: Agregado 0xFFFFFFFF como 5to argumento (Tinte Blanco)
            GE_DrawSprite(ctx, bg, 0, 0, 0xFFFFFFFF); 
        } else {
            GE_Clear(ctx, 0xFF200020); // Fallback color morado
        }

        // CAPA 2: Personaje (Nave)
        if (ship) {
            // Centrar el sprite en el mouse (offset de 32px aprox)
            int shipX = (int)m.x - 32; 
            int shipY = (int)m.y - 32;

            // Efecto visual: Vibración al disparar
            if (isShooting) {
                shipX += (rand() % 10) - 5;
                shipY += (rand() % 10) - 5;
            }

            // CORRECCIÓN APLICADA: Tinte Blanco
            GE_DrawSprite(ctx, ship, shipX, shipY, 0xFFFFFFFF);
        } else {
            // Fallback: Círculo azul si no hay imagen
            GE_FillCircle(ctx, (int)m.x, (int)m.y, 30, 0xFF0000FF);
        }

        // CAPA 3: Interfaz de Usuario (Texto)
        if (font) {
            GE_DrawText(ctx, font, "DEMO MULTIMEDIA", 20, 30, 0xFFFFFFFF);
            
            char volText[64];
            sprintf(volText, "Volumen Musica: %d%% (Flechas Arriba/Abajo)", (int)(volume * 100));
            GE_DrawText(ctx, font, volText, 20, SCREEN_H - 40, 0xFFBBBBBB);

            GE_DrawTextAligned(ctx, font, "[Click] Disparar SFX", SCREEN_W - 20, SCREEN_H - 40, GE_ALIGN_RIGHT, 0xFFFFFF00);
        }

        // CAPA 4: Cursor (Siempre encima)
        if (cursor) {
            // CORRECCIÓN APLICADA: Tinte Blanco
            GE_DrawSprite(ctx, cursor, (int)m.x - 16, (int)m.y - 16, 0xFFFFFFFF);
        } else {
            // Fallback: Punto rojo
            GE_FillCircle(ctx, (int)m.x, (int)m.y, 5, 0xFFFF0000);
        }
    }

    // ------------------------------------------------------------------------
    // 5. LIMPIEZA DE MEMORIA
    // ------------------------------------------------------------------------
    GE_UnloadSprite(cursor);
    GE_UnloadSprite(ship);
    GE_UnloadSprite(bg);
    
    GE_UnloadSound(sfx);
    GE_UnloadSound(music);
    
    GE_UnloadFont(font);
    GE_Close(ctx);
    
    return 0;
}
