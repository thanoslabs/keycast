// Trigger expressions use + for simultaneous keys and | for alternatives.
// Codes are matched exactly against the live input stream.
export function triggerMatches(expression: string | undefined, pressed: ReadonlySet<string>): boolean {
  if (!expression) return false;
  return expression.split("|").some(group => {
    const codes = group.split("+").map(code => code.trim());
    return codes.length > 0 && codes.every(code => code.length > 0 && pressed.has(code));
  });
}
