/*
 * publieur_MQTT_MCP9808.c
 * 
 * (c) décembre 2020
 * Guy Gauthier
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include <bcm2835.h>

#define MQTT_HOSTNAME "localhost"
#define MQTT_PORT 1883
#define MQTT_TOPIC1 "capteur/garage/temperature"
//#define MQTT_TOPIC2 "capteur/zone1/pression"
#define MQTT_TOPIC3 "DEL_rouge"	   // Sujet (topic) DEL
#define MQTT_TOPIC2 "DEL_bleu"	   // Sujet (topic) DEL

/*  
 * Routine appelée lors de la réception d'un message du Broker.
 */
void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	// Impression des messages et des sujets reçu par l'abonné.
	// On n'y retrouve que les sujets auxquels le client est
	// abonné.
	printf("Topic : %s  --> ", (char*) message->topic);
	printf("Message : %s\n", (char*) message->payload);
	//printf("MID: %s\n", (char*) message->mid);
	//printf("Retain: %d\n", (int) message->retain);
	//printf("Length : %d\n", (int) message->payloadlen);
	
	// En pratiquem si l'abonnement ne concerne qu'un seul sujet
	// cette vérification est inutile. Elle le devient si le client
	// est abonné à plusieurs sujets pour adapter la réaction
	// au sujet du message reçu...
	if (!strcmp(message->topic,MQTT_TOPIC3))
	{
		// Vérification si le message demande d'allumer ou
		// d'éteindre le DEL roubge
		if (atoi(message->payload)==1){
			bcm2835_gpio_write(20, HIGH);
		}
		else
		{
			bcm2835_gpio_write(20, LOW);
		}
	}
	
	if (!strcmp(message->topic,MQTT_TOPIC2))
	{
		if (atoi(message->payload)==1){
			bcm2835_gpio_write(21, HIGH);
		}
		else
		{
			bcm2835_gpio_write(21, LOW);
		}
	}
}

int main(int argc, char **argv)
{
	char rwBuf[4];
	int valeur;
	char text[40];
	int ret;
	struct mosquitto *mosq = NULL;
	
	if (!bcm2835_init()){
		return 1;
	}
	
	bcm2835_gpio_fsel(19,BCM2835_GPIO_FSEL_INPT);
	
	if (!bcm2835_i2c_begin()){
		printf("Echec d'initialisation du I2C du bcm2835...\n");
		return 1;
	}
	bcm2835_i2c_set_baudrate(100000);  // Baudrate 100 kbits/sec
	bcm2835_delay(50);
	
	bcm2835_i2c_setSlaveAddress(0x18); // Adresse du MCP9808
	
	// Configuration du MCP9808
	rwBuf[0] = 0x08; // Registre de résolution 
	rwBuf[1] = 0x02; // Résolution de +/- 0.0125 C
	
	bcm2835_i2c_write(rwBuf,2);
	bcm2835_delay(50);
	
	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, NULL);
	if (!mosq)
	{
		fprintf(stderr,"Ne peut initialiser la librairie de Mosquitto\n");
		exit(-1);
	}
	
	// Association de la routine "my_message_calback" au client MQTT.
	// Cette routine sera donéravant exécutée quand un message
	// contenant un juset pour lequel on est abonné est reçu.
	mosquitto_message_callback_set(mosq, my_message_callback);
	
	ret = mosquitto_connect(mosq, MQTT_HOSTNAME, MQTT_PORT, 60);
	if (ret)
	{
		fprintf(stderr,"Ne peut se connecter au serveur Mosquitto\n");
		exit(-1);
	}
	
	// Abonnement au sujet DEL. On ne garde pas l'identification (id)
	// du message reçu, QoS de 0.
	ret = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC3, 0);
	if (ret)
	{
		fprintf(stderr,"Ne s'abonner sur le broker Mosquitto\n");
		exit(-1);
	}
	// Pour s'abonner à d'autres sujets, simplement répéter la séquence
	 ret = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC2, 0);
	if (ret)
	{
		fprintf(stderr,"Ne peut publier sur le serveur Mosquitto\n");
		exit(-1);
	}
	
	mosquitto_loop_start(mosq);
	
	while(bcm2835_gpio_lev(19)){
		rwBuf[0] = 0x05; // Registre de température
		bcm2835_i2c_write(rwBuf,1);
		bcm2835_delay(300);
		
		bcm2835_i2c_read(rwBuf,2);
		valeur = ((0x1F&rwBuf[0])<<8)+rwBuf[1];
		
		sprintf(text, "%5.3f",0.0625*valeur);
		ret = mosquitto_publish(mosq, NULL, MQTT_TOPIC1, strlen(text), text, 0, false);
		if (ret)
		{
			fprintf(stderr,"Ne peut publier sur le serveur Mosquitto\n");
			exit(-1);
		}
		bcm2835_delay(1000);
	}
	mosquitto_loop_stop(mosq, true);
	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	
	bcm2835_i2c_end();
	bcm2835_close();
	
	return 0;
}

