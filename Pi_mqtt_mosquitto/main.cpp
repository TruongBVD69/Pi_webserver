#include <iostream>
#include <mosquitto.h>
#include <json-c/json.h>
#include <cstring>
#include <thread>
#include <chrono>
#include <vector>

const char* SERVER_ADDRESS = "test.mosquitto.org";
const int SERVER_PORT = 1883;
const char* CLIENT_ID = "RaspberryPi2W_Client";

const char* TOPIC_HMI_1 = "hmi/display/system_status";
const char* TOPIC_EVC_1 = "hmi/app/battery";
const char* TOPIC_EVC_2 = "hmi/app/charging";
const char* TOPIC_EVC_3 = "hmi/app/charging_cash";

std::vector<std::string> screen_statuses = {"Available", "Unavailable", "Faulted", "SuspendedEV", "SuspendedEVSE", "Finishing", "Reservation", "Unplug", "Reset", "Charging"};

void publish_message(mosquitto* mosq, const char* topic, json_object* payload) {
    if (!mosq) return;
    const char* json_payload = json_object_to_json_string(payload);
    int ret = mosquitto_publish(mosq, nullptr, topic, strlen(json_payload), json_payload, 1, false);
    json_object_put(payload);
    if (ret == MOSQ_ERR_SUCCESS) {
        std::cout << "Published to " << topic << ": " << json_payload << std::endl;
    } else {
        std::cerr << "Failed to publish to " << topic << " (Error: " << ret << ")" << std::endl;
    }
}

int main() {
    mosquitto_lib_init();
    struct mosquitto* mosq = mosquitto_new(CLIENT_ID, true, nullptr);
    if (!mosq) {
        std::cerr << "Failed to create Mosquitto instance" << std::endl;
        return 1;
    }
    
    if (mosquitto_connect(mosq, SERVER_ADDRESS, SERVER_PORT, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to MQTT broker" << std::endl;
        return 1;
    }
    
    int battery_level = 0;
    int charging_time = 0;
    int energy_delivered = 0;
    int charging_cash = 0;
    
    while (true) {
        for (const auto& status : screen_statuses) {
            json_object *status_payload = json_object_new_object();
            json_object_object_add(status_payload, "screen_status", json_object_new_string(status.c_str()));
            const char* json_status_payload = json_object_to_json_string(status_payload);
            mosquitto_publish(mosq, nullptr, TOPIC_HMI_1, strlen(json_status_payload), json_status_payload, 1, false);
            std::cout << "Published: " << json_status_payload << std::endl;
            json_object_put(status_payload); // Giải phóng bộ nhớ
            
            if (status == "Charging") {
                while (battery_level <= 100) {
                    json_object *battery_payload = json_object_new_object();
                    json_object_object_add(battery_payload, "battery", json_object_new_int(battery_level));
                    json_object_object_add(battery_payload, "temperature", json_object_new_double(35 + (rand() % 11))); 
                    const char* json_battery_payload = json_object_to_json_string(battery_payload);
                    mosquitto_publish(mosq, nullptr, TOPIC_EVC_1, strlen(json_battery_payload), json_battery_payload, 1, false);
                    std::cout << "Published: " << json_battery_payload << std::endl;
                    json_object_put(battery_payload); // Giải phóng bộ nhớ

                    json_object *charging_payload = json_object_new_object();
                    json_object_object_add(charging_payload, "power", json_object_new_double(80 + (rand() % 11))); 
                    json_object_object_add(charging_payload, "voltage", json_object_new_double(220 + (rand() % 10 - 5))); 
                    json_object_object_add(charging_payload, "current", json_object_new_double(10 + (rand() % 5 - 2))); 
                    json_object_object_add(charging_payload, "delivered_energy", json_object_new_int(energy_delivered));
                    json_object_object_add(charging_payload, "remaining_time", json_object_new_int(100 - battery_level));
                    json_object_object_add(charging_payload, "charging_time", json_object_new_int(charging_time));
                    const char* json_charging_payload = json_object_to_json_string(charging_payload);
                    mosquitto_publish(mosq, nullptr, TOPIC_EVC_2, strlen(json_charging_payload), json_charging_payload, 1, false);
                    std::cout << "Published: " << json_charging_payload << std::endl;
                    json_object_put(charging_payload); // Giải phóng bộ nhớ

                    json_object *cash_payload = json_object_new_object();
                    json_object_object_add(cash_payload, "charging_cash", json_object_new_int(charging_cash));
                    const char* json_cash_payload = json_object_to_json_string(cash_payload);
                    mosquitto_publish(mosq, nullptr, TOPIC_EVC_3, strlen(json_cash_payload), json_cash_payload, 1, false);
                    std::cout << "Published: " << json_cash_payload << std::endl;
                    json_object_put(cash_payload); // Giải phóng bộ nhớ
                    
                    
                    // std::cout << "Battery: " << battery_level << "% - Charging Time: " << charging_time << "s - Energy Delivered: " << energy_delivered << " kWh - Cash: $" << charging_cash << std::endl;
                    
                    if (battery_level >= 100) {
                        // json_object *completed_payload = json_object_new_object();
                        // json_object_object_add(completed_payload, "status", json_object_new_string("Completed"));
                        // publish_message(mosq, TOPIC_HMI_1, completed_payload);
                        std::cout << "Charging completed. Waiting for 5 seconds before restarting..." << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(5));
                        battery_level = 0;
                        charging_time = 0;
                        energy_delivered = 0;
                        charging_cash = 0;
                        break;
                    } else {
                        battery_level++;
                        charging_time++;
                        energy_delivered += 1;
                        charging_cash += 2;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    mosquitto_loop(mosq, -1, 1);
                }
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                mosquitto_loop(mosq, -1, 1);
            }  
        }
    }
    
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}