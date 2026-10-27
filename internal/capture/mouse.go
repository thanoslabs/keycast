package capture

import "github.com/thanoslabs/keycast/internal/events"

const (
	mouseLeftDown    uint16 = 0x0001
	mouseLeftUp      uint16 = 0x0002
	mouseRightDown   uint16 = 0x0004
	mouseRightUp     uint16 = 0x0008
	mouseMiddleDown  uint16 = 0x0010
	mouseMiddleUp    uint16 = 0x0020
	mouseButton4Down uint16 = 0x0040
	mouseButton4Up   uint16 = 0x0080
	mouseButton5Down uint16 = 0x0100
	mouseButton5Up   uint16 = 0x0200
	mouseWheel       uint16 = 0x0400
	mouseHWheel      uint16 = 0x0800
)

// DecodeMouseButtons translates Raw Input mouse button flags into state transitions.
// Wheel events are emitted as a down/up pulse; movement and absolute coordinates are ignored.
func DecodeMouseButtons(flags uint16, data uint16) []events.KeyEvent {
	out := make([]events.KeyEvent, 0, 12)
	for _, button := range []struct {
		down     uint16
		up       uint16
		code     string
	}{
		{mouseLeftDown, mouseLeftUp, "MouseLeft"},
		{mouseRightDown, mouseRightUp, "MouseRight"},
		{mouseMiddleDown, mouseMiddleUp, "MouseMiddle"},
		{mouseButton4Down, mouseButton4Up, "MouseButton4"},
		{mouseButton5Down, mouseButton5Up, "MouseButton5"},
	} {
		if flags&button.down != 0 {
			out = append(out, events.KeyEvent{Code: button.code, Action: events.Down})
		}
		if flags&button.up != 0 {
			out = append(out, events.KeyEvent{Code: button.code, Action: events.Up})
		}
	}
	for _, wheel := range []struct {
		flag     uint16
		positive string
		negative string
	}{
		{mouseWheel, "WheelUp", "WheelDown"},
		{mouseHWheel, "WheelRight", "WheelLeft"},
	} {
		if flags&wheel.flag != 0 && int16(data) != 0 {
			code := wheel.positive
			if int16(data) < 0 {
				code = wheel.negative
			}
			out = append(out, events.KeyEvent{Code: code, Action: events.Down}, events.KeyEvent{Code: code, Action: events.Up})
		}
	}
	return out
}
