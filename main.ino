#include <hidboot.h>
#include <usbhub.h>
#include <Keyboard.h>

USB     Usb;
USBHub  Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD> HidKeyboard(&Usb); // Renamed to avoid conflict

class KbdRptParser : public KeyboardReportParser
{
  protected:
    void OnControlKeysChanged(uint8_t before, uint8_t after);
    void OnKeyDown(uint8_t mod, uint8_t key);
    void OnKeyPressed(uint8_t key);
    void OnKeyUp(uint8_t mod, uint8_t key);

  private:
    bool aHeld = false;
    bool dHeld = false;
    bool aSuspended = false;
    bool dSuspended = false;
};

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after)
{
  // Handle modifier keys
  uint8_t changes = before ^ after;

  if (changes & 0x01) {
    if (after & 0x01) Keyboard.press(KEY_LEFT_CTRL);
    else Keyboard.release(KEY_LEFT_CTRL);
  }

  if (changes & 0x02) {
    if (after & 0x02) Keyboard.press(KEY_LEFT_SHIFT);
    else Keyboard.release(KEY_LEFT_SHIFT);
  }

  if (changes & 0x04) {
    if (after & 0x04) Keyboard.press(KEY_LEFT_ALT);
    else Keyboard.release(KEY_LEFT_ALT);
  }

  if (changes & 0x08) {
    if (after & 0x08) Keyboard.press(KEY_LEFT_GUI);
    else Keyboard.release(KEY_LEFT_GUI);
  }

  if (changes & 0x10) {
    if (after & 0x10) Keyboard.press(KEY_RIGHT_CTRL);
    else Keyboard.release(KEY_RIGHT_CTRL);
  }

  if (changes & 0x20) {
    if (after & 0x20) Keyboard.press(KEY_RIGHT_SHIFT);
    else Keyboard.release(KEY_RIGHT_SHIFT);
  }

  if (changes & 0x40) {
    if (after & 0x40) Keyboard.press(KEY_RIGHT_ALT);
    else Keyboard.release(KEY_RIGHT_ALT);
  }

  if (changes & 0x80) {
    if (after & 0x80) Keyboard.press(KEY_RIGHT_GUI);
    else Keyboard.release(KEY_RIGHT_GUI);
  }
}

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  uint8_t c = OemToAscii(mod, key);

  if (c) {
    Serial.println(c);
    if (c == 97) { // 'A'
      if (dHeld) {
        Keyboard.release(100); // Release 'D'
        dSuspended = true;
      }
      aHeld = true;
      aSuspended = false;
      Keyboard.press(97);
    } else if (c == 100) { // 'D'
      if (aHeld) {
        Keyboard.release(97); // Release 'A'
        aSuspended = true;
      }
      dHeld = true;
      dSuspended = false;
      Keyboard.press(100);
    } else {
      Keyboard.press(c);
    }
  } else {
    // Handle special keys
    switch (key) {
      case 0x28: Keyboard.press(KEY_RETURN); break;
      case 0x29: Keyboard.press(KEY_ESC); break;
      case 0x2A: Keyboard.press(KEY_BACKSPACE); break;
      case 0x2B: Keyboard.press(KEY_TAB); break;
      case 0x2C: Keyboard.press(' '); break;  // Space key
      case 0x4F: Keyboard.press(KEY_RIGHT_ARROW); break;
      case 0x50: Keyboard.press(KEY_LEFT_ARROW); break;
      case 0x51: Keyboard.press(KEY_DOWN_ARROW); break;
      case 0x52: Keyboard.press(KEY_UP_ARROW); break;
      // Add more keycodes as needed
    }
  }
}

void KbdRptParser::OnKeyPressed(uint8_t key)
{
  // This function can be used to handle any special cases
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  uint8_t c = OemToAscii(mod, key);

  if (c) {
    if (c == 97) { // 'A'
      aHeld = false;
      if (!aSuspended) {
        Keyboard.release(97);
      }
      if (dSuspended) {
        dSuspended = false;
        dHeld = true;
        Keyboard.press(100);
      }
    } else if (c == 100) { // 'D'
      dHeld = false;
      if (!dSuspended) {
        Keyboard.release(100);
      }
      if (aSuspended) {
        aSuspended = false;
        aHeld = true;
        Keyboard.press(97);
      }
    } else {
      Keyboard.release(c);
    }
  } else {
    // Handle special keys
    switch (key) {
      case 0x28: Keyboard.release(KEY_RETURN); break;
      case 0x29: Keyboard.release(KEY_ESC); break;
      case 0x2A: Keyboard.release(KEY_BACKSPACE); break;
      case 0x2B: Keyboard.release(KEY_TAB); break;
      case 0x2C: Keyboard.release(' '); break;  // Space key
      case 0x4F: Keyboard.release(KEY_RIGHT_ARROW); break;
      case 0x50: Keyboard.release(KEY_LEFT_ARROW); break;
      case 0x51: Keyboard.release(KEY_DOWN_ARROW); break;
      case 0x52: Keyboard.release(KEY_UP_ARROW); break;
      // Add more keycodes as needed
    }
  }
}

KbdRptParser KbdPrs;

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay(200);

  HidKeyboard.SetReportParser(0, &KbdPrs); // Attach parser to HID keyboard
  Keyboard.begin();
}

void loop()
{
  Usb.Task();
}
