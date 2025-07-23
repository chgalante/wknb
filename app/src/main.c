#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/class/usb_hid.h>
#include <zephyr/usb/usb_device.h>

#ifdef CONFIG_DEBUG
#define DEBUG_PRINT(fmt, ...) printk(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)                                                  \
  do {                                                                         \
  } while (0)
#endif

#define ROTARY_A_PIN 14
#define ROTARY_B_PIN 15
#define BUTTON_PIN 16
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
static const struct gpio_dt_spec button = {
    .port = DEVICE_DT_GET(DT_NODELABEL(gpio0)),
    .pin = BUTTON_PIN,
    .dt_flags = GPIO_PULL_UP};
static const struct gpio_dt_spec led = {.port =
                                            DEVICE_DT_GET(DT_NODELABEL(gpio0)),
                                        .pin = LED_PIN,
                                        .dt_flags = GPIO_ACTIVE_HIGH};

static const struct device *hid_dev;
static int last_rotary_state = 0;
static int led_blink_count = 0;
static int detent_counter = 0;
static bool usb_suspended = false;
static struct gpio_callback button_cb_data;

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

void button_pressed(const struct device *dev, struct gpio_callback *cb,
                    uint32_t pins) {
  DEBUG_PRINT("Button pressed! USB suspended: %s, Remote wakeup enabled: %s\n",
              usb_suspended ? "YES" : "NO",
              usb_get_remote_wakeup_status() ? "YES" : "NO");

  if (usb_suspended && usb_get_remote_wakeup_status()) {
    DEBUG_PRINT("Attempting USB wake-up...\n");
    int ret = usb_wakeup_request();
    if (ret == 0) {
      DEBUG_PRINT("USB wake-up request successful!\n");
      gpio_pin_set_dt(&led, 1);
      k_sleep(K_MSEC(100));
      gpio_pin_set_dt(&led, 0);
    } else {
      DEBUG_PRINT("USB wake-up request failed: %d\n", ret);
    }
  } else {
    DEBUG_PRINT(
        "Wake-up conditions not met - device not suspended or wake-up not "
        "enabled\n");
  }
}
void usb_status_cb(enum usb_dc_status_code status, const uint8_t *param) {
  switch (status) {
  case USB_DC_SUSPEND:
    usb_suspended = true;
    DEBUG_PRINT("USB SUSPENDED - wake-up now possible\n");
    break;
  case USB_DC_RESUME:
    usb_suspended = false;
    DEBUG_PRINT("USB RESUMED - wake-up no longer needed\n");
    break;
  case USB_DC_CONFIGURED:
    DEBUG_PRINT("USB CONFIGURED\n");
    break;
  case USB_DC_CONNECTED:
    DEBUG_PRINT("USB CONNECTED\n");
    break;
  case USB_DC_DISCONNECTED:
    DEBUG_PRINT("USB DISCONNECTED\n");
    break;
  default:
    DEBUG_PRINT("USB status change: %d\n", status);
    break;
  }
}

void main(void) {
  int ret;

  // Small delay to let system stabilize
  k_sleep(K_MSEC(1000));

  DEBUG_PRINT("=== WKNB USB Wake-up Device Starting ===\n");
  DEBUG_PRINT("Starting wknb initialization...\n");

  // Initialize USB HID
  hid_dev = device_get_binding("HID_0");
  if (hid_dev == NULL) {
    DEBUG_PRINT("ERROR: Failed to get HID device binding\n");
    return;
  }
  DEBUG_PRINT("HID device binding successful\n");

  usb_hid_register_device(hid_dev, hid_report_desc, sizeof(hid_report_desc),
                          NULL);
  DEBUG_PRINT("HID device registered\n");

  ret = usb_hid_init(hid_dev);
  if (ret != 0) {
    DEBUG_PRINT("ERROR: USB HID init failed: %d\n", ret);
    return;
  }
  DEBUG_PRINT("USB HID init successful\n");

  ret = usb_enable(usb_status_cb);
  if (ret != 0) {
    DEBUG_PRINT("ERROR: USB enable failed: %d\n", ret);
    return;
  }
  DEBUG_PRINT("USB enabled successfully\n");

  // Configure GPIO pins
  if (!device_is_ready(rotary_a.port) || !device_is_ready(rotary_b.port) ||
      !device_is_ready(button.port) || !device_is_ready(led.port)) {
    DEBUG_PRINT("ERROR: GPIO devices not ready\n");
    return;
  }
  DEBUG_PRINT("GPIO devices ready\n");

  ret = gpio_pin_configure_dt(&rotary_a, GPIO_INPUT);
  if (ret != 0) {
    DEBUG_PRINT("ERROR: Failed to configure rotary_a: %d\n", ret);
    return;
  }
  ret = gpio_pin_configure_dt(&rotary_b, GPIO_INPUT);
  if (ret != 0) {
    DEBUG_PRINT("ERROR: Failed to configure rotary_b: %d\n", ret);
    return;
  }
  ret = gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_INT_EDGE_FALLING);
  if (ret != 0) {
    DEBUG_PRINT("ERROR: Failed to configure button: %d\n", ret);
    return;
  }
  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
  if (ret != 0) {
    DEBUG_PRINT("ERROR: Failed to configure LED: %d\n", ret);
    return;
  }
  DEBUG_PRINT("GPIO pins configured successfully\n");

  // Configure button interrupt
  gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
  ret = gpio_add_callback(button.port, &button_cb_data);
  if (ret != 0) {
    DEBUG_PRINT("ERROR: Failed to add button callback: %d\n", ret);
    return;
  }
  ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);
  if (ret != 0) {
    DEBUG_PRINT("ERROR: Failed to configure button interrupt: %d\n", ret);
    return;
  }
  DEBUG_PRINT("Button interrupt configured successfully\n");

  // Initial LED test - blink 3 times to show device is working
  DEBUG_PRINT("Starting LED blink test\n");
  for (int i = 0; i < 3; i++) {
    DEBUG_PRINT("LED blink %d\n", i + 1);
    gpio_pin_set_dt(&led, 1);
    k_sleep(K_MSEC(200));
    gpio_pin_set_dt(&led, 0);
    k_sleep(K_MSEC(200));
  }
  DEBUG_PRINT("LED blink test completed\n");

  // Initialize rotary encoder state
  int a_val = gpio_pin_get_dt(&rotary_a);
  int b_val = gpio_pin_get_dt(&rotary_b);
  last_rotary_state = (b_val << 1) | a_val;
  DEBUG_PRINT("Rotary encoder initialized, entering main loop\n");

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

        // Send HID report only after required detents in same direction
        if (detent_counter >= NUMBER_OF_DETENTS_PER_REPORT) {
          DEBUG_PRINT("Volume Up detected (detent_counter: %d)\n",
                      detent_counter);
          uint8_t report[] = {0x01, 0xE9, 0x00}; // Volume Up
          hid_int_ep_write(hid_dev, report, sizeof(report), NULL);

          // Send key release after short delay
          k_sleep(K_MSEC(15));
          uint8_t release[] = {0x01, 0x00, 0x00};
          hid_int_ep_write(hid_dev, release, sizeof(release), NULL);

          // Reset counter by number of detents per report
          detent_counter -= NUMBER_OF_DETENTS_PER_REPORT;
        } else if (detent_counter <= -NUMBER_OF_DETENTS_PER_REPORT) {
          DEBUG_PRINT("Volume Down detected (detent_counter: %d)\n",
                      detent_counter);
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
