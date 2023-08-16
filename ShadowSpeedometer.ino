

#include <U8glib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

U8GLIB_SSD1309_128X64 u8g(7, 6, 4, 5);

SoftwareSerial gpsSerial(10, 11);
TinyGPSPlus gps;

// These use a lot of memory, might need to trim it down
char speed_buffer[16] = "NaN mph";
char temp_speed_buffer[16] = "0";
char sat_count_buffer[16] = "X";
char time_buffer[16] = "00:00";
char top_speed_buffer[16] = "-Not Yet-";

int mph = 0;
int top_speed = 0;
bool dst = true;

#define weewoo_bmp_width 20
#define weewoo_bmp_height 21
const unsigned char weewoo_bmp [] PROGMEM=
{
  0x18, 0x86, 0x01, 0xfc, 0xff, 0x03, 0xfe, 0xff, 0x07, 0xfc,
  0xff, 0x07, 0xfc, 0xff, 0x03, 0xfc, 0xff, 0x03, 0xfc, 0xf9,
  0x03, 0xfe, 0xf9, 0x07, 0xfe, 0xf1, 0x07, 0x1e, 0x80, 0x07,
  0x3e, 0xc0, 0x0f, 0x7f, 0xe0, 0x0f, 0x7f, 0xe0, 0x0f, 0x7f,
  0xe0, 0x0f, 0x7e, 0xef, 0x07, 0xfe, 0xff, 0x07, 0xf8, 0xff,
  0x01, 0xe0, 0x7f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x06, 0x00,
  0x00, 0x00, 0x00
};

int timezone_correct(int hour)
{
  if (hour > 4)
    return hour-4;
  return (23+hour)-4;
}

void draw(void)
{
  u8g.setFont(u8g_font_unifont);

  // Draw border
  u8g.drawLine(0, 0, 128, 0); // top
  u8g.drawLine(0, 63, 128, 63); // bottom
  u8g.drawLine(0, 0, 0, 64); // left
  u8g.drawLine(127, 0, 127, 64); // right

  // Show number of connected satellites
  u8g.drawStr(110, 16, sat_count_buffer);

  // Show speed
  u8g.drawStr(0, 16, speed_buffer);

  // Show top speed
  u8g.drawStr(0, 32, top_speed_buffer);

  // Show time
  u8g.drawStr(1, 48, time_buffer);

  // Going too fast, display the weewoo
  if (mph > 85)
    u8g.drawXBMP(90, 25, weewoo_bmp_width, weewoo_bmp_height, weewoo_bmp);
}

void draw_v2()
{

}

void update_info()
{
  // sprintf is slow as fuck
  // only update strings if the gps has pulled new data
  if (gps.speed.isUpdated())
  {
    mph = gps.speed.mph();
    dtostrf(mph, 3, 0, temp_speed_buffer);
    sprintf(speed_buffer, "%s mph", temp_speed_buffer);

    // Update top speed over 90 mph
    if (mph > top_speed && mph > 90)
    {
      top_speed = gps.speed.mph();
      sprintf(top_speed_buffer, "%s top", temp_speed_buffer);
    }
  }

  if (gps.time.isUpdated())
  {
    // All of this is hacky as fuck
    int h = timezone_correct(gps.time.hour());
    if (dst && h > 12) h++;
    sprintf(time_buffer, "%02d:%02d %s", h > 12 ? h-12 : h, gps.time.minute(), h > 12 ? "pm": "am");
  }

  if (gps.satellites.isUpdated())
  {
    Serial.print("New sat count: ");
    Serial.println(gps.satellites.value());
    sprintf(sat_count_buffer, "%2d", gps.satellites.value());
  }
}

void setup(void)
{
  // Basic setup stuff
  Serial.begin(9600);
  gpsSerial.begin(9600);
  Serial.println("GPS Start");
  u8g.setColorIndex(1);

  // Reset pin for oled
  pinMode(8, OUTPUT);

}

void loop(void)
{
  // Render to oled
  u8g.firstPage();
  do {
    // Pull gps data
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }
    update_info();
    //
    draw();
  } while( u8g.nextPage() );
}

