#!/usr/bin/env pwsh
Set-StrictMode -Version latest
$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot
Set-Location ..

git push origin master:refs/heads/latest
