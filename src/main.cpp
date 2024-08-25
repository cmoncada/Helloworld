#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SPI.h>

// Define DHT GPIO PORT
#define DHTPIN 27

// Define DH Sensor Model
#define DHTTYPE DHT22

// Setup sensor configuration
DHT dht(DHTPIN, DHTTYPE);

#define MIN_TEMPERATURE 20
#define MAX_TEMPERATURE 50

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char *buf)
{
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

lv_obj_t *arc;

// Set the temperature value in the arc and text label
static void set_temp(void *text_label_temp_value, int32_t v)
{
  float temperature = dht.readTemperature();
  if (temperature <= 10.0)
  {
    lv_obj_set_style_text_color((lv_obj_t *)text_label_temp_value, lv_palette_main(LV_PALETTE_BLUE), 0);
  }
  else if (temperature > 10.0 && temperature <= 29)
  {
    lv_obj_set_style_text_color((lv_obj_t *)text_label_temp_value, lv_palette_main(LV_PALETTE_GREEN), 0);
  }
  else
  {
    lv_obj_set_style_text_color((lv_obj_t *)text_label_temp_value, lv_palette_main(LV_PALETTE_RED), 0);
  }
  const char degree_symbol[] = "\u00B0C";

  lv_arc_set_value(arc, map(int(temperature), MIN_TEMPERATURE, MAX_TEMPERATURE, 0, 100));

  String temperature_text = String(temperature) + degree_symbol;
  lv_label_set_text((lv_obj_t *)text_label_temp_value, temperature_text.c_str());
  Serial.print("Temperature: ");
  Serial.println(temperature_text);
}

void lv_create_main_gui(void)
{
  // Create an Arc
  arc = lv_arc_create(lv_screen_active());
  lv_obj_set_size(arc, 210, 210);
  lv_arc_set_rotation(arc, 135);
  lv_arc_set_bg_angles(arc, 0, 270);
  lv_obj_set_style_arc_color(arc, lv_color_hex(0x666666), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(arc, lv_color_hex(0x333333), LV_PART_KNOB);
  lv_obj_align(arc, LV_ALIGN_CENTER, 0, 10);

  // Create a text label in font size 32 to display the latest temperature reading
  lv_obj_t *text_label_temp_value = lv_label_create(lv_screen_active());
  lv_label_set_text(text_label_temp_value, "--.--");
  lv_obj_set_style_text_align(text_label_temp_value, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(text_label_temp_value, LV_ALIGN_CENTER, 0, 10);
  static lv_style_t style_temp;
  lv_style_init(&style_temp);
  lv_style_set_text_font(&style_temp, &lv_font_montserrat_32);
  lv_obj_add_style(text_label_temp_value, &style_temp, 0);

  // Create an animation to update with the latest temperature value every 10 seconds
  lv_anim_t a_temp;
  lv_anim_init(&a_temp);
  lv_anim_set_exec_cb(&a_temp, set_temp);
  lv_anim_set_duration(&a_temp, 100);
  lv_anim_set_playback_duration(&a_temp, 100);
  lv_anim_set_var(&a_temp, text_label_temp_value);
  lv_anim_set_values(&a_temp, 0, 100);
  lv_anim_set_repeat_count(&a_temp, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a_temp);
}

void setup()
{
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  dht.begin();

  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Create a display object
  lv_display_t *disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));

  // Function to draw the GUI
  lv_create_main_gui();
}

void loop()
{
  lv_task_handler(); // let the GUI do its work
  lv_tick_inc(1);    // tell LVGL how much time has passed
  delay(1);          // let this time pass
}