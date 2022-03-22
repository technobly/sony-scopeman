/*
  TWO SQUARES

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

#define BUFFER_LEN 80

uint32_t buf[BUFFER_LEN];

int i2s_num = 0;
size_t i2s_bytes_written = 0;

i2s_config_t i2s_config = {
  .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
  .sample_rate = 50000,
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

void drawStuff(uint16_t sz) {

  static int x_off = -1024;
  static int y_off = -1024;
  static int dir = 0;

  switch (dir) {
    case 0: {
        if (y_off < 1024) {
          y_off++;
          x_off = -1024;
        } else {
          dir = 1;
        }
        break;
      }
    case 1: {
        if (x_off < 1024) {
          x_off++;
          y_off = 1024;
        } else {
          dir = 2;
        }
        break;
      }
    case 2: {
        if (y_off > -1024) {
          y_off--;
          x_off = 1024;
        } else {
          dir = 3;
        }
        break;
      }
    case 3: {
        if (x_off > -1024) {
          x_off--;
          y_off = -1024;
        } else {
          dir = 0;
        }
        break;
      }
  };

  buf[0] = calc_buf_pos(-500 + x_off, -500 + y_off); // 1
  buf[1] = calc_buf_pos(-500 + x_off, -400 + y_off);
  buf[2] = calc_buf_pos(-500 + x_off, -300 + y_off);
  buf[3] = calc_buf_pos(-500 + x_off, -200 + y_off);
  buf[4] = calc_buf_pos(-500 + x_off, -100 + y_off);
  buf[5] = calc_buf_pos(-500 + x_off, 0 + y_off);
  buf[6] = calc_buf_pos(-500 + x_off, 100 + y_off);
  buf[7] = calc_buf_pos(-500 + x_off, 200 + y_off);
  buf[8] = calc_buf_pos(-500 + x_off, 300 + y_off);
  buf[9] = calc_buf_pos(-500 + x_off, 400 + y_off);
  buf[10] = calc_buf_pos(-500 + x_off, 500 + y_off); // 2

  buf[11] = calc_buf_pos(-400 + x_off, 500 + y_off);
  buf[12] = calc_buf_pos(-300 + x_off, 500 + y_off);
  buf[13] = calc_buf_pos(-200 + x_off, 500 + y_off);
  buf[14] = calc_buf_pos(-100 + x_off, 500 + y_off);
  buf[15] = calc_buf_pos(0 + x_off, 500 + y_off);
  buf[16] = calc_buf_pos(100 + x_off, 500 + y_off);
  buf[17] = calc_buf_pos(200 + x_off, 500 + y_off);
  buf[18] = calc_buf_pos(300 + x_off, 500 + y_off);
  buf[19] = calc_buf_pos(400 + x_off, 500 + y_off);
  buf[20] = calc_buf_pos(500 + x_off, 500 + y_off); // 3

  buf[21] = calc_buf_pos(500 + x_off, 400 + y_off);
  buf[22] = calc_buf_pos(500 + x_off, 300 + y_off);
  buf[23] = calc_buf_pos(500 + x_off, 200 + y_off);
  buf[24] = calc_buf_pos(500 + x_off, 100 + y_off);
  buf[25] = calc_buf_pos(500 + x_off, 0 + y_off);
  buf[26] = calc_buf_pos(500 + x_off, -100 + y_off);
  buf[27] = calc_buf_pos(500 + x_off, -200 + y_off);
  buf[28] = calc_buf_pos(500 + x_off, -300 + y_off);
  buf[29] = calc_buf_pos(500 + x_off, -400 + y_off);
  buf[30] = calc_buf_pos(500 + x_off, -500 + y_off); // 4

  buf[31] = calc_buf_pos(400 + x_off, -500 + y_off);
  buf[32] = calc_buf_pos(300 + x_off, -500 + y_off);
  buf[33] = calc_buf_pos(200 + x_off, -500 + y_off);
  buf[34] = calc_buf_pos(100 + x_off, -500 + y_off);
  buf[35] = calc_buf_pos(0 + x_off, -500 + y_off);
  buf[36] = calc_buf_pos(-100 + x_off, -500 + y_off);
  buf[37] = calc_buf_pos(-200 + x_off, -500 + y_off);
  buf[38] = calc_buf_pos(-300 + x_off, -500 + y_off);
  buf[39] = calc_buf_pos(-400 + x_off, -500 + y_off);

  buf[40] = calc_buf_pos(-500 + -x_off, -500 + -y_off); // 1
  buf[41] = calc_buf_pos(-500 + -x_off, -400 + -y_off);
  buf[42] = calc_buf_pos(-500 + -x_off, -300 + -y_off);
  buf[43] = calc_buf_pos(-500 + -x_off, -200 + -y_off);
  buf[44] = calc_buf_pos(-500 + -x_off, -100 + -y_off);
  buf[45] = calc_buf_pos(-500 + -x_off, 0 + -y_off);
  buf[46] = calc_buf_pos(-500 + -x_off, 100 + -y_off);
  buf[47] = calc_buf_pos(-500 + -x_off, 200 + -y_off);
  buf[48] = calc_buf_pos(-500 + -x_off, 300 + -y_off);
  buf[49] = calc_buf_pos(-500 + -x_off, 400 + -y_off);
  buf[50] = calc_buf_pos(-500 + -x_off, 500 + -y_off); // 2

  buf[51] = calc_buf_pos(-400 + -x_off, 500 + -y_off);
  buf[52] = calc_buf_pos(-300 + -x_off, 500 + -y_off);
  buf[53] = calc_buf_pos(-200 + -x_off, 500 + -y_off);
  buf[54] = calc_buf_pos(-100 + -x_off, 500 + -y_off);
  buf[55] = calc_buf_pos(0 + -x_off, 500 + -y_off);
  buf[56] = calc_buf_pos(100 + -x_off, 500 + -y_off);
  buf[57] = calc_buf_pos(200 + -x_off, 500 + -y_off);
  buf[58] = calc_buf_pos(300 + -x_off, 500 + -y_off);
  buf[59] = calc_buf_pos(400 + -x_off, 500 + -y_off);
  buf[60] = calc_buf_pos(500 + -x_off, 500 + -y_off); // 3

  buf[61] = calc_buf_pos(500 + -x_off, 400 + -y_off);
  buf[62] = calc_buf_pos(500 + -x_off, 300 + -y_off);
  buf[63] = calc_buf_pos(500 + -x_off, 200 + -y_off);
  buf[64] = calc_buf_pos(500 + -x_off, 100 + -y_off);
  buf[65] = calc_buf_pos(500 + -x_off, 0 + -y_off);
  buf[66] = calc_buf_pos(500 + -x_off, -100 + -y_off);
  buf[67] = calc_buf_pos(500 + -x_off, -200 + -y_off);
  buf[68] = calc_buf_pos(500 + -x_off, -300 + -y_off);
  buf[69] = calc_buf_pos(500 + -x_off, -400 + -y_off);
  buf[70] = calc_buf_pos(500 + -x_off, -500 + -y_off); // 4

  buf[71] = calc_buf_pos(400 + -x_off, -500 + -y_off);
  buf[72] = calc_buf_pos(300 + -x_off, -500 + -y_off);
  buf[73] = calc_buf_pos(200 + -x_off, -500 + -y_off);
  buf[74] = calc_buf_pos(100 + -x_off, -500 + -y_off);
  buf[75] = calc_buf_pos(0 + -x_off, -500 + -y_off);
  buf[76] = calc_buf_pos(-100 + -x_off, -500 + -y_off);
  buf[77] = calc_buf_pos(-200 + -x_off, -500 + -y_off);
  buf[78] = calc_buf_pos(-300 + -x_off, -500 + -y_off);
  buf[79] = calc_buf_pos(-400 + -x_off, -500 + -y_off);
  i2s_write_expand((i2s_port_t)i2s_num, (const char *)&buf, i2s_config.dma_buf_len * 8, 16, 16, &i2s_bytes_written, 100);
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
  Serial.println("\r\nVectorboy booted!!");

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

void loop() {
  drawStuff(BUFFER_LEN);
}
