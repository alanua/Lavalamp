param(
  [string]$WledVersion = "v0.15.3",
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

Push-Location $target
try {
  if (-not (Select-String -Path "wled00/usermods_list.cpp" -Pattern "USERMOD_CYLINDER_LAVA" -Quiet)) {
    git apply --ignore-whitespace $patch
  }
  git status --short
}
finally {
  Pop-Location
}

Write-Host ""
Write-Host "WLED Stage 1 cylinder lava workspace is ready at $target"
Write-Host "Build with:"
Write-Host "  cd $target"
Write-Host "  pio run -e cylinder_lava_esp32"
