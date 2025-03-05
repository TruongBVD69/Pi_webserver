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

std::vector<std::string> screen_statuses = {"Available", "Unavailable", "Charging", "Faulted", "SuspendedEV", "SuspendedEVSE", "Finishing", "Reservation", "Unplug", "Reset"};

void publish_message(mosquitto* mosq, const char* topic, json_object* payload) {
    const char* json_payload = json_object_to_json_string(payload);
    mosquitto_publish(mosq, nullptr, topic, strlen(json_payload), json_payload, 1, false);
    json_object_put(payload);
    std::cout << "Published to " << topic << ": " << json_payload << std::endl;
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
    int status_index = 0;
    
    while (true) {
        // Publish trạng thái màn hình
        json_object *status_payload = json_object_new_object();
        json_object_object_add(status_payload, "status", json_object_new_string(screen_statuses[status_index].c_str()));
        publish_message(mosq, TOPIC_HMI_1, status_payload);
        
        status_index = (status_index + 1) % screen_statuses.size();
        std::this_thread::sleep_for(std::chrono::seconds(6));

        // Fake dữ liệu pin
        json_object *battery_payload = json_object_new_object();
        json_object_object_add(battery_payload, "battery", json_object_new_int(battery_level));
        json_object_object_add(battery_payload, "temperature", json_object_new_double(35 + (rand() % 11))); // 35-45 độ C
        publish_message(mosq, TOPIC_EVC_1, battery_payload);

        // Fake dữ liệu sạc
        json_object *charging_payload = json_object_new_object();
        json_object_object_add(charging_payload, "power", json_object_new_double(80 + (rand() % 11))); // 80-90 W
        json_object_object_add(charging_payload, "voltage", json_object_new_double(220 + (rand() % 10 - 5))); // 215-225 V
        json_object_object_add(charging_payload, "current", json_object_new_double(10 + (rand() % 5 - 2))); // 8-12 A
        json_object_object_add(charging_payload, "delivered_energy", json_object_new_int(energy_delivered));
        json_object_object_add(charging_payload, "remaining_time", json_object_new_int(100 - battery_level));
        json_object_object_add(charging_payload, "charging_time", json_object_new_int(charging_time));
        publish_message(mosq, TOPIC_EVC_2, charging_payload);

        // Fake dữ liệu tiền sạc
        json_object *cash_payload = json_object_new_object();
        json_object_object_add(cash_payload, "charging_cash", json_object_new_int(charging_cash));
        publish_message(mosq, TOPIC_EVC_3, cash_payload);

        // Tăng mức pin
        if (battery_level >= 100) {
            std::cout << "Charging completed. Waiting for 5 seconds before restarting..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            battery_level = 0;
            charging_time = 0;
            energy_delivered = 0;
            charging_cash = 0;
        } else {
            battery_level++;
            charging_time++;
            energy_delivered += 1;
            charging_cash += 2; // Giả lập chi phí tăng
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
        mosquitto_loop(mosq, -1, 1);
    }
    
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
