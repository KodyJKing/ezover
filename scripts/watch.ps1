# See https://powershell.one/tricks/filesystem/filesystemwatcher

param(
    [string]$watchPath,
    [string]$command
)

$global:process = $null
$global:needsRestart = $false

function Run-Command {
    $global:process = Start-Process powershell -NoNewWindow -PassThru -ArgumentList "-Command", $command
}

function Kill-Tree {
    Param([int]$ppid)
    Get-CimInstance Win32_Process | Where-Object { $_.ParentProcessId -eq $ppid } | ForEach-Object { Kill-Tree $_.ProcessId }
    Stop-Process -Id $ppid -Force
}

function On-Exit {
    Write-Host "Unwatching file system." -ForegroundColor Blue
    $watcher.EnableRaisingEvents = $false
    $handlers | ForEach-Object { Unregister-Event -SourceIdentifier $_.Name }
    $handlers | Remove-Job
    $watcher.Dispose()

    if ($global:process) {
        Write-Host "Cleaning up process." -ForegroundColor Blue
        Kill-Tree $global:process.Id
    } else {
        Write-Host "No process to clean up." -ForegroundColor Blue
    }
}

try {
    Write-Host "Watching $watchPath for changes" -ForegroundColor Green

    Run-Command

    $watcher = New-Object System.IO.FileSystemWatcher
    $watcher.Path = $watchPath
    $watcher.IncludeSubdirectories = $true
    $watcher.EnableRaisingEvents = $true

    $action = {
        if ($needsRestart) { return }
        $path = $Event.SourceEventArgs.FullPath
        $changeType = $Event.SourceEventArgs.ChangeType
        $changeType = $changeType.ToString().ToLower()
        Write-Host ""
        Write-Host "File $path $changeType." -ForegroundColor Yellow
        Write-Host ""
        $global:needsRestart = $true
    }

    $handlers = . {
        Register-ObjectEvent -InputObject $watcher -EventName "Created" -Action $action
        Register-ObjectEvent -InputObject $watcher -EventName "Changed" -Action $action
        Register-ObjectEvent -InputObject $watcher -EventName "Deleted" -Action $action
        Register-ObjectEvent -InputObject $watcher -EventName "Renamed" -Action $action
    }

    do {
        Start-Sleep -Milliseconds 20

        # Press R to manually restart the process
        if ([Console]::KeyAvailable) {
            $key = [Console]::ReadKey($true)
            if ($key.Key -eq "R") {
                $global:needsRestart = $true
            }
        }

        if ($global:needsRestart) {
            $global:needsRestart = $false
            if ($global:process) {
                Kill-Tree $global:process.Id
            }
            Run-Command
        }

    } while ($true)

} finally {
    On-Exit
}
