package capture

import (
	"testing"

	"github.com/thanoslabs/keycast/internal/events"
)

func TestDecodeMouseButtons(t *testing.T) {
	tests := []struct {
		name string
		flags uint16
		data uint16
		want []events.KeyEvent
	}{
		{"left press", mouseLeftDown, 0, []events.KeyEvent{{Code: "MouseLeft", Action: events.Down}}},
		{"multiple releases", mouseRightUp | mouseMiddleUp, 0, []events.KeyEvent{{Code: "MouseRight", Action: events.Up}, {Code: "MouseMiddle", Action: events.Up}}},
		{"wheel up", mouseWheel, 120, []events.KeyEvent{{Code: "WheelUp", Action: events.Down}, {Code: "WheelUp", Action: events.Up}}},
		{"wheel down", mouseWheel, uint16(0xff88), []events.KeyEvent{{Code: "WheelDown", Action: events.Down}, {Code: "WheelDown", Action: events.Up}}},
		{"no movement events", 0, 0, nil},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			got := DecodeMouseButtons(tc.flags, tc.data)
			if len(got) != len(tc.want) {
				t.Fatalf("got %v, want %v", got, tc.want)
			}
			for i := range got {
				if got[i] != tc.want[i] {
					t.Fatalf("got %v, want %v", got, tc.want)
				}
			}
		})
	}
}
