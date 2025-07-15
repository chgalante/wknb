#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/class/usb_hid.h>
#include <zephyr/usb/usb_device.h>

#define ROTARY_A_PIN 14
#define ROTARY_B_PIN 15
#define LED_PIN 25

#define NUMBER_OF_DETENTS_PER_REPORT 6

static const struct gpio_dt_spec rotary_a = {
    .port = DEVICE_DT_GET(DT_NODELABEL(gpio0)),
    .pin = ROTARY_A_PIN,
    .dt_flags = GPIO_PULL_UP};
static const struct gpio_dt_spec rotary_b = {
    .port = DEVICE_DT_GET(DT_NODELABEL(gpio0)),
    .pin = ROTARY_B_PIN,
    .dt_flags = GPIO_PULL_UP};
static const struct gpio_dt_spec led = {.port =
                                            DEVICE_DT_GET(DT_NODELABEL(gpio0)),
                                        .pin = LED_PIN,
                                        .dt_flags = GPIO_ACTIVE_HIGH};

static const struct device *hid_dev;
static int last_rotary_state = 0;
static int led_blink_count = 0;
static int detent_counter = 0;

// HID Report Descriptor for Consumer Control (volume/media keys)
static const uint8_t hid_report_desc[] = {
    0x05, 0x0C,       // USAGE_PAGE (Consumer Devices)
    0x09, 0x01,       // USAGE (Consumer Control)
    0xA1, 0x01,       // COLLECTION (Application)
    0x85, 0x01,       //   REPORT_ID (1)
    0x15, 0x00,       //   LOGICAL_MINIMUM (0)
    0x26, 0x9C, 0x02, //   LOGICAL_MAXIMUM (668)
    0x19, 0x00,       //   USAGE_MINIMUM (0)
    0x2A, 0x9C, 0x02, //   USAGE_MAXIMUM (668)
    0x75, 0x10,       //   REPORT_SIZE (16)
    0x95, 0x01,       //   REPORT_COUNT (1)
    0x81, 0x00,       //   INPUT (Data,Array,Abs)
    0xC0              // END_COLLECTION
};

void main(void) {
  int ret;

  // Initialize USB HID first
  hid_dev = device_get_binding("HID_0");
  if (hid_dev == NULL) {
    return;
  }

  usb_hid_register_device(hid_dev, hid_report_desc, sizeof(hid_report_desc),
                          NULL);

  ret = usb_hid_init(hid_dev);
  if (ret != 0) {
    return;
  }

  ret = usb_enable(NULL);
  if (ret != 0) {
    return;
  }

  // Configure GPIO pins
  if (!device_is_ready(rotary_a.port) || !device_is_ready(rotary_b.port) ||
      !device_is_ready(led.port)) {
    return;
  }

  ret = gpio_pin_configure_dt(&rotary_a, GPIO_INPUT);
  if (ret != 0) {
    return;
  }
  ret = gpio_pin_configure_dt(&rotary_b, GPIO_INPUT);
  if (ret != 0) {
    return;
  }
  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
  if (ret != 0) {
    return;
  }

  // Initial LED test - blink 3 times to show device is working
  for (int i = 0; i < 3; i++) {
    gpio_pin_set_dt(&led, 1);
    k_sleep(K_MSEC(200));
    gpio_pin_set_dt(&led, 0);
    k_sleep(K_MSEC(200));
  }

  // Initialize rotary encoder state
  int a_val = gpio_pin_get_dt(&rotary_a);
  int b_val = gpio_pin_get_dt(&rotary_b);
  last_rotary_state = (b_val << 1) | a_val;

  while (1) {
    // Poll rotary encoder
    a_val = gpio_pin_get_dt(&rotary_a);
    b_val = gpio_pin_get_dt(&rotary_b);
    int current_state = (b_val << 1) | a_val;

    if (current_state != last_rotary_state) {
      // Quadrature state transition table
      static const int8_t transition_table[4][4] = {/*00*/ {0, 1, -1, 0},
                                                    /*01*/ {-1, 0, 0, 1},
                                                    /*10*/ {1, 0, 0, -1},
                                                    /*11*/ {0, -1, 1, 0}};

      int8_t direction = transition_table[last_rotary_state][current_state];

      if (direction != 0) {
        led_blink_count = direction > 0 ? 1 : 2;

        // Increment detent counter in the direction of rotation
        detent_counter += direction;

        // Send HID report only after 2 detents in same direction
        if (detent_counter >= NUMBER_OF_DETENTS_PER_REPORT) {
          uint8_t report[] = {0x01, 0xE9, 0x00}; // Volume Up
          hid_int_ep_write(hid_dev, report, sizeof(report), NULL);

          // Send key release after short delay
          k_sleep(K_MSEC(15));
          uint8_t release[] = {0x01, 0x00, 0x00};
          hid_int_ep_write(hid_dev, release, sizeof(release), NULL);

          // Reset counter by number of detents per report
          detent_counter -= NUMBER_OF_DETENTS_PER_REPORT;
        } else if (detent_counter <= -NUMBER_OF_DETENTS_PER_REPORT) {
          uint8_t report[] = {0x01, 0xEA, 0x00}; // Volume Down
          hid_int_ep_write(hid_dev, report, sizeof(report), NULL);

          // Send key release after short delay
          k_sleep(K_MSEC(15));
          uint8_t release[] = {0x01, 0x00, 0x00};
          hid_int_ep_write(hid_dev, release, sizeof(release), NULL);

          // Reset counter by number of detents per report
          detent_counter += NUMBER_OF_DETENTS_PER_REPORT;
        }
      }

      last_rotary_state = current_state;
    }

    k_sleep(K_MSEC(10));
  }
}
