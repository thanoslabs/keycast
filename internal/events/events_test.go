package events

import (
	"encoding/json"
	"testing"
)

func TestKeyEventWireFormat(t *testing.T) {
	e := Envelope{Version: 1, Type: "input.key", Sequence: 1, Data: KeyEvent{Code: "KeyA", Action: Down}}
	b, err := json.Marshal(e)
	if err != nil { t.Fatal(err) }
	var decoded Envelope
	if err := json.Unmarshal(b, &decoded); err != nil { t.Fatal(err) }
	if decoded.Data.Code != "KeyA" || decoded.Data.Action != Down || decoded.Version != 1 {
		t.Fatalf("unexpected event: %+v", decoded)
	}
}
