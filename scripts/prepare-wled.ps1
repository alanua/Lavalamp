param(
  [string]$WledVersion = "v16.0.1",
  [string]$WledPath = "firmware/WLED"
)

$ErrorActionPreference = "Stop"
$WledCommit = "29b389df1c1aaec6ff53aea742d17063b985906c"
if ($WledVersion -ne "v16.0.1") { throw "Only stable WLED v16.0.1 is supported" }

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$target = Join-Path $repoRoot $WledPath
$overlay = Join-Path $repoRoot "overlays/wled"

if (-not (Test-Path $target)) {
  git clone --branch $WledVersion --depth 1 https://github.com/wled/WLED.git $target
  if ($LASTEXITCODE -ne 0) { throw "WLED clone failed" }
}

$actualCommit = git -C $target rev-parse HEAD
if ($LASTEXITCODE -ne 0 -or $actualCommit -ne $WledCommit) {
  throw "WLED source must be exactly $WledCommit; existing workspace was not changed"
}

Copy-Item -Path (Join-Path $overlay "platformio_override.ini") -Destination (Join-Path $target "platformio_override.ini") -Force
New-Item -ItemType Directory -Force -Path (Join-Path $target "usermods/cylinder_lava") | Out-Null
Copy-Item -Path (Join-Path $overlay "usermods/cylinder_lava/*") -Destination (Join-Path $target "usermods/cylinder_lava") -Force

Push-Location $target
try {
  foreach ($patchName in @("wled-usermods-list-cylinder-lava.patch", "wled-native-2d-cylinder-adapter.patch")) {
    $patchFile = Join-Path $repoRoot "patches/$patchName"
    git apply --reverse --check $patchFile 2>$null
    if ($LASTEXITCODE -eq 0) { continue } # already applied, exact patch
    git apply --check $patchFile
    if ($LASTEXITCODE -ne 0) { throw "Incompatible WLED integration patch: $patchName" }
    git apply $patchFile
    if ($LASTEXITCODE -ne 0) { throw "Failed WLED integration patch: $patchName" }
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
