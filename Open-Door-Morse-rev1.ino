//#############################################################################################
//PINES DE SISTEMA
const byte PIN_PULSADOR = 13;
const byte PIN_PUERTA = 12;
const byte PIN_CONMUTADOR1 = 7;
const byte PIN_CONMUTADOR2 = 6;
const byte PIN_CONMUTADOR3 = 5;
const byte PIN_CONMUTADOR4 = 4;
const byte PIN_CONMUTADOR5 = 3;

//#############################################################################################
const byte PULSADO = 1;
const byte DESPULSADO = 0;
const byte ABIERTA = 1;
const byte CERRADA = 0;
const byte CORTO = 0;
const byte LARGO = 1;
const byte ERR = 2;

//#############################################################################################
//CONSTANTES DE TIEMPO
const unsigned long PULSO_CORTO = 1000;     //TIEMPO DE PULSO CORTO 1 SEG
const unsigned long PULSO_LARGO = 2000;     // TIEMPO DE PULSO LARGO 2 SEG
const unsigned long TIEMPO_RESET = 10000;   // TIEMPO DE REINICIO DE ACTIVIDAD SI NO PASA NADA DE 10 SEG
const unsigned UMBRAL_REBOTE = 50;

//#############################################################################################
//#############################################################################################
//#############################################################################################
struct TPortero{
  byte pin;
  byte estado;
  unsigned long inicio_pulso;
  unsigned long ultimo_pulso;
  
};

typedef byte TTono [5];
struct TSistema{
  TTono tono;
  unsigned contador;		//ES SIMPLEMENTE UN INDICE PARA RECORRER EL ARRAY "TONO"
};

struct TPuerta{
  byte pin;
  byte estado;
  unsigned long inicio_apertura;
};

//#############################################################################################
//#############################################################################################
//#############################################################################################
//INICIALIZACIONES...
TPortero portero;
TSistema sistema;
TPuerta puerta;

void setup_portero(TPortero &portero){
  Serial.println("Inicializando portero...");
  pinMode(PIN_PULSADOR, INPUT);
  portero.pin = PIN_PULSADOR;
  portero.estado = DESPULSADO;
  portero.inicio_pulso = 0;
  portero.ultimo_pulso = 0;
}

void setup_sistema(TSistema &sistema){
  Serial.println("Inicializando sistema Arduino y la palabra obtenida del switch...");
  sistema.contador = 0;
  pinMode(PIN_CONMUTADOR1, INPUT_PULLUP);	//HAY QUE ACTIVAR LA RESISTENCIA INTERNA DE PULLUP PARA USAR EL SWITCH, O NO ACTIVARLA Y PONERLAS EN HARDWARE
  pinMode(PIN_CONMUTADOR2, INPUT_PULLUP);
  pinMode(PIN_CONMUTADOR3, INPUT_PULLUP);
  pinMode(PIN_CONMUTADOR4, INPUT_PULLUP);
  pinMode(PIN_CONMUTADOR5, INPUT_PULLUP);
                          //PARA VOLVER A ESTABLECER EL TONO HAY QUE APAGAR EL SISTEMA, MODIFICAR Y ENCENDER.
  sistema.tono[0] = digitalRead(PIN_CONMUTADOR1);
  sistema.tono[1] = digitalRead(PIN_CONMUTADOR2);
  sistema.tono[2] = digitalRead(PIN_CONMUTADOR3);
  sistema.tono[3] = digitalRead(PIN_CONMUTADOR4);
  sistema.tono[4] = digitalRead(PIN_CONMUTADOR5);

 Serial.print(digitalRead(PIN_CONMUTADOR1));
 Serial.print(digitalRead(PIN_CONMUTADOR2));
 Serial.print(digitalRead(PIN_CONMUTADOR3));
 Serial.print(digitalRead(PIN_CONMUTADOR4));
 Serial.print(digitalRead(PIN_CONMUTADOR5));
 Serial.println();
 Serial.println();
}

void setup_puerta(TPuerta &puerta){
  Serial.println("Inicializando puerta...");
  pinMode(PIN_PUERTA, OUTPUT);
  puerta.pin=PIN_PUERTA;
  puerta.estado = CERRADA;
  puerta.inicio_apertura = 0;
}

//#############################################################################################
//#############################################################################################
//#############################################################################################
byte identificarPulso(TPortero &portero, TSistema &sistema){
  byte tipo;
  do{											//LEE EL ESTADO DEL BOTON DEL PORTERO CADA 50MS Y CUANDO SU ESTADO CAMBIA, GUARDA CUÁNDO LO HIZO
    delay(UMBRAL_REBOTE);
    portero.estado = digitalRead(portero.pin);
  }while(portero.estado == PULSADO);
  portero.ultimo_pulso = millis();
  
  if(millis() - portero.inicio_pulso <= PULSO_CORTO) {
    Serial.println("Pulso de tipo corto detectado!");
    tipo = CORTO;
  }
  else{
    Serial.println("Pulso de tipo largo detectado!");
    tipo = LARGO;
  }
  Serial.print("Tiempo del boton pulsado: ");
  Serial.println(millis()-portero.inicio_pulso);

  Serial.print("Tipo encontrado: ");
  Serial.print(tipo);
  Serial.print(" , tipo esperado: ");
  Serial.println(sistema.tono[sistema.contador]);
  Serial.print("Comprobado el pulso numero ");
  Serial.print(sistema.contador);
  Serial.println();
  
  if(tipo != sistema.tono[sistema.contador]) tipo = ERR;	
    
  return tipo;
}


void controlInactividad(TSistema &sistema, TPortero &portero){
	if (sistema.contador != 0 && millis() - portero.ultimo_pulso >= TIEMPO_RESET ){	//SI EL SISTEMA NO ESTA EN EL ESTADO INICIAL (ESPERANDO EL TONO 0) Y HAN PASADO 10 SEG, SE REINICIA A DICHO ESTADO
		Serial.println("");
		Serial.println("El sistema se ha reseteado por inactividad!");
		Serial.println("");
		sistema.contador = 0;
		portero.ultimo_pulso = 0;
		portero.inicio_pulso = 0;
  }
	
}

void controlPuerta(TSistema &sistema, TPuerta &puerta){
	++sistema.contador;
    if(sistema.contador == 5){				//SI SE HA LLEGADO A 5, SE HA COMPROBADO LA "PALABRA" CORRECTAMENTE Y SE ABRE LA PUERTA
		digitalWrite(puerta.pin, ABIERTA);
        Serial.println("");
        Serial.println("Puerta abierta!");
        Serial.println("");
        delay(10000);
        digitalWrite(puerta.pin, CERRADA);	//ESTA LINEA ES SOLO PARA DEPURACION USANDO EL LED
		sistema.contador = 0;
	}
}
//#############################################################################################
//#############################################################################################
//#############################################################################################
//SE EJECUTA UNA VEZ, AL ENCENDERSE.
void setup() {
  Serial.begin(9600);
  setup_portero(portero);
  setup_sistema(sistema);
  setup_puerta(puerta);
}

void loop() {
  delay(UMBRAL_REBOTE);								//CADA 50MS COMPRUEBA EL ESTADO DEL PORTERO.
  portero.estado = digitalRead(portero.pin);
  
  controlInactividad(sistema, portero);				
  
  if(portero.estado == PULSADO){					
	  portero.inicio_pulso = millis();
	
    Serial.println("");
    Serial.println("-------------------------");
    Serial.println("Boton pulsado!");
	
    byte tipo = identificarPulso(portero, sistema);
	
    if(tipo != ERR){			 
      controlPuerta(sistema, puerta);
    }
    else sistema.contador = 0;  
  }
}
