#include <iostream>
#include <mosquitto.h>
#include <json-c/json.h>
#include <cstring>
#include <thread>
#include <chrono>

const char* SERVER_ADDRESS = "test.mosquitto.org";
const int SERVER_PORT = 1883;
const char* CLIENT_ID = "RaspberryPi2W_Client";
const char* TOPIC = "evc/charger";

void on_message(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
    std::cout << "Received message: " << (char*)message->payload << " on topic: " << message->topic << std::endl;
}

int main() {
    mosquitto_lib_init();
    struct mosquitto* mosq = mosquitto_new(CLIENT_ID, true, nullptr);
    if (!mosq) {
        std::cerr << "Failed to create Mosquitto instance" << std::endl;
        return 1;
    }
    
    mosquitto_message_callback_set(mosq, on_message);
    
    if (mosquitto_connect(mosq, SERVER_ADDRESS, SERVER_PORT, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to MQTT broker" << std::endl;
        return 1;
    }
    
    mosquitto_subscribe(mosq, nullptr, TOPIC, 1);
    std::cout << "Subscribed to topic: " << TOPIC << std::endl;
    
    int battery_level = 0;
    int charging_time = 0;
    while (true) {
        // Xác định trạng thái pin
        const char* charge_status = (battery_level >= 100) ? "completed" : "charging";

        // Tạo JSON payload bằng libjson-c
        json_object *payload = json_object_new_object();
        json_object_object_add(payload, "temperature", json_object_new_double(25.3 + (rand() % 10 - 5))); // Fake nhiệt độ
        json_object_object_add(payload, "voltage", json_object_new_double(220 + (rand() % 10 - 5))); // Fake điện áp
        json_object_object_add(payload, "current", json_object_new_double(10 + (rand() % 5 - 2))); // Fake dòng điện
        json_object_object_add(payload, "power", json_object_new_double(2000 + (rand() % 100 - 50))); // Fake công suất
        json_object_object_add(payload, "battery", json_object_new_int(battery_level));
        json_object_object_add(payload, "charging_time", json_object_new_int(charging_time)); // Thời gian sạc tăng dần
        json_object_object_add(payload, "remaining_time", json_object_new_int(100 - battery_level)); // Thời gian còn lại giảm dần
        json_object_object_add(payload, "charge_status", json_object_new_string(charge_status)); // Trạng thái sạc
        
        const char* json_payload = json_object_to_json_string(payload);
        mosquitto_publish(mosq, nullptr, TOPIC, strlen(json_payload), json_payload, 1, false);
        json_object_put(payload); // Giải phóng bộ nhớ
        
        std::cout << "Published: " << json_payload << std::endl;
        
        // Tăng mức pin từ 0 lên 100
        battery_level = (battery_level + 1) % 101;
        if (battery_level == 0) {
            charging_time = 0;
        } else {
            charging_time++;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        mosquitto_loop(mosq, -1, 1);
    }
    
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
