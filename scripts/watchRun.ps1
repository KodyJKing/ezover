param(
    [string]$Config = "Debug",
    [string]$Platform = "Win64"
)

& "./scripts/watch.ps1" "src" "./scripts/build.ps1 $Config $Platform; ./scripts/run.ps1 $Config $Platform"
