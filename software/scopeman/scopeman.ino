/*
  Driving deflection coils with an external DAC using ESP32

  Copyright (C) 2020-2022 Niklas Fauth, Technobly
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "BluetoothA2DPSink.h"
#include <Arduino.h>
#include "driver/i2s.h"
#include "driver/ledc.h"

BluetoothA2DPSink a2dp_sink;

#define CODEC_EN   13
#define SPEAKER_EN 18
#define FLYBACK    19
#define TUNE       34
#define SWITCH     22 // This changes the mode of operation. 1 = BT Audio; 0 = Lorentz Attractor Simulation
#define BLANK      25 // Blanking pin

#define BUFFER_LEN 400

uint32_t buf[BUFFER_LEN];

int16_t x_buf[BUFFER_LEN];
int16_t y_buf[BUFFER_LEN];
int16_t z_buf[BUFFER_LEN];

int i2s_num = 0;
size_t i2s_bytes_written = 0;

i2s_config_t i2s_config = {
  .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
  .sample_rate = 48000,
  .bits_per_sample = (i2s_bits_per_sample_t)16,
  .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
  .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_STAND_I2S),
  .intr_alloc_flags = 0, // default interrupt priority
  .dma_buf_count = 2,
  .dma_buf_len = 256,
  .use_apll = false
};

static const i2s_pin_config_t pin_config = {
  .bck_io_num = 12,
  .ws_io_num = 27,
  .data_out_num = 14,
  .data_in_num = I2S_PIN_NO_CHANGE
};

uint32_t calc_buf_pos(int16_t x, int16_t y) { // +/- 1024
  uint16_t x_i = x * 2;
  uint16_t y_i = y * 2;
  uint32_t buf;

  buf = x_i | y_i << 16;
  return buf;
}

float x = 0.01;
float y, z;

float a = 10.0;//10.;
float b = 28.0;//28.;
float c = 2.667;//2.66;
float dt = 0.015;
float dx, dy, dz;

void fillBuffer(uint16_t sz, float rot) {

  dx = (a * (y - x)) * dt;
  x = x + dx;

  dy = (x * (b - z) - y)  * dt;
  y = y + dy;

  dz = ((x * y) - (c * z)) * dt;
  z = z + dz;

  float scale = 1.3;

  x_buf[sz / 2] = (x * 64) * scale;
  y_buf[sz / 2] = (y * 64) * scale;
  z_buf[sz / 2] = (z * 64 - 1200) * scale;

  //Serial.println(x_buf[sz/2]);

  for (uint16_t i = 0; i < sz / 2; i++) {
    x_buf[i] = x_buf[i + 1];
    y_buf[i] = y_buf[i + 1];
    z_buf[i] = z_buf[i + 1];
  }

  for (uint16_t i = sz / 2 + 1; i < sz; i++) {
    x_buf[i] = x_buf[sz - i];
    y_buf[i] = y_buf[sz - i];
    z_buf[i] = z_buf[sz - i];
  }

  float sin_z = sin(rot);
  float cos_z = cos(rot);

  float sin_x = sin(rot * 0.5);
  float cos_x = cos(rot * 0.5);

  for (uint16_t i = 0; i < sz; i++) {

    // rotate Z axis
    int new_x = x_buf[i] * cos_z - y_buf[i] * sin_z;
    int new_y = x_buf[i] * sin_z + y_buf[i] * cos_z;

    // rotate ZY axis
    new_x = z_buf[i] * cos_x - new_y * sin_x;
    new_y = z_buf[i] * sin_x + new_y * cos_x;

    buf[i] = calc_buf_pos(new_x, new_y - 150);
  }
}

void setup() {
  pinMode(CODEC_EN, OUTPUT);
  digitalWrite(CODEC_EN, HIGH);

  pinMode(SPEAKER_EN, OUTPUT);
  digitalWrite(SPEAKER_EN, HIGH);

  // Just leave BLANK alone for now
  //  pinMode(BLANK, OUTPUT);
  //  digitalWrite(BLANK, LOW);

  Serial.begin(115200);
  delay(1000);
  Serial.println("\r\nVectorboy Booted!!");

  // 16kHz PWM output, 40% high duty cycle
  ledc_timer_config_t ledc_timer;
  ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;  // timer mode
  ledc_timer.duty_resolution = LEDC_TIMER_8_BIT; // resolution of PWM duty
  ledc_timer.timer_num = LEDC_TIMER_0;           // timer index
  ledc_timer.freq_hz = 16000;                    // frequency of PWM signal
  // Set configuration of timer0 for high speed channels
  esp_err_t result = ledc_timer_config(&ledc_timer);
  if (result == ESP_OK) {
    Serial.printf("frequency: %d", ledc_get_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0));
  }
  ledc_channel_config_t ledc_channel = {
    .gpio_num   = 19, // GPIO PIN 19
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel    = LEDC_CHANNEL_0,
    .intr_type  = LEDC_INTR_DISABLE,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 100,
    .hpoint     = 0
  };
  // Set LED Controller with previously prepared configuration
  ledc_channel_config(&ledc_channel);

  i2s_driver_uninstall((i2s_port_t)i2s_num);
  btStop();
  //initialize i2s with configurations above
  i2s_config.dma_buf_count = 2;
  i2s_config.dma_buf_len = BUFFER_LEN / 2;
  i2s_driver_install((i2s_port_t)i2s_num, &i2s_config, 0, NULL);
  i2s_set_pin((i2s_port_t)i2s_num, &pin_config);
}

float rotation = 0;
uint8_t sw_mode = 1;
uint8_t initialized = 0;

void loop() {
  if (sw_mode != !digitalRead(SWITCH)) {
    delay(30); // crude debounce
    sw_mode = !digitalRead(SWITCH);
    initialized = 0;
  }

  if (sw_mode == 1) { // Lorenz Attactor Mode
    if (!initialized) {
      i2s_driver_uninstall((i2s_port_t)i2s_num);
      btStop();
      //initialize i2s with configurations above
      i2s_config.dma_buf_count = 2;
      i2s_config.dma_buf_len = BUFFER_LEN / 2;
      i2s_driver_install((i2s_port_t)i2s_num, &i2s_config, 0, NULL);
      i2s_set_pin((i2s_port_t)i2s_num, &pin_config);
      initialized = 1;
    }

    fillBuffer(BUFFER_LEN, rotation);
    i2s_write_expand((i2s_port_t)i2s_num, (const char *)&buf, i2s_config.dma_buf_len * 8, 16, 16, &i2s_bytes_written, 100);
    dt = analogRead(TUNE) / 100000.0f;
    rotation = rotation + 0.001;
    if (rotation > M_PI_2 * 720.0) {
      rotation = 0;
    }
  } else { // BT Audio Mode
    if (!initialized) {
      i2s_config.dma_buf_count = 32;
      i2s_config.dma_buf_len = 256;
      i2s_driver_uninstall((i2s_port_t)i2s_num);
      a2dp_sink.set_pin_config(pin_config);
      a2dp_sink.set_i2s_config(i2s_config);
      a2dp_sink.start("Vectorboy");
      initialized = 1;
    }
  }
}
