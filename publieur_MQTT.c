/*
 * publieur_minimaliste_MQTT.c
 * 
 * (c) mars2019
 * Guy Gauthier
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>

#define MQTT_HOSTNAME "localhost"
#define MQTT_PORT 1883
#define MQTT_TOPIC1 "capteur/zone1/temperature"
#define MQTT_TOPIC2 "capteur/zone1/pression"

int main(int argc, char **argv)
{
	char text[40];
	int ret;
	struct mosquitto *mosq = NULL;
	
	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, NULL);
	if (!mosq)
	{
		fprintf(stderr,"Ne peut initialiser la librairie de Mosquitto\n");
		exit(-1);
	}
	
	ret = mosquitto_connect(mosq, MQTT_HOSTNAME, MQTT_PORT, 60);
	if (ret)
	{
		fprintf(stderr,"Ne peut se connecter au serveur Mosquitto\n");
		exit(-1);
	}
	
	sprintf(text, "%5.2f C",23.75);
	ret = mosquitto_publish(mosq, NULL, MQTT_TOPIC1, strlen(text), text, 0, false);
	if (ret)
	{
		fprintf(stderr,"Ne peut publier sur le serveur Mosquitto\n");
		exit(-1);
	}
	sleep(1);
	sprintf(text, "%8.2f Pa",101325.2);
	ret = mosquitto_publish(mosq, NULL, MQTT_TOPIC2, strlen(text), text, 1, false);
	if (ret)
	{
		fprintf(stderr,"Ne peut publier sur le serveur Mosquitto\n");
		exit(-1);
	}
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	
	return 0;
}

