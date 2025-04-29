#include <iostream>
#include <windows.h>
#include <string>
//partes de funciones de allegro que se van a usar
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>

using namespace std;

//RESOLUCION DE LA VENTANA
const int ANCHO_PANTALLA = 1200;
const int ALTO_PANTALLA = 800;

//NUMERO DE ASTEROIDES
const int MINASTEROIDES = 4; // Número mínimo de asteroides
const int MAXASTEROIDES = 12; // Número máximo de asteroides

// Estructura para los asteroides
struct Asteroide {
	float x, y; // Posición en la pantalla del asteroide
	float velocidadX, velocidadY; //velocidad en X y Y, puede ser de -2 a 2, siendo las negativas las que le permiten desplazarse a la izq o hacia abajo, las positivas de derecha hacia arriba
	int tamano; //Grande = 3, mediano = 2, pequeño = 1
	bool estado; // true si está activo, false si ha sido destruido
};

//FUNCIONES
void dibujarPantalla();
void inicializarAsteroides(Asteroide*, int); //INICIALIZAR ASTEROIDES
void moverAsteroides(Asteroide*, int); //MOVER ASTEROIDES USANDO UN VECTOR Y UN NUMERO DE ASTEROIDES A MOVER
void dividirAsteroides(Asteroide*, int); //DIVIDIR ASTEROIDES USANDO UN VECROR Y EL NUMERO DE DISPAROS, SE LLAMA CUANDO SE USA LA FUNCIÓN DE DETECTAR COLISIONES
void mostrarExplosion(int x, int y);

int main(){
	//obtener la resolucion de la pantalla para poder centrarla
	int ancho, alto;

	ancho = GetSystemMetrics(SM_CXSCREEN);
	alto = GetSystemMetrics(SM_CYSCREEN);

	//inciializa allegro y detecte que no haya errores 
	if (!al_init()) {
		al_show_native_message_box(NULL, "Error Critico", "Error 404", "No se pudo cargar correctamente la librería allegro", NULL, ALLEGRO_MESSAGEBOX_OK_CANCEL);
		return -1;
	}

	//INSTALACION DE TODAS LAS FUNCIONES ALLEGRO A UTILIZAR BASICAS
	al_init_font_addon(); //para uso de las fuentes
	al_init_ttf_addon(); //uso de fuentes con formato ttf
	al_init_primitives_addon(); //dibujar figuras primitivas (rectangulo, circulo, etc)
	al_init_image_addon(); //para imagenes
	al_install_mouse(); //para uso del mouse
	al_install_keyboard(); //para uso de las teclas del teclado

	//VARIABLES DE COLOR 
	ALLEGRO_COLOR negro = al_map_rgb(0,0,0);
	ALLEGRO_COLOR blanco = al_map_rgb(255,255,255);

	//VENTANA Y ACCIONES, DELCARACIONES DE OBJETOS DEL TIPO ALLEGRO
	ALLEGRO_DISPLAY* ventana = al_create_display(ANCHO_PANTALLA, ALTO_PANTALLA); //crea una ventana de nxm
	ALLEGRO_FONT* roboto = al_load_font("roboto.ttf", 30, 0); //importar roboto

	ALLEGRO_TIMER* segundoTimer = al_create_timer(1.0); //segundero
	ALLEGRO_TIMER* FPS = al_create_timer(1.0/35); //timer de fps 

	ALLEGRO_EVENT evento; //varable de evento
	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue(); //manejar cola de eventos

	ALLEGRO_BITMAP* nave = al_load_bitmap("assets/img/nave.png"); //imagen de la nave
	ALLEGRO_BITMAP* asteroide = al_load_bitmap("assets/img/asteroide.png");
	if (!asteroide) {
		al_show_native_message_box(NULL, "Error", "No se pudo cargar la imagen del asteroide", NULL, NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}
	//ALLEGRO_BITMAP* ovni_1 = al_load_bitmap("ovni_1.png"); //imagen del ovni tipo 1
	//ALLEGRO_BITMAP* ovni_2 = al_load_bitmap("ovni_2.png"); //imagen del ovni tipo 2



	ALLEGRO_KEYBOARD_STATE keyState; //para manjejar el estado del teclado


	//REGISTRO DE EVENTOS E INCIIALIZACION
	al_register_event_source(queue, al_get_timer_event_source(segundoTimer)); //registrar eventos, en este caso, el timer
	al_register_event_source(queue, al_get_timer_event_source(FPS));
	al_register_event_source(queue, al_get_mouse_event_source()); //mouse
	al_register_event_source(queue, al_get_keyboard_event_source()); //teclado

	al_start_timer(segundoTimer); //inicia el timer
	al_start_timer(FPS); //inicia el timer
	al_get_keyboard_state(&keyState);

	al_set_window_title(ventana, "Asteroides Chafa");
	al_set_window_position(ventana, ancho/2 - ANCHO_PANTALLA / 2, alto/2 - ALTO_PANTALLA/2);


	//VARIABLES DE VENTANA
	int segundos = 0, fotogramas = 0;
	int x, y;
	bool running = true;

	srand(time(NULL)); // Inicializar la semilla para números aleatorios
	int numeroAsteroides = 15; //minimo 4 asteroides
	Asteroide* asteroides = new Asteroide[numeroAsteroides];

	inicializarAsteroides(asteroides, numeroAsteroides);


	//CICLO DE LA EJECUCION DEL PROGRAMA
	while (running) {

		al_wait_for_event(queue, &evento); //esperamos al evento


		if (evento.type == ALLEGRO_EVENT_TIMER) {
			if (evento.timer.source == segundoTimer) {
				segundos++;
			}

			if (evento.timer.source == FPS) {

				// Mover asteroides
				moverAsteroides(asteroides, numeroAsteroides);

				al_clear_to_color(negro);

				for (int i = 0; i < numeroAsteroides; i++) {
					if (asteroides[i].estado) {
						int nuevoAncho, nuevoAlto;

						// Cambiar tamaño según el tamaño del asteroide (puedes ajustar esto)
						if (asteroides[i].tamano == 3) { // Grande
							nuevoAncho = 55;
							nuevoAlto = 55;
						}
						else if (asteroides[i].tamano == 2) { // Mediano
							nuevoAncho = 40;
							nuevoAlto = 40;
						}
						else if(asteroides[i].tamano) { // Pequeño
							nuevoAncho = 25;
							nuevoAlto = 25;
						}

						// Dibujar asteroide con el tamaño ajustado
						al_draw_scaled_bitmap(
							asteroide,
							0, 0, al_get_bitmap_width(asteroide), al_get_bitmap_height(asteroide), // Usar toda la imagen
							asteroides[i].x, asteroides[i].y,  // Posición en la pantalla
							nuevoAncho, nuevoAlto,             // Nuevo tamaño basado en el tipo de asteroide
							0); // Sin rotación
						//al_draw_filled_circle(asteroides[i].x + 25, asteroides[i].y + 25, 10, al_map_rgb(255, 0, 0)); // Rojo para visualización
					}

						
				
				}

				al_flip_display();


			}
		}
		/*else if (evento.type == ALLEGRO_EVENT_KEY_DOWN) {
			for (int i = 0; i < numeroAsteroides; i++) {
				if (asteroides[i].estado) {
					switch (evento.keyboard.keycode) {
					case ALLEGRO_KEY_LEFT:
						asteroides[i].x -= 5;
						break;
					case ALLEGRO_KEY_RIGHT:
						asteroides[i].x += 5;
						break;
					}

				}
			}
		}*/

		//al_rest(1000); //pausa por n segundos
	}//while

	delete[] asteroides;
	al_destroy_display(ventana); //destruir la ventana tras terminar la ejecucion

	return 0;
}

//INICIALIZACION DE LAS FUNCIONES

void dibujarPantalla() {

}

void inicializarAsteroides(Asteroide *asteroides, int numAsteroides) {
	// Recorre el arrglo de asteroides para asignarles sus 
	for (int i = 0; i < numAsteroides; i++) {

		asteroides[i].x = rand() % (ANCHO_PANTALLA); // Posición X entre 0 y 799
		asteroides[i].y = rand() % (ALTO_PANTALLA); // Posición Y entre 0 y 599

		asteroides[i].velocidadX = (rand() % 5) - 2; // Velocidad X entre -2 y 2
		asteroides[i].velocidadY = (rand() % 5) - 2; // Velocidad Y entre -2 y 2


		asteroides[i].tamano = rand() % 3 + 1; // Tamaño aleatorio (1=pequeño, 2=mediano, 3=grande)

		asteroides[i].estado = true; //todos los asteroides están activos inicialmente
	}
}

void moverAsteroides(Asteroide* asteroides, int numAsteroides) {
	for (int i = 0; i < numAsteroides; i++) {
		if (asteroides[i].estado) {  // Solo mover asteroides activos
			asteroides[i].x += asteroides[i].velocidadX; //se les suma la velocidad en el eje X y el eje Y para que se muevan
			asteroides[i].y += asteroides[i].velocidadY;

			//validar si salen de la pantalla para reubicarlos en el extremo contrario
			if (asteroides[i].x > ANCHO_PANTALLA) { //si toca la pantalla en la derecga en el eje X, aparece del aldo izquierdo
				asteroides[i].x = 0;
			}
			if (asteroides[i].x < 0) { //si está tocando el extremo izquiero, aparece en la derecha
				asteroides[i].x = ANCHO_PANTALLA;
			}
			if (asteroides[i].y > ALTO_PANTALLA) { //si llega al final de la parte superior, se regresa a la parte inferior
				asteroides[i].y = 0;
			}
			if (asteroides[i].y < 0) { //si llega a la parte inferior, se regresa a la superior
				asteroides[i].y = ALTO_PANTALLA;
			}
		}
	}
}

void dividirAsteroides(Asteroide *asteroides, int indice) {
	Asteroide original = asteroides[indice];

	// Simula una animación de explosión
	mostrarExplosion(original.x, original.y);

	if (original.tamano < 2) {
		for (int i = 0; i < 2; ++i) {
			Asteroide nuevo;
			nuevo.tamano = original.tamano + 1;

			nuevo.x = original.x;
			nuevo.y = original.y;

			// velocidad aleatoria
			nuevo.velocidadX = (rand() % 5) - 2; // Velocidad X entre -2 y 2
			nuevo.velocidadY = (rand() % 5) - 2; // Velocidad Y entre -2 y 2

			nuevo.estado = true; //todos los asteroides están activos inicialmente

			//asteroides.push_back(nuevo); // añadimos
		}
	}

		// Eliminar asteroide original
		//asteroides.erase(asteroides.begin() + indice);

}

void mostrarExplosion(int x, int y) {

}