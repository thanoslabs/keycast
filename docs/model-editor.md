# Keycast model editor specification

The model editor will be an OBS dock, not a web server. It must enumerate
Cubism parameters and motions from the loaded model and allow binding inputs
to parameters, including press and release values and response time.

## Minimum editor interactions
1. Select a model from the OBS source properties.
2. Preview the model with transparent background.
3. Select a Cubism parameter and set a test value.
4. Bind keyboard/mouse events to the parameter and preview press/release.
5. Save a portable profile referencing the model by relative path.

## Safety
Do not capture input while OBS is not focused unless the user explicitly enables
global input capture. Do not store raw keystroke history. Disable input capture
when the source is destroyed. No remote HTTP endpoint.
