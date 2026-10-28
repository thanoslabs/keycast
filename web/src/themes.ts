export const themePresets = {
  violet: { label: "Violet", accent: "#6d53c5", opacity: 85 },
  ocean: { label: "Ocean", accent: "#167cb4", opacity: 90 },
  ember: { label: "Ember", accent: "#df653e", opacity: 90 },
  mint: { label: "Mint", accent: "#208b76", opacity: 90 },
  rose: { label: "Rose", accent: "#c14d83", opacity: 85 },
  mono: { label: "Monochrome", accent: "#707b8d", opacity: 95 }
} as const;
export type ThemePreset = keyof typeof themePresets;
