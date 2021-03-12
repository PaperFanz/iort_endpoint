void mqtt_init();
void wifi_init();       //Network ssid and password set in messaging.c
bool mqtt_subscribe(const char* topic);
bool mqtt_publish(const char* topic, char* msg);

