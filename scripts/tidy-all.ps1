#!/usr/bin/env pwsh
Set-StrictMode -Version latest
$ErrorActionPreference = "Stop"

clang-format -dump-config >/dev/null

$files = @()
$files += Get-ChildItem -Path "cli", "gui", "library", "test-binary" -Filter "*.c" -Exclude 3rdparty -Recurse
$files += Get-ChildItem -Path "cli", "gui", "library", "test-binary" -Filter "*.cpp" -Exclude 3rdparty -Recurse

$files | ForEach-Object {
	Write-Host -NoNewline -ForegroundColor Gray "`r$($_.FullName)`e[J"
	$output = clang-tidy $_.FullName -fix -checks="readability-braces-around-statements" -p .vscode/ 2>&1
	if ($LASTEXITCODE -ne 0) {
		Write-Host -ForegroundColor Yellow "`r$($_.FullName)"
		Write-Host $output
	}
}

Write-Host -ForegroundColor Green "`r`e[Jcomplete"
