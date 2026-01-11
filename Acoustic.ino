#include <Arduino.h>
#include "driver/i2s.h"

#define I2S_PORT I2S_NUM_0

#define I2S_BCLK 26
#define I2S_WS   25
#define I2S_DIN  33
//SEL = GND
#define SAMPLE_RATE 16000
#define DMA_BUF_LEN 256

int32_t samples[DMA_BUF_LEN];
static int32_t dc_estimate = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,

    //  KEY FIX
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,

    //  KEY FIX
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,

    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = DMA_BUF_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_DIN
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);

  Serial.println("SPH0645 READY");
}

void loop() {
  size_t bytes_read = 0;

  i2s_read(
    I2S_PORT,
    samples,
    sizeof(samples),
    &bytes_read,
    portMAX_DELAY
  );

  int frames = bytes_read / 8; // 2 samples per frame (L + R)

  for (int i = 0; i < frames; i++) {

    // Take LEFT channel only
    int32_t raw = samples[i * 2];

    // SPH0645 → 18-bit data, MSB aligned
    int32_t audio = raw >> 14;

    // Sign extend 18-bit
    if (audio & 0x00020000) {
      audio |= 0xFFFC0000;
    }
    // High-pass DC removal filter
    dc_estimate += (audio - dc_estimate) >> 8;
    int32_t filtered = audio - dc_estimate;
    int16_t pcm16 = filtered >> 2;

    Serial.println(pcm16);
  }
}
