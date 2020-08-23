#include "planificadores.h"

void planificadorLargoPlazo(){
    while(1){
        sem_wait(&puedeEntrarAlSistema);
        sem_wait(&cantProcesosEnNew);
        DTB* dtbDummy = desencolarDTB(colaNew, m_colaNew);
        dtbDummy->status = READY;
        dtbDummy->flagInicializado = 1; //0, para pruebas lo dejamos en 1, dsp se cambia a 0
        encolarDTB(colaReady, dtbDummy, m_colaReady);
        sem_post(&cantProcesosEnReady);
    }
}

//al quantum accedo por datosConfigSAFA->quantum
void planificarSegunRR(CPU* cpu){
    DTB* dtbAEjecutar;
        while(1){
			if(queue_size(colaReady) == 0){
				log_trace(logger, "Se espera a que haya GDT's en la cola Ready");
			}
      
			sem_wait(&cantProcesosEnReady);
			sleep(1);

            pthread_mutex_lock(&m_puedePlanificar);
			dtbAEjecutar = obtenerDTBAEjecutarSegunRR();
            pthread_mutex_unlock(&m_puedePlanificar);

            if(dtbAEjecutar != NULL){ //si no fue finalizado
                dtbAEjecutar->status = RUNNING;
                if(dtbAEjecutar->flagInicializado == 0){
                    log_trace(logger, "Se va a ejecutar el DTB-Dummy del GDT de id = %d", dtbAEjecutar->id);            
                }else{
                    log_trace(logger, "Segun RR el DTB a ejecutar ahora es el de id = %d en la CPU de id %d", dtbAEjecutar->id, cpu->id);
                }

                char string_id[2];
                sprintf(string_id, "%i", dtbAEjecutar->id);

                char string_flagInicializacion[2];
                sprintf(string_flagInicializacion, "%i", dtbAEjecutar->flagInicializado); 

                char string_pc[2];
                sprintf(string_pc, "%i", dtbAEjecutar->PC);

                char string_quantumAEjecutar[2];
                sprintf(string_quantumAEjecutar, "%i", datosConfigSAFA->quantum);

                pthread_mutex_lock(&m_listaEjecutando);
                list_add(listaEjecutando, dtbAEjecutar);
                pthread_mutex_unlock(&m_listaEjecutando);

                runFunction(cpu->socket,"ejecutarCPU",5, string_id,
            										 dtbAEjecutar->rutaScript,
													 string_pc,
                                                     string_flagInicializacion,
                                                     string_quantumAEjecutar);

                sem_wait(&cpu->aviso);  
             
            }
	    }

}


void planificarSegunVRR(CPU* cpu){
    DTB* dtbAEjecutar;
        while(1){
			if(queue_size(colaReady) == 0){
				log_trace(logger, "Se espera a que haya GDT's en la cola Ready");
			}
			sem_wait(&cantProcesosEnReady);
			sleep(1);

            pthread_mutex_lock(&m_puedePlanificar);
			dtbAEjecutar = obtenerDTBAEjecutarSegunVRR();
            pthread_mutex_unlock(&m_puedePlanificar);

            if(dtbAEjecutar != NULL){ //si no fue finalizado
                dtbAEjecutar->status = RUNNING;
            if(dtbAEjecutar->flagInicializado == 0){
                log_trace(logger, "Se va a ejecutar el DTB-Dummy del GDT de id = %d", dtbAEjecutar->id);            
            }else{
                log_trace(logger, "Segun V-RR el DTB a ejecutar ahora es el de id = %d en la CPU de id %d", dtbAEjecutar->id, cpu->id);
            }

            char string_id[2];
            sprintf(string_id, "%i", dtbAEjecutar->id);

            char string_flagInicializacion[2];
            sprintf(string_flagInicializacion, "%i", dtbAEjecutar->flagInicializado); 

            char string_pc[2];
            sprintf(string_pc, "%i", dtbAEjecutar->PC);

            char string_quantumAEjecutar[2];
            if(dtbAEjecutar->quantumFaltante == 0)
                sprintf(string_quantumAEjecutar, "%i", datosConfigSAFA->quantum);
            else
                sprintf(string_quantumAEjecutar, "%i", dtbAEjecutar->quantumFaltante);

            pthread_mutex_lock(&m_listaEjecutando);
            list_add(listaEjecutando, dtbAEjecutar);
            pthread_mutex_unlock(&m_listaEjecutando);

            runFunction(cpu->socket,"ejecutarCPU",5, string_id,
            										 dtbAEjecutar->rutaScript,
													 string_pc,
                                                     string_flagInicializacion,
                                                     string_quantumAEjecutar);

            sem_wait(&cpu->aviso);   
			
	    }
    }
}

//IOBF (IO Bound First): Priorización de DTBs IO bound. 
//    Aquellos procesos que realicen muchas E/S (operaciones de MDJ) 
//  deberán ser favorecidos a la hora de la planificación.
void planificarSegunIOBF(CPU* cpu){
    DTB* dtbAEjecutar;
        while(1){
			if(queue_size(colaReady) == 0){
				log_trace(logger, "Se espera a que haya GDT's en la cola Ready");
			}
			sem_wait(&cantProcesosEnReady);
			sleep(1);

            pthread_mutex_lock(&m_puedePlanificar);
			dtbAEjecutar = obtenerDTBAEjecutarSegunIOBF();
            pthread_mutex_unlock(&m_puedePlanificar);

            if(dtbAEjecutar != NULL){ //si no fue finalizado
                dtbAEjecutar->status = RUNNING;
            if(dtbAEjecutar->flagInicializado == 0){
                log_trace(logger, "Se va a ejecutar el DTB-Dummy del GDT de id = %d", dtbAEjecutar->id);            
            }else{
                log_trace(logger, "Segun IOBF el DTB a ejecutar ahora es el de id = %d en la CPU de id %d", dtbAEjecutar->id, cpu->id);
            }

            char string_id[2];
            sprintf(string_id, "%i", dtbAEjecutar->id);

            char string_flagInicializacion[2];
            sprintf(string_flagInicializacion, "%i", dtbAEjecutar->flagInicializado); 

            char string_pc[2];
            sprintf(string_pc, "%i", dtbAEjecutar->PC);

            pthread_mutex_lock(&m_listaEjecutando);
            list_add(listaEjecutando, dtbAEjecutar);
            pthread_mutex_unlock(&m_listaEjecutando);

            runFunction(cpu->socket,"ejecutarCPU",5, string_id,
            										 dtbAEjecutar->rutaScript,
													 string_pc,
                                                     string_flagInicializacion,
                                                     "100");

            sem_wait(&cpu->aviso);   
			
	    }
    }
}

DTB* obtenerDTBAEjecutarSegunRR(){ // como FIFO
    DTB* dtb = desencolarDTB(colaReady, m_colaReady);
    return dtb;
}

DTB* obtenerDTBAEjecutarSegunVRR(){
    pthread_mutex_lock(&m_colaReady);
    list_sort(colaReady->elements, (void*) &closureSortVRR);
    DTB* dtb = queue_pop(colaReady);
    pthread_mutex_unlock(&m_colaReady);
    return dtb;
}

//true si p1 aparece antes que p2 en la lista
bool closureSortVRR(DTB* p1, DTB* p2){
    return p1->quantumFaltante != 0 || p2->quantumFaltante == 0;
} //ANDA

DTB* obtenerDTBAEjecutarSegunIOBF(){
    pthread_mutex_lock(&m_colaReady);
    list_sort(colaReady->elements, (void*) &closureSortIOBF);
    DTB* dtb = queue_pop(colaReady);
    pthread_mutex_unlock(&m_colaReady);
    return dtb;
}

bool closureSortIOBF(DTB* p1, DTB* p2){
    return p1->cantIOs >= p2->cantIOs;
} //ANDA

//callable remote function
//args[0]: el tipo del proceso que se conectó
//Esta funcion es para que por cada CPU se cree un hilo para planificarla
void  identificarProceso(socket_connection * connection ,char** args)
{
     log_info(logger,"Se ha conectado %s en el socket NRO %d  con IP %s,  PUERTO %d\n", args[0],connection->socket,connection->ip,connection-> port);
	 if(strcmp(args[0], "CPU") == 0){		 
		 CPU* cpu = malloc(sizeof(CPU));
		 cpu->socket = connection->socket;
		 cpu->id = generadorDeIdsCPU;
         generadorDeIdsCPU++;
		 sem_init(&cpu->aviso, 0, 0);
		 list_add(listaCPUs, cpu);
         
         char stringQuantum[2];
         sprintf(stringQuantum, "%i", datosConfigSAFA->quantum);
         char string_idCPU[2];   
         sprintf(string_idCPU, "%i", cpu->id);
    
		 runFunction(connection->socket, "establecerQuantumYID",2, stringQuantum, string_idCPU);
         

		 pthread_t hiloPCP;
		 if( strcmp( datosConfigSAFA->algoritmoPlanif, "RR") == 0 ){
			pthread_create(&hiloPCP, NULL, (void*)&planificarSegunRR, cpu);
		 } else if( strcmp( datosConfigSAFA->algoritmoPlanif, "VRR") == 0 ){
			 pthread_create(&hiloPCP, NULL, (void*)&planificarSegunVRR, cpu);
		 } else{
			 pthread_create(&hiloPCP, NULL, (void*)&planificarSegunIOBF, cpu);
		 }
		 pthread_detach(hiloPCP);
		 list_add(hilos, &hiloPCP);
	 }
} 
