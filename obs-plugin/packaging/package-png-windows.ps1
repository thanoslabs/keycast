param(
  [Parameter(Mandatory=$true)][string]$PluginDll,
  [Parameter(Mandatory=$true)][string]$OutputZip,
  [string]$AssetsDirectory
)
$ErrorActionPreference = "Stop"
if (!(Test-Path -LiteralPath $PluginDll -PathType Leaf)) { throw "Missing compiled keycast.dll" }
if ([IO.Path]::GetExtension($PluginDll) -ne ".dll") { throw "Expected DLL" }
$root = Join-Path $env:TEMP ("keycast-png-" + [guid]::NewGuid())
try {
  $bin = Join-Path $root "keycast/bin/64bit"
  $data = Join-Path $root "keycast/data"
  New-Item -ItemType Directory -Force -Path $bin,$data | Out-Null
  Copy-Item -LiteralPath $PluginDll -Destination (Join-Path $bin "keycast.dll")
  if ($AssetsDirectory) {
    if (!(Test-Path -LiteralPath $AssetsDirectory -PathType Container)) { throw "Missing PNG assets directory" }
    $assets = Join-Path $data "avatars/club-penguin"
    New-Item -ItemType Directory -Force -Path $assets | Out-Null
    Get-ChildItem -LiteralPath $AssetsDirectory -Filter "*.png" -File | Copy-Item -Destination $assets
    if (!(Get-ChildItem -LiteralPath $assets -Filter "*.png" -File)) { throw "No PNG assets supplied" }
  }
  $destination = [IO.Path]::GetFullPath($OutputZip)
  New-Item -ItemType Directory -Force -Path (Split-Path $destination -Parent) | Out-Null
  Compress-Archive -Path (Join-Path $root "keycast") -DestinationPath $destination -Force
  Write-Host "Created $destination"
} finally {
  Remove-Item -LiteralPath $root -Recurse -Force -ErrorAction SilentlyContinue
}
