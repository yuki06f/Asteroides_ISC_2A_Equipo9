#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
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
const int MAX_BALAS = 100; //cantidad máxima de las balas

// Estructura para los asteroides
struct Asteroide {
	float x, y; // Posición en la pantalla del asteroide
	float velocidadX, velocidadY; //velocidad en X y Y, puede ser de -2 a 2, siendo las negativas las que le permiten desplazarse a la izq o hacia abajo, las positivas de derecha hacia arriba
	int tamano; //Grande = 3, mediano = 2, pequeño = 1
	bool estado; // true si está activo, false si ha sido destruido
};

//Estructura de las balas
struct Bala {
	float x, y;
	bool activa;
};

//Estructura de la nave
struct Nave {
	float x, y;
	Nave() {
		x = ANCHO_PANTALLA / 2;
		y = ALTO_PANTALLA - 100;
	}
};


//PROTOTIPOS DE FUNCIONES
//Asteroides
void dibujarPantalla();
void inicializarAsteroides(Asteroide*, int); //INICIALIZAR ASTEROIDES
void moverAsteroides(Asteroide*, int); //MOVER ASTEROIDES USANDO UN VECTOR Y UN NUMERO DE ASTEROIDES A MOVER
void dividirAsteroides(Asteroide*, int); //DIVIDIR ASTEROIDES USANDO UN VECROR Y EL NUMERO DE DISPAROS, SE LLAMA CUANDO SE USA LA FUNCIÓN DE DETECTAR COLISIONES
void mostrarExplosion(int x, int y);

//Bala y colisiones
void inicializarBalas(Bala*);
void dispararBala(Bala*, float, float);
void moverBalas(Bala*);
void dibujarBalas(Bala*);
void detectarColisiones(Bala*, Asteroide*, int);

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
	ALLEGRO_TIMER* FPS = al_create_timer(1.0/60); //timer de fps 

	ALLEGRO_EVENT evento; //varable de evento
	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue(); //manejar cola de eventos
	ALLEGRO_KEYBOARD_STATE keyState; //para manjejar el estado del teclado

	ALLEGRO_BITMAP* naveImg = al_load_bitmap("assets/img/nave.png"); //imagen de la nave
	ALLEGRO_BITMAP* asteroideImg = al_load_bitmap("assets/img/asteroide.png");
	if (!asteroideImg) {
		al_show_native_message_box(NULL, "Error", "No se pudo cargar la imagen del asteroide", NULL, NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}
	//ALLEGRO_BITMAP* ovni_1 = al_load_bitmap("ovni_1.png"); //imagen del ovni tipo 1
	//ALLEGRO_BITMAP* ovni_2 = al_load_bitmap("ovni_2.png"); //imagen del ovni tipo 2



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
	int numeroAsteroides = 8; //minimo 4 asteroides
	Asteroide* asteroides = new Asteroide[numeroAsteroides];
	Bala balas[MAX_BALAS];
	Nave nave;

	inicializarAsteroides(asteroides, numeroAsteroides);
	nave.x = ANCHO_PANTALLA / 2;
	nave.y = ALTO_PANTALLA - 60;

	inicializarBalas(balas);


	//CICLO DE LA EJECUCION DEL PROGRAMA
	while (running) {

		al_wait_for_event(queue, &evento); //esperamos al evento
		al_get_keyboard_state(&keyState);

		if (evento.type == ALLEGRO_EVENT_TIMER) {
			if (evento.timer.source == segundoTimer) {
				segundos++;
			}

			//if(asteroides == 0)

			if (evento.timer.source == FPS) {

				// Mover asteroides
				moverAsteroides(asteroides, numeroAsteroides);
				moverBalas(balas);
				detectarColisiones(balas, asteroides, numeroAsteroides);
				al_clear_to_color(negro);

				// Dibuja la nave
				al_draw_scaled_bitmap(naveImg, 0, 0,
					al_get_bitmap_width(naveImg),
					al_get_bitmap_height(naveImg),
					nave.x, nave.y,
					30, 30, 0); // dibuja la nave a 50x50 px

				// Dibuja las balas
				dibujarBalas(balas);


				//Dibujar tamañps de asteroides
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
						else if(asteroides[i].tamano == 1) { // Pequeño
							nuevoAncho = 25;
							nuevoAlto = 25;
						}

						// Dibujar asteroide con el tamaño ajustado
						al_draw_scaled_bitmap(
							asteroideImg,
							0, 0, al_get_bitmap_width(asteroideImg), al_get_bitmap_height(asteroideImg), // Usar toda la imagen
							asteroides[i].x, asteroides[i].y,  // Posición en la pantalla
							nuevoAncho, nuevoAlto,             // Nuevo tamaño basado en el tipo de asteroide
							0); // Sin rotación

						//al_draw_filled_circle(asteroides[i].x + 25, asteroides[i].y + 25, 10, al_map_rgb(255, 0, 0)); // Rojo para visualización
					}

						
				
				}

				al_flip_display();


			}//FPS
		}//Timer
		
		if (al_key_down(&keyState, ALLEGRO_KEY_LEFT) && nave.x > 0) {
			nave.x -= 5;
		}

		if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT) && nave.x < ANCHO_PANTALLA - 50) {
			nave.x += 5;
		}

		if (al_key_down(&keyState, ALLEGRO_KEY_SPACE)) {
			dispararBala(balas, nave.x, nave.y);
		}

		//al_rest(1000); //pausa por n segundos
	}//while

	//DESTRUCCION DE ELEMENTOS
	delete[] asteroides;
	al_destroy_display(ventana); //destruir la ventana tras terminar la ejecucion
	al_destroy_bitmap(naveImg);
	al_destroy_bitmap(asteroideImg);
	al_destroy_font(roboto);
	al_destroy_timer(segundoTimer);
	al_destroy_timer(FPS);
	al_destroy_event_queue(queue);

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
void inicializarAsteroide(Asteroide asteroide) {
	asteroide.x = rand() % (ANCHO_PANTALLA); // Posición X entre 0 y 799
	asteroide.y = rand() % (ALTO_PANTALLA); // Posición Y entre 0 y 599

	asteroide.velocidadX = (rand() % 5) - 2; // Velocidad X entre -2 y 2
	asteroide.velocidadY = (rand() % 5) - 2; // Velocidad Y entre -2 y 2

	asteroide.estado = true; //todos los asteroides están activos inicialmente
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

//
void mostrarExplosion(int x, int y) {

}

// Inicializar balas
void inicializarBalas(Bala *balas) {
	for (int i = 0; i < MAX_BALAS; i++) {
		balas[i].activa = false;
	}
}

// Disparar una bala
void dispararBala(Bala* balas, float x, float y) {
	for (int i = 0; i < MAX_BALAS; i++) {
		if (!balas[i].activa) {
			balas[i].x = x;
			balas[i].y = y;
			balas[i].activa = true;
			break;
		}
	}
}

// Mover balas
void moverBalas(Bala* balas) {
	for (int i = 0; i < MAX_BALAS; i++) {
		if (balas[i].activa) {
			balas[i].y -= 5;
			if (balas[i].y < 0) {
				balas[i].activa = false;
			}
		}
	}
}

// Dibujar balas
void dibujarBalas(Bala* balas) {
	for (int i = 0; i < MAX_BALAS; i++) {
		if (balas[i].activa) {
			al_draw_filled_rectangle(balas[i].x - 2, balas[i].y - 10, balas[i].x + 2, balas[i].y, al_map_rgb(255, 255, 255));
		}
	}
}

// Colisión bala-asteroide
void detectarColisiones(Bala* balas, Asteroide* asteroides, int numAsteroides) {
	for (int i = 0; i < MAX_BALAS; i++) {
		if (balas[i].activa) {
			for (int j = 0; j < numAsteroides; j++) {
				if (asteroides[j].estado) {
					float dx = balas[i].x - asteroides[j].x;
					float dy = balas[i].y - asteroides[j].y;
					float distancia = sqrt(dx * dx + dy * dy);

					float radio_bala = 3;
					float radio_asteroide = asteroides[j].tamano * 5;

					if (distancia < (radio_bala + radio_asteroide)) {
						balas[i].activa = false;
						if (asteroides[j].estado) {
							switch (asteroides[j].tamano) {
								case 1:
									asteroides[j].estado = false;
									break;
								case 2:
									mostrarExplosion(asteroides[j].x, asteroides[j].y);
									asteroides[j].tamano = 1;
									asteroides[numAsteroides + 1];
									Asteroide nuevo;
									inicializarAsteroide(nuevo);
									break;
								case 3:
									mostrarExplosion(asteroides[j].x, asteroides[j].y);
									asteroides[j].tamano = 2;
									asteroides[numAsteroides + 2];
									Asteroide nuevo1, nuevo2;
									inicializarAsteroide(nuevo1);
									inicializarAsteroide(nuevo2);
									break;
							}
						}

						break;
					}
				}
			}
			balas[i].activa = false;
		}//if bala activa
		else {

		}
	}
}
