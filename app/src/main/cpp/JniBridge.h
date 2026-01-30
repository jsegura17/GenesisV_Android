#ifndef GENESISV_JNIBRIDGE_H
#define GENESISV_JNIBRIDGE_H

struct android_app;

/*! Índice del ejemplo seleccionado en el menú (0 = Base, 1 = 001, … 16 = 015). */
int getExampleIndex();

/*! Pide a la Activity que cierre (vuelve al menú). Llamar desde el hilo de render. */
void requestFinishActivity(android_app *app);

/*! Datos pendientes de textura "Back Menu" para el overlay. */
struct PendingBackLabel {
    int width = 0;
    int height = 0;
    uint8_t *pixels = nullptr;
};
void setPendingBackButtonLabel(int width, int height, const uint8_t *pixels);
bool getPendingBackButtonLabel(PendingBackLabel *out);
void clearPendingBackButtonLabel();

#endif
