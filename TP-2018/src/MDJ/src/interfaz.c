#include  "interfaz.h"
#include "libMDJ.h"
// Con esto se maneja cada mensaje que se le mando al dam, si es 0 es porque es false, si es 1 es porque es true
t_metadata_filemetadata * metadata;


char * obtenerPtoMontaje()
{
t_config* fileConfig  = config_create("MDJ.config");
char * ptoMontaje = string_new();
string_append(&ptoMontaje,config_get_string_value(fileConfig,"PTO_MONTAJE"));
config_destroy(fileConfig);
if( strcmp(ptoMontaje,"") == 0){
log_error(logger,"No se puedo obtener el pto de montaje");
}
return ptoMontaje;
}

//falta probar y hacer algunos free de los malloc.

t_metadata_filesystem *  obtenerMetadata() {
t_metadata_filesystem * fs = malloc(sizeof(t_metadata_filesystem));
char * motanjeMasBin = string_new();
string_append(&motanjeMasBin,obtenerPtoMontaje());
string_append(&motanjeMasBin,"/Metadata/Metadata.bin");
t_config * metadata = config_create(motanjeMasBin);
fs->tamanio_bloques = config_get_int_value(metadata,"TAMANIO_BLOQUES");
fs->cantidad_bloques = config_get_int_value(metadata,"CANTIDAD_BLOQUES");
char * magic_number = string_new();
string_append(&magic_number,config_get_string_value(metadata,"MAGIC_NUMBER"));
fs->magic_number = magic_number;
if( strcmp(magic_number,"") > 0 && fs->tamanio_bloques != 0 && fs->cantidad_bloques != 0){
log_trace(logger,"Se obtuvo correctamente la metadata y el pto montaje");
}
else
{
log_error(logger,"No se puedo obtener la metadata ni el pto montaje");
}
config_destroy(metadata);
free(magic_number);
return fs;
}



void aplicarRetardo()
{
t_config* fileConfig  = config_create("MDJ.config");
int ret = config_get_int_value(fileConfig,"RETARDO");
usleep(ret);
config_destroy(fileConfig);
}
//args[0]: path, args[1]: socketCPU
void  validarArchivo(socket_connection * connection,char ** args){
char strEstado[2];
t_archivo *  archivo = malloc(sizeof(t_archivo));	
archivo->path = args[0];
archivo->fd =  1;//verificarSiExisteArchivo(archivo->path);
if(archivo->fd == noExiste){
archivo->estado =noExiste;
}
else {
archivo->estado=  existe;
}
sprintf(strEstado,"%i", archivo->estado);
aplicarRetardo();
free(archivo);
runFunction(connection->socket,"MDJ_DAM_existeArchivo",2, strEstado, args[1]);
}

//void* mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

//args[0]: rutaDelArchivo, args[1]: cantidadDeBytes
void crearArchivo(socket_connection * connection ,char** args)
{
t_archivo *  archivo= malloc(sizeof(t_archivo));
t_metadata_bitmap * bitMap = malloc(sizeof(t_metadata_bitmap));	
t_metadata_filesystem * fs = obtenerMetadata();
archivo->path = args[0];
size_t tsize = atoi(args[1]);
t_list * bloquesLibres = list_create();
t_list * bloquesOcupados = list_create();
archivo->size =  (tsize - 1);
size_t tamanioBloques = (fs->tamanio_bloques - 1);
char * pathMasArchivos = string_new();
char * tam = malloc(archivo->size); 
char * bloques = malloc(sizeof(fs->cantidad_bloques));
bitMap->bitarray = crearBitmap(fs->cantidad_bloques);
for(int i = 0; i < fs->cantidad_bloques; i++)
{
if(bitarray_test_bit(bitMap->bitarray,i) == 0)
{
list_add(bloquesLibres,i);    
}
else
{
 list_add(bloquesOcupados,i);  
}
}
t_bloques * bloques = asignarBloques(bloquesLibres,bloquesOcupados,archivo->size);
archivo->fd = verificarSiExisteArchivo(archivo->path);
if(archivo->fd == -1)
{
sprintf(tam,"%i",tsize);
string_append(&tamBloq,"TAMANIO=");
string_append(&tamBloq,tam);
string_append(&pathMasArchivos,obtenerPtoMontaje());
string_append(&pathMasArchivos,"/Archivos/");
string_append(&pathMasArchivos,archivo->path);
archivo-> fd = open(pathMasArchivos,O_RDWR|O_CREAT);
flock(archivo->fd,LOCK_SH);	
char *  nVeces = string_repeat('\n',archivo->size);
lseek(archivo->fd,archivo->size, SEEK_SET);
write(archivo->fd, "",1);
char * file = mmap(0, archivo->size, PROT_READ | PROT_WRITE, MAP_SHARED, archivo->fd, 0);
memcpy(file,nVeces,strlen(nVeces));
memcpy(file,tamBloq,strlen(tamBloq));
msync(file,archivo->size, MS_SYNC);
munmap(file,archivo->size);
close(archivo->fd);
log_trace(logger,"Archivo %s creado correctamente en %s/Archivos",archivo->path,obtenerPtoMontaje());
flock(archivo->fd,LOCK_UN);
archivo->estado = recienCreado;
}
else
{
archivo->estado = noCreado;
}
sprintf(strEstado, "%i", archivo->estado);
aplicarRetardo();
runFunction(connection->socket,"MDJ_DAM_verificarArchivoCreado",2,strEstado,archivo->path);
}
char * destino = string_new();
for(int j = 0;j< fs->cantidad_bloques; j++){ 
sprintf(destino,"%s/Bloques/%d.bin",archivo->path,j);
fdBloques[j] = open(destino,O_RDWR | O_CREAT);
flock(fdBloques[j],LOCK_EX);
lseek(fdBloques[j],tamanioBloques, SEEK_SET);
write(fdBloques[j],"",1);
archivo->bloques[j] = mmap(0,fs->tamanio_bloques,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED,fdBloques[j],0);
memcpy(file,nVeces,strlen(nVeces));
msync(file,archivo->size, MS_SYNC);
munmap(file,archivo->size);
flock(archivo->fd,LOCK_UN);
close(archivo->fd);
}
free(archivo);
free(bitMap);
//list_destroy_and_destroy_elements(bloquesLibres,(void*) free);
}

//off_t lseek(int fildes, off_t offset, int whence);
size_t  obtenerDatos(socket_connection * connection,char ** args){
t_archivo *  archivo= malloc(sizeof(t_archivo));
size_t leidos;
archivo->path = args[0];
off_t  offset = atoi(args[1]);
size_t tsize =  atoi(args[2]);
archivo->size =  tsize;	
archivo->fd = 1;//verificarSiExisteArchivo(archivo->path);
if(archivo->fd == noExiste)
{
archivo->estado = noExiste;	
}
else
{
flock(archivo->fd,LOCK_EX);	
off_t posCorrida = lseek(archivo->fd,offset,SEEK_CUR);
long  tamanioRestante = tsize - posCorrida;
char * bufferDeBytes = malloc(tamanioRestante);	
leidos = read(archivo->fd,bufferDeBytes,posCorrida);
flock(archivo->fd,LOCK_UN);
}
return leidos;
}

void guardarDatos(socket_connection * connection ,char * path,off_t  * offset,size_t *  size,char * buffer){
}

void borrarArchivo(socket_connection* connection,char ** args){
char strEstado[2];
t_archivo * archivo= malloc(sizeof(t_archivo));
archivo->path = args[0];
archivo->fd  = 1; //verificarSiExisteArchivo(archivo->path);
if (archivo->fd  == existe)
{
archivo->estado = recienBorrado;
}
else
{
archivo->estado = noBorrado;
}
sprintf(strEstado, "%i", archivo->estado);
aplicarRetardo();
runFunction(connection->socket,"MDJ_DAM_verificameSiArchivoFueBorrado",1,strEstado);
}

t_bitarray * crearBitmap(int  size){
char * montajeMasBitmap = string_new();
string_append(&montajeMasBitmap,obtenerPtoMontaje());
string_append(&montajeMasBitmap,"/Metadata/Bitmap.bin");
int bitmap = open(montajeMasBitmap,O_RDWR);
struct stat mystat;
if (fstat(bitmap, &mystat) < 0) {
	printf("Error al establecer fstat\n");
	close(bitmap);
	}
char * bmap = mmap(0,mystat.st_size,PROT_EXEC | PROT_WRITE | PROT_READ, MAP_SHARED, bitmap, 0);
if (bmap == MAP_FAILED) {
			printf("Error al mapear a memoria: %s\n", strerror(errno));

	}
t_bitarray * bitarray = bitarray_create_with_mode(bmap,size/8,MSB_FIRST);
return bitarray;
}


t_bloques *  asignarBloques(t_list * libres,t_list * ocupados ,size_t size)
{
t_metadata_filesystem * fs = obtenerMetadata();
t_bloques * bloques = malloc(sizeof(t_bloques));
t_list * temp = list_create();
t_bitarray * bitarray = crearBitmap(fs->cantidad_bloques);
int nBloques = 0;
if(size % fs->tamanio_bloques == 0)
{
nBLoques = size / fs->tamanio_bloques;
}
else
{
nBLoques = (size / fs->tamanio_bloques) + 1;    
}
if (list_size(libres) >= nBloques){
temp  = list_take_and_remove(libres,nBloques);
list_add_all(ocupados,temp);
}
else
{
log_error("No hay bloques disponibles , vuelva a intentarlo");
}
bloques->bloqLibres = list_duplicate(libres);
bloques->bloqOcupados = list_duplicate(ocupados);
return bloques;
}


int verificarSiExisteArchivo(char * path)
{	
char * pathMasArchivos = string_new();
string_append(&pathMasArchivos,obtenerPtoMontaje());
string_append(&pathMasArchivos,"/Archivos/");
string_append(&pathMasArchivos,path);
int fd = open(pathMasArchivos,O_RDONLY);
if(fd < 0){
    close(fd);
    return -1;
}
else
{
    close(fd);
    return fd;
}
}
