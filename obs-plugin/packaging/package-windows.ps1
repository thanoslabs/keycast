param(
  [Parameter(Mandatory=$true)][string]$PluginDll,
  [Parameter(Mandatory=$true)][string]$CubismModel,
  [Parameter(Mandatory=$true)][string]$OutputZip
)
$ErrorActionPreference = "Stop"
if (!(Test-Path -PathType Leaf $PluginDll)) { throw "Compiled plugin DLL missing: $PluginDll" }
if ([IO.Path]::GetExtension($PluginDll) -ne ".dll") { throw "Expected a Windows plugin DLL" }
if (!(Test-Path -PathType Leaf $CubismModel)) { throw "Cubism model manifest missing" }
if (!$CubismModel.EndsWith(".model3.json")) { throw "Expected a .model3.json export" }
$root = Join-Path $env:TEMP ("keycast-package-" + [guid]::NewGuid())
try {
  $bin = Join-Path $root "obs-plugins/64bit"
  $assets = Join-Path $root "data/obs-plugins/keycast/avatars/club-penguin"
  New-Item -ItemType Directory -Force $bin,$assets | Out-Null
  Copy-Item $PluginDll (Join-Path $bin "keycast.dll")
  Copy-Item (Split-Path $CubismModel -Parent) (Split-Path $assets -Parent) -Recurse -Force
  $destination = [IO.Path]::GetFullPath($OutputZip)
  Compress-Archive -Path (Join-Path $root "*") -DestinationPath $destination -Force
  Write-Host "Created package: $destination"
} finally {
  Remove-Item $root -Recurse -Force -ErrorAction SilentlyContinue
}
