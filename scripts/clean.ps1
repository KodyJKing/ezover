# Delete /bin and /obj folders.
Get-ChildItem -Path . -Include bin,obj -Recurse | Remove-Item -Recurse -Force

# Delete .sln and .vcxproj files, but ignore /vendor.
Get-ChildItem -Path . -Include *.sln,*.vcxproj -Recurse | Where-Object { $_.DirectoryName -notlike "*\vendor*" } | Remove-Item -Force

# Delete vendor/*/bin and vendor/*/obj folders.
Get-ChildItem -Path vendor -Directory | ForEach-Object {
    Get-ChildItem -Path $_.FullName -Include bin,obj -Recurse | Remove-Item -Recurse -Force
}

# Delete imgui.ini.
Remove-Item -Path imgui.ini -Force