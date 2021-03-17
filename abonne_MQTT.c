/*
 * abonne_minimaliste_MQTT.c
 * 
 * (c) mars 2019
 * Guy Gauthier
 */
 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mosquitto.h>

#define MQTT_HOSTNAME "localhost"
#define MQTT_QoS 1
#define MQTT_PORT 1883
#define MQTT_TOPIC1 "capteur/+/temperature"
#define MQTT_TOPIC2 "capteur/zone1/pression"

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	char *ptr;
	printf("Topic : %s  --> ", (char*) message->topic);
	printf("Message : %s\n", (char*) message->payload);
	
	double valeur = strtod(message->payload,&ptr);
	if (!strcmp(message->topic,"capteur/zone1/temperature"))
	{
		printf("La temperature dans le salon est de %lf C.\n",valeur);
	}
	if (!strcmp(message->topic,"capteur/zone2/temperature"))
	{
		printf("La temperature dans la chambre est de %lf C\n",valeur);
	}
	if (!strcmp(message->topic,"capteur/zone3/temperature"))
	{
		printf("La temperature dans la cuisine est de %lf C\n",valeur);
	}
	if (!strcmp(message->topic,MQTT_TOPIC2))
	{
		printf("La pression atmospherique est de %lf Pa.\n",valeur);
	}
}

int main(int argc, char **argv)
{
	int ret;
	struct mosquitto *mosq = NULL;
	
	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, NULL);
	if (!mosq)
	{
		fprintf(stderr,"Ne peut initialiser la librairie de Mosquitto\n");
		exit(-1);
	}
	mosquitto_message_callback_set(mosq, my_message_callback);
	
	ret = mosquitto_connect(mosq, MQTT_HOSTNAME, MQTT_PORT, 60);
	if (ret)
	{
		fprintf(stderr,"Ne peut se connecter au serveur Mosquitto\n");
		exit(-1);
	}
	ret = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC1, MQTT_QoS);
	if (ret)
	{
		fprintf(stderr,"Ne peut publier sur le serveur Mosquitto\n");
		exit(-1);
	}
	ret = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC2, MQTT_QoS);
	if (ret)
	{
		fprintf(stderr,"Ne peut publier sur le serveur Mosquitto\n");
		exit(-1);
	}
	mosquitto_loop_start(mosq);
	sleep(120); // On laisse l'abonne en fonction 2 minutes
				// pour avoir le temps de tester...
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	
	return 0;
}

