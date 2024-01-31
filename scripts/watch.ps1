param(
    [string]$watchPath,
    [string]$command
)

Write-Host "Watching $watchPath for changes"

# Define global copy of command
$global:command = $command
$global:process = Start-Process powershell -NoNewWindow -PassThru -ArgumentList "-Command", $global:command

try {
    $watcher = New-Object System.IO.FileSystemWatcher
    $watcher.Path = $watchPath
    $watcher.IncludeSubdirectories = $true
    $watcher.EnableRaisingEvents = $true

    $action = {
        $path = $Event.SourceEventArgs.FullPath
        $changeType = $Event.SourceEventArgs.ChangeType
        Write-Host "File $path $changeType"

        # Clean up the process
        if ($global:process) {
            # Have to define this twice due to scoping issues :/
            function Kill-Tree { # Source https://stackoverflow.com/a/55942155
                Param([int]$ppid)
                Get-CimInstance Win32_Process | Where-Object { $_.ParentProcessId -eq $ppid } | ForEach-Object { Kill-Tree $_.ProcessId }
                Stop-Process -Id $ppid -Force
            }
            Kill-Tree $global:process.Id
        }

        $global:process = Start-Process powershell -NoNewWindow -PassThru -ArgumentList "-Command", $global:command
    }

    Register-ObjectEvent -InputObject $watcher -EventName "Created" -Action $action
    Register-ObjectEvent -InputObject $watcher -EventName "Changed" -Action $action
    Register-ObjectEvent -InputObject $watcher -EventName "Deleted" -Action $action
    Register-ObjectEvent -InputObject $watcher -EventName "Renamed" -Action $action

    # See https://powershell.one/tricks/filesystem/filesystemwatcher#implementation
    do {
        # Wait-Event waits for a second and stays responsive to events
        Wait-Event -Timeout 1
    } while ($true)

} finally {

    # Clean up the watcher
    Write-Host "Watch stopped."
    $watcher.Dispose()

    # Clean up the process
    if ($global:process) {
        Write-Host "Cleaning up process."
        function Kill-Tree {
            Param([int]$ppid)
            Get-CimInstance Win32_Process | Where-Object { $_.ParentProcessId -eq $ppid } | ForEach-Object { Kill-Tree $_.ProcessId }
            Stop-Process -Id $ppid -Force
        }
        Kill-Tree $global:process.Id
    } else {
        Write-Host "No process to clean up."
    }

}
