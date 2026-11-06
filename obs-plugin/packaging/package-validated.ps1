param(
  [Parameter(Mandatory=$true)][string]$PluginDll,
  [Parameter(Mandatory=$true)][string]$CubismModel,
  [Parameter(Mandatory=$true)][string]$OutputZip,
  [string]$CoreDll
)
$ErrorActionPreference = "Stop"
if (!(Test-Path -LiteralPath $PluginDll -PathType Leaf)) { throw "Plugin DLL missing" }
if (!(Test-Path -LiteralPath $CubismModel -PathType Leaf)) { throw "Cubism model manifest missing" }
if (!$CubismModel.EndsWith(".model3.json")) { throw "Expected a .model3.json export" }
$validator = Join-Path $PSScriptRoot "../tools/validate_penguin_export.py"
python $validator $CubismModel
if ($LASTEXITCODE -ne 0) { throw "Cubism model export validation failed" }
$manifest = Get-Content -Raw -LiteralPath $CubismModel | ConvertFrom-Json
if (!$manifest.FileReferences.Moc) { throw "Model manifest has no Moc" }
if ($CoreDll -and !(Test-Path -LiteralPath $CoreDll -PathType Leaf)) { throw "Core DLL missing" }
$staging = Join-Path ([IO.Path]::GetTempPath()) ("keycast-" + [guid]::NewGuid())
try {
  $binary = Join-Path $staging "obs-plugins/64bit"
  $data = Join-Path $staging "data/obs-plugins/keycast/avatars/club-penguin"
  New-Item -ItemType Directory -Force -Path $binary,$data | Out-Null
  Copy-Item -LiteralPath $PluginDll -Destination (Join-Path $binary "keycast.dll")
  if ($CoreDll) { Copy-Item -LiteralPath $CoreDll -Destination $binary }
  Get-ChildItem -LiteralPath (Split-Path $CubismModel -Parent) | Copy-Item -Destination $data -Recurse -Force
  $destination = [IO.Path]::GetFullPath($OutputZip)
  Compress-Archive -Path (Join-Path $staging "*") -DestinationPath $destination -Force
} finally {
  Remove-Item -LiteralPath $staging -Recurse -Force -ErrorAction SilentlyContinue
}
