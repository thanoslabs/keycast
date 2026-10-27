# Mouse Raw Input (draft)

Windows Raw Input now registers both the keyboard (usage 6) and mouse (usage 2) on the generic desktop usage page (1).

Mouse button press/release events share the existing version-1 input key envelope with codes `MouseLeft`, `MouseRight`, `MouseMiddle`, `MouseButton4` and `MouseButton5`. Vertical and horizontal wheel events are represented by immediate down/up pulses with codes `WheelUp`, `WheelDown`, `WheelLeft`, and `WheelRight`.

The adapter intentionally does not publish cursor coordinates, mouse movement, typed text, or raw device identifiers. The existing overlay will show these codes while held, but a dedicated mouse visualizer and visible wheel animation are separate follow-up work.

## Windows acceptance tests

- Verify left, right, middle and side-button down/up while Keycast is not focused.
- Verify vertical and horizontal scrolling in both directions.
- Verify simultaneous keyboard and mouse presses and snapshot recovery after reconnect.
- Confirm movement alone does not create events.
- Check for stuck buttons when devices disconnect or Keycast shuts down.

CI cross-compilation cannot replace interactive Windows validation.
