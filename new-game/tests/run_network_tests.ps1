param(
    [Parameter(Mandatory=$true)][string]$GodotExe,
    [int]$PlayerCount = 2,
    [int]$TargetRevision = 999,
    [int]$Port = 24568
)

$project = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$output = Join-Path $project '.test-home'
New-Item -ItemType Directory -Force -Path $output,(Join-Path $output 'Roaming'),(Join-Path $output 'Local') | Out-Null
$env:APPDATA = Join-Path $output 'Roaming'
$env:LOCALAPPDATA = Join-Path $output 'Local'
$processes = @()
try {
    for ($seat = 0; $seat -lt $PlayerCount; $seat++) {
        $role = if ($seat -eq 0) { 'host' } else { "client$seat" }
        $args = @('--headless', '--path', $project, '--script', 'res://tests/network_peer.gd', '--',
            "role=$role", "players=$PlayerCount", "moves=$TargetRevision", "port=$Port")
        $stdout = Join-Path $output "peer$seat.out"
        $stderr = Join-Path $output "peer$seat.err"
        $processes += Start-Process -FilePath $GodotExe -ArgumentList $args -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
        Start-Sleep -Milliseconds 250
    }
    foreach ($process in $processes) {
        if (-not $process.WaitForExit(60000)) {
            throw "Godotプロセス $($process.Id) がタイムアウトしました"
        }
    }
    $sequences = @()
    for ($seat = 0; $seat -lt $PlayerCount; $seat++) {
        $stdout = Join-Path $output "peer$seat.out"
        $stderr = Join-Path $output "peer$seat.err"
        $lines = @(Get-Content -LiteralPath $stdout)
        $errors = @(Get-Content -LiteralPath $stderr | Where-Object { $_ -notmatch 'root certificate store|^   at: get_system_ca_certificates' })
        if ($processes[$seat].ExitCode -ne 0 -or -not ($lines -match ' OK scores=')) {
            throw "席$seat が失敗: $($lines -join ' / ') $($errors -join ' / ')"
        }
        $sequences += ,@($lines | Where-Object { $_ -match ' STATE ' } | ForEach-Object { ($_ -split ' STATE ')[1] } | Select-Object -Unique)
        Write-Output "席$seat $($lines | Where-Object { $_ -match ' OK scores=' })"
        Write-Output ($lines | Where-Object { $_ -match ' REJECT ' })
    }
    for ($seat = 1; $seat -lt $PlayerCount; $seat++) {
        if (($sequences[0] -join '|') -ne ($sequences[$seat] -join '|')) {
            throw "席0と席$seatのrevision・盤面ハッシュが一致しません"
        }
    }
    Write-Output "$PlayerCount 人別プロセス対戦: 全revisionの盤面一致"
}
finally {
    foreach ($process in $processes) {
        if (-not $process.HasExited) {
            $process.Kill()
        }
    }
}
