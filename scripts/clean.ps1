# Delete /bin and /obj folders.
Get-ChildItem -Path . -Include bin,obj -Recurse | Remove-Item -Recurse -Force
# Delete .sln and .vcxproj files.
Get-ChildItem -Path . -Include *.sln,*.vcxproj -Recurse | Remove-Item -Force
# Delete vendor/*/bin and vendor/*/obj folders.
Get-ChildItem -Path vendor -Directory | ForEach-Object {
    Get-ChildItem -Path $_.FullName -Include bin,obj -Recurse | Remove-Item -Recurse -Force
}
