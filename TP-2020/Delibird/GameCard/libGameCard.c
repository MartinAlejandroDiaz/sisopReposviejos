#include <stdio.h>
#include <stdlib.h>
#include "libGameCard.h"


void creacion_logger(){

	char* path = "gameCard.log";
	char* nombrePrograma = "GameCard";

	logger = log_create(path , nombrePrograma, 0, LOG_LEVEL_INFO);
	log_info(logger, "El Logger del GameaCard Fue Creado");

}


t_config_gameCard* read_and_log_config(){

	t_config* archivo_Config = config_create("gameCard.config");

	if(archivo_Config == NULL){
		log_error(logger, "No se encuentra la direccion del Archivo de Configuracion");
		exit(1);
	}

	datosConfigGameCard = malloc(sizeof(t_config_gameCard));

	datosConfigGameCard->ipBroker = config_get_string_value(archivo_Config, "IP_BROKER");
	datosConfigGameCard->puertoBroker = config_get_string_value(archivo_Config, "PUERTO_BROKER");
	datosConfigGameCard->ipGameCard = config_get_string_value(archivo_Config, "IP_GAMECARD");
	datosConfigGameCard->puertoGameCard = config_get_string_value(archivo_Config, "PUERTO_GAMECARD");
	datosConfigGameCard->tiempo_de_reintento_reconexion = config_get_int_value(archivo_Config, "TIEMPO_DE_REINTENTO_CONEXION");
	datosConfigGameCard->tiempo_de_reintento_operacion= config_get_int_value(archivo_Config, "TIEMPO_DE_REINTENTO_OPERACION");
	datosConfigGameCard->tiempo_retardo_operacion = config_get_int_value(archivo_Config, "TIEMPO_RETARDO_OPERACION");
	datosConfigGameCard->punto_montaje_glass= config_get_string_value(archivo_Config, "PUNTO_MONTAJE_TALLGRASS");
	datosConfigGameCard->configuracion = archivo_Config;

	return datosConfigGameCard;
}

void crearPuntoDeMontajeGC(){
	char* path = string_new();
	string_append(&path, datosConfigGameCard->punto_montaje_glass);
	mkdir(path, 0700);
	char* copiaDireccion = string_duplicate(datosConfigGameCard->punto_montaje_glass);
	log_info(logger, "Se Creo El Punto De Montaje %s", copiaDireccion);
	free(copiaDireccion);
	free(path);
}

void establecerLaMetadataDeGC() {

	struct stat *buf = malloc(sizeof(struct stat));
	char* meta = "Metadata";
	char* metabin = "Metadata.bin";
	char* path = string_new();
	string_append(&path, datosConfigGameCard->punto_montaje_glass);
	string_append_with_format(&path, "/%s", meta);

	if (stat(path, buf) != -1) {
		log_warning(logger, "El Directorio %s Ya Existe", meta);
		free(buf);
		free(path);
		return;
	}

	mkdir(path, 0700);
	free(buf);
	log_info(logger, "Se Creo La Carpeta %s", meta);

	string_append_with_format(&path, "/%s", metabin);
	FILE* punteroMetadata = fopen(path, "w+");

	log_info(logger, "Se Creo El Archivo %s", metabin);

	fprintf(punteroMetadata, "BLOCK_SIZE=64\n");
	fprintf(punteroMetadata, "BLOCKS=1024\n");
	fprintf(punteroMetadata, "MAGIC_NUMBER=TALL_GRASS\n");

	log_info(logger, "Se Escribieron Los Datos Del Metadata");
	fclose(punteroMetadata);
	free(path);
}

void setearMetadataBloques(){

	char* path = string_new();
	char* metadata = "Metadata";
	char* metaBin = "Metadata.bin";

	string_append(&path, datosConfigGameCard->punto_montaje_glass);
	string_append_with_format(&path, "/%s/%s", metadata, metaBin);

	t_config* configMeta = config_create(path);

	int tamanioBloques = config_get_int_value(configMeta, "BLOCK_SIZE");
	int cantidadBloques = config_get_int_value(configMeta, "BLOCKS");
	char* magicNumber = config_get_string_value(configMeta, "MAGIC_NUMBER");

	datosFileSystem = malloc(sizeof(t_data_GC));
	datosFileSystem->tamanio_bloques = (size_t) tamanioBloques;
	datosFileSystem->cantidad_bloques = cantidadBloques;
	datosFileSystem->magic_number = magicNumber;

	config_destroy(configMeta);
	free(path);
}

void crearArchivosBinTotalesFS() {

	char* pathBloques = string_new();
	char* dirBloques = "Bloques";

	string_append(&pathBloques, datosConfigGameCard->punto_montaje_glass);
	string_append_with_format(&pathBloques, "/%s", dirBloques);

	mkdir(pathBloques, 0700);

	FILE* bloques[datosFileSystem->cantidad_bloques];

	for (int index = 1; index < (datosFileSystem->cantidad_bloques +1); index++) {
		char* otroPath = string_new();
		string_append(&otroPath, datosConfigGameCard->punto_montaje_glass);
		string_append_with_format(&otroPath, "/%s", dirBloques);
		string_append_with_format(&otroPath, "/%d.bin", index);
		bloques[index] = fopen(otroPath, "w+");
		fclose(bloques[index]);
		free(otroPath);
	}

	free(pathBloques);
}

void crearBitMapGC(){

	char* meta = "Metadata";
	char* bitmap = "Bitmap.bin";
	char* path = string_new();
	int cantidadBloques = datosFileSystem->cantidad_bloques / 8;

	string_append(&path, datosConfigGameCard->punto_montaje_glass);
	string_append_with_format(&path, "/%s", meta);
	string_append_with_format(&path, "/%s", bitmap);

	int fdbitmap = open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	ftruncate(fdbitmap, cantidadBloques);

	char* bmap = mmap(0, cantidadBloques, PROT_WRITE | PROT_READ, MAP_SHARED, fdbitmap,
			0);

	if (bmap == MAP_FAILED) {
		log_error(logger, "Error al mapear a memoria: %s\n", strerror(errno));
		close(fdbitmap);
		free(path);
		free(bmap);
		return;
	}

	bitarray = bitarray_create_with_mode(bmap, cantidadBloques, MSB_FIRST);
	msync(bitarray, sizeof(bitarray), MS_SYNC);

	free(path);
}

void liberarMapeoEnMemoria(){
		if (munmap(bitarray->bitarray, (datosFileSystem->cantidad_bloques)/8) == -1) {
			log_error(logger, "Error Un-Mapping The File");
		}
		free(bitarray);
}

void liberarDatosFileSystem() {
	//free(datosFileSystem->magic_number);
	free(datosFileSystem);
}


void crearDirectorioParaPokemones(){

	char* files = "Files";
	char* pokemon = "Pokemon";
	char* metadata = "Metadata.bin";

	char* path = string_new();
	string_append(&path, datosConfigGameCard->punto_montaje_glass);
	string_append_with_format(&path, "/%s", files);

	if(mkdir(path, 0700) == -1){
		log_info(logger, "El Directorio %s ya existe", files);
		free(path);
		return;
	}

	string_append_with_format(&path, "/%s", pokemon);
	mkdir(path, 0700);
	log_info(logger, "Se Creo el Directorio %s", pokemon);

	string_append_with_format(&path, "/%s", metadata);

	FILE* punteroArchivo = fopen(path, "w+");
	log_info(logger, "Se creo el %s", metadata);

	fprintf(punteroArchivo, "DIRECTORY=Y");

	fclose(punteroArchivo);
	free(path);
}

int buscarBloqueLibre() {

	int posicionLibre;

	for (int i = 1; i < bitarray_get_max_bit(bitarray)+1 ; i++) {

		pthread_mutex_lock(&m_bitarray);
		if (bitarray_test_bit(bitarray, i) == 0) {
			posicionLibre = i;
			bitarray_set_bit(bitarray, i);
			pthread_mutex_unlock(&m_bitarray);
			return posicionLibre;
		}
		pthread_mutex_unlock(&m_bitarray);
	}
	return -1;
}

// Funcion Para Cambiar el OPEN Y -> N
void cerrarArchivo(char* path, char* pokemon){
	t_config* c = config_create(path);
	config_set_value(c, "OPEN", "N");
	t_exclusion_pokemon* pokemonMutex = obtenerMutexDelPokemon(pokemon);
	pthread_mutex_lock(&pokemonMutex->mutexPokemon);
	config_save(c);
	pthread_mutex_unlock(&pokemonMutex->mutexPokemon);
	config_destroy(c);
}

void agregarNuevaAparicionPokemon(t_msg_NP* pokemonNuevo){

	char* metadata = "Metadata.bin";
	struct stat *buf = malloc(sizeof(struct stat));

	char* path = retornarDireccionPokemon(pokemonNuevo->nombrePokemon);

	// Este Mutex tiene como idea no permitir si llegase la situacion de que 2 Hilos busquen y traten
	// de crear el Directorio del Mismo Pokemon y su Metadata
	pthread_mutex_lock(&m_crearPokemon);
	if(stat(path, buf) == -1){
		log_info(logger, "No existe el Directorio %s", pokemonNuevo->nombrePokemon);
		crearDirectorioYMetadataPokemonNuevo(path, pokemonNuevo->nombrePokemon);
	}
	pthread_mutex_unlock(&m_crearPokemon);

	string_append_with_format(&path, "/%s", metadata);

	// Hasta que no pueda Acceder al Archivo se Quedara dentro del LOOP Reintentando
	while (true) {

		if (puedeAccederAlArchivo(path, pokemonNuevo->nombrePokemon)) {
			agregarPosicionPokemon(pokemonNuevo);
			cerrarArchivo(path, pokemonNuevo->nombrePokemon);
			sleep(datosConfigGameCard->tiempo_retardo_operacion);
			break;
		}
		sleep(datosConfigGameCard->tiempo_de_reintento_operacion);
	}

	free(path);
	free(buf);
}

bool capturarPokemon(t_msg_1* pokemonACapturar){

	char* metadata = "Metadata.bin";
	char* path = retornarDireccionPokemon(pokemonACapturar->pokemon);
	struct stat *buf = malloc(sizeof(struct stat));
	bool accionRealizada = true;

	pthread_mutex_lock(&m_crearPokemon);
	if(stat(path, buf) == -1){
		log_info(logger, "No existe el Directorio %s", pokemonACapturar->pokemon);
		accionRealizada = false;
		free(path);
		free(buf);
		return accionRealizada;
	}
	pthread_mutex_unlock(&m_crearPokemon);

	string_append_with_format(&path, "/%s", metadata);

	while (true) {

		if (puedeAccederAlArchivo(path, pokemonACapturar->pokemon)) {
			if (!reducirCantidadPosicionPokemon(pokemonACapturar)) {
				log_error(logger, "La Posicion = (%d,%d) No Existe",
						pokemonACapturar->posicionX,
						pokemonACapturar->posicionY);
				accionRealizada = false;
			}
			cerrarArchivo(path, pokemonACapturar->pokemon);
			sleep(datosConfigGameCard->tiempo_retardo_operacion);
			break;
		}
		sleep(datosConfigGameCard->tiempo_de_reintento_operacion);
	}

	free(path);
	free(buf);

	return accionRealizada;
}

// Retornar Una LISTA DE POSICIONES
t_list* obtenerTodasLasPosicionesDelPokemon(t_msg_3* getPokemon){

	char* metadata = "Metadata.bin";
	char* path = retornarDireccionPokemon(getPokemon->pokemon);
	struct stat *buf = malloc(sizeof(struct stat));
	t_list* listaPosiciones;

	pthread_mutex_lock(&m_crearPokemon);
	if(stat(path, buf) == -1){
		log_info(logger, "No existe el Directorio %s", getPokemon->pokemon);
		pthread_mutex_unlock(&m_crearPokemon);
		listaPosiciones = list_create();
		free(path);
		free(buf);
		return listaPosiciones;
	}
	pthread_mutex_unlock(&m_crearPokemon);

	string_append_with_format(&path, "/%s", metadata);

	while (true) {

		if (puedeAccederAlArchivo(path, getPokemon->pokemon)) {
			listaPosiciones = obtenerListaDePosiciones(getPokemon->pokemon);
			sleep(datosConfigGameCard->tiempo_retardo_operacion);
			cerrarArchivo(path, getPokemon->pokemon);
			break;
		}

		sleep(datosConfigGameCard->tiempo_de_reintento_operacion);
	}

	free(path);
	free(buf);

	return listaPosiciones;
}

t_list* obtenerListaDePosiciones(char* pokemon){

	char* path = retornarMetadataPokemon(pokemon);
	t_list* listaPosiciones = list_create();

	t_config* configuracion = config_create(path);
	char** bloquesParticion = config_get_array_value(configuracion, "BLOCKS");
	config_destroy(configuracion);

	char* pathTemporal = escribirBloquesEnArchivoTemporal(pokemon, bloquesParticion);

	char* lineaLeida = NULL;
	size_t bufferSize = 0;

	FILE* punteroArchivo = fopen(pathTemporal, "r+");

	while(getline(&lineaLeida, &bufferSize, punteroArchivo) != -1){

		char* posicionesEnCadena = strtok(lineaLeida, "=");
		char* posicionX = strtok(posicionesEnCadena, "-");
		char* posicionY = strtok(NULL,"");

		t_posiciones* posiciones = malloc(sizeof(t_posiciones));
		posiciones->posicionX = (uint32_t) atoi(posicionX);
		posiciones->posicionY = (uint32_t) atoi(posicionY);
		list_add(listaPosiciones, posiciones);
	}

	fclose(punteroArchivo);
	remove(pathTemporal);
	free(pathTemporal);
	free(lineaLeida);
	free(path);
	string_iterate_lines(bloquesParticion, (void*) free);
	free(bloquesParticion);

	return listaPosiciones;
}

void crearDirectorioYMetadataPokemonNuevo(char* pathPokemon, char* nombrePokemon){

	char* metadata = "Metadata.bin";
	char* pathDuplicado = string_duplicate(pathPokemon);

	mkdir(pathDuplicado, 0700);
	log_info(logger, "Se creo el Directorio %s", nombrePokemon);

	string_append_with_format(&pathDuplicado, "/%s", metadata);

	FILE* punteroPokemon = fopen(pathDuplicado, "w+");
	int bloqueLibre = buscarBloqueLibre();

	fprintf(punteroPokemon, "DIRECTORY=N\n");
	fprintf(punteroPokemon, "SIZE=0\n");
	fprintf(punteroPokemon, "BLOCKS=[%d]\n", bloqueLibre);
	fprintf(punteroPokemon, "OPEN=N\n");

	fclose(punteroPokemon);
	free(pathDuplicado);

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	t_exclusion_pokemon* exclusion = malloc(sizeof(t_exclusion_pokemon));
	exclusion->nombrePokemon = string_duplicate(nombrePokemon);
	exclusion->mutexPokemon = mutex;
	list_add(listaPokemonMutex, exclusion);
}

t_exclusion_pokemon* obtenerMutexDelPokemon(char* nombrePokemon){

	bool coincideElNombre(t_exclusion_pokemon* pokemon){
		return string_contains(pokemon->nombrePokemon, nombrePokemon);
	}

	return list_find(listaPokemonMutex, (void*) coincideElNombre);
}

bool puedeAccederAlArchivo(char* path, char* pokemon){

	t_exclusion_pokemon* pokemonMutex = obtenerMutexDelPokemon(pokemon);

	pthread_mutex_lock(&pokemonMutex->mutexPokemon);
	t_config* config = config_create(path);
	char* respuesta = config_get_string_value(config, "OPEN");

	if(string_contains(respuesta, "N")){
		config_set_value(config, "OPEN", "Y");
		config_save(config);
		config_destroy(config);
		pthread_mutex_unlock(&pokemonMutex->mutexPokemon);
		return true;
	}
	config_destroy(config);
	pthread_mutex_unlock(&pokemonMutex->mutexPokemon);

	return false;
}

void agregarPosicionPokemon(t_msg_NP* pokemonNuevo) {

	char* path = retornarMetadataPokemon(pokemonNuevo->nombrePokemon);

	t_config* configuracion = config_create(path);
	int tamanioArchivo = config_get_int_value(configuracion, "SIZE");
	char** bloquesParticiones = config_get_array_value(configuracion, "BLOCKS");

	char* otroPath = escribirBloquesEnArchivoTemporal(
			pokemonNuevo->nombrePokemon, bloquesParticiones);
	int cantidadElementos = retornarCantidadDeBlocksEnParticion(
			bloquesParticiones);

	actualizarOAgregarKey(otroPath, pokemonNuevo);

	FILE* punteroArchivo = fopen(otroPath, "r");

	fseek(punteroArchivo, 0, SEEK_END);
	int tamanioTotalArchivo = ftell(punteroArchivo);
	int copiaTamanio = tamanioTotalArchivo;
	fseek(punteroArchivo, 0, SEEK_SET);

	char* nuevaCantidadBloques;

	if (tamanioTotalArchivo
			> (datosFileSystem->tamanio_bloques * cantidadElementos)) {
		int bloqueLibre = buscarBloqueLibre();
		nuevaCantidadBloques = retornarNuevaListaBloques(bloquesParticiones,
				cantidadElementos, bloqueLibre);
		string_iterate_lines(bloquesParticiones, (void*) free);
		free(bloquesParticiones);
		bloquesParticiones = string_get_string_as_array(nuevaCantidadBloques);
	}

	else if (tamanioTotalArchivo
			< (datosFileSystem->tamanio_bloques * cantidadElementos)) {

		if (tamanioTotalArchivo
				< (datosFileSystem->tamanio_bloques * (cantidadElementos - 1))) {
			liberarBloqueParticion(bloquesParticiones[cantidadElementos - 1]);
			bloquesParticiones[cantidadElementos - 1] = NULL;
		}
		nuevaCantidadBloques = retornarNuevaListaBloques(bloquesParticiones,
				cantidadElementos, -1);
		string_iterate_lines(bloquesParticiones, (void*) free);
		free(bloquesParticiones);
		bloquesParticiones = string_get_string_as_array(nuevaCantidadBloques);
	}

	int nuevaCantidadElementos = retornarCantidadDeBlocksEnParticion(
			bloquesParticiones);

	reescribirDentroDeLosBloquesAsignados(tamanioTotalArchivo,
			nuevaCantidadElementos, bloquesParticiones, punteroArchivo);

	fclose(punteroArchivo);
	remove(otroPath);
	free(otroPath);

	char* copia = string_itoa(copiaTamanio);
	config_set_value(configuracion, "SIZE", copia);
	config_set_value(configuracion, "BLOCKS", nuevaCantidadBloques);

	t_exclusion_pokemon* pokemonMutex = obtenerMutexDelPokemon(pokemonNuevo->nombrePokemon);
	pthread_mutex_lock(&pokemonMutex->mutexPokemon);
	config_save(configuracion);
	pthread_mutex_unlock(&pokemonMutex->mutexPokemon);
	config_destroy(configuracion);

	free(nuevaCantidadBloques);
	string_iterate_lines(bloquesParticiones, (void*) free);
	free(bloquesParticiones);
	free(copia);
	free(path);
}

char* escribirBloquesEnArchivoTemporal(char* nombrePokemon, char** bloques) {

	char* path = retornarDireccionPokemon(nombrePokemon);
	string_append_with_format(&path, "/%s.temp", nombrePokemon);

	char c;
	FILE* filePokemon = fopen(path, "w+");

	int cantidadElementos = retornarCantidadDeBlocksEnParticion(bloques);

	for (int i = 0; i < cantidadElementos; i++) {

		char* pathBloque = retornarDireccionBloque(bloques[i]);
		FILE* punteroBloque = fopen(pathBloque, "r");

		while ((c = fgetc(punteroBloque)) != EOF) {
			fputc(c, filePokemon);
		}

		fclose(punteroBloque);
		free(pathBloque);
	}
	fclose(filePokemon);

	return path;
}

void reescribirDentroDeLosBloquesAsignados(int tamanioArchivo, int cantidadElementos, char** bloquesParticiones, FILE* punteroArchivo){

	char c;
	int tamanioValido = datosFileSystem->tamanio_bloques;

		for (int j = 0; j < cantidadElementos; j++) {

			int contador = 0;

			char* pathBloque = retornarDireccionBloque(bloquesParticiones[j]);
			FILE* punteroBloque = fopen(pathBloque, "w");

			if ((tamanioArchivo -= tamanioValido) < 0) {
				tamanioValido = tamanioArchivo + tamanioValido;
			}

			while (((c = fgetc(punteroArchivo)) != EOF) && (contador != tamanioValido)) {
				fputc(c, punteroBloque);
				contador++;
			}

			if (c != EOF) {
				fseek(punteroArchivo, -1, SEEK_CUR);
			}

			fclose(punteroBloque);
			free(pathBloque);
		}
}

void actualizarOAgregarKey(char* path, t_msg_NP* pokemonNuevo) {

	char* key = string_new();
	string_append_with_format(&key, "%d-%d", pokemonNuevo->posicionX,
			pokemonNuevo->posicionY);

	// Si EXISTE KEY actualizo, sino AGREGO
	t_config* config = config_create(path);
	char* cantidad = config_get_string_value(config, key);

	if (cantidad != NULL) {
		cantidad = string_itoa((atoi(cantidad) + pokemonNuevo->cantidad));
		config_set_value(config, key, cantidad);
	}

	else {
		cantidad = string_itoa(pokemonNuevo->cantidad);
		config_set_value(config, key, cantidad);
	}

	free(cantidad);
	free(key);
	config_save(config);
	config_destroy(config);

}

bool reducirCantidadPosicionPokemon(t_msg_1* pokemonACapturar){

	char* path = retornarMetadataPokemon(pokemonACapturar->pokemon);
	bool existePosicion = false;

	char* key = string_new();
	string_append_with_format(&key, "%d-%d", pokemonACapturar->posicionX, pokemonACapturar->posicionY);

	t_config* configuracion = config_create(path);
	int tamanioArchivo = config_get_int_value(configuracion, "SIZE");
	char** bloquesParticiones = config_get_array_value(configuracion, "BLOCKS");

	int cantidadElementos = retornarCantidadDeBlocksEnParticion(bloquesParticiones);

	char* pathTemporal = escribirBloquesEnArchivoTemporal(pokemonACapturar->pokemon, bloquesParticiones);

	t_config* configPok = config_create(pathTemporal);

	char* cantidad = config_get_string_value(configPok, key);

	if(cantidad != NULL){

		int valorCantidad = atoi(cantidad);
		valorCantidad--;

		if(valorCantidad == 0){
			config_remove_key(configPok, key);
		}

		else{
			char* valorEnString = string_itoa(valorCantidad);
			config_set_value(configPok, key, valorEnString);
			free(valorEnString);
		}

		existePosicion = true;
		config_save(configPok);
	}

	config_destroy(configPok);

	FILE* punteroArchivo = fopen(pathTemporal, "r");

	fseek(punteroArchivo, 0, SEEK_END);
	int tamanioTotalArchivo = ftell(punteroArchivo);
	int copiaTamanio = tamanioTotalArchivo;
	fseek(punteroArchivo, 0, SEEK_SET);

	char* nuevaCantidadBloques;

	if (tamanioTotalArchivo
			> (datosFileSystem->tamanio_bloques * cantidadElementos)) {
		int bloqueLibre = buscarBloqueLibre();
		nuevaCantidadBloques = retornarNuevaListaBloques(bloquesParticiones,
				cantidadElementos, bloqueLibre);
		string_iterate_lines(bloquesParticiones, (void*) free);
		free(bloquesParticiones);
		bloquesParticiones = string_get_string_as_array(nuevaCantidadBloques);
	}

	else if (tamanioTotalArchivo
			< (datosFileSystem->tamanio_bloques * cantidadElementos)) {

		if (tamanioTotalArchivo
				< (datosFileSystem->tamanio_bloques * (cantidadElementos - 1))) {
			liberarBloqueParticion(bloquesParticiones[cantidadElementos - 1]);
			cantidadElementos--;
		}
		nuevaCantidadBloques = retornarNuevaListaBloques(bloquesParticiones,
				cantidadElementos, -1);
		string_iterate_lines(bloquesParticiones, (void*) free);
		free(bloquesParticiones);
		bloquesParticiones = string_get_string_as_array(nuevaCantidadBloques);
	}

	int nuevaCantidadElementos = retornarCantidadDeBlocksEnParticion(
			bloquesParticiones);

	reescribirDentroDeLosBloquesAsignados(tamanioTotalArchivo,
			nuevaCantidadElementos, bloquesParticiones, punteroArchivo);

	fclose(punteroArchivo);
	remove(pathTemporal);
	free(pathTemporal);

	char* tamanioEnString = string_itoa(copiaTamanio);
	config_set_value(configuracion, "SIZE", tamanioEnString);
	config_set_value(configuracion, "BLOCKS", nuevaCantidadBloques);

	t_exclusion_pokemon* pokemonMutex = obtenerMutexDelPokemon(pokemonACapturar->pokemon);
	pthread_mutex_lock(&pokemonMutex->mutexPokemon);
	config_save(configuracion);
	pthread_mutex_unlock(&pokemonMutex->mutexPokemon);
	config_destroy(configuracion);

	string_iterate_lines(bloquesParticiones, (void*) free);
	free(nuevaCantidadBloques);
	free(tamanioEnString);
	free(bloquesParticiones);
	free(key);
	free(path);
	return existePosicion;
}

int retornarCantidadDeBlocksEnParticion(char** listaBloques){
	int cantidadElementos = 0;
	while(listaBloques[cantidadElementos] != NULL) { cantidadElementos++; }
	return cantidadElementos;
}

char* retornarMetadataPokemon(char* nombrePokemon){
	char* path = string_new();
	string_append(&path, datosConfigGameCard->punto_montaje_glass);
	string_append_with_format(&path, "/Files/Pokemon/%s/Metadata.bin", nombrePokemon);
	return path;
}

char* retornarDireccionPokemon(char* nombrePokemon){

	char* files = "Files";
	char* pokemon = "Pokemon";

	char* path = string_new();
	string_append(&path, datosConfigGameCard->punto_montaje_glass);
	string_append_with_format(&path, "/%s/%s/%s", files, pokemon, nombrePokemon);

	return path;
}

char* retornarDireccionBloque(char* nroBloque){

	char* path = string_new();
	string_append(&path, datosConfigGameCard->punto_montaje_glass);
	string_append_with_format(&path, "/Bloques/%s.bin", nroBloque);
	return path;
}

void actualizarCantidadPokemon(t_msg_NP* pokemon, int cantidad, t_config* configuracion, char* key){

		char* cantidadNueva = string_itoa((pokemon->cantidad + cantidad));
		config_set_value(configuracion, key, cantidadNueva);
		config_save(configuracion);

		char* registroGuardado = string_new();
		string_append_with_format(&registroGuardado,"%d-%d=%d\n", pokemon->posicionX, pokemon->posicionY, cantidad);
		int largoRegistroGuardado = string_length(registroGuardado);

		char* registroNuevo = string_new();
		string_append_with_format(&registroNuevo,"%d-%d=%d\n", pokemon->posicionX, pokemon->posicionY, atoi(cantidadNueva));
		int largoRegistroNuevo = string_length(registroNuevo);

		char* path = string_new();
		string_append(&path, datosConfigGameCard->punto_montaje_glass);
		string_append_with_format(&path, "/Files/Pokemon/%s/Metadata.bin", pokemon->nombrePokemon);

		t_config* c = config_create(path);
		int sizeArchivo = config_get_int_value(c, "SIZE");
		sizeArchivo = (sizeArchivo - largoRegistroGuardado) + largoRegistroNuevo;
		char* size = string_itoa(sizeArchivo);
		config_set_value(c, "SIZE", size);
		config_save(c);
		log_info(logger, "Se Actualizo La Cantidad de la Posicion %d-%d de %s", pokemon->posicionX, pokemon->posicionY, pokemon->nombrePokemon);
		config_destroy(c);

		free(size);
		free(cantidadNueva);
		free(registroGuardado);
		free(registroNuevo);
		free(path);
}

char* retornarDireccionBloqueAEscribir(int cantidadElementos, char** bloquesParticiones) {

	char* pathBloqueActual;

	for (int indice = 0; indice < cantidadElementos; indice++) {

		char* path = string_new();
		string_append(&path, datosConfigGameCard->punto_montaje_glass);
		string_append_with_format(&path, "/Bloques/%s.bin",
				bloquesParticiones[indice]);

		FILE* punteroArchivo = fopen(path, "r+");
		fseek(punteroArchivo, 0, SEEK_END);
		int tamanioFinal = ftell(punteroArchivo);

		if (tamanioFinal != datosFileSystem->tamanio_bloques) {
			fclose(punteroArchivo);
			return path;
		}

		fclose(punteroArchivo);
		free(path);
	}

	return NULL;
}

char* retornarNuevaListaBloques(char** bloquesParticiones,
		int cantidadElementos, int posicionBloque) {

	char* bloques = string_new();

	string_append(&bloques, "[");

	for (int i = 0; i < cantidadElementos; i++) {

		if (i == 0) {
			string_append_with_format(&bloques, "%s", bloquesParticiones[i]);
		} else {
			string_append_with_format(&bloques, ",%s", bloquesParticiones[i]);
		}

	}

	if (posicionBloque != -1) {
		string_append_with_format(&bloques, ",%d", posicionBloque);
	}
	string_append(&bloques, "]");
	return bloques;
}

void liberarBloqueParticion(char* bloqueALiberar){

	int bloqueEntero = atoi(bloqueALiberar);

	pthread_mutex_lock(&m_bitarray);
	bitarray_clean_bit(bitarray, bloqueEntero);
	pthread_mutex_unlock(&m_bitarray);

	recrearBloqueVacio(bloqueALiberar);
}

void recrearBloqueVacio(char* bloque) {
	char* path = retornarDireccionBloque(bloque);
	FILE* archivo = fopen(path, "w");
	fclose(archivo);
	free(path);
}

void finalizarPrograma(){

	necesitaContinuar = false;

	pthread_mutex_destroy(&m_bitarray);
	pthread_mutex_destroy(&m_crearPokemon);

	liberarMapeoEnMemoria();
	liberarDatosFileSystem();

	list_clean_and_destroy_elements(listaPokemonMutex, (void*) eliminarMutexPokemon);
	list_destroy(listaPokemonMutex);

	for(int i=0; i < 3; i++){
		sem_wait(&salidaSubscripcion);
	}

	config_destroy(datosConfigGameCard->configuracion);
	free(datosConfigGameCard);
	log_destroy(logger);

	exit(1);
}

void eliminarMutexPokemon(t_exclusion_pokemon* pokemonMutex){

	free(pokemonMutex->nombrePokemon);
	pthread_mutex_destroy(&pokemonMutex->mutexPokemon);
	free(pokemonMutex);
}
