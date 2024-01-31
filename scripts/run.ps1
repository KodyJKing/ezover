param(
    [string]$Config = "Debug",
    [string]$Platform = "Win64"
)

. scripts/definitions.ps1
& "bin/$Config-$Platform/$project/$project.exe"
