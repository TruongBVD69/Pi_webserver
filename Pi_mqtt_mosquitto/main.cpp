#include <iostream>
#include <mosquitto.h>
#include "hmi_messages.pb-c.h" 
#include <cstring>
#include <thread>
#include <chrono>
#include <vector>

const char* SERVER_ADDRESS = "localhost";
const int SERVER_PORT = 1883;
const char* CLIENT_ID = "RaspberryPi3B_Client";

const char* TOPICS[] = {
    "hmi/rfid/connection_status",       // Trạng thái kết nối RFID ------------ (TOPICS[0])
    "hmi/rfid/error_code",              // Lỗi RFID --------------------------- (TOPICS[1])
    "hmi/rfid/uid",                     // UID RFID --------------------------- (TOPICS[2])
    "hmi/display/connection_status",    // Trạng thái kết nối ----------------- (TOPICS[3])
    "hmi/display/error_code",           // Mã lỗi ----------------------------- (TOPICS[4])
    "hmi/display/screen_id_current",    // ID màn hình thời điểm hiện tại ----- (TOPICS[5])
    "hmi/display/screen_id_active",     // ID màn hình đang hoạt động --------- (TOPICS[6])
    "hmi/ext/connection_status",        // Trạng thái kết nối thiết bị mở rộng- (TOPICS[7])
    "hmi/ext/error_code",               // Lỗi thiết bị mở rộng --------------- (TOPICS[8])
    "hmi/app/error_code",               // Lỗi ứng dụng ----------------------- (TOPICS[9])
    "hmi/app/gun_status",               // Trạng thái súng sạc ---------------- (TOPICS[10])
    "hmi/app/gun_error",                // Lỗi súng sạc ----------------------- (TOPICS[11])
    "hmi/app/gun_cable_check",          // Kiểm tra cáp súng sạc -------------- (TOPICS[12])
    "hmi/app/hlc_status",               // Trạng thái HLC --------------------- (TOPICS[13])
    "hmi/app/icon_4g",                  // Icon 4G ---------------------------- (TOPICS[14])
    "hmi/app/icon_wifi",                // Icon Wifi -------------------------- (TOPICS[15])
    "hmi/app/icon_lan",                 // Icon LAN --------------------------- (TOPICS[16])
    "hmi/app/icon_cloud",               // Icon Cloud ------------------------- (TOPICS[17])
    "hmi/app/account",                  // Tài khoản -------------------------- (TOPICS[18])
    "hmi/app/batt_percent",             // Pin % ------------------------------ (TOPICS[19])
    "hmi/app/temperature",              // Nhiệt độ --------------------------- (TOPICS[20])
    "hmi/app/voltage",                  // Điện áp ---------------------------- (TOPICS[21])
    "hmi/app/current",                  // Dòng điện -------------------------- (TOPICS[22])
    "hmi/app/remaing_time",             // Thời gian còn lại ------------------ (TOPICS[23])
    "hmi/app/charging_time",            // Thời gian sạc ---------------------- (TOPICS[24])
    "hmi/app/charging_power",           // Công suất sạc ---------------------- (TOPICS[25])
    "hmi/app/charging_energy",          // Năng lượng sạc --------------------- (TOPICS[26])
    "hmi/app/charging_cash",            // Tiền sạc --------------------------- (TOPICS[27])
    "hmi/app/wait_ev_connect_timeout",  // Thời gian chờ kết nối EV ----------- (TOPICS[28])
    "hmi/app/active_socket",            // Ổ cắm đang hoạt động --------------- (TOPICS[29])
    "hmi/app/hmi_system_status",        // Trạng thái hệ thống ---------------- (TOPICS[30])
    "hmi/app/error_code_data"           // Dữ liệu lỗi ------------------------ (TOPICS[31])
};

std::vector<Broker__ScreenStatus> screen_statuses = {
    BROKER__SCREEN_STATUS__RESERVATION,\
    BROKER__SCREEN_STATUS__RESET, BROKER__SCREEN_STATUS__CHARGING
};

void publish_message(mosquitto* mosq, const char* topic, void* message, size_t message_size) {
    if (!mosq) return;

    // Convert payload to HEX string for debugging
    std::cout << "Publishing to topic [" << topic << "] - Payload (size: " << message_size << "): ";
    for (size_t i = 0; i < message_size; ++i) {
        printf("%02X ", ((uint8_t*)message)[i]);
    }
    std::cout << std::endl;

    int ret = mosquitto_publish(mosq, nullptr, topic, message_size, message, 1, false);
    if (ret == MOSQ_ERR_SUCCESS) {
        std::cout << "Published successfully to " << topic << "\n\r";
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
    
    uint32_t battery_level = 0;
    
    while (true) {
        Broker__ActiveSocket activeSocket_msg = BROKER__ACTIVE_SOCKET__INIT;
        activeSocket_msg.active_socket = false;
        size_t activeSocket_msg_size = broker__active_socket__get_packed_size(&activeSocket_msg);
        uint8_t* packed_activeSocket = (uint8_t*)malloc(activeSocket_msg_size);
        broker__active_socket__pack(&activeSocket_msg, packed_activeSocket);
        publish_message(mosq, TOPICS[29], packed_activeSocket, activeSocket_msg_size);
        free(packed_activeSocket);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        activeSocket_msg.active_socket = true;
        activeSocket_msg_size = broker__active_socket__get_packed_size(&activeSocket_msg);
        packed_activeSocket = (uint8_t*)malloc(activeSocket_msg_size);
        broker__active_socket__pack(&activeSocket_msg, packed_activeSocket);
        publish_message(mosq, TOPICS[29], packed_activeSocket, activeSocket_msg_size);
        free(packed_activeSocket);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        Broker__GunError Gun_error_msg = BROKER__GUN_ERROR__INIT;
        Gun_error_msg.gun_error = true;
        Gun_error_msg.station_error = false;
        size_t gun_error_msg_size = broker__gun_error__get_packed_size(&Gun_error_msg);
        uint8_t* packed_gun_error = (uint8_t*)malloc(gun_error_msg_size);
        broker__gun_error__pack(&Gun_error_msg, packed_gun_error);
        publish_message(mosq, TOPICS[11], packed_gun_error, gun_error_msg_size);
        free(packed_gun_error);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        Gun_error_msg.gun_error = false;
        Gun_error_msg.station_error = true;
        gun_error_msg_size = broker__gun_error__get_packed_size(&Gun_error_msg);
        packed_gun_error = (uint8_t*)malloc(gun_error_msg_size);
        broker__gun_error__pack(&Gun_error_msg, packed_gun_error);
        publish_message(mosq, TOPICS[11], packed_gun_error, gun_error_msg_size);
        free(packed_gun_error);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        for (const auto& status : screen_statuses) {
            // Tạo message ScreenStatus
            Broker__HMIStatus status_msg = BROKER__HMISTATUS__INIT;
            status_msg.screen_status = status;
            size_t status_msg_size = broker__hmistatus__get_packed_size(&status_msg);
            uint8_t* packed_status = (uint8_t*)malloc(status_msg_size);
            broker__hmistatus__pack(&status_msg, packed_status);
            publish_message(mosq, TOPICS[30], packed_status, status_msg_size);
            free(packed_status);
            
            if (status == BROKER__SCREEN_STATUS__CHARGING) {
                while (battery_level <= 100) {          
                    Broker__HMIStatus charging_status_msg = BROKER__HMISTATUS__INIT;
                    charging_status_msg.screen_status = BROKER__SCREEN_STATUS__CHARGING;
                    size_t charging_status_msg_size = broker__hmistatus__get_packed_size(&charging_status_msg);
                    uint8_t* packed_charging_status = (uint8_t*)malloc(charging_status_msg_size);
                    broker__hmistatus__pack(&charging_status_msg, packed_charging_status);
                    publish_message(mosq, TOPICS[30], packed_charging_status, charging_status_msg_size);
                    free(packed_charging_status);

                    Broker__BattPercent battPercent_msg = BROKER__BATT_PERCENT__INIT;
                    battPercent_msg.batt_percent = battery_level;
                    size_t battPercent_msg_size = broker__batt_percent__get_packed_size(&battPercent_msg);
                    uint8_t* packed_battPercent = (uint8_t*)malloc(battPercent_msg_size);
                    broker__batt_percent__pack(&battPercent_msg, packed_battPercent);
                    publish_message(mosq, TOPICS[19], packed_battPercent, battPercent_msg_size);
                    free(packed_battPercent);

                    Broker__Temperature temperature_msg = BROKER__TEMPERATURE__INIT;
                    temperature_msg.temperature = 35 + (rand() % 11);
                    size_t temperature_msg_size = broker__temperature__get_packed_size(&temperature_msg);
                    uint8_t* packed_temperature = (uint8_t*)malloc(temperature_msg_size);
                    broker__temperature__pack(&temperature_msg, packed_temperature);
                    publish_message(mosq, TOPICS[20], packed_temperature, temperature_msg_size);
                    free(packed_temperature);

                    Broker__Voltage voltage_msg = BROKER__VOLTAGE__INIT;
                    voltage_msg.voltage = 220 + (rand() % 11);
                    size_t voltage_msg_size = broker__voltage__get_packed_size(&voltage_msg);
                    uint8_t* packed_voltage = (uint8_t*)malloc(voltage_msg_size);
                    broker__voltage__pack(&voltage_msg, packed_voltage);
                    publish_message(mosq, TOPICS[21], packed_voltage, voltage_msg_size);
                    free(packed_voltage);

                    Broker__Current current_msg = BROKER__CURRENT__INIT;
                    current_msg.current = 10 + (rand() % 11);
                    size_t current_msg_size = broker__current__get_packed_size(&current_msg);
                    uint8_t* packed_current = (uint8_t*)malloc(current_msg_size);
                    broker__current__pack(&current_msg, packed_current);
                    publish_message(mosq, TOPICS[22], packed_current, current_msg_size);
                    free(packed_current);

                    Broker__RemainingTime remainingTime_msg = BROKER__REMAINING_TIME__INIT;
                    remainingTime_msg.remaining_time = 3600 + (rand() % 3601);
                    size_t remainingTime_msg_size = broker__remaining_time__get_packed_size(&remainingTime_msg);
                    uint8_t* packed_remainingTime = (uint8_t*)malloc(remainingTime_msg_size);
                    broker__remaining_time__pack(&remainingTime_msg, packed_remainingTime);
                    publish_message(mosq, TOPICS[23], packed_remainingTime, remainingTime_msg_size);
                    free(packed_remainingTime);

                    Broker__ChargingTime chargingTime_msg = BROKER__CHARGING_TIME__INIT;
                    chargingTime_msg.charging_time = 3600 + (rand() % 3601);
                    size_t chargingTime_msg_size = broker__charging_time__get_packed_size(&chargingTime_msg);
                    uint8_t* packed_chargingTime = (uint8_t*)malloc(chargingTime_msg_size);
                    broker__charging_time__pack(&chargingTime_msg, packed_chargingTime);
                    publish_message(mosq, TOPICS[24], packed_chargingTime, chargingTime_msg_size);
                    free(packed_chargingTime);

                    Broker__ChargingPower chargingPower_msg = BROKER__CHARGING_POWER__INIT;
                    chargingPower_msg.charging_power = 80 + (rand() % 11);
                    size_t chargingPower_msg_size = broker__charging_power__get_packed_size(&chargingPower_msg);
                    uint8_t* packed_chargingPower = (uint8_t*)malloc(chargingPower_msg_size);
                    broker__charging_power__pack(&chargingPower_msg, packed_chargingPower);
                    publish_message(mosq, TOPICS[25], packed_chargingPower, chargingPower_msg_size);
                    free(packed_chargingPower);

                    Broker__ChargingEnergy chargingEnergy_msg = BROKER__CHARGING_ENERGY__INIT;
                    chargingEnergy_msg.charging_energy = 80 + (rand() % 11);
                    size_t chargingEnergy_msg_size = broker__charging_energy__get_packed_size(&chargingEnergy_msg);
                    uint8_t* packed_chargingEnergy = (uint8_t*)malloc(chargingEnergy_msg_size);
                    broker__charging_energy__pack(&chargingEnergy_msg, packed_chargingEnergy);
                    publish_message(mosq, TOPICS[26], packed_chargingEnergy, chargingEnergy_msg_size);
                    free(packed_chargingEnergy);

                    Broker__ChargingCash chargingCash_msg = BROKER__CHARGING_CASH__INIT;
                    chargingCash_msg.charging_cash = 1000 + (rand() % 1001);
                    size_t chargingCash_msg_size = broker__charging_cash__get_packed_size(&chargingCash_msg);
                    uint8_t* packed_chargingCash = (uint8_t*)malloc(chargingCash_msg_size);
                    broker__charging_cash__pack(&chargingCash_msg, packed_chargingCash);
                    publish_message(mosq, TOPICS[27], packed_chargingCash, chargingCash_msg_size);
                    free(packed_chargingCash);

                    if (battery_level > 100) {
                        // Đã sạc đầy
                        Broker__HMIStatus completed_status_msg = BROKER__HMISTATUS__INIT;
                        completed_status_msg.screen_status = BROKER__SCREEN_STATUS__FINISHING;
                        size_t completed_status_msg_size = broker__hmistatus__get_packed_size(&completed_status_msg);
                        uint8_t* packed_completed_status = (uint8_t*)malloc(completed_status_msg_size);
                        broker__hmistatus__pack(&completed_status_msg, packed_completed_status);
                        publish_message(mosq, TOPICS[30], packed_completed_status, completed_status_msg_size);
                        free(packed_completed_status);

                        std::this_thread::sleep_for(std::chrono::seconds(5));

                        // Chuẩn bị lại

			Broker__HMIStatus preparing_status_msg = BROKER__HMISTATUS__INIT;
                        preparing_status_msg.screen_status = BROKER__SCREEN_STATUS__PREPARING;
                        size_t preparing_status_msg_size = broker__hmistatus__get_packed_size(&preparing_status_msg);
                        uint8_t* packed_preparing_status = (uint8_t*)malloc(preparing_status_msg_size);
                        broker__hmistatus__pack(&preparing_status_msg, packed_preparing_status);
                        publish_message(mosq, TOPICS[30], packed_preparing_status, preparing_status_msg_size);
                        free(packed_preparing_status);

			std::cout << "Charging completed. Waiting for 10 seconds before restarting..." << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(10));


                        battery_level = 0;
                        break;
                    } else {
                        battery_level++;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    mosquitto_loop(mosq, -1, 1);
                }
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                mosquitto_loop(mosq, -1, 1);
            }  
        }
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
