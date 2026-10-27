package events

// Action describes a physical input state transition.
type Action string

const (
	Down Action = "down"
	Up   Action = "up"
)

// KeyEvent identifies a physical key without recording typed text.
type KeyEvent struct {
	Code   string `json:"code"`
	Action Action `json:"action"`
	Repeat bool   `json:"repeat"`
}

// Envelope is the versioned wire representation of an event.
type Envelope struct {
	Version  int      `json:"version"`
	Type     string   `json:"type"`
	Sequence uint64   `json:"sequence"`
	Data     KeyEvent `json:"data"`
}
