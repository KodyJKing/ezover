param(
    [string]$Config = "Debug",
    [string]$Platform = "Win64"
)

& "./scripts/vendor/premake/premake5.exe" "vs2022"
. scripts/definitions.ps1
& "MSBuild.exe" "$workspace.sln" "/t:Build" "/p:Configuration=$Config" "/p:Platform=$Platform"
