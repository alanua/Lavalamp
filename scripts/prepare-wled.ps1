param(
  [string]$WledVersion = "v16.0.0",
  [string]$WledPath = "firmware/WLED"
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$target = Join-Path $repoRoot $WledPath
$overlay = Join-Path $repoRoot "overlays/wled"
$patch = Join-Path $repoRoot "patches/wled-usermods-list-cylinder-lava.patch"

if (-not (Test-Path $target)) {
  git clone --branch $WledVersion --depth 1 https://github.com/wled/WLED.git $target
}

Copy-Item -Path (Join-Path $overlay "platformio_override.ini") -Destination (Join-Path $target "platformio_override.ini") -Force
New-Item -ItemType Directory -Force -Path (Join-Path $target "usermods/cylinder_lava") | Out-Null
Copy-Item -Path (Join-Path $overlay "usermods/cylinder_lava/*") -Destination (Join-Path $target "usermods/cylinder_lava") -Force

$libraryJson = Join-Path $target "usermods/cylinder_lava/library.json"
Set-Content -Path $libraryJson -Value @'
{
  "name": "cylinder_lava",
  "version": "1.0.0",
  "build": {
    "libArchive": false
  }
}
'@ -NoNewline

$sourceShim = Join-Path $target "usermods/cylinder_lava/cylinder_lava.cpp"
Set-Content -Path $sourceShim -Value @'
#include "usermod_cylinder_lava.h"
'@ -NoNewline

Push-Location $target
try {
  if ($WledVersion -notmatch "^v?16" -and -not (Select-String -Path "wled00/usermods_list.cpp" -Pattern "USERMOD_CYLINDER_LAVA" -Quiet)) {
    git apply --ignore-whitespace $patch
  }
  git status --short
}
finally {
  Pop-Location
}

Write-Host ""
Write-Host "WLED 16 cylinder lava workspace is ready at $target"
Write-Host "Build with:"
Write-Host "  cd $target"
Write-Host "  pio run -e cylinder_lava_esp32"
