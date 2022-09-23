/*
 * ESPHome component for Viessmann boilers.
 *
 * Tested with OptoLink ESP32 adapter from here:
 * https://github.com/openv/openv/wiki/Bauanleitung-ESP32-Adafruit-Feather-Huzzah32-and-Proto-Wing
 *
 * Example yaml:
 *
 * sensor:
 *   - platform: custom
 *     lambda: |-
 *       auto link = new optolink();
 *       App.register_component(link);
 *       return {link->sensor_outside_temp, link->sensor_boiler_temp};
 *      sensors:
 *        - name: ${name} outside temperature
 *          unit_of_measurement: °C
 *          accuracy_decimals: 1
 *          filters:
 *            - multiply: 0.1
 *        - name: ${name} boiler temperature
 *          unit_of_measurement: °C
 *          accuracy_decimals: 1
 *          filters:
 *            - multiply: 0.1
 *
 */

#pragma once

#include <sstream>
#include "esphome.h"
#include "VitoWiFi.h"
#include "datapoints.h"

using namespace std;
using namespace std::placeholders;

#define RX2 16
#define TX2 17

string _betriebsart[] = {
};

class optolink : public Component {
  private:
    // Optolink data points
    dp_float16 _dp_outside_temp;
    dp_float16 _dp_boiler_temp;
    dp_float16 _dp_speicher_temp;
    dp_float16 _dp_raum_soll_temp;
    dp_uint8 _dp_brenner_status;
    dp_uint8 _dp_betriebsart;
    dp_uint8 _dp_sparbetrieb;
    dp_uint8 _dp_partybetrieb;

    // Callbacks for optolink data points
    void _float_cb(Sensor *sensor, const IDatapoint& dp, DPValue value);
    void _uint8_cb(Sensor *sensor, const IDatapoint& dp, DPValue value);
    void _binary_cb(BinarySensor *sensor, const IDatapoint& dp, DPValue value);
    void _betriebsart_cb(TextSensor *sensor, const IDatapoint& dp, DPValue value);

    // Scheduled call for initiating communication via optolink
    void _comm();

  public:
    Sensor *sensor_outside_temp = new Sensor();
    Sensor *sensor_boiler_temp = new Sensor();
    Sensor *sensor_speicher_temp = new Sensor();
    Sensor *sensor_raum_soll_temp = new Sensor();
    BinarySensor *sensor_brenner_status = new BinarySensor();
    BinarySensor *sensor_sparbetrieb = new BinarySensor();
    BinarySensor *sensor_partybetrieb = new BinarySensor();
    TextSensor *sensor_betriebsart = new TextSensor();

    optolink();
    void setup() override;
    void loop() override;
}; //class optolink

VitoWiFi_setProtocol(KW);

// optolink class definitions
optolink::optolink() :
  _dp_outside_temp("outsideTemp", "boiler", 0x5525),
  _dp_boiler_temp("boilerTemp", "boiler", 0x0810),
  _dp_speicher_temp("speicherTemp", "speicher", 0x0804),
  _dp_raum_soll_temp("raumSollTemp", "boiler", 0x2000),
  _dp_brenner_status("brennerStatus", "boiler", 0x551E),
  _dp_betriebsart("betriebsart", "boiler", 0x2301),
  _dp_sparbetrieb("sparbetrieb", "boiler", 0x2302),
  _dp_partybetrieb("partybetrieb", "boiler", 0x2303)
{
}

void optolink::_comm() {
  VitoWiFi.readAll();
}

void optolink::_float_cb(Sensor* sensor, const IDatapoint& dp, DPValue value) {
  ESP_LOGD("optolink", "Datapoint %s - %s: %f", dp.getGroup(), dp.getName(), value.getFloat());
  sensor->publish_state(value.getFloat());
}

void optolink::_uint8_cb(Sensor* sensor, const IDatapoint& dp, DPValue value) {
  ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getU8());
  sensor->publish_state(value.getU8());
}

void optolink::_binary_cb(BinarySensor* sensor, const IDatapoint& dp, DPValue value) {
  ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getU8());
  sensor->publish_state(value.getU8() != 0);
}

void optolink::_betriebsart_cb(TextSensor* sensor, const IDatapoint& dp, DPValue value) {
  ESP_LOGD("optolink", "Datapoint %s - %s: %d", dp.getGroup(), dp.getName(), value.getU8());
  int code = value.getU8();
  stringstream t;
  switch(code) {
    case 0:
      t << "Abschaltbetrieb";
      break;
    case 1:
      t << "Nur Warmwasser";
      break;
    case 2:
      t << "Heizen, Kühlen, Warmwasser";
      break;
    case 3:
      t << "Undefiniert (3)";
      break;
    case 4:
      t << "Dauernd reduziert";
      break;
    case 5:
      t << "Dauernd normal";
      break;
    case 6:
      t << "Normal abschalt";
      break;
    case 7:
      t << "Nur kühlen";
      break;
    default:
      t << "Undefiniert (" <<code <<")";
      break;
  }
  sensor->publish_state(t.str());
}

void optolink::setup() {
  _dp_outside_temp.setCallback(bind(&optolink::_float_cb, this, sensor_outside_temp, _1, _2));
  _dp_boiler_temp.setCallback(bind(&optolink::_float_cb, this, sensor_boiler_temp, _1, _2));
  _dp_speicher_temp.setCallback(bind(&optolink::_float_cb, this, sensor_speicher_temp, _1, _2));
  _dp_raum_soll_temp.setCallback(bind(&optolink::_float_cb, this, sensor_raum_soll_temp, _1, _2));
  _dp_sparbetrieb.setCallback(bind(&optolink::_binary_cb, this, sensor_brenner_status, _1, _2));
  _dp_partybetrieb.setCallback(bind(&optolink::_binary_cb, this, sensor_brenner_status, _1, _2));
  _dp_brenner_status.setCallback(bind(&optolink::_binary_cb, this, sensor_brenner_status, _1, _2));
  _dp_betriebsart.setCallback(bind(&optolink::_betriebsart_cb, this, sensor_betriebsart, _1, _2));
  VitoWiFi.setup(&Serial2, RX2, TX2);  // 
  set_interval("optolink_comm", 10000, bind(&optolink::_comm, this));
  ESP_LOGI("OptoLink", "Component Initialized.");
}

void optolink::loop() {
  VitoWiFi.loop();
}

optolink* boiler = new optolink();
// end optolink class definitions
