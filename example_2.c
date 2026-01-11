/*
    ============================================================================
    EJEMPLO 2: CANVAS DE DIBUJO INTERACTIVO (PAINT)
    ============================================================================
    
    DESCRIPCIÓN:
    Este programa demuestra cómo crear una aplicación de dibujo simple pero 
    funcional utilizando la librería "engine.h". Sirve como ejemplo práctico 
    para el manejo avanzado del Mouse (diferenciando entre mantener presionado 
    y un solo click), renderizado de primitivas y uso de texto en pantalla.

    CÓMO USARLO:
    1. Ejecuta el programa. Se ocultará el cursor de Windows y aparecerá un 
       pincel personalizado.
    2. Mantén presionado el CLICK IZQUIERDO para dibujar Círculos.
    3. Mantén presionado el CLICK DERECHO para dibujar Cuadrados.
    4. Pulsa una vez el CLICK CENTRAL (Rueda) para cambiar el color del pincel.
    5. Pulsa ESPACIO para borrar todo el lienzo.

    CONCEPTOS CLAVE:
    - Input Continuo (IsKeyDown) vs Input Discreto (Lógica de Flanco/Debounce).
    - Gestión de Arreglos (Array) para almacenar el historial de dibujo.
    - Renderizado de Interfaz de Usuario (HUD) sobre el contenido del juego.
    - Ocultar y personalizar el cursor del sistema.
    ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include "engine.h"

// Configuración de la Ventana
#define SCREEN_W 800
#define SCREEN_H 600

// Cantidad máxima de figuras que podemos dibujar antes de dejar de registrar
#define MAX_SHAPES 5000 

// Estructura para guardar la información de cada "mancha" de pintura
typedef struct {
    int x, y;       // Posición
    int type;       // 0 = Círculo, 1 = Cuadrado
    GE_Color color; // Color con el que se dibujó
    int size;       // Tamaño del pincel en ese momento
} Shape;

// Función auxiliar para generar un color aleatorio opaco (Alpha 255)
GE_Color RandomColor() {
    uint8_t r = rand() % 256;
    uint8_t g = rand() % 256;
    uint8_t b = rand() % 256;
    // Combinamos los canales en un entero de 32 bits: 0xAARRGGBB
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

int main() {
    // ------------------------------------------------------------------------
    // 1. INICIALIZACIÓN DEL MOTOR
    // ------------------------------------------------------------------------
    GE_Context* ctx = GE_Init("Canvas Interactivo + Instrucciones", SCREEN_W, SCREEN_H);
    if (!ctx) {
        printf("Error: No se pudo iniciar el motor.\n");
        return -1;
    }

    // Cargamos la fuente para el texto (Asegúrate de tener la carpeta assets junto al exe)
    GE_Font* font = GE_LoadFont("assets/fonts/Roboto-Medium.ttf", 20.0f);

    // ------------------------------------------------------------------------
    // 2. VARIABLES DE ESTADO DEL JUEGO
    // ------------------------------------------------------------------------
    Shape shapes[MAX_SHAPES]; // Memoria para guardar los dibujos
    int shapeCount = 0;       // Contador actual de figuras dibujadas
    
    GE_Color currentColor = 0xFF00FF00; // Color inicial del pincel (Verde)
    int currentSize = 25;               // Tamaño del pincel en píxeles

    // Variable para controlar el "Click Único" del botón central (evitar cambios rápidos)
    bool prevMiddleMouse = false; 

    // Ocultamos el cursor del sistema operativo para dibujar el nuestro propio
    GE_ShowCursor(false);

    // ------------------------------------------------------------------------
    // 3. BUCLE PRINCIPAL (GAME LOOP)
    // ------------------------------------------------------------------------
    while (GE_PollEvents(ctx)) {
        // A. Limpiar pantalla en cada frame (Fondo Gris Oscuro)
        GE_Clear(ctx, 0xFF202020); 

        // B. Obtener posición del mouse
        GE_Point m = GE_GetMousePosition(ctx);

        // --- PROCESAMIENTO DE INPUT ---

        // DIBUJAR: Click Izquierdo (Círculos) - Se ejecuta mientras se MANTIENE presionado
        if (GE_IsKeyDown(ctx, GE_MOUSE_LEFT)) {
            if (shapeCount < MAX_SHAPES) {
                // Guardamos una nueva forma en el array
                shapes[shapeCount] = (Shape){ (int)m.x, (int)m.y, 0, currentColor, currentSize };
                shapeCount++;
            }
        }

        // DIBUJAR: Click Derecho (Cuadrados) - Se ejecuta mientras se MANTIENE presionado
        if (GE_IsKeyDown(ctx, GE_MOUSE_RIGHT)) {
            if (shapeCount < MAX_SHAPES) {
                shapes[shapeCount] = (Shape){ (int)m.x, (int)m.y, 1, currentColor, currentSize };
                shapeCount++;
            }
        }

        // CAMBIAR COLOR: Click Central - Lógica de "Flanco de Subida" (Solo al pulsar)
        bool currentMiddleMouse = GE_IsKeyDown(ctx, GE_MOUSE_MIDDLE);
        
        // Si ahora está presionado Y antes no lo estaba...
        if (currentMiddleMouse && !prevMiddleMouse) {
            currentColor = RandomColor(); // Cambiamos el color una sola vez
        }
        prevMiddleMouse = currentMiddleMouse; // Guardamos estado para el siguiente frame

        // BORRAR: Tecla Espacio
        if (GE_IsKeyPressed(ctx, GE_KEY_SPACE)) {
            shapeCount = 0; // Reiniciamos el contador (efectivamente borra todo)
        }

        // --- RENDERIZADO DEL CONTENIDO ---

        // 1. Dibujar todas las formas guardadas en el historial
        for (int i = 0; i < shapeCount; i++) {
            if (shapes[i].type == 0) {
                // Dibujar Círculo
                GE_FillCircle(ctx, shapes[i].x, shapes[i].y, shapes[i].size / 2, shapes[i].color);
            } else {
                // Dibujar Cuadrado (Centrado en X,Y)
                GE_FillRect(ctx, shapes[i].x - shapes[i].size/2, shapes[i].y - shapes[i].size/2, 
                            shapes[i].size, shapes[i].size, shapes[i].color);
            }
        }

        // --- RENDERIZADO DE LA INTERFAZ (HUD) ---

        // 2. Caja de muestra de Color Actual (Esquina Superior Izquierda)
        int hudSize = 50;
        GE_FillRect(ctx, 20, 20, hudSize, hudSize, currentColor);   // Fondo de color
        GE_DrawRect(ctx, 20, 20, hudSize, hudSize, 0xFFFFFFFF);     // Borde blanco
        
        // 3. Textos e Instrucciones
        if (font) {
            // Título Principal Centrado
            GE_DrawTextAligned(ctx, font, "CANVAS DE DIBUJO", SCREEN_W / 2, 30, GE_ALIGN_CENTER, 0xFFFFFFFF);

            // Lista de Controles (Lado Izquierdo)
            int textY = 100;
            int lineHeight = 30;
            
            GE_DrawText(ctx, font, "CONTROLES:", 20, textY, 0xFFDDDDDD);
            GE_DrawText(ctx, font, "[Click Izq]   Pincel Redondo", 20, textY + lineHeight * 1, 0xFF88FF88); 
            GE_DrawText(ctx, font, "[Click Der]   Pincel Cuadrado", 20, textY + lineHeight * 2, 0xFF8888FF); 
            GE_DrawText(ctx, font, "[Rueda Mouse] Cambiar Color", 20, textY + lineHeight * 3, 0xFFFFFF88); 
            GE_DrawText(ctx, font, "[Espacio]     Borrar Todo", 20, textY + lineHeight * 4, 0xFFAAAAAA); 

            // Estadísticas (Abajo Derecha)
            char stats[64];
            sprintf(stats, "Objetos: %d / %d", shapeCount, MAX_SHAPES);
            GE_DrawTextAligned(ctx, font, stats, SCREEN_W - 20, SCREEN_H - 30, GE_ALIGN_RIGHT, 0xFF808080);
            
            // Etiqueta "Color Actual"
            GE_DrawText(ctx, font, "Color Actual", 80, 35, 0xFFFFFFFF);
        }

        // --- RENDERIZADO DEL CURSOR PERSONALIZADO ---
        // Se dibuja al final para que siempre esté encima de todo (incluidos los textos)
        
        // Círculo blanco que indica el tamaño del pincel
        GE_DrawCircle(ctx, (int)m.x, (int)m.y, currentSize / 2, 0xFFFFFFFF); 
        
        // Punto central para precisión
        GE_FillCircle(ctx, (int)m.x, (int)m.y, 2, 0xFFFFFFFF);
        
        // Indicador de color "flotante" (Zoom junto al cursor)
        GE_FillCircle(ctx, (int)m.x + 25, (int)m.y + 25, 8, currentColor);
        GE_DrawCircle(ctx, (int)m.x + 25, (int)m.y + 25, 8, 0xFFFFFFFF);
    }

    // ------------------------------------------------------------------------
    // 4. LIMPIEZA Y CIERRE
    // ------------------------------------------------------------------------
    GE_ShowCursor(true); // Restauramos el cursor de Windows antes de salir
    GE_UnloadFont(font); // Liberamos la memoria de la fuente
    GE_Close(ctx);       // Cerramos la ventana y el motor
    return 0;
}
