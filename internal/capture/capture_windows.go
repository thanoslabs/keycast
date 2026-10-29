//go:build windows

package capture

import (
	"context"
	"fmt"
	"log"
	"runtime"
	"syscall"
	"unsafe"

	"github.com/thanoslabs/keycast/internal/events"
)

const (
	wmInput         = 0x00FF
	wmDestroy       = 0x0002
	wmQuit          = 0x0012
	ridInput        = 0x10000003
	rimTypeKeyboard = 1
	rimTypeMouse    = 0
	ridevInputSink  = 0x00000100
	riKeyBreak      = 0x0001
	riKeyE0         = 0x0002
	riKeyE1         = 0x0004
)

type point struct{ X, Y int32 }
type message struct {
	Hwnd    uintptr
	Msg     uint32
	_       uint32
	WParam  uintptr
	LParam  uintptr
	Time    uint32
	Pt      point
	Private uint32
}
type wndClass struct {
	Size       uint32
	Style      uint32
	WndProc    uintptr
	ClsExtra   int32
	WndExtra   int32
	Instance   uintptr
	Icon       uintptr
	Cursor     uintptr
	Background uintptr
	MenuName   *uint16
	ClassName  *uint16
	SmallIcon  uintptr
}
type rawDevice struct {
	UsagePage uint16
	Usage     uint16
	Flags     uint32
	Target    uintptr
}
type rawHeader struct {
	Type   uint32
	Size   uint32
	Device uintptr
	WParam uintptr
}
type rawMouse struct {
	Flags            uint16
	ButtonFlags      uint16
	ButtonData       uint16
	RawButtons       uint32
	LastX            int32
	LastY            int32
	ExtraInformation uint32
}

type rawKeyboard struct {
	MakeCode         uint16
	Flags            uint16
	Reserved         uint16
	VKey             uint16
	Message          uint32
	ExtraInformation uint32
}

var (
	user32            = syscall.NewLazyDLL("user32.dll")
	kernel32          = syscall.NewLazyDLL("kernel32.dll")
	registerClass     = user32.NewProc("RegisterClassExW")
	createWindow      = user32.NewProc("CreateWindowExW")
	destroyWindow     = user32.NewProc("DestroyWindow")
	defWindowProc     = user32.NewProc("DefWindowProcW")
	registerDevices   = user32.NewProc("RegisterRawInputDevices")
	getRawData        = user32.NewProc("GetRawInputData")
	getMessage        = user32.NewProc("GetMessageW")
	translateMessage  = user32.NewProc("TranslateMessage")
	dispatchMessage   = user32.NewProc("DispatchMessageW")
	postThreadMessage = user32.NewProc("PostThreadMessageW")
	getThreadID       = kernel32.NewProc("GetCurrentThreadId")
	getModuleHandle   = kernel32.NewProc("GetModuleHandleW")
)

type session struct{ emit func(events.KeyEvent) }

var active *session

// Run captures physical keyboard transitions on a dedicated Windows message thread.
// Only one capture session is supported per process.
func Run(ctx context.Context, emit func(events.KeyEvent)) error {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()
	thread, _, _ := getThreadID.Call()
	instance, _, _ := getModuleHandle.Call(0)
	className, _ := syscall.UTF16PtrFromString("KeycastRawInputReceiver")
	callback := syscall.NewCallback(func(hwnd uintptr, msg uint32, wp, lp uintptr) uintptr {
		if msg == wmInput && active != nil {
			active.read(lp)
		}
		if msg == wmDestroy {
			return 0
		}
		result, _, _ := defWindowProc.Call(hwnd, uintptr(msg), wp, lp)
		return result
	})
	wc := wndClass{Size: uint32(unsafe.Sizeof(wndClass{})), WndProc: callback, Instance: instance, ClassName: className}
	if atom, _, err := registerClass.Call(uintptr(unsafe.Pointer(&wc))); atom == 0 {
		return fmt.Errorf("RegisterClassExW: %v", err)
	}
	hwnd, _, err := createWindow.Call(0, uintptr(unsafe.Pointer(className)), uintptr(unsafe.Pointer(className)), 0, 0, 0, 0, 0, 0, 0, instance, 0)
	if hwnd == 0 {
		return fmt.Errorf("CreateWindowExW: %v", err)
	}
	defer destroyWindow.Call(hwnd)
	active = &session{emit: emit}
	defer func() { active = nil }()
	devices := [2]rawDevice{
		{UsagePage: 1, Usage: 6, Flags: ridevInputSink, Target: hwnd},
		{UsagePage: 1, Usage: 2, Flags: ridevInputSink, Target: hwnd},
	}
	ok, _, err := registerDevices.Call(uintptr(unsafe.Pointer(&devices[0])), uintptr(len(devices)), unsafe.Sizeof(devices[0]))
	if ok == 0 {
		return fmt.Errorf("RegisterRawInputDevices: %v", err)
	}
	log.Printf("native input capture initialized (keyboard and mouse Raw Input registered)")
	done := make(chan struct{})
	defer close(done)
	go func() {
		select {
		case <-ctx.Done():
			postThreadMessage.Call(thread, wmQuit, 0, 0)
		case <-done:
		}
	}()
	var msg message
	for {
		status, _, err := getMessage.Call(uintptr(unsafe.Pointer(&msg)), 0, 0, 0)
		if int32(status) == -1 {
			return fmt.Errorf("GetMessageW: %v", err)
		}
		if status == 0 {
			return nil
		}
		translateMessage.Call(uintptr(unsafe.Pointer(&msg)))
		dispatchMessage.Call(uintptr(unsafe.Pointer(&msg)))
	}
}

func (s *session) read(handle uintptr) {
	var size uint32
	headerSize := uint32(unsafe.Sizeof(rawHeader{}))
	result, _, _ := getRawData.Call(handle, ridInput, 0, uintptr(unsafe.Pointer(&size)), uintptr(headerSize))
	if uint32(result) == ^uint32(0) || size < headerSize+uint32(unsafe.Sizeof(rawKeyboard{})) || size > 65536 {
		return
	}
	buf := make([]byte, size)
	result, _, _ = getRawData.Call(handle, ridInput, uintptr(unsafe.Pointer(&buf[0])), uintptr(unsafe.Pointer(&size)), uintptr(headerSize))
	if uint32(result) == ^uint32(0) || result < uintptr(headerSize)+unsafe.Sizeof(rawKeyboard{}) {
		return
	}
	header := (*rawHeader)(unsafe.Pointer(&buf[0]))
	if header.Type == rimTypeMouse {
		if size < headerSize+uint32(unsafe.Sizeof(rawMouse{})) {
			return
		}
		mouse := (*rawMouse)(unsafe.Pointer(&buf[headerSize]))
		for _, event := range DecodeMouseButtons(mouse.ButtonFlags, mouse.ButtonData) {
			s.emit(event)
		}
		return
	}
	if header.Type != rimTypeKeyboard || size < headerSize+uint32(unsafe.Sizeof(rawKeyboard{})) {
		return
	}
	key := (*rawKeyboard)(unsafe.Pointer(&buf[headerSize]))
	code := physicalCode(key.MakeCode, key.Flags, key.VKey)
	if code == "" {
		return
	}
	action := events.Down
	if key.Flags&riKeyBreak != 0 {
		action = events.Up
	}
	s.emit(events.KeyEvent{Code: code, Action: action})
}

func physicalCode(scan, flags, vkey uint16) string {
	if vkey == 255 {
		return ""
	}
	// Preserve scan code and extended-key distinction; UI labels are mapped separately.
	if flags&riKeyE1 != 0 {
		return fmt.Sprintf("ScanE1_%02X", scan)
	}
	if flags&riKeyE0 != 0 {
		return fmt.Sprintf("ScanE0_%02X", scan)
	}
	return fmt.Sprintf("Scan_%02X", scan)
}
