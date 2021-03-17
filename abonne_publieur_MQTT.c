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
#define MQTT_QoS_abonne 1
#define MQTT_QoS_publieur 2
#define MQTT_PORT 1883
#define MQTT_TOPIC1 "sujet"
#define MQTT_TOPIC2 "copie"

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	int ret;
	printf("Topic : %s  --> ", (char*) message->topic);
	printf("Message : %s\n", (char*) message->payload);
	ret = mosquitto_publish(mosq, NULL, MQTT_TOPIC2, strlen(message->payload), message->payload, MQTT_QoS_publieur, false);
	if (ret)
	{
		fprintf(stderr,"Ne peut publier sur le serveur Mosquitto\n");
		exit(-1);
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
	ret = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC1, MQTT_QoS_abonne);
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

