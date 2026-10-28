//go:build windows

package analog

import (
	"context"
	"fmt"
	"math"
	"syscall"
	"time"
	"unsafe"
)

// Sample contains normalized physical key travel, not actuation state.
type Sample struct {
	Code  string  `json:"code"`
	Depth float32 `json:"depth"`
}

// Run polls the Wooting Analog SDK only when explicitly enabled.
// Install the SDK via Wootility or place its official distributable DLL beside the executable.
func Run(ctx context.Context, emit func([]Sample)) error {
	dll := syscall.NewLazyDLL("wooting_analog_sdk_dist.dll")
	init := dll.NewProc("wooting_analog_initialise")
	mode := dll.NewProc("wooting_analog_set_keycode_mode")
	read := dll.NewProc("wooting_analog_read_full_buffer")
	closeSDK := dll.NewProc("wooting_analog_uninitialise")
	for _, proc := range []*syscall.LazyProc{init, mode, read, closeSDK} {
		if err := proc.Find(); err != nil {
			return fmt.Errorf("Wooting Analog SDK unavailable: %w", err)
		}
	}
	result, _, _ := init.Call()
	if int32(result) < 0 {
		return fmt.Errorf("Wooting Analog SDK initialization failed: %d", int32(result))
	}
	defer closeSDK.Call()
	// ScanCode1 maps to Keycast's existing Scan_XX labels.
	result, _, _ = mode.Call(1)
	if int32(result) < 0 {
		return fmt.Errorf("Wooting keycode mode failed: %d", int32(result))
	}
	ticker := time.NewTicker(16 * time.Millisecond)
	defer ticker.Stop()
	previous := map[string]float32{}
	for {
		select {
		case <-ctx.Done():
			return nil
		case <-ticker.C:
			var codes [128]uint16
			var depths [128]float32
			count, _, _ := read.Call(uintptr(unsafe.Pointer(&codes[0])), uintptr(unsafe.Pointer(&depths[0])), uintptr(len(codes)))
			if int32(count) < 0 {
				return fmt.Errorf("Wooting analog read failed: %d", int32(count))
			}
			next := make(map[string]float32)
			for i := 0; i < int(count) && i < len(codes); i++ {
				depth := depths[i]
				if math.IsNaN(float64(depth)) || depth < 0 || depth > 1 {
					continue
				}
				code := fmt.Sprintf("Scan_%02X", codes[i])
				if codes[i] > 0xff {
					code = fmt.Sprintf("ScanE0_%02X", codes[i]&0xff)
				}
				if depth > 0 {
					next[code] = depth
				}
			}
			changed := make([]Sample, 0)
			for code, depth := range next {
				if math.Abs(float64(depth-previous[code])) >= 0.01 {
					changed = append(changed, Sample{code, depth})
				}
			}
			for code := range previous {
				if _, ok := next[code]; !ok {
					changed = append(changed, Sample{code, 0})
				}
			}
			if len(changed) > 0 {
				emit(changed)
			}
			previous = next
		}
	}
}
